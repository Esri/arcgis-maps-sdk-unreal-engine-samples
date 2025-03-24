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

#include "sample_project/InputManager.h"

AGeocoder::AGeocoder()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGeocoder::BeginPlay()
{
	Super::BeginPlay();

	inputManager->OnInputTrigger.AddDynamic(this, &AGeocoder::SelectLocation);

	// Create the UI and add it to the viewport
	if (UIWidgetClass != nullptr)
	{
		auto self = this;
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			UIWidget->AddToViewport();
			auto widgetFunction = UIWidget->FindFunction(FName("SetGeoCoder"));
			HideInstructions = UIWidget->FindFunction(FName("HideDirections"));
			if (widgetFunction) {
				UIWidget->ProcessEvent(widgetFunction, &self);
			}
			WidgetSetInfoFunction = UIWidget->FindFunction(FName("SetInfoString"));
		}
	}
}

void AGeocoder::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	inputManager->OnInputTrigger.RemoveDynamic(this, &AGeocoder::SelectLocation);
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

FString AGeocoder::GetAPIKey()
{
	const auto mapComponentActor = UGameplayStatics::GetActorOfClass(GetWorld(), UArcGISMapComponent::StaticClass());
	const auto mapComponent = Cast<UArcGISMapComponent>(mapComponentActor);
	auto apiKey = mapComponent ? mapComponent->GetAPIKey() : "";  

	if (apiKey.IsEmpty())
	{
		if (auto settings = GetDefault<UArcGISMapsSDKProjectSettings>())
		{
			apiKey = settings->APIKey;
		}
	}

	return apiKey;
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
	FString url = "https://geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates";
	FString apiKey = GetAPIKey();
	FString query;

	// Set up the query 
	auto request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &AGeocoder::ProcessAddressQueryResponse);
	query = FString::Printf(TEXT("%s/?f=json&token=%s&address=%s"), *url, *apiKey, *Address);
	request->SetURL(query.Replace(TEXT(" "), TEXT("%20")));
	request->SetVerb("GET");
	request->SetHeader("Content-Type", "x-www-form-urlencoded");
	request->ProcessRequest();
	bWaitingForResponse = true;
}

// Parse the response for a geocoding query
void AGeocoder::ProcessAddressQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	FString responseAddress = "";
	TSharedPtr<FJsonObject> jsonObj;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	// Check if the query was successful
	if (FJsonSerializer::Deserialize(reader, jsonObj) &&
		Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {
		const TArray<TSharedPtr<FJsonValue>>* candidates;
		TSharedPtr<FJsonValue> location;
		TSharedPtr<FJsonValue> error;
		FString message;
		double pointX, pointY;

		if (jsonObj->TryGetArrayField(TEXT("candidates"), candidates)) {
			if (candidates->Num() > 0) {
				TSharedPtr<FJsonValue> candidate = (*candidates)[0];

				jsonObj = candidate->AsObject();
				if (!jsonObj->TryGetStringField(TEXT("Address"), responseAddress)) {
					responseAddress = TEXT("Query did not return valid response");
				}
				if ((location = jsonObj->TryGetField(TEXT("location")))) {
					jsonObj = location->AsObject();
					jsonObj->TryGetNumberField("x", pointX);
					jsonObj->TryGetNumberField("y", pointY);

					// Spawn a QueryLocation actor if not already created
					if (QueryLocation == nullptr) {
						auto spawnParam = FActorSpawnParameters();
						spawnParam.Owner = this;
						QueryLocation = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(), FVector3d(0.), FRotator3d(0.), spawnParam);
					}
					// Update the QueryLocation actor with the query response and place it at high altitude
					QueryLocation->SetupAddressQuery(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
						pointX, pointY, 10000,
						UArcGISSpatialReference::CreateArcGISSpatialReference(4326)), responseAddress);
				}
			}

			// Show a notification if the query returned no results or more than one candidate
			if (candidates->Num() != 1 && WidgetSetInfoFunction)
			{
				message = candidates->Num() > 1 ?
					"The query returned multiple results. If the shown location is not the intended one, make your input more specific." :
					"The query didn't return any results. Adjust the input and, if necessary, make it more specific.";
					
				UIWidget->ProcessEvent(WidgetSetInfoFunction, &message);
			}
		}
		// If the server responded with an error, show the error message
		else if ((error = jsonObj->TryGetField(TEXT("error")))) {
			jsonObj = error->AsObject();
			if (WidgetSetInfoFunction && jsonObj->TryGetStringField(TEXT("message"), message)) {
				UIWidget->ProcessEvent(WidgetSetInfoFunction, &message);
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
	FString url = "https://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer/reverseGeocode";
	FString query;
	UArcGISPoint* point(InPoint);

	// If the geographic coordinates of the point are not in terms of lat & lon, project them 
	if (InPoint->GetSpatialReference()->GetWKID() != 4326) {
		auto projectedGeometry = UArcGISGeometryEngine::Project(InPoint,
			UArcGISSpatialReference::CreateArcGISSpatialReference(4326));
		if (projectedGeometry != nullptr)
		{
			point = static_cast<UArcGISPoint*>(projectedGeometry);
		}
	}
	// Set up the query 
	auto request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &AGeocoder::ProcessLocationQueryResponse);
	query = FString::Printf(TEXT("%s/?f=json&langCode=en&location=%f,%f"), *url, point->GetX(), point->GetY());
	request->SetURL(query.Replace(TEXT(" "), TEXT("%20")));
	request->SetVerb("GET");
	request->SetHeader("Content-Type", "x-www-form-urlencoded");
	request->ProcessRequest();
	bWaitingForResponse = true;
}

// Parse the response for a reverse geocoding query
void AGeocoder::ProcessLocationQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully) {
	FString responseAddress = "";
	FString message;
	TSharedPtr<FJsonValue> error;
	TSharedPtr<FJsonObject> jsonObj;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	// Check if the query was successful
	if (FJsonSerializer::Deserialize(reader, jsonObj) &&
		Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {

		TSharedPtr<FJsonValue> addressField;
		if ((addressField = jsonObj->TryGetField((TEXT("address"))))) {
			jsonObj = addressField->AsObject();
			if (!jsonObj->TryGetStringField(TEXT("Match_addr"), responseAddress)) {
				responseAddress = TEXT("Query did not return valid response");
			}
		} 
		// If the server responded with an error, show the error message
		else if ((error = jsonObj->TryGetField(TEXT("error")))) {
			jsonObj = error->AsObject();
			if (WidgetSetInfoFunction && jsonObj->TryGetStringField(TEXT("message"), message)) {
				UIWidget->ProcessEvent(WidgetSetInfoFunction, &message);
			}
		}
	}
	// Show the received address 
	if (QueryLocation != nullptr) {
		QueryLocation->UpdateAddressCue(responseAddress);
	}
	bWaitingForResponse = false;
}

// Identify the location clicked on and make a line trace from there to find the corresponding point on the map
void AGeocoder::SelectLocation()
{
	UE_LOG(LogTemp, Warning, TEXT("InputValue"));
	if (bWaitingForResponse) {
		return;
	}

	FHitResult traceHit;
	FVector worldLocation;
	FVector worldDirection;
	float traceLength = 100000000.f;
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	playerController->DeprojectMousePositionToWorld(worldLocation, worldDirection);

	if (GetWorld()->LineTraceSingleByChannel(traceHit,
		worldLocation, worldLocation + traceLength * worldDirection, ECC_Visibility, FCollisionQueryParams()))
	{
		if (traceHit.GetActor()->GetClass() == AArcGISMapActor::StaticClass())
		{
			// Spawn a QueryLocation actor if it doesn't already exist
			if (QueryLocation == nullptr)
			{
				auto spawnParam = FActorSpawnParameters();
				spawnParam.Owner = this;
				QueryLocation = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(),
					FVector3d(0.), FRotator(0.), spawnParam);
			}

			// Update the QueryLocation actor with the selected location
			QueryLocation->SetupLocationQuery(traceHit.ImpactPoint);
			AddTickPrerequisiteComponent(QueryLocation->ArcGISLocation);
			bShouldSendLocationQuery = true;
		}
	}
}
