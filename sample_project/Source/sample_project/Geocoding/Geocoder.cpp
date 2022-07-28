// Fill out your copyright notice in the Description page of Project Settings.


#include "Geocoder.h"

// Sets default values
AGeocoder::AGeocoder()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AGeocoder::SendRequest()
{
	//if (InAddress.IsEmpty()) {

	//	SendAddressQuery(TEXT("1600 Pennsylvania Ave NW,DC"));
	//}
	//else {
	//	SendAddressQuery(InAddress);

	//}




}

// Called when the game starts or when spawned
void AGeocoder::BeginPlay()
{
	Super::BeginPlay();

	
	// Make sure mouse cursor remains visible
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
	}


}

// Called every frame
void AGeocoder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



}

//// Bind the handler for selecting a stop point
//void AGeocoder::SetupInput()
//{
//	InputComponent = NewObject<UInputComponent>(this);
//	InputComponent->RegisterComponent();
//
//	if (InputComponent)
//	{
//		InputComponent->BindAction("LeftClick", IE_Pressed, this, &AGeocoder::SendLocationQuery);
//		EnableInput(GetWorld()->GetFirstPlayerController());
//	}
//}
//// Identify the location clicked on and make a line trace from there to find the point for placing the stop 
//void ARouteManager::AddStop()
//{
//	float DistanceAboveGround = 50000.0f;
//	float TraceLength = 10000000.f;
//	FVector WorldLocation;
//	FVector WorldDirection;
//	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
//	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);
//
//	FVector PlaneOrigin(0.0f, 0.0f, DistanceAboveGround);
//	FVector ActorWorldLocation = FMath::LinePlaneIntersection(
//		WorldLocation,
//		WorldLocation + WorldDirection,
//		PlaneOrigin,
//		FVector::UpVector);
//
//	FHitResult TraceHit;
//	if (!bIsRouting && GetWorld()->LineTraceSingleByChannel(TraceHit,
//		WorldLocation, WorldLocation + TraceLength * WorldDirection, ECC_Visibility, FCollisionQueryParams()))
//	{
//		if (TraceHit.bBlockingHit)
//		{
//			FActorSpawnParameters SpawnParam = FActorSpawnParameters();
//			SpawnParam.Owner = this;
//			Stops.AddHead(GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), TraceHit.ImpactPoint, FRotator(0.f), SpawnParam));
//
//			// Update the list of stops
//			if (Stops.Num() > StopCount) {
//				auto OldStop = Stops.GetTail();
//
//				OldStop->GetValue()->Destroy();
//				Stops.RemoveNode(OldStop);
//			}
//
//			// Make a routing query if enough stops added 
//			if (Stops.Num() == StopCount) {
//				bIsRouting = true;
//				PostRoutingRequest();
//
//			}
//		}
//	}
//}

// Make a query for routing between the selected stops 
void AGeocoder::SendAddressQuery(FString Address)
{
	UArcGISMapComponent* MapComponent = UArcGISMapComponent::GetMapComponent(this);

	FString Url = "https://geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates";
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";
	FString Query;

	//"GET geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates?f=pjson&singleLine=1600PennsylvaniaAveNW,DC&token=<ACCESS_TOKEN>HTTP/1.1"

	// Set up the query 
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGeocoder::ProcessQueryResponse);

	Query = FString::Printf(TEXT("%s/?f=json&token=%s&address=%s"), *Url, *APIToken, *Address);



	//Request->SetURL("https://geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates/?f=json&&token=AAPKd92e3919a64e4758ae48061d88d35ee49rXW5paRxGMNDnw3m2hBpZXAElRU7MszfXmiCooHvKGW-vwB4patDMQdu72nTYiL&address=123%201st%20st,NYC");
	//Request->SetURL("https://geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates");
	Request->SetURL(Query.Replace(TEXT(" "), TEXT("%20")));
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "x-www-form-urlencoded"); //application/json  ---  application/x-www-form-urlencoded
	//Request->SetHeader("User-Agent", "X-UnrealEngine-Agent");

	//Request->SetTimeout(3);

	//// Make a string of the coordinates of the stops
	//for (auto Stop : Stops) {
	//	Point = Stop->ArcGISLocation->GetPosition();

	//	// If the geographic coordinates of the stop are not in terms of lat & lon, project them 
	//	if (Point->GetSpatialReference()->GetWKID() != 4326) {
	//		auto ProjectedGeometry = UArcGISGeometryEngine::Project(Point,
	//			UArcGISSpatialReference::CreateArcGISSpatialReference(4326));
	//		if (ProjectedGeometry != nullptr)
	//		{
	//			Point = static_cast<UArcGISPoint*>(ProjectedGeometry);
	//		}
	//	}
	//	StopCoordinates.Append(FString::Printf(TEXT("%f,%f;"), Point->GetX(), Point->GetY()));
	//}
	//StopCoordinates.RemoveFromEnd(";");


	// Read the API key from the map component


	// Set the request body and sent it
//	RequestBody = TEXT("f=json&token=") + APIToken + TEXT("&address=") + Address;
//
////	RequestBody = TEXT("&token=AAPKd92e3919a64e4758ae48061d88d35ee49rXW5paRxGMNDnw3m2hBpZXAElRU7MszfXmiCooHvKGW-vwB4patDMQdu72nTYiL&address=123 1stst,NYC");
//	RequestBody = TEXT("f=json");
//	
	UE_LOG(LogTemp, Warning, TEXT("--------%s"), *Query);
	//Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}

//void AGeocoder::SendLocationQuery(UArcGISPoint Point)
//{
//}

void AGeocoder::ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	FString ResponseAddress = "";
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	UE_LOG(LogTemp, Warning, TEXT("response code %d"), Response->GetResponseCode());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, JsonObj) && Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {
		const TArray<TSharedPtr<FJsonValue>>* Candidates;
		TSharedPtr<FJsonValue> Location;
		double PointX, PointY;
		//AQueryLocation* LocationActor;

		if (JsonObj->TryGetArrayField(TEXT("candidates"), Candidates)) {

			//for (auto address : *Candidates) {
			TSharedPtr<FJsonValue> candidate = (*Candidates)[0];


			JsonObj = candidate->AsObject();

			if (JsonObj->TryGetStringField(TEXT("Address"), ResponseAddress)) {

				UE_LOG(LogTemp, Warning, TEXT("Address: %s"), *ResponseAddress);
			}

			if (Location = JsonObj->TryGetField(TEXT("location"))) {
				JsonObj = Location->AsObject();
				JsonObj->TryGetNumberField("x", PointX);
				JsonObj->TryGetNumberField("y", PointY);


				if (QueryLocation == nullptr) {
					// Create a breadcrumb for each path point and set its location from the query response
					FActorSpawnParameters SpawnParam = FActorSpawnParameters();
					SpawnParam.Owner = this;
					QueryLocation = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(), FVector3d(0.), FRotator3d(0.), SpawnParam);

				}

				QueryLocation->ApplyQueryResults(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
					PointX, PointY, 10000,
					UArcGISSpatialReference::CreateArcGISSpatialReference(4326)), ResponseAddress);
			}
		}

	}


	//// Process the response if the query was successful
	//if (FJsonSerializer::Deserialize(Reader, JsonObj) && Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {
	//	float TraceLength = 100000.;

	//	FHitResult TraceHit;
	//	FVector3d WorldLocation;
	//	ABreadcrumb* BC;
	//	double Minutes;
	//	TSharedPtr<FJsonValue> RoutesField;
	//	TSharedPtr<FJsonValue> GeometryField;
	//	TSharedPtr<FJsonValue> AttributesField;
	//	const TArray<TSharedPtr<FJsonValue>>* FeaturesField;
	//	const TArray<TSharedPtr<FJsonValue>>* PathsField;
	//	const TArray<TSharedPtr<FJsonValue>>* PointsField;
	//	const TArray<TSharedPtr<FJsonValue>>* StopCoordinates;

	//	// Remove the old breadcrumbs
	//	for (auto Breadcrumb : Breadcrumbs) {
	//		Breadcrumb->Destroy();
	//	}
	//	Breadcrumbs.Empty();

	//	// Parse the query response
	//	if (RoutesField = JsonObj->TryGetField(TEXT("routes"))) {
	//		JsonObj = RoutesField->AsObject();
	//		if (JsonObj->TryGetArrayField(TEXT("features"), FeaturesField)) {

	//			for (auto feature : *FeaturesField) {
	//				JsonObj = feature->AsObject();
	//				AttributesField = JsonObj->TryGetField(TEXT("attributes")); // checked later
	//				if (GeometryField = JsonObj->TryGetField(TEXT("geometry"))) {
	//					JsonObj = GeometryField->AsObject();
	//					if (JsonObj->TryGetArrayField(TEXT("paths"), PathsField)) {
	//						for (auto path : *PathsField) {
	//							PointsField = &path->AsArray();
	//							for (auto point : *PointsField) {
	//								StopCoordinates = &point->AsArray();

	//								// Create a breadcrumb for each path point and set its location from the query response
	//								FActorSpawnParameters SpawnParam = FActorSpawnParameters();
	//								SpawnParam.Owner = this;
	//								BC = GetWorld()->SpawnActor<ABreadcrumb>(ABreadcrumb::StaticClass(), FVector3d(0.), FRotator3d(0.), SpawnParam);

	//								BC->ArcGISLocation->SetPosition(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(
	//									(*StopCoordinates)[0]->AsNumber(),
	//									(*StopCoordinates)[1]->AsNumber(),
	//									UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));

	//								// Make sure this actor doesn't tick before all Breadcrumb actors have ticked. 
	//								// Ensures that the transform of all Breadcrum actors have been updated before adding the route cue
	//								AddTickPrerequisiteComponent(BC->ArcGISLocation);
	//								Breadcrumbs.AddHead(BC);
	//							}
	//						}
	//					}
	//				}
	//				if (AttributesField) {
	//					JsonObj = AttributesField->AsObject();
	//					if (JsonObj->TryGetNumberField(TEXT("Total_TravelTime"), Minutes)) {

	//						// Call a function from the UI widget to set the travel time
	//						UFunction* WidgetFunction = UIWidget->FindFunction(FName("SetTravelTime"));
	//						if (WidgetFunction) {
	//							UIWidget->ProcessEvent(WidgetFunction, &Minutes);
	//						}
	//					}
	//				}
	//			}
	//			// Visualize the route in the next tick
	//			bShouldUpdateBreadcrums = true;
	//		}
	//	}
	//}
	//bIsRouting = false;
}
