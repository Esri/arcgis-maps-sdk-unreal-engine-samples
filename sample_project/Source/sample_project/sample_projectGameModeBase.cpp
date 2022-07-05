 // Copyright Epic Games, Inc. All Rights Reserved.

#include "Json.h"
#include "sample_projectGameModeBase.h"

void Asample_projectGameModeBase::StartPlay()
{
	Super::StartPlay();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &Asample_projectGameModeBase::OnResponseRecieved);
	Request->SetURL("https://services.arcgis.com/P3ePLMYs2RVChkJx/ArcGIS/rest/services/Major_League_Baseball_Stadiums/FeatureServer/0/query?f=geojson&where=1=1&outfields=TEAM,NAME,LEAGUE");
	Request->SetVerb("Get");
	Request->ProcessRequest();

}

void Asample_projectGameModeBase::OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	TSharedPtr<FJsonObject> ResponseObj;
	const FString ResponseBody = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	if (FJsonSerializer::Deserialize(Reader, ResponseObj))
	{
		UE_LOG(LogTemp, Display, TEXT("Name: %s"), *Response->GetContentAsString());
		TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");
		UE_LOG(LogTemp, Display, TEXT("Number of Features: %d"), Features.Num());
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
			//UE_LOG(LogTemp, Display, TEXT("Name: %s, Team: %s, League: %s, X Value: %f, Y Value %f"), *data->NAME, *data->TEAM, *data->LEAGUE, data->longitude, data->latitude);
			UE_LOG(LogTemp, Display, TEXT("Name: %s, Team: %s, League: %s, X Value: %f, Y Value %f"), *data->NAME[i], *data->TEAM[i], *data->LEAGUE[i], data->longitude[i], data->latitude[i]);
		}
		UE_LOG(LogTemp, Display, TEXT("Name: %s, Team: %s, League: %s, X Value: %f, Y Value %f"), *data->NAME[5], *data->TEAM[5], *data->LEAGUE[5], data->longitude[5], data->latitude[5]);
	}
}
//UBaseballProperties& UBaseballProperties::operator=(const UBaseballProperties& BaseballProperties) 
//{
//	UBaseballProperties baseballProperties;
//	baseballProperties.NAME = BaseballProperties.NAME;
//	baseballProperties.LEAGUE = BaseballProperties.LEAGUE;
//	baseballProperties.TEAM = BaseballProperties.TEAM;
//	return baseballProperties;
//}