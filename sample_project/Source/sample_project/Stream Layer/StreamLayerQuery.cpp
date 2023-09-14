// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamLayerQuery.h"
#include "Json.h"
#include "WebSocketsModule.h"

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
		const auto Data = Message;
		TryParseAndUpdatePlane(Data);
	});
	
	WebSocket->Connect();
}

void AStreamLayerQuery::TryParseAndUpdatePlane(FString data)
{
	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(data);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		FPlaneFeature feature;
		auto attributes = JsonParsed->GetObjectField("attributes");
		auto coordinates = JsonParsed->GetObjectField("geometry");
		feature.Geometry.x = coordinates->GetNumberField("x");
		feature.Geometry.y = coordinates->GetNumberField("y");
		feature.Geometry.z = coordinates->GetNumberField("z");
		feature.attributes.Name = attributes->GetStringField("ACID");
		feature.attributes.heading = attributes->GetNumberField("Heading");
		feature.attributes.speed = attributes->GetNumberField("GroundSpeedKnots");
		
		//DateTimeStamp
		if(PlaneFeatures.Num() < 10)
		{
			PlaneFeatures.Add(feature);
		}
	}
}

void AStreamLayerQuery::PredictLocation(double intervalMilliseconds)
{
	FPlaneFeature feature;
	auto cGroundSpeedKnots = feature.attributes.speed;
	auto metersPerSec = cGroundSpeedKnots * 0.51444444444;
	auto simulationSpeedFactor = 1.5;
	auto timespanSec = (intervalMilliseconds / 1000.0) * simulationSpeedFactor;
	TArray<double> currentPoint = { feature.predictedPoint.x, feature.predictedPoint.y, feature.predictedPoint.z };
	auto headingDegrees = feature.attributes.heading;
	auto drPoint = DeadReckoning.DeadReckoningPoint(metersPerSec, timespanSec, currentPoint, headingDegrees);
	feature.predictedPoint.x = drPoint[0];
	feature.predictedPoint.y = drPoint[1];
	feature.predictedPoint.z = currentPoint[2];
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

}

void AStreamLayerQuery::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WebSocket->Close();
	PlaneFeatures.Empty();
}


