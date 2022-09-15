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

constexpr double InterpolationInterval = 100;
constexpr int TraceLength = 1000000;

AMeasure::AMeasure()
{
	PrimaryActorTick.bCanEverTick = true;

	// Load the necessary assets
	ConstructorHelpers::FObjectFinder<UClass> WidgetAsset(TEXT("/Game/SampleViewer/Samples/Measure/Blueprints/UI_Measure.UI_Measure_C"));
	if (WidgetAsset.Succeeded())
	{
		UIWidgetClass = WidgetAsset.Object;
	}

	ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/SampleViewer/Samples/Routing/Geometries/Cube.Cube"));

	if (MeshAsset.Succeeded())
	{
		RouteMesh = MeshAsset.Object;
	}
}

// Called when the game starts or when spawned
void AMeasure::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();

	// Make sure mouse cursor remains visible
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (playerController)
	{
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
	}

	// Create the UI and add it to the viewport
	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			UIWidget->AddToViewport();
			WidgetFunction = UIWidget->FindFunction(FName("SetDistance"));
			UnitDropdown = (UComboBoxString*)UIWidget->GetWidgetFromName(TEXT("UnitDropDown"));
		}
	}

	UnitText = " m";
	Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters);
	CurrentUnit = meters;
}

void AMeasure::SetupInput()
{
	InputComponent = NewObject<UInputComponent>(this);
	InputComponent->RegisterComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &AMeasure::AddStop);
		EnableInput(GetWorld()->GetFirstPlayerController());
	}
}
void AMeasure::AddStop()
{
	FVector position;
	FVector direction;
	FHitResult hit;
	FVector tangent;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(position, direction);

	bool bTraceSuccess =
		GetWorld()->LineTraceSingleByChannel(hit, position, position + TraceLength * direction, ECC_Visibility, FCollisionQueryParams());

	if (bTraceSuccess && hit.GetActor()->GetClass() == AArcGISMapActor::StaticClass() && hit.bBlockingHit)
	{
		FActorSpawnParameters SpawnParam = FActorSpawnParameters();
		SpawnParam.Owner = this;

		auto lineMarker = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), hit.ImpactPoint, FRotator(0), SpawnParam);

		auto thisPoint = lineMarker->ArcGISLocation->GetPosition();

		if (!Stops.IsEmpty())
		{
			auto lastStop = Stops.Last();
			auto lastPoint = lastStop->ArcGISLocation->GetPosition();

			//calculate distance from last point to this point
			double distance = UArcGISGeometryEngine::DistanceGeodetic(lastPoint, thisPoint, Unit,
																	  UArcGISAngularUnit::CreateArcGISAngularUnit(EArcGISAngularUnitId::Degrees),
																	  EArcGISGeodeticCurveType::Geodesic)->GetDistance();
			GeodeticDistance += distance;
			GeodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), round(GeodeticDistance * 1000.0) / 1000.0, *UnitText);
			UIWidget->ProcessEvent(WidgetFunction, &GeodeticDistanceText);

			//interpolate points between last point/start and this point(end)

			float numInterpolation = floor((float)distance / InterpolationInterval);
			double dx = (lineMarker->GetActorLocation().X - lastStop->GetActorLocation().X) / numInterpolation;
			double dy = (lineMarker->GetActorLocation().Y - lastStop->GetActorLocation().Y) / numInterpolation;

			auto pre = lastStop->GetActorLocation();

			//calculate numInterpolation intepolation points/n segments because the last segment is already created by the end point
			for (int i = 0; i < numInterpolation - 1; i++)
			{
				SpawnParam.Owner = this;
				//calculate transform of next point
				float nextX = pre.X + (float)dx;
				float nextY = pre.Y + (float)dy;
				auto next = GetWorld()->SpawnActor<ABreadcrumb>(ABreadcrumb::StaticClass(), FVector(nextX, nextY, 0), FRotator3d(0), SpawnParam);

				//define height
				SetElevation(next);

				FeaturePoints.Add(next);

				pre = next->GetActorLocation();
			}
		}
		FeaturePoints.Add(lineMarker);
		Stops.Add(lineMarker);

		TObjectPtr<USplineMeshComponent> SplineMesh;

		for (int i = 1; i < FeaturePoints.Num(); i++)
		{
			SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			SplineMesh->RegisterComponent();
			SplineMesh->SetMobility(EComponentMobility::Movable);

			FVector end = FeaturePoints[i]->GetActorLocation();
			FVector start = FeaturePoints[i - 1]->GetActorLocation();

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
}

// Do a line trace from high above to update the elevation info of feature points
void AMeasure::SetElevation(AActor* stop)
{
	FVector position = stop->GetActorLocation();
	FVector raycastStart = FVector(position.X, position.Y, position.Z + TraceLength);
	FHitResult hitInfo;
	float elevationOffset = 200.0f;

	bool bTraceSuccess = GetWorld()->LineTraceSingleByChannel(hitInfo, raycastStart, position + TraceLength * FVector3d::DownVector, ECC_Visibility,
															  FCollisionQueryParams());
	position.Z = bTraceSuccess ? hitInfo.ImpactPoint.Z + elevationOffset : 0.0f;

	stop->SetActorLocation(position);
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
		GeodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), GeodeticDistance, *UnitText);
		UIWidget->ProcessEvent(WidgetFunction, &GeodeticDistanceText);
	}
}
void AMeasure::UnitChanged()
{
	if (UnitDropdown->GetSelectedOption() == "Meters")
	{
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters);
		GeodeticDistance = ConvertUnits(GeodeticDistance, CurrentUnit, meters);
		CurrentUnit = meters;
		UnitText = " m";
	}
	else if (UnitDropdown->GetSelectedOption() == "Kilometers")
	{
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Kilometers);
		GeodeticDistance = ConvertUnits(GeodeticDistance, CurrentUnit, kilometers);
		CurrentUnit = kilometers;
		UnitText = " km";
	}
	else if (UnitDropdown->GetSelectedOption() == "Miles")
	{
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Miles);
		GeodeticDistance = ConvertUnits(GeodeticDistance, CurrentUnit, miles);
		CurrentUnit = miles;
		UnitText = " mi";
	}
	else if (UnitDropdown->GetSelectedOption() == "Feet")
	{
		Unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Feet);
		GeodeticDistance = ConvertUnits(GeodeticDistance, CurrentUnit, feet);
		CurrentUnit = feet;
		UnitText = " ft";
	}

	GeodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), round(GeodeticDistance * 1000.0) / 1000.0, *UnitText);
	UIWidget->ProcessEvent(WidgetFunction, &GeodeticDistanceText);
}
double AMeasure::ConvertUnits(double value, UnitType from, UnitType to)
{
	double factor[4][4] = {
		{1, 0.001, 0.000621371, 3.28084}, {1000, 1, 0.621371, 3280.84}, {1609.344, 1.609344, 1, 5280}, {0.3048, 0.0003048, 0.00018939, 1}};

	return value * factor[(int)from][(int)to];
}