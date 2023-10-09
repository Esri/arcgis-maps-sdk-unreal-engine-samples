// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamLayerQuery.h"

#include <string>

#include "Json.h"
#include "WebSocketsModule.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AStreamLayerQuery::AStreamLayerQuery()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AStreamLayerQuery::Connect()
{
	WebSocket = FWebSocketsModule::Get().CreateWebSocket("wss://geoeventsample1.esri.com:6143/arcgis/ws/services/FAAStream/StreamServer/subscribe");
	
	WebSocket->OnConnected().AddLambda([]() -> void
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Successfully Connected");
	});
    
	WebSocket->OnConnectionError().AddLambda([](const FString & Error) -> void
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Not Connected" + Error);
	});
	
	WebSocket->OnMessage().AddLambda([this](const FString & Message) -> void
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, Message);
		TryParseAndUpdatePlane(Message);
	});
	
	WebSocket->Connect();
}

void AStreamLayerQuery::TryParseAndUpdatePlane(FString data)
{
	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(data);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		auto attributes = JsonParsed->GetObjectField("attributes");
		auto coordinates = JsonParsed->GetObjectField("geometry");
		auto x = coordinates->GetNumberField("x");
		auto y = coordinates->GetNumberField("y");
		auto z = coordinates->GetNumberField("z");
		auto Name = attributes->GetStringField("ACID");
		auto heading = attributes->GetNumberField("Heading");
		auto speed = attributes->GetNumberField("GroundSpeedKnots");
		long timestampMS = attributes->GetNumberField("DateTimeStamp");
		FDateTime datetimeOffset = FDateTime::FromUnixTimestamp(timestampMS);
		auto dateTimeStamp = datetimeOffset.GetDate();
		auto planeFeature = FPlaneFeature::Create(Name, x, y, z, heading, speed, dateTimeStamp);
		PlaneFeatures.Add(planeFeature);
	}
}

void AStreamLayerQuery::DisplayPlaneData()
{
	for (auto i = 0; i < PlaneFeatures.Num(); i++)
	{
		FString name = "PlaneController_" + FString::FromInt(i);
		if(auto Plane = FindObject<APlaneController>(ANY_PACKAGE, *name, true))
		{
			FTimespan timespan = FDateTime::Now() - PlaneFeatures[i].attributes.dateTimeStamp.UtcNow();

			if (timespan.GetTotalMinutes() > timeToLive)
			{
				
			}
			
			Plane->PredictPoint(GetWorld()->GetDeltaSeconds() * 1000);

			Plane->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
			PlaneFeatures[i].predictedPoint.x, PlaneFeatures[i].predictedPoint.y, PlaneFeatures[i].predictedPoint.z,
			UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			auto gObj = GetWorld()->SpawnActor<APlaneController>
			(
				APlaneController::StaticClass(),
				GetActorLocation(),
				GetActorRotation(),
				SpawnInfo
			);
			gObj->featureData = PlaneFeatures[i];
			planes.Add(gObj);
			gObj->SetActorLabel(*PlaneFeatures[i].attributes.Name);
			gObj->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
			PlaneFeatures[i].Geometry.x, PlaneFeatures[i].Geometry.y, PlaneFeatures[i].Geometry.z,
			UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
			UE_LOG(LogTemp, Warning, TEXT("name: %s"), *gObj->GetName());	
		}
	}
}


// Called when the game starts or when spawned
void AStreamLayerQuery::BeginPlay()
{
	Super::BeginPlay();
	Connect();
}

// Called every frame
void AStreamLayerQuery::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	DisplayPlaneData();
}

void AStreamLayerQuery::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WebSocket->Close();
	PlaneFeatures.Empty();
}


