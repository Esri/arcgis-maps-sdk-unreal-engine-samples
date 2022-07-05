// Fill out your copyright notice in the Description page of Project Settings.

#include "Json.h"
#include "FeatureLayer.h"

void AFeatureLayer::OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
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
			TSharedPtr<FJsonObject> properties = test->GetObjectField("properties");
			TArray <TSharedPtr<FJsonObject>> attributes;
			attributes.Add(properties);
			for (int j = 0; j != attributes.Num(); j++)
			{
				TSharedPtr<FJsonObject> Name = attributes[j];
				PropertyData.NAME = Name->GetStringField("NAME");
				PropertyData.LEAGUE = Name->GetStringField("LEAGUE");
				PropertyData.TEAM = Name->GetStringField("TEAM");
				features.properties = PropertyData;
			}
		}
		//for (int i = 0; i != Features.Num(); i++)
		//{
		//	TSharedPtr<FJsonObject> feature = Features[i]->AsObject();
		//	TSharedPtr<FJsonObject> Geometry = feature->GetObjectField("geometry");
		//	TArray <TSharedPtr<FJsonObject>> GeoAttributes;
		//	GeoAttributes.Add(Geometry);
		//	for (int j = 0; j != GeoAttributes.Num(); j++)
		//	{
		//		TSharedPtr<FJsonObject> longitude = GeoAttributes[j];
		//		TSharedPtr<FJsonObject> latitude = GeoAttributes[j];
		//		double xCoordinate = longitude->GetIntegerField("x");
		//		double yCoordinate = latitude->GetIntegerField("y");
		//		geometry.coordinates.Add(xCoordinate);
		//		geometry.coordinates.Add(yCoordinate);
		//	}
		//}
		if (PropertyData.NAME.IsEmpty())
		{
			UE_LOG(LogTemp, Display, TEXT("Empty %s"));
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("You have features :)"));
			features.properties = PropertyData;
			//features.geometry = geometry;
			collection.features.Add(features);
		}
	}
	for (int i = 0; i != collection.features.Num(); i++)
	{

	}
}

// Sets default values
AFeatureLayer::AFeatureLayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

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

// Called every frame
void AFeatureLayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

