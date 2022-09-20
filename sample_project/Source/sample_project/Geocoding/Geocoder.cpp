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

#include "Geocoder.h"

AGeocoder::AGeocoder()
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UClass> WidgetAsset(TEXT("WidgetBlueprint'/Game/SampleViewer/Samples/Geocoding/GeocodingUI.GeocodingUI_C'"));
	if (WidgetAsset.Succeeded()) {
		UIWidgetClass = WidgetAsset.Object;
	}
}

void AGeocoder::BeginPlay()
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
	if (UIWidgetClass != nullptr)
	{
		AActor* self = this;
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			UIWidget->AddToViewport();
			UFunction* WidgetFunction = UIWidget->FindFunction(FName("SetGeoCoder"));
			if (WidgetFunction) {
				UIWidget->ProcessEvent(WidgetFunction, &self);
			}
			WidgetSetInfoFunction = UIWidget->FindFunction(FName("SetInfoString"));
		}
	}
}

void AGeocoder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If a location has been selected, send a reverse geocoding query 
	if (bShouldSendLocationQuery) {
		SendLocationQuery(QueryLocation->ArcGISLocation->GetPosition());
		RemoveTickPrerequisiteComponent(QueryLocation->ArcGISLocation);
		bShouldSendLocationQuery = false;
	}
}

// Bind the handler for selecting a location on the map
void AGeocoder::SetupInput()
{
	if (!InputComponent) {
		InputComponent = NewObject<UInputComponent>(this);
	}

	InputComponent->RegisterComponent();
	InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &AGeocoder::SelectLocation);
	EnableInput(GetWorld()->GetFirstPlayerController());
}

// Make a geocoding query for an address
void AGeocoder::SendAddressQuery(FString Address)
{
	// Skip if another query is in progress
	if (bWaitingForResponse) {
		return;
	}
	if (WidgetSetInfoFunction) {
		FString temp = "";
		UIWidget->ProcessEvent(WidgetSetInfoFunction, &temp);
	}
	FString Url = "https://geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates";
	UArcGISMapComponent* MapComponent = UArcGISMapComponent::GetMapComponent(this);
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";
	FString Query;

	// Set up the query 
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGeocoder::ProcessAddressQueryResponse);
	Query = FString::Printf(TEXT("%s/?f=json&token=%s&address=%s"), *Url, *APIToken, *Address);
	Request->SetURL(Query.Replace(TEXT(" "), TEXT("%20")));
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "x-www-form-urlencoded");
	Request->ProcessRequest();
	bWaitingForResponse = true;
}

// Parse the response for a geocoding query
void AGeocoder::ProcessAddressQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	FString ResponseAddress = "";
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	// Check if the query was successful
	if (FJsonSerializer::Deserialize(Reader, JsonObj) &&
		Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {
		const TArray<TSharedPtr<FJsonValue>>* Candidates;
		TSharedPtr<FJsonValue> Location;
		TSharedPtr<FJsonValue> Error;
		FString Message;
		double PointX, PointY;

		if (JsonObj->TryGetArrayField(TEXT("candidates"), Candidates)) {
			if (Candidates->Num() > 0) {
				TSharedPtr<FJsonValue> candidate = (*Candidates)[0];

				JsonObj = candidate->AsObject();
				if (!JsonObj->TryGetStringField(TEXT("Address"), ResponseAddress)) {
					ResponseAddress = TEXT("Query did not return valid response");
				}
				if ((Location = JsonObj->TryGetField(TEXT("location")))) {
					JsonObj = Location->AsObject();
					JsonObj->TryGetNumberField("x", PointX);
					JsonObj->TryGetNumberField("y", PointY);

					// Spawn a QueryLocation actor if not already created
					if (QueryLocation == nullptr) {
						FActorSpawnParameters SpawnParam = FActorSpawnParameters();
						SpawnParam.Owner = this;
						QueryLocation = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(), FVector3d(0.), FRotator3d(0.), SpawnParam);
					}
					// Update the QueryLocation actor with the query response and place it at high altitude
					QueryLocation->SetupAddressQuery(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
						PointX, PointY, 10000,
						UArcGISSpatialReference::CreateArcGISSpatialReference(4326)), ResponseAddress);
					
					// If there are more than 1 candidate, show a notification
					Message = FString::Printf(
						TEXT("The query returned multiple results. If the shown location is not the intended one, make your input more specific."));
					if (WidgetSetInfoFunction && Candidates->Num() > 1) {
						UIWidget->ProcessEvent(WidgetSetInfoFunction, &Message);
					}
				}
			}
		}
		// If the server responded with an error, show the error message
		else if ((Error = JsonObj->TryGetField(TEXT("error")))) {
			JsonObj = Error->AsObject();
			if (WidgetSetInfoFunction && JsonObj->TryGetStringField(TEXT("message"), Message)) {
				UIWidget->ProcessEvent(WidgetSetInfoFunction, &Message);
			}
		}
	}
	bWaitingForResponse = false;
}

// Make a reverse geocoding query for a location
void AGeocoder::SendLocationQuery(UArcGISPoint* InPoint)
{
	// Skip if another query is in progress
	if (bWaitingForResponse) {
		return;
	}
	if (WidgetSetInfoFunction) {
		FString temp = "";
		UIWidget->ProcessEvent(WidgetSetInfoFunction, &temp);
	}
	FString Url = "https://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer/reverseGeocode";
	UArcGISMapComponent* MapComponent = UArcGISMapComponent::GetMapComponent(this);
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";
	FString Query;
	UArcGISPoint* Point(InPoint);

	// If the geographic coordinates of the point are not in terms of lat & lon, project them 
	if (InPoint->GetSpatialReference()->GetWKID() != 4326) {
		auto ProjectedGeometry = UArcGISGeometryEngine::Project(InPoint,
			UArcGISSpatialReference::CreateArcGISSpatialReference(4326));
		if (ProjectedGeometry != nullptr)
		{
			Point = static_cast<UArcGISPoint*>(ProjectedGeometry);
		}
	}
	// Set up the query 
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGeocoder::ProcessLocationQueryResponse);
	Query = FString::Printf(TEXT("%s/?f=json&location=%f,%f"), *Url, Point->GetX(), Point->GetY());
	Request->SetURL(Query.Replace(TEXT(" "), TEXT("%20")));
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "x-www-form-urlencoded");
	Request->ProcessRequest();
	bWaitingForResponse = true;
}

// Parse the response for a reverse geocoding query
void AGeocoder::ProcessLocationQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully) {
	FString ResponseAddress = "";
	FString Message;
	TSharedPtr<FJsonValue> Error;
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	// Check if the query was successful
	if (FJsonSerializer::Deserialize(Reader, JsonObj) &&
		Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {

		TSharedPtr<FJsonValue> AddressField;
		if ((AddressField = JsonObj->TryGetField((TEXT("address"))))) {
			JsonObj = AddressField->AsObject();
			if (!JsonObj->TryGetStringField(TEXT("Match_addr"), ResponseAddress)) {
				ResponseAddress = TEXT("Query did not return valid response");
			}
		} 
		// If the server responded with an error, show the error message
		else if ((Error = JsonObj->TryGetField(TEXT("error")))) {
			JsonObj = Error->AsObject();
			if (WidgetSetInfoFunction && JsonObj->TryGetStringField(TEXT("message"), Message)) {
				UIWidget->ProcessEvent(WidgetSetInfoFunction, &Message);
			}
		}
	}
	// Show the receivedd address 
	if (QueryLocation != nullptr) {
		QueryLocation->UpdateAddressCue(ResponseAddress);
	}
	bWaitingForResponse = false;
}

// Identify the location clicked on and make a line trace from there to find the corresponding point on the map
void AGeocoder::SelectLocation()
{
	if (bWaitingForResponse) {
		return;
	}

	FHitResult TraceHit;
	FVector WorldLocation;
	FVector WorldDirection;
	float TraceLength = 100000000.f;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	if (GetWorld()->LineTraceSingleByChannel(TraceHit,
		WorldLocation, WorldLocation + TraceLength * WorldDirection, ECC_Visibility, FCollisionQueryParams()))
	{
		if (TraceHit.GetActor()->GetClass() == AArcGISMapActor::StaticClass())
		{
			// Spawn a QueryLocation actor if it doesn't already exist
			if (QueryLocation == nullptr)
			{
				FActorSpawnParameters SpawnParam = FActorSpawnParameters();
				SpawnParam.Owner = this;
				QueryLocation = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(),
					FVector3d(0.), FRotator(0.), SpawnParam);
			}

			// Update the QueryLocation actor with the selected location
			QueryLocation->SetupLocationQuery(TraceHit.ImpactPoint);
			AddTickPrerequisiteComponent(QueryLocation->ArcGISLocation);
			bShouldSendLocationQuery = true;
		}
	}
}
