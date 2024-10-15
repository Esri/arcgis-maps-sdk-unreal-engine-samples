/* Copyright 2022 Esri
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

#include "Measure.h"

constexpr float ElevationOffset = 200.0f;
constexpr double InterpolationInterval = 10000;
constexpr int TraceLength = 1000000;

AMeasure::AMeasure()
{
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AMeasure::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		SetupPlayerInputComponent(PlayerController->InputComponent);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	// Make sure mouse cursor remains visible
	auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (playerController)
	{
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
	}

	// Create the UI and add it to the viewport

	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		HideInstructions = UIWidget->FindFunction(FName("HideDirections"));
		if (UIWidget)
		{
			UIWidget->AddToViewport();
			WidgetFunction = UIWidget->FindFunction(FName("SetDistance"));
			UnitDropdown = (UComboBoxString*)UIWidget->GetWidgetFromName(TEXT("UnitDropDown"));
		}
	}
	
	Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Miles);
}

void AMeasure::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(mousePress, ETriggerEvent::Started, this, &AMeasure::AddStop);
	}
}

void AMeasure::AddStop(const FInputActionValue& value)
{
	FVector direction;
	FHitResult hit;
	FVector position;

	auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(position, direction);

	bool bTraceSuccess =
		GetWorld()->LineTraceSingleByChannel(hit, position, position + TraceLength * direction, ECC_Visibility, FCollisionQueryParams());

	if (bTraceSuccess && hit.GetActor()->GetClass() == AArcGISMapActor::StaticClass() && hit.bBlockingHit)
	{
		SpawnParam.Owner = this;

		auto lineMarker = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), hit.ImpactPoint, FRotator(0), SpawnParam);

		auto thisPoint = lineMarker->ArcGISLocation->GetPosition();

		if (!Stops.IsEmpty())
		{
			auto lastStop = Stops.Last();
			auto lastPoint = lastStop->ArcGISLocation->GetPosition();

			//Calculate distance from last point to this point
			SegmentDistance = UArcGISGeometryEngine::DistanceGeodetic(lastPoint, thisPoint, Unit,
																	  UArcGISAngularUnit::CreateArcGISAngularUnit(EArcGISAngularUnitId::Degrees),
																	  EArcGISGeodeticCurveType::Geodesic)
								  ->GetDistance();
			GeodeticDistance += SegmentDistance;

			// Confirm FeaturePoints list does not already contain element

			if (!FeaturePoints.Contains(lastStop))
			{
				FeaturePoints.Add(lastStop);
			}

			Interpolate(lastStop, lineMarker);

			// Confirm FeaturePoints list does not already contain element

			if (!FeaturePoints.Contains(lineMarker))
			{
				FeaturePoints.Add(lineMarker);
			}
		}

		// Confirm Stops list does not already contain element

		if (!Stops.Contains(lineMarker))
		{
			Stops.Add(lineMarker);
		}
		RenderLine();
	}
}

//Interpolate points between last point/start and this point(end)
void AMeasure::Interpolate(AActor* start, AActor* end)
{
	int numInterpolation = floor(FVector::Distance(start->GetActorLocation(), end->GetActorLocation()) / InterpolationInterval);
	double dx = (end->GetActorLocation().X - start->GetActorLocation().X) / numInterpolation;
	double dy = (end->GetActorLocation().Y - start->GetActorLocation().Y) / numInterpolation;
	double dz = (end->GetActorLocation().Z - start->GetActorLocation().Z) / numInterpolation;

	auto previousInterpolation = start->GetActorLocation() + FVector(0, 0, MarkerHeight);

	for (int i = 0; i < numInterpolation - 1; i++)
	{
		SpawnParam.Owner = this;
		//Calculate transform of nextInterpolation point
		float nextInterpolationX = previousInterpolation.X + (float)dx;
		float nextInterpolationY = previousInterpolation.Y + (float)dy;
		float nextInterpolationZ = previousInterpolation.Z + (float)dz;
		auto nextInterpolation =
			GetWorld()->SpawnActor<ABreadcrumb>(ABreadcrumb::StaticClass(), FVector(nextInterpolationX, nextInterpolationY, nextInterpolationZ), FRotator3d(0), SpawnParam);

		// Confirm FeaturePoints list does not already contain element

		if (!FeaturePoints.Contains(nextInterpolation))
		{
			FeaturePoints.Add(nextInterpolation);
		}

		previousInterpolation = nextInterpolation->GetActorLocation();
	}
}

// Do a line trace from high above to update the elevation info of feature points
void AMeasure::SetElevation(AActor* stop)
{
	FHitResult hitInfo;
	FVector position = stop->GetActorLocation();
	FVector raycastStart = FVector(position.X, position.Y, position.Z + TraceLength);

	bool bTraceSuccess = GetWorld()->LineTraceSingleByChannel(hitInfo, raycastStart, position + TraceLength * FVector3d::DownVector, ECC_Visibility,
															  FCollisionQueryParams());

	position.Z = bTraceSuccess ? hitInfo.ImpactPoint.Z + ElevationOffset : 0.0f;

	stop->SetActorLocation(position);
}

void AMeasure::RenderLine()
{
	TObjectPtr<USplineMeshComponent> SplineMesh;
	FVector tangent;

	for (int i = 1; i < FeaturePoints.Num(); i++)
	{
		SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		SplineMesh->RegisterComponent();
		SplineMesh->SetMobility(EComponentMobility::Movable);

		FVector end = FeaturePoints[i]->GetActorLocation();

		// Since interpolations are already at correct elevation, only alter the route markers

		if (Cast<ARouteMarker>(FeaturePoints[i]) != nullptr)
		{
			end.Z = end.Z + MarkerHeight;
		}

		FVector start = FeaturePoints[i - 1]->GetActorLocation();

		// Since interpolations are already at correct elevation, only alter the route markers

		if (Cast<ARouteMarker>(FeaturePoints[i - 1]) != nullptr)
		{
			start.Z += MarkerHeight;
		}	

		tangent = end - start;
		tangent.Normalize();
		tangent = tangent * 100;

		SplineMesh->SetStartAndEnd(start, tangent, end, tangent);
		SplineMesh->SetStartScale(RouteCueScale);
		SplineMesh->SetEndScale(RouteCueScale);
		SplineMesh->SetStaticMesh(RouteMesh);
		SplineMeshComponents.AddHead(SplineMesh);
	}
}

void AMeasure::ClearLine()
{
	if (!SplineMeshComponents.IsEmpty())
	{
		for (auto point : SplineMeshComponents)
		{
			point->UnregisterComponent();
			point->DestroyComponent();
		}

		for (auto stop : FeaturePoints)
		{
			stop->Destroy();
		}

		for (auto stop : Stops)
		{
			stop->Destroy();
		}

		SplineMeshComponents.Empty();
		FeaturePoints.Empty();
		Stops.Empty();
		GeodeticDistance = 0;
	}
}

void AMeasure::UnitChanged()
{
	if (Selection == isMetes)
	{
		GeodeticDistance = Unit->ConvertTo(UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters), GeodeticDistance);
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters);
	}
	else if (Selection == isKilometers)
	{
		GeodeticDistance = Unit->ConvertTo(UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Kilometers), GeodeticDistance);
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Kilometers);
	}
	else if (Selection == isMiles)
	{
		GeodeticDistance = Unit->ConvertTo(UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Miles), GeodeticDistance);
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Miles);
	}
	else if (Selection == isFeet)
	{
		GeodeticDistance = Unit->ConvertTo(UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Feet), GeodeticDistance);
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Feet);
	}
}

void AMeasure::HideDirections()
{
	AActor* self = this;
	if (HideInstructions) {
		UIWidget->ProcessEvent(HideInstructions, &self);
	}
}

