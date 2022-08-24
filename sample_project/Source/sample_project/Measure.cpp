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

		float TraceLength = 10000000.;
		float HeightOffset = 200.;
		FHitResult TraceHit;
		FVector3d WorldLocation;
		bool bTraceSuccess = false;
		uint16 Counter = 0;
		FVector3d Tangent;
		USplineMeshComponent* SplineMesh;
		FVector3d* BCLocations = new FVector3d[featurePoints.Num()];

		float DistanceAboveGround = 50000.0f;
		float TraceLength = 10000000.f;
		FVector WorldLocation;
		FVector WorldDirection;
		APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

		FHitResult TraceHit;
		bool bTraceSuccess = GetWorld()->LineTraceSingleByChannel(TraceHit, WorldLocation,
			WorldLocation + TraceLength * WorldDirection, ECC_Visibility, FCollisionQueryParams());

		if (bTraceSuccess && TraceHit.GetActor()->GetClass() == AArcGISMapActor::StaticClass() && TraceHit.bBlockingHit)
		{
			FActorSpawnParameters SpawnParam = FActorSpawnParameters();
			SpawnParam.Owner = this;

			auto lineMarker = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), TraceHit.ImpactPoint, FRotator(0.f), SpawnParam);
			auto thisPoint = TraceHit.ImpactPoint;

			lineMarker.findComponentBy= true;
			lineMarker->GetComponent(UArcGISLocationComponent) = thisPoint;
			lineMarker.GetComponent<ArcGISLocationComponent>().Rotation = new ArcGISRotation(0, 90, 0);
			if (!stops.IsEmpty())
			{
				auto lastStop = stops.Peek();
				auto lastPoint = lastStop.GetComponent<ArcGISLocationComponent>().Position;

				//calculate distance from last point to this point
				geodedicDistance += ArcGISGeometryEngine.DistanceGeodetic(lastPoint, thisPoint, new ArcGISLinearUnit(unit), new ArcGISAngularUnit(unitDegree), ArcGISGeodeticCurveType.Geodesic).Distance;
				GeodedicDistanceText.text = "Distance: " + Math.Round(geodedicDistance, 3).ToString() + unitTxt;

				featurePoints.Add(lastStop);
				//interpolate middle points between last point and this point
				Interpolate(lastStop, lineMarker, featurePoints);
				featurePoints.Add(lineMarker);
			}

			stops.Enqueue(lineMarker);
			RenderLine(featurePoints);
		}
		

	void AMeasure::SetupInput()
	{
		InputComponent = NewObject<UInputComponent>(this);
		InputComponent->RegisterComponent();

		if (InputComponent)
		{
			InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &ARouteManager::AddStop);
			EnableInput(GetWorld()->GetFirstPlayerController());
		}
	}
	void SetBreadcrumbHeight()
	{
		for (int i = 0; i < featurePoints.Num(); i++)
		{
			SetElevation(featurePoints[i]);
		}
	}
	// Do a line trace to determine the height of breadcrumbs
	void SetElevation(GameObject stop)
	{
		
		if (Physics.Raycast(raycastStart, Vector3.down, out RaycastHit hitInfo)
		{
			RemoveTickPrerequisiteComponent(BC->ArcGISLocation);			
			WorldLocation = FVector3d(
				BC->GetActorLocation().X, BC->GetActorLocation().Y, TraceLength / 2.f);
			bTraceSuccess = GetWorld()->LineTraceSingleByChannel(TraceHit, WorldLocation,
				WorldLocation + TraceLength * FVector3d::DownVector, ECC_Visibility, FCollisionQueryParams());
			WorldLocation.Z = bTraceSuccess ? TraceHit.ImpactPoint.Z + HeightOffset : 0.;

			BC->SetActorLocation(WorldLocation);
			featurePoints[Counter] = WorldLocation;
		}
	}
	void RenderLine(TArray<ARouteMarker*> featurePoints;)
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

	
}

