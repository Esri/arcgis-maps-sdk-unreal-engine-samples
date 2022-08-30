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
	static ConstructorHelpers::FObjectFinder<UClass> WidgetAsset(TEXT("/Game/SampleViewer/Samples/Routing/Blueprints/UI.UI_C"));
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
		}
	}
	unitTxt = " m";
}

// Called every frame
void AMeasure::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If new routing information has been received, visualize the route

	float raycastHeight = 50000.0f;
	float traceLength = 10000000.f;
	FVector position;
	FVector direction;
	FHitResult hit;
	float elevationOffset = 200.;
	uint16 Counter = 0;
	FVector3d Tangent;
	FVector3d* BCLocations = new FVector3d[featurePoints.Num()];
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(position, direction);

}

	void AMeasure::SetupInput()
	{
		InputComponent = NewObject<UInputComponent>(this);
		InputComponent->RegisterComponent();

		if (InputComponent)
		{
			InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &AMeasure::AddStop);
		//	InputComponent->BindAction("IntepolatePoint", IE_Pressed, this, &AMeasure::Interpolate);
			EnableInput(GetWorld()->GetFirstPlayerController());
		}
	}
	void AMeasure::AddStop()
	{
		float raycastHeight = 50000.0f;
		float traceLength = 10000000.f;
		FVector position;
		FVector direction;
		FHitResult hit;
		float elevationOffset = 200.;
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
			auto ArcGISLocation = lineMarker->ArcGISLocation;

			auto thisPoint = lineMarker->ArcGISLocation->GetPosition();

			if (!stops.IsEmpty())
			{
				auto lastStop = *stops.Peek();
				auto lastPoint = lastStop->ArcGISLocation->GetPosition();

				//calculate distance from last point to this point
				geodeticDistance += UArcGISGeometryEngine::DistanceGeodetic(lastPoint, thisPoint, UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters), UArcGISAngularUnit::CreateArcGISAngularUnit(EArcGISAngularUnitId::Degrees), EArcGISGeodeticCurveType::Geodesic)->GetDistance();
				GeodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), round(geodeticDistance * 1000.0) / 1000.0,  *unitTxt);
				UFunction* WidgetFunction = UIWidget->FindFunction(FName("SetTravelInfo"));
				UIWidget->ProcessEvent(WidgetFunction, &GeodeticDistanceText);

				featurePoints.Add(lastStop);
				//interpolate middle points between last point and this point
				Interpolate(lastStop, lineMarker);
				featurePoints.Add(lineMarker);
			}

			stops.Enqueue(lineMarker);

			// Add a spline mesh for each segment of the route
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
				//	SplineMeshComponents.AddHead(SplineMesh);

			}
		}
	}
	// Do a line trace from high above to update the elevation info of feature points
	void AMeasure::SetElevation(ARouteMarker* stop)
	{
		float raycastHeight = 1000000.0f;
		float traceLength = 1000000.f;
		FVector position=stop->GetActorLocation();
		FVector raycastStart = FVector(position.X, position.Y, position.Z+raycastHeight);
		FHitResult hitInfo;
		float elevationOffset = 200.;


		bool bTraceSuccess = GetWorld()->LineTraceSingleByChannel(hitInfo, raycastStart,
			position + traceLength * FVector3d::DownVector, ECC_Visibility, FCollisionQueryParams());
		position.Z = bTraceSuccess ? hitInfo.ImpactPoint.Z + elevationOffset : 0.;

		stop->SetActorLocation(position);

	
	}

	void AMeasure::Interpolate(ARouteMarker* start, ARouteMarker* end)
	{
		auto startPoint = start->ArcGISLocation->GetPosition();
		auto endPoint = end->ArcGISLocation->GetPosition();

		double d = UArcGISGeometryEngine::DistanceGeodetic(startPoint, endPoint, UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters), UArcGISAngularUnit::CreateArcGISAngularUnit(EArcGISAngularUnitId::Degrees), EArcGISGeodeticCurveType::Geodesic)->GetDistance();
		float n = floor((float)d / InterpolationInterval);
		double dx = (end->GetActorLocation().X - start->GetActorLocation().X) / n;
		double dy = (end->GetActorLocation().Y - start->GetActorLocation().Y) / n;

		auto pre = start->GetActorLocation();

		//calculate n-1 intepolation points/n-1 segments because the last segment is already created by the end point 
		for (int i = 0; i < n - 1; i++)
		{
			FActorSpawnParameters SpawnParam = FActorSpawnParameters();
			SpawnParam.Owner = this;
			auto next = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), FVector(0,0,0), FRotator(0.f), SpawnParam);

			//calculate transform of next point
			float nextX = pre.X + (float)dx;
			float nextY = pre.Y + (float)dy;
			next->SetActorLocation(FVector(nextX, 0, nextY));

			//set default location component of next point
		//	next.GetComponent<ArcGISLocationComponent>().Rotation = new ArcGISRotation(0, 90, 0);

			//define height
			SetElevation(next);

			featurePoints.Add(next);

			pre = next->GetActorLocation();
		}

	}
/*	void ClearLine()
	{
		USplineMeshComponent* SplineMesh=getactorofclasss
		for (auto point : SplineMeshComponents) {
			point->UnregisterComponent();
			point->DestroyComponent();
		}
		SplineMeshComponents.Empty();

	}*/

	


