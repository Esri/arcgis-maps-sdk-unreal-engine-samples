// Fill out your copyright notice in the Description page of Project Settings.


#include "StreamLayerQuery.h"
#include "IWebSocket.h"
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
	const FString ServerURL = TEXT("wss://geoeventsample1.esri.com:6143/arcgis/ws/services/FAAStream/StreamServer/subscribe");
	const FString ServerProtocol = TEXT("wss");
	TSharedPtr<IWebSocket> Socket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, ServerProtocol);
	
	Socket->OnConnected().AddLambda([]() -> void {
		// This code will run once connected.
	});
    
	Socket->OnConnectionError().AddLambda([](const FString & Error) -> void {
		// This code will run if the connection failed. Check Error to see what happened.
	});
	
	Socket->OnMessage().AddLambda([](const FString & Message) -> void {
		// This code will run when we receive a string message from the server.
	});
	
	Socket->Connect();
	if(Socket->IsConnected())
	{
		UE_LOG(LogTemp, Warning, TEXT("connected"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("not connected"));
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

}

