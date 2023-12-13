// Fill out your copyright notice in the Description page of Project Settings.


#include "WeatherQuery.h"

#include "Blueprint/UserWidget.h"

// Sets default values
AWeatherQuery::AWeatherQuery()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UClass> WidgetAsset(TEXT("WidgetBlueprint'/Game/SampleViewer/Samples/RealTime_Weather/UserInterface/Weather_WBP.Weather_WBP_C'"));
	if (WidgetAsset.Succeeded()) {
		UIWidgetClass = WidgetAsset.Object;
	}
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
						WeatherData.stationName = feature->GetObjectField("properties")->GetStringField("STATION_NAME");
						WeatherData.country = feature->GetObjectField("properties")->GetStringField("COUNTRY");	
						WeatherData.skyCondition = feature->GetObjectField("properties")->GetStringField("SKY_CONDTN");
						WeatherData.tempurature = feature->GetObjectField("properties")->GetNumberField("TEMP");
						WeatherData.weather = feature->GetObjectField("properties")->GetStringField("WEATHER");
						
						//this will get the geometry or coordinates of the feature
						auto coordinates = feature->GetObjectField("geometry")->GetArrayField("coordinates");
						//To avoid crashes, this checks to see if the type of feature is Point, if so it will get the geometry
						//if not, it will return an error
						//current the only type of data supported by this sample is Point Layers, but more will be added in the future.
						WeatherData.coordinates.longitude = coordinates[0]->AsNumber();
						WeatherData.coordinates.latitude = coordinates[1]->AsNumber();
						//Add the data recieved into the object and load the object into an array for use later.
						weather.Add(WeatherData);
					}
				}
			}
		}
	}
}

//Process the request in order to get the data
void AWeatherQuery::ProcessWebRequest()
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AWeatherQuery::OnResponseRecieved);
	Request->SetURL(webLink);
	Request->SetVerb("Get");
	Request->ProcessRequest();
	bDataUpdate = true;
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
		CityNames.Add(address->GetStringField("City"));
	}
}

void AWeatherQuery::GetCityNames()
{
	for (auto Item : weather)
	{
		SendCityQuery(Item.coordinates.longitude, Item.coordinates.latitude);
	}
}


// Called when the game starts or when spawned
void AWeatherQuery::BeginPlay()
{
	Super::BeginPlay();
	ProcessWebRequest();

	FTimerHandle handle;
	GetWorldTimerManager().SetTimer(handle, this, &AWeatherQuery::GetCityNames, 1.5f, false);
	// Create the UI and add it to the viewport
	if (UIWidgetClass != nullptr)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			UIWidget->AddToViewport();
		}
	}
	
	FTimerHandle timerHandle;
	GetWorldTimerManager().SetTimer(timerHandle, this, &AWeatherQuery::ProcessWebRequest, 60.0f, true);
}