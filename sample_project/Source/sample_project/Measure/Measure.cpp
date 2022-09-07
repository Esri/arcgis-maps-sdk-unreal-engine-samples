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

 // Sets default values
AMeasure::AMeasure()
{
	PrimaryActorTick.bCanEverTick = true;

	// Load the necessary assets
	static ConstructorHelpers::FObjectFinder<UClass> WidgetAsset(TEXT("/Game/SampleViewer/Samples/Measure/Blueprints/UI_Measure.UI_Measure_C"));
	if (WidgetAsset.Succeeded()) {
		UIWidgetClass = WidgetAsset.Object;
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/SampleViewer/Samples/Routing/Geometries/Cube.Cube"));
	if (MeshAsset.Succeeded()) {
		RouteMesh = MeshAsset.Object;
	}

}

// Called when the game starts or when spawned
void AMeasure::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();

	// Make sure mouse cursor remains visible
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
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
	unitTxt = " m";
	unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters);
	currentUnit = m;
}

// Called every frame
void AMeasure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
	float traceLength = 10000000.f;
	FVector position;
	FVector direction;
	FHitResult hit;
	FVector tangent;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(position, direction);

	bool bTraceSuccess = GetWorld()->LineTraceSingleByChannel(hit, position,
		position + traceLength * direction, ECC_Visibility, FCollisionQueryParams());

	if (bTraceSuccess && hit.GetActor()->GetClass() == AArcGISMapActor::StaticClass() && hit.bBlockingHit)
	{
		FActorSpawnParameters SpawnParam = FActorSpawnParameters();
		SpawnParam.Owner = this;

		auto lineMarker = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), hit.ImpactPoint, FRotator(0.f), SpawnParam);

		auto thisPoint = lineMarker->ArcGISLocation->GetPosition();

		if (!stops.IsEmpty())
		{
			auto lastStop = stops.Last();
			auto lastPoint = lastStop->ArcGISLocation->GetPosition();

			//calculate distance from last point to this point
			double d = UArcGISGeometryEngine::DistanceGeodetic(lastPoint, thisPoint, unit, UArcGISAngularUnit::CreateArcGISAngularUnit(EArcGISAngularUnitId::Degrees), EArcGISGeodeticCurveType::Geodesic)->GetDistance();
			geodeticDistance += d;
			geodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), round(geodeticDistance * 1000.0) / 1000.0, *unitTxt);
			UIWidget->ProcessEvent(WidgetFunction, &geodeticDistanceText);

			//interpolate points between last point/start and this point(end)

			float n = floor((float)d / InterpolationInterval);
			double dx = (lineMarker->GetActorLocation().X - lastStop->GetActorLocation().X) / n;
			double dy = (lineMarker->GetActorLocation().Y - lastStop->GetActorLocation().Y) / n;

			auto pre = lastStop->GetActorLocation();

			//calculate n intepolation points/n segments because the last segment is already created by the end point 
			for (int i = 0; i < n - 1; i++)
			{
				SpawnParam.Owner = this;
				//calculate transform of next point
				float nextX = pre.X + (float)dx;
				float nextY = pre.Y + (float)dy;
				auto next = GetWorld()->SpawnActor<ABreadcrumb>(ABreadcrumb::StaticClass(), FVector(nextX, nextY, 0), FRotator3d(0.), SpawnParam);

				//define height
				SetElevation(next);

				featurePoints.Add(next);

				pre = next->GetActorLocation();
			}
		}
		featurePoints.Add(lineMarker);
		stops.Add(lineMarker);

		USplineMeshComponent* SplineMesh;
		for (int i = 1; i < featurePoints.Num(); i++)
		{
			SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			SplineMesh->RegisterComponent();
			SplineMesh->SetMobility(EComponentMobility::Movable);

			FVector end = featurePoints[i]->GetActorLocation();
			FVector start = featurePoints[i - 1]->GetActorLocation();

			tangent = end - start;
			tangent.Normalize();
			tangent = tangent * 100.;

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
	float raycastHeight = 1000000.0f;
	float traceLength = 1000000.f;
	FVector position = stop->GetActorLocation();
	FVector raycastStart = FVector(position.X, position.Y, position.Z + raycastHeight);
	FHitResult hitInfo;
	float elevationOffset = 200.;

	bool bTraceSuccess = GetWorld()->LineTraceSingleByChannel(hitInfo, raycastStart,
		position + traceLength * FVector3d::DownVector, ECC_Visibility, FCollisionQueryParams());
	position.Z = bTraceSuccess ? hitInfo.ImpactPoint.Z + elevationOffset : 0.;

	stop->SetActorLocation(position);
}

void AMeasure::ClearLine()
{
	if (!SplineMeshComponents.IsEmpty())
	{
		for (auto point : SplineMeshComponents) {
			point->UnregisterComponent();
			point->DestroyComponent();
		}
		for (auto stop : featurePoints)
		{
			stop->Destroy();
		}
		for (auto stop : stops)
		{
			stop->Destroy();
		}
		SplineMeshComponents.Empty();
		featurePoints.Empty();
		stops.Empty();
		geodeticDistance = 0;
		geodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), geodeticDistance, *unitTxt);
		UIWidget->ProcessEvent(WidgetFunction, &geodeticDistanceText);
	}
}
void AMeasure::UnitChanged()
{
	if (UnitDropdown->GetSelectedOption() == "Meters")
	{
		unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters);
		geodeticDistance = ConvertUnits(geodeticDistance, currentUnit, m);
		currentUnit = m;
		unitTxt = " m";
	}

	else if (UnitDropdown->GetSelectedOption() == "Kilometers")
	{
		unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Kilometers);
		geodeticDistance = ConvertUnits(geodeticDistance, currentUnit, km);
		currentUnit = km;
		unitTxt = " km";
	}
	else if (UnitDropdown->GetSelectedOption() == "Miles")
	{
		unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Miles);
		geodeticDistance = ConvertUnits(geodeticDistance, currentUnit, mi);
		currentUnit = mi;
		unitTxt = " mi";
	}
	else if (UnitDropdown->GetSelectedOption() == "Feet")
	{
		unit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Feet);
		geodeticDistance = ConvertUnits(geodeticDistance, currentUnit, ft);
		currentUnit = ft;
		unitTxt = " ft";
	}
	geodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), round(geodeticDistance * 1000.0) / 1000.0, *unitTxt);
	UIWidget->ProcessEvent(WidgetFunction, &geodeticDistanceText);
}
double AMeasure::ConvertUnits(double value, UnitType from, UnitType to)
{
	double factor[4][4] =
	{
		{ 1, 0.001, 0.000621371, 3.28084 },
		{ 1000,   1,     0.621371,   3280.84},
		{ 1609.344,     1.609344,       1,   5280},
		{ 0.3048,    0.0003048,  0.00018939,    1}
	};

	return value * factor[(int)from][(int)to];
}




