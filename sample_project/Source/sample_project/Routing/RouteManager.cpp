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

#include "RouteManager.h"

// Sets default values
ARouteManager::ARouteManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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
void ARouteManager::BeginPlay()
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
void ARouteManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If new routing information has been received, visualize the route
	if (bShouldUpdateBreadcrums) {
		float TraceLength = 10000000.;
		float HeightOffset = 200.;
		FHitResult TraceHit;
		FVector3d WorldLocation;
		bool bTraceSuccess = false;
		uint16 Counter = 0;
		FVector3d Tangent;
		USplineMeshComponent* SplineMesh;
		FVector3d* BCLocations = new FVector3d[Breadcrumbs.Num()];

		for (auto BC: Breadcrumbs){
			RemoveTickPrerequisiteComponent(BC->ArcGISLocation);

			// Do a line trace to determine the height of breadcrumbs
			WorldLocation = FVector3d(
				BC->GetActorLocation().X, BC->GetActorLocation().Y, TraceLength / 2.f);
			bTraceSuccess = GetWorld()->LineTraceSingleByChannel(TraceHit, WorldLocation,
				WorldLocation + TraceLength * FVector3d::DownVector, ECC_Visibility, FCollisionQueryParams());
			WorldLocation.Z = bTraceSuccess ? TraceHit.ImpactPoint.Z + HeightOffset : 0.;

			BC->SetActorLocation(WorldLocation);
			BCLocations[Counter] = WorldLocation;
			Counter++;

		}

		// Remove the old route cue
		for (auto SPC : SplineMeshComponents) {
			SPC->UnregisterComponent();
			SPC->DestroyComponent();
		}
		SplineMeshComponents.Empty();

		// Add a spline mesh for each segment of the route
		for (int i = 0; i < Counter; i++) {
			if (i > 0) {
				SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
				SplineMesh->AttachToComponent(RootComponent,FAttachmentTransformRules::KeepWorldTransform);
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
		bShouldUpdateBreadcrums = false;
	}

}

// Bind the handler for selecting a stop point
void ARouteManager::SetupInput()
{
	InputComponent = NewObject<UInputComponent>(this);
	InputComponent->RegisterComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &ARouteManager::AddStop);
		EnableInput(GetWorld()->GetFirstPlayerController());
	}
}

// Identify the location clicked on and make a line trace from there to find the point for placing the stop 
void ARouteManager::AddStop()
{
	float DistanceAboveGround = 50000.0f;
	float TraceLength = 10000000.f;
	FVector WorldLocation;
	FVector WorldDirection;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FVector PlaneOrigin(0.0f, 0.0f, DistanceAboveGround);
	FVector ActorWorldLocation = FMath::LinePlaneIntersection(
		WorldLocation,
		WorldLocation + WorldDirection,
		PlaneOrigin,
		FVector::UpVector);

	FHitResult TraceHit;
	if (!bIsRouting && GetWorld()->LineTraceSingleByChannel(TraceHit,
		WorldLocation, WorldLocation + TraceLength * WorldDirection, ECC_Visibility, FCollisionQueryParams()))
	{
		if (TraceHit.bBlockingHit)
		{
			FActorSpawnParameters SpawnParam = FActorSpawnParameters();
			SpawnParam.Owner = this;
			Stops.AddHead(GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), TraceHit.ImpactPoint, FRotator(0.f), SpawnParam));

			// Update the list of stops
			if (Stops.Num() > StopCount) {
				auto OldStop = Stops.GetTail();

				OldStop->GetValue()->Destroy();
				Stops.RemoveNode(OldStop);
			}

			// Make a routing query if enough stops added 
			if (Stops.Num() == StopCount) {
				bIsRouting = true;
				PostRoutingRequest();
				
			}
		}
	}
}

// Make a query for routing between the selected stops 
void ARouteManager::PostRoutingRequest()
{
	FString RequestBody;
	FString StopCoordinates;
	UArcGISPoint* Point;

	// Set up the query 
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();	
	Request->OnProcessRequestComplete().BindUObject(this, &ARouteManager::ProcessQueryResponse);
	Request->SetURL("https://route-api.arcgis.com/arcgis/rest/services/World/Route/NAServer/Route_World/solve");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded");
	
	// Make a string of the coordinates of the stops
	for (auto Stop: Stops){
		Point = Stop->ArcGISLocation->GetPosition();

		// If the geographic coordinates of the stop are not in terms of lat & lon, project them 
		if (Point->GetSpatialReference()->GetWKID() != 4326) {
			auto ProjectedGeometry = UArcGISGeometryEngine::Project(Point, 
				UArcGISSpatialReference::CreateArcGISSpatialReference(4326));
			if (ProjectedGeometry != nullptr)
			{
				Point = static_cast<UArcGISPoint*>(ProjectedGeometry);
			}
		}
		StopCoordinates.Append(FString::Printf(TEXT("%f,%f;"), Point->GetX(), Point->GetY()));
	}
	StopCoordinates.RemoveFromEnd(";");

	MapComponent = UArcGISMapComponent::GetMapComponent(this);

	// Read the API key from the map component
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";

	// Set the request body and sent it
	RequestBody = TEXT("f=json&returnRoutes=true&token=") + APIToken + "&stops=" + StopCoordinates;
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

void ARouteManager::ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	// Process the response if the query was successful
	if (FJsonSerializer::Deserialize(Reader, JsonObj) && Response->GetResponseCode()>199 && Response->GetResponseCode() < 300) {
		float TraceLength = 100000.;

		FHitResult TraceHit;
		FVector3d WorldLocation;
		ABreadcrumb* BC;
		double Minutes;
		TSharedPtr<FJsonValue> RoutesField;
		TSharedPtr<FJsonValue> GeometryField;
		TSharedPtr<FJsonValue> AttributesField;
		const TArray<TSharedPtr<FJsonValue>>* FeaturesField;
		const TArray<TSharedPtr<FJsonValue>>* PathsField;
		const TArray<TSharedPtr<FJsonValue>>* PointsField;
		const TArray<TSharedPtr<FJsonValue>>* StopCoordinates;

		// Remove the old breadcrumbs
		for (auto Breadcrumb : Breadcrumbs) {
			Breadcrumb->Destroy();
		}
		Breadcrumbs.Empty();

		// Parse the query response
		if (RoutesField = JsonObj->TryGetField(TEXT("routes"))) {
			JsonObj = RoutesField->AsObject();
			if (JsonObj->TryGetArrayField(TEXT("features"), FeaturesField)) {

				for (auto feature : *FeaturesField) {
					JsonObj = feature->AsObject();
					AttributesField = JsonObj->TryGetField(TEXT("attributes")); // checked later
					if (GeometryField = JsonObj->TryGetField(TEXT("geometry"))) {
						JsonObj = GeometryField->AsObject();
						if (JsonObj->TryGetArrayField(TEXT("paths"), PathsField)) {
							for (auto path : *PathsField) {
								PointsField = &path->AsArray();
								for (auto point : *PointsField) {
									StopCoordinates = &point->AsArray();
									
									// Create a breadcrumb for each path point and set its location from the query response
									FActorSpawnParameters SpawnParam = FActorSpawnParameters();
									SpawnParam.Owner = this;
									BC = GetWorld()->SpawnActor<ABreadcrumb>(ABreadcrumb::StaticClass(),FVector3d(0.), FRotator3d(0.), SpawnParam);
									
									BC->ArcGISLocation->SetPosition(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(
										(*StopCoordinates)[0]->AsNumber(), 
										(*StopCoordinates)[1]->AsNumber(), 
										UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
									
									// Make sure this actor doesn't tick before all Breadcrumb actors have ticked. 
									// Ensures that the transform of all Breadcrum actors have been updated before adding the route cue
									AddTickPrerequisiteComponent(BC->ArcGISLocation);
									Breadcrumbs.AddHead(BC);
								}
							}
						}
					}
					if (AttributesField) {
						JsonObj = AttributesField->AsObject();
						if (JsonObj->TryGetNumberField(TEXT("Total_TravelTime"), Minutes)) {
							
							// Call a function from the UI widget to set the travel time
							UFunction* WidgetFunction = UIWidget->FindFunction(FName("SetTravelTime"));
							if (WidgetFunction) {
								UIWidget->ProcessEvent(WidgetFunction, &Minutes);
							}
						}
					}
				}
				// Visualize the route in the next tick
				bShouldUpdateBreadcrums = true;
			}
		}
	}
	bIsRouting = false;
}
