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
//	USplineMeshComponent* SplineMesh;
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
		//	USplineMeshComponent* SplineMesh;
		float elevationOffset = 200.;
		uint16 Counter = 0;
		FVector3d Tangent;
		FVector3d* BCLocations = new FVector3d[featurePoints.Num()];
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
			lineMarker->ArcGISLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(0, 90, 0));

			if (!stops.IsEmpty())
			{
				auto lastStop = *stops.Peek();
				auto lastPoint = lastStop->ArcGISLocation->GetPosition();

				//calculate distance from last point to this point
				geodeticDistance += UArcGISGeometryEngine::DistanceGeodetic(lastPoint, thisPoint, UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Meters), UArcGISAngularUnit::CreateArcGISAngularUnit(EArcGISAngularUnitId::Degrees), EArcGISGeodeticCurveType::Geodesic)->GetDistance();
			//	GeodeticDistanceText = FString::Printf(TEXT("Distance: %f %s"), round(geodeticDistance * 1000.0) / 1000,  unitTxt);

				featurePoints.Add(lastStop);
				//interpolate middle points between last point and this point
				//Interpolate(lastStop, lineMarker, featurePoints);
				featurePoints.Add(lineMarker);
			}

			stops.Enqueue(lineMarker);
			//RenderLine(featurePoints);
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
	/*void RenderLine(TArray<ARouteMarker*> featurePoints;)
	{
		// Add a spline mesh for each segment of the route
		for (int i = 0; i < Counter; i++) {
			if (i > 0) {
				SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
				SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
				SplineMesh->RegisterComponent();
				SplineMesh->SetMobility(EComponentMobility::Movable);

				Tangent = BCLocations[i] - BCLocations[i - 1];
				Tangent.Normalize();
				Tangent = Tangent * 100.;

				SplineMesh->SetStartAndEnd(BCLocations[i - 1], Tangent, BCLocations[i], Tangent);
				SplineMesh->SetStartScale(RouteCueScale);
				SplineMesh->SetEndScale(RouteCueScale);

				SplineMesh->SetStaticMesh(RouteMesh);

				SplineMeshComponents.AddHead(SplineMesh);
			}
		}
		delete[] BCLocations;
	}
	void ClearLine()
	{
		for (auto SPC : SplineMeshComponents) {
			SPC->UnregisterComponent();
			SPC->DestroyComponent();
		}
		SplineMeshComponents.Empty();

	}
*/
	


