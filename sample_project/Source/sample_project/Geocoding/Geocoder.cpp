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
	QueryRequest(TEXT("1600 Pennsylvania Ave NW,DC"));
}

// Called when the game starts or when spawned
void AGeocoder::BeginPlay()
{
	Super::BeginPlay();

	//QueryRequest(TEXT("1600 Pennsylvania Ave NW,DC"));
}

// Called every frame
void AGeocoder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Make a query for routing between the selected stops 
void AGeocoder::QueryRequest(FString Address)
{
	UArcGISMapComponent* MapComponent = UArcGISMapComponent::GetMapComponent(this);

	FString Url = "https://geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates";
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";
	FString Query;

	UE_LOG(LogTemp, Warning, TEXT("--------query"));
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

void AGeocoder::ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	UE_LOG(LogTemp, Warning, TEXT("-----------response"));

	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());


	UE_LOG(LogTemp, Warning, TEXT("response code %d"), Response->GetResponseCode());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, JsonObj) && Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {
		const TArray<TSharedPtr<FJsonValue>>* Candidates;
		TSharedPtr<FJsonValue> Location;
		double PointX, PointY;
		AQueryLocation* LocationActor;

		if (JsonObj->TryGetArrayField(TEXT("candidates"), Candidates)) {

			//for (auto address : *Candidates) {
			TSharedPtr<FJsonValue> address = (*Candidates)[0];

			JsonObj = address->AsObject();

			if (Location = JsonObj->TryGetField(TEXT("location"))) {
				JsonObj = Location->AsObject();
				JsonObj->TryGetNumberField("x", PointX);
				JsonObj->TryGetNumberField("y", PointY);
				UE_LOG(LogTemp, Warning, TEXT("x: %f   y:%f"), PointX, PointY);

				// Create a breadcrumb for each path point and set its location from the query response
				FActorSpawnParameters SpawnParam = FActorSpawnParameters();
				SpawnParam.Owner = this;
				LocationActor = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(), FVector3d(0.), FRotator3d(0.), SpawnParam);

				LocationActor->SetGeographicLocationXY(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(
					PointX, PointY,
					UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
			}



			//}


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
