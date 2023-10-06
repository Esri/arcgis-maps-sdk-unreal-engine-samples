// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamLayerQuery.h"
#include "Json.h"
#include "WebSocketsModule.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"

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
		auto attributes = JsonParsed->GetObjectField("attributes");
		auto coordinates = JsonParsed->GetObjectField("geometry");
		auto x = coordinates->GetNumberField("x");
		auto y = coordinates->GetNumberField("y");
		auto z = coordinates->GetNumberField("z");
		auto Name = attributes->GetStringField("ACID");
		auto heading = attributes->GetNumberField("Heading");
		auto speed = attributes->GetNumberField("GroundSpeedKnots");
		auto dateTimeStamp = FDateTime::Now();
		auto planeFeature = FPlaneFeature::Create(Name, x, y, z, heading, speed, dateTimeStamp);
		
		if(PlaneFeatures.Num() < 10)
		{
			PlaneFeatures.Add(planeFeature);
		}
	}
}

void AStreamLayerQuery::DisplayPlaneData()
{
	for (auto plane : PlaneFeatures)
	{
		FActorSpawnParameters SpawnInfo;
		auto gObj = GetWorld()->SpawnActor<APlaneController>
		(
			APlaneController::StaticClass(),
			GetActorLocation(),
			GetActorRotation(),
			SpawnInfo
		);
		gObj->featureData = plane;
		planes.Add(gObj);
		gObj->SetActorLabel(*plane.attributes.Name);
		gObj->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		plane.Geometry.x, plane.Geometry.y, plane.Geometry.z,
		UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
	}
}


// Called when the game starts or when spawned
void AStreamLayerQuery::BeginPlay()
{
	Super::BeginPlay();
	PlaneController = NewObject<APlaneController>();
	Connect();
	FTimerHandle TimerHandle;
	GetWorldTimerManager().SetTimer(TimerHandle, this, &AStreamLayerQuery::DisplayPlaneData, 1.0f, true);
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


