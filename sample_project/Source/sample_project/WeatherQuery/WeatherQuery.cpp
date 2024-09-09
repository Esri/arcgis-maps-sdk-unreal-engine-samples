/* Copyright 2024 Esri
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


#include "WeatherQuery.h"

#include "Blueprint/UserWidget.h"


// Sets default values
AWeatherQuery::AWeatherQuery()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

void AWeatherQuery::OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	if (bConnectedSucessfully)
	{
		TSharedPtr<FJsonObject> ResponseObj;
		const auto ResponseBody = Response->GetContentAsString();
		auto Reader = TJsonReaderFactory<>::Create(ResponseBody);

		//deserialize the json data received in the http request
		if (FJsonSerializer::Deserialize(Reader, ResponseObj))
		{
			//get the array field features
			TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");
			//parse through the features in order to get individual properties associated with the features
			for (auto WeatherStat : Features)
			{
				if (!WeatherStat->IsNull())
				{
					//create a new feature object to store the data received associated with this feature iteration
					FWeatherData WeatherData;
					auto feature = WeatherStat->AsObject();
					if (feature->GetObjectField("properties")->GetStringField("COUNTRY").Contains("United States Of America"))
					{
						//outfield can be set in the scene on bp_feature
						//this loop will take each outfield set in the scene and check to see if the outfield exists
						//if it does exist, it will return the result of the outfield associated with this feature
						//if it does not exist, it will return and error message in the scene
						WeatherData.StationName = feature->GetObjectField("properties")->GetStringField("STATION_NAME");
						WeatherData.Country = feature->GetObjectField("properties")->GetStringField("COUNTRY");	
						WeatherData.SkyCondition = feature->GetObjectField("properties")->GetStringField("SKY_CONDTN");
						WeatherData.Tempurature = feature->GetObjectField("properties")->GetNumberField("TEMP");
						WeatherData.Weather = feature->GetObjectField("properties")->GetStringField("WEATHER");
						
						//this will get the geometry or coordinates of the feature
						auto coordinates = feature->GetObjectField("geometry")->GetArrayField("coordinates");
						//To avoid crashes, this checks to see if the type of feature is Point, if so it will get the geometry
						//if not, it will return an error
						//current the only type of data supported by this sample is Point Layers, but more will be added in the future.
						WeatherData.Coordinates.Longitude = coordinates[0]->AsNumber();
						WeatherData.Coordinates.Latitude = coordinates[1]->AsNumber();
						//Add the data recieved into the object and load the object into an array for use later.
						Weather.Add(WeatherData);
					}
				}
			}
		}
	}
}

//Process the request in order to get the data
void AWeatherQuery::ProcessWebRequest()
{
	Weather.Empty();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AWeatherQuery::OnResponseRecieved);
	Request->SetURL(webLink);
	Request->SetVerb("Get");
	Request->ProcessRequest();
}

void AWeatherQuery::SendCityQuery(float X, float Y)
{
	FString Url = "https://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer/reverseGeocode";
	UArcGISMapComponent* MapComponent = UArcGISMapComponent::GetMapComponent(this);
	FString APIToken = MapComponent ? MapComponent->GetAPIKey() : "";
	FString Query;

	// Set up the query 
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AWeatherQuery::ProcessCityQueryResponse);
	FString latLong = FString::SanitizeFloat(X) + "," + FString::SanitizeFloat(Y);
	Query = FString::Printf(TEXT("%s/?f=json&token=%s&location=%s"), *Url, *APIToken, *latLong);
	Request->SetURL(Query);
	Request->SetVerb("GET");
	Request->ProcessRequest();
	CityName = "";
}

void AWeatherQuery::ProcessCityQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	// Check if the query was successful
	TSharedPtr<FJsonObject> ResponseObj;
	const auto ResponseBody = Response->GetContentAsString();
	auto Reader = TJsonReaderFactory<>::Create(ResponseBody);

	//deserialize the json data received in the http request
	if (FJsonSerializer::Deserialize(Reader, ResponseObj))
	{
		auto address = ResponseObj->GetObjectField("address");
		if (address->GetStringField("City").Len() > 0)
		{
			CityName = address->GetStringField("City") + ", " + address->GetStringField("RegionAbbr");	
		}
	}
}

// Called when the game starts or when spawned
void AWeatherQuery::BeginPlay()
{
	Super::BeginPlay();
	ProcessWebRequest();

	// Create the UI and add it to the viewport
	if (UIWidgetClass != nullptr)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			UIWidget->AddToViewport();
			APlayerController* const PlayerController = UGameplayStatics::GetPlayerController(this,0);
			PlayerController->SetInputMode(FInputModeGameAndUI());
			PlayerController->SetShowMouseCursor(true);
		}
	}
}