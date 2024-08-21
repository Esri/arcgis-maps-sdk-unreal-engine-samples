/* Copyright 2024 Esri
 *
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "XRTabletopComponent.h"


UXRTabletopComponent::UXRTabletopComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UXRTabletopComponent::OnRegister()
{
	Super::OnRegister();

	SetupReferences();

	if (MapComponent.IsValid())
	{
		CenterPosition = FGeoPosition(MapComponent->GetExtent().ExtentCenter);
		ExtentDimensions = MapComponent->GetExtent().ShapeDimensions;
		Shape = MapComponent->GetExtent().ExtentShape;

		if (ExtentChangeHandle.IsValid())
		{
			MapComponent->OnExtentChanged.Remove(ExtentChangeHandle);
		}

		ExtentChangeHandle = MapComponent->OnExtentChanged.AddUObject(this, &UXRTabletopComponent::PostUpdateTabletop);
	}
	PreUpdateTabletop();
}

void UXRTabletopComponent::OnUnregister()
{
	if (MapComponent.IsValid())
	{
		MapComponent->OnExtentChanged.RemoveAll(this);
		ExtentChangeHandle.Reset();
	}

	Super::OnUnregister();
}

#if WITH_EDITOR
void UXRTabletopComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	auto propertyName = PropertyChangedEvent.MemberProperty->GetFName();

	if (propertyName == GET_MEMBER_NAME_CHECKED(UXRTabletopComponent, ElevationOffset))
	{
		bNeedsOffsetChange = true;
		PreUpdateTabletop();
	}
	else if (propertyName == GET_MEMBER_NAME_CHECKED(UXRTabletopComponent, ExtentDimensions) ||
			 propertyName == GET_MEMBER_NAME_CHECKED(UXRTabletopComponent, Shape) ||
			 propertyName == GET_MEMBER_NAME_CHECKED(UXRTabletopComponent, CenterPosition))
	{
		bNeedsExtentChange = true;
		PreUpdateTabletop();
	}

	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UXRTabletopComponent::BeginPlay()
{
	Super::BeginPlay();

	SetupReferences();
	GetTabletopController();

	LocalZOffset = WrapperActor->GetRootComponent()->GetRelativeLocation().Z;
}

void UXRTabletopComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	auto relativeLocation = WrapperActor->GetRootComponent()->GetRelativeLocation();
	relativeLocation.Z = LocalZOffset;
	WrapperActor->SetActorRelativeLocation(relativeLocation);

	Super::EndPlay(EndPlayReason);
}

void UXRTabletopComponent::PreUpdateTabletop()
{
	if (!bNeedsExtentChange && !bNeedsOffsetChange && !bExtentChanged)
	{
		return;
	}

	if (!MapComponent.IsValid() || !ArcGISCameraLocation || !WrapperActor)
	{
		return;
	}

	if (bNeedsExtentChange || bExtentChanged)
	{
		FArcGISExtentInstanceData newExtent{CenterPosition, Shape, ExtentDimensions};

		double radiusDistance = 0.0;
		if (Shape == EMapExtentShapes::Circle)
		{
			radiusDistance = ExtentDimensions.X;
		}
		else if (Shape == EMapExtentShapes::Square)
		{
			radiusDistance = (ExtentDimensions.X / 2) * FMath::Sqrt(2.);
		}
		else if (Shape == EMapExtentShapes::Rectangle)
		{
			radiusDistance = FMath::Sqrt(FMath::Pow(ExtentDimensions.X / 2, 2) + FMath::Pow(ExtentDimensions.Y / 2, 2));
		}

		if (bExtentChanged)
		{
			MapComponent->SetOriginPosition(newExtent.ExtentCenter);

			WrapperActor->SetActorRelativeScale3D(FVector3d(WrapperScaleFactor / radiusDistance));

			bExtentChanged = false;
		}
		else
		{
			MapComponent->SetExtent(newExtent);

			if (Shape != EMapExtentShapes::Rectangle)
			{
				ExtentDimensions.Y = ExtentDimensions.X;
			}

			bNeedsExtentChange = false;
		}

		auto newHeight = CameraHeightFactor / ArcGISCameraLocation->GetOwner()->GetActorScale3D().Z;
		ArcGISCameraLocation->GetOwner()->SetActorRelativeLocation(FVector3d(0, 0, newHeight));
	}

	if (bNeedsOffsetChange)
	{
		UpdateOffset();
		bNeedsOffsetChange = false;
	}
}

void UXRTabletopComponent::PostUpdateTabletop(FVector3d InAreaMin, FVector3d InAreaMax, Esri::GameEngine::Extent::ArcGISExtentType InType)
{
	if (!MapComponent.IsValid())
	{
		return;
	}

	auto areaMin = FVector3d::Min(InAreaMin, InAreaMax);
	auto areaMax = FVector3d::Max(InAreaMin, InAreaMax);

	auto width = areaMax.X - areaMin.X;
	auto height = areaMax.Y - areaMin.Y;
	auto altitude = areaMax.Z - areaMin.Z;

	auto center = FVector3d(areaMin.X + width / 2.0, areaMin.Y + height / 2.0, areaMin.Z + altitude / 2.0);
	auto point = MapComponent->GetView()->WorldToGeographic(center);

	MapComponent->SetOriginPosition(point);
	CenterPosition = FGeoPosition(point);

	ExtentDimensions = FVector2D(width, height) / (Shape == EMapExtentShapes::Circle ? 2 : 1);

	UpdateOffset();

	bExtentChanged = true;

	PreUpdateTabletop();
}

void UXRTabletopComponent::UpdateOffset()
{
	auto relativeLocation = WrapperActor->GetRootComponent()->GetRelativeLocation();
	relativeLocation.Z = ElevationOffset * WrapperActor->GetActorScale3D().Z + LocalZOffset;
	WrapperActor->SetActorRelativeLocation(relativeLocation);
}

void UXRTabletopComponent::MoveExtentCenter(FVector3d WorldPos)
{
	if (!MapComponent.IsValid())
	{
		return;
	}

	CenterPosition = FGeoPosition(MapComponent->GetView()->WorldToGeographic(WorldPos));
	bNeedsExtentChange = true;

	PreUpdateTabletop();
}

void UXRTabletopComponent::ZoomMap(float ZoomValue)
{
	if (ZoomValue == 0)
	{
		return;
	}

	ExtentDimensions.X = FMath::Clamp(ExtentDimensions.X * (1. - ZoomValue * ZoomFactor), MinExtentDimension, MaxExtentDimension);

	if (Shape == EMapExtentShapes::Rectangle)
	{
		ExtentDimensions.Y = FMath::Clamp(ExtentDimensions.Y * (1. - ZoomValue * ZoomFactor), MinExtentDimension, MaxExtentDimension);
	}

	bNeedsExtentChange = true;

	PreUpdateTabletop();
}

bool UXRTabletopComponent::Raycast(FVector InRayOrigin, FVector InRayDirection, OUT FVector& HitLocation)
{
	if (!MapComponent.IsValid())
	{
		return false;
	}

	auto planeCenter = MapComponent->GetComponentTransform().TransformPosition(FVector3d::ZeroVector);
	auto referencePlane = new FPlane(planeCenter, FVector::UpVector);
	bool bIsInsideExtent = false;

	if (InRayDirection.Dot(FVector3d::UpVector) == 0)
	{
		return false;
	}

	HitLocation = FMath::RayPlaneIntersection(InRayOrigin, InRayDirection, *referencePlane);

	auto relativeHitLocation = MapComponent->FromEnginePosition(HitLocation) - MapComponent->FromEnginePosition(planeCenter);

	if (Shape == EMapExtentShapes::Circle)
	{
		bIsInsideExtent = relativeHitLocation.Length() <= ExtentDimensions.X;
	}
	else if (Shape == EMapExtentShapes::Square)
	{
		bIsInsideExtent = FMath::Abs(relativeHitLocation.X) < ExtentDimensions.X / 2 && FMath::Abs(relativeHitLocation.Y) < ExtentDimensions.X / 2;
	}
	else
	{
		bIsInsideExtent = FMath::Abs(relativeHitLocation.X) < ExtentDimensions.X / 2 && FMath::Abs(relativeHitLocation.Y) < ExtentDimensions.Y / 2;
	}

	return bIsInsideExtent;
}

double UXRTabletopComponent::GetElevationOffset()
{
	return ElevationOffset;
}

void UXRTabletopComponent::SetElevationOffset(double InValue)
{
	if (ElevationOffset != InValue)
	{
		ElevationOffset = InValue;

		bNeedsOffsetChange = true;

		PreUpdateTabletop();
	}
}

FVector2D UXRTabletopComponent::GetDimensions()
{
	return ExtentDimensions;
}

void UXRTabletopComponent::SetDimensions(FVector2D InValue)
{
	if (ExtentDimensions != InValue && InValue.X > 0 && InValue.Y > 0)
	{
		ExtentDimensions = InValue;

		bNeedsExtentChange = true;

		PreUpdateTabletop();
	}
}

FGeoPosition UXRTabletopComponent::GetExtentCenter()
{
	return CenterPosition;
}
void UXRTabletopComponent::SetExtentCenter(FGeoPosition InValue)
{
	if (CenterPosition != InValue)
	{
		CenterPosition = InValue;

		bNeedsExtentChange = true;

		PreUpdateTabletop();
	}
}

EMapExtentShapes UXRTabletopComponent::GetShape()
{
	return Shape;
}
void UXRTabletopComponent::SetShape(EMapExtentShapes InValue)
{
	if (Shape != InValue)
	{
		Shape = InValue;

		bNeedsExtentChange = true;

		PreUpdateTabletop();
	}
}

FVector UXRTabletopComponent::GetMapComponentLocation()
{
	if (MapComponent.IsValid())
	{
		return MapComponent->GetComponentLocation();
	}

	return FVector(0);
}

FTransform UXRTabletopComponent::GetFromEngineTransform()
{
	if (MapComponent.IsValid())
	{
		return MapComponent->FromEngineTransform();
	}

	return FTransform();
}

bool UXRTabletopComponent::FindArcGISCameraLocationInHierarchy()
{
	ArcGISCameraLocation = nullptr;
	TArray<AActor*> actors;
	GetOwner()->GetAttachedActors(actors, true, true);
	for (auto a : actors)
	{
		if (a->FindComponentByClass<UArcGISCameraComponent>() && a->FindComponentByClass<UArcGISLocationComponent>())
			ArcGISCameraLocation = a->FindComponentByClass<UArcGISLocationComponent>();
		if (ArcGISCameraLocation)
		{
			break;
		}
	}
	return ArcGISCameraLocation != nullptr;
}

bool UXRTabletopComponent::SetupReferences()
{
	if (!MapComponent.IsValid())
	{
		MapComponent = GetOwner()->FindComponentByClass<UArcGISMapComponent>();
		if (!MapComponent.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Tabletop component must be attached to an ArcGISMapActor."));
		}
	}

	if (!ArcGISCameraLocation)
	{
		if (!FindArcGISCameraLocationInHierarchy())
		{
			UE_LOG(LogTemp, Warning, TEXT("Tabletop component requires the ArcGISMapActor to be parent to an ArcGISCamera."));
		}
	}

	if (!WrapperActor)
	{
		WrapperActor = GetOwner();
	}

	return MapComponent.IsValid() && ArcGISCameraLocation != nullptr && WrapperActor != nullptr;
}

void UXRTabletopComponent::GetTabletopController()
{
	if(const auto controller = Cast<AXRTableTopInteractor>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0)))
	{
		TabletopController = controller;
	}
}