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
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Not Connected");
	});
	
	WebSocket->OnMessage().AddLambda([](const FString & Message) -> void {
		// This code will run when we receive a string message from the server.
	});
	
	WebSocket->Connect();
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

