// Fill out your copyright notice in the Description page of Project Settings.


#include "FeatureLayer.h"
#include "Json.h"

// Sets default values
AFeatureLayer::AFeatureLayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AFeatureLayer::OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	const FString ResponseBody = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	if (FJsonSerializer::Deserialize(Reader, ResponseObj))
	{
		TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");
		for (int i = 0; i != Features.Num(); i++)
		{
			TArray<double> geoCoordinates = {};
			TSharedPtr<FJsonObject> feature = Features[i]->AsObject();
			TSharedPtr<FJsonObject> properties = feature->GetObjectField("properties");
			TSharedPtr<FJsonObject> Geometry = feature->GetObjectField("geometry");
			TArray<TSharedPtr<FJsonValue>> coordinates = Geometry->GetArrayField("coordinates");
			data->NAME.Add(properties->GetStringField("NAME"));
			data->LEAGUE.Add(properties->GetStringField("LEAGUE"));
			data->TEAM.Add(properties->GetStringField("TEAM"));
			data->longitude.Add(coordinates[0]->AsNumber());
			data->latitude.Add(coordinates[1]->AsNumber());
		}
	}
}

// Called when the game starts or when spawned
void AFeatureLayer::BeginPlay()
{
	Super::BeginPlay();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AFeatureLayer::OnResponseRecieved);
	Request->SetURL("https://services.arcgis.com/P3ePLMYs2RVChkJx/ArcGIS/rest/services/Major_League_Baseball_Stadiums/FeatureServer/0/query?f=geojson&where=1=1&outfields=TEAM,NAME,LEAGUE");
	Request->SetVerb("Get");
	Request->ProcessRequest();
}
