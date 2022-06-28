 // Copyright Epic Games, Inc. All Rights Reserved.

#include "Json.h"
#include "sample_projectGameModeBase.h"

void Asample_projectGameModeBase::StartPlay()
{
	Super::StartPlay();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &Asample_projectGameModeBase::OnResponseRecieved);
	Request->SetURL("https://services.arcgis.com/P3ePLMYs2RVChkJx/ArcGIS/rest/services/Major_League_Baseball_Stadiums/FeatureServer/0/query?f=pjson&where=1=1&outfields=TEAM,NAME,LEAGUE");
	Request->SetVerb("Get");
	Request->ProcessRequest();

}

void Asample_projectGameModeBase::OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	const FString ResponseBody = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	Feature features;
	FeatureCollection collection;
	BaseballProperties PropertyData;
	Geometry geometry;
	if (FJsonSerializer::Deserialize(Reader, ResponseObj)) 
	{
		TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");
		for (int i = 0; i != Features.Num(); i++)
		{
			TSharedPtr<FJsonObject> test = Features[i]->AsObject();
			TSharedPtr<FJsonObject> properties = test->GetObjectField("attributes");
			TArray <TSharedPtr<FJsonObject>> attributes;
			attributes.Add(properties);
			for (int j = 0; j != attributes.Num(); j++) 
			{
				TSharedPtr<FJsonObject> Name = attributes[j];
				PropertyData.NAME = Name->GetStringField("NAME");
				PropertyData.LEAGUE = Name->GetStringField("LEAGUE");
				PropertyData.TEAM = Name->GetStringField("TEAM");
			}
		}
		for (int i = 0; i != Features.Num(); i++)
		{
			TSharedPtr<FJsonObject> feature = Features[i]->AsObject();
			TSharedPtr<FJsonObject> Geometry = feature->GetObjectField("geometry");
			TArray <TSharedPtr<FJsonObject>> GeoAttributes;
			GeoAttributes.Add(Geometry);
			for (int j = 0; j != GeoAttributes.Num(); j++) 
			{
				TSharedPtr<FJsonObject> longitude = GeoAttributes[j];
				TSharedPtr<FJsonObject> latitude = GeoAttributes[j];
				double xCoordinate = longitude->GetIntegerField("x");
				double yCoordinate = latitude->GetIntegerField("y");
				geometry.coordinates.Add(xCoordinate);
				geometry.coordinates.Add(yCoordinate);
			}
		}
		if (PropertyData.NAME.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Empty %s"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("You have features :) %s"));
			features.properties = PropertyData;
			features.geometry = geometry;
			collection.features.Add(features);
		}
	}
	for (int i = 0; i != collection.features.Num(); i++) 
	{

	}
}
void Asample_projectGameModeBase::GetFeatures(FString Response)
{

}
