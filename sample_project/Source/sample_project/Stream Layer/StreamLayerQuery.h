// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IWebSocket.h"
#include "PlaneController.h"
#include "StreamLayerQuery.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AStreamLayerQuery : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStreamLayerQuery();
	void Connect();
	void TryParseAndUpdatePlane(FString data);
	void DisplayPlaneData();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FPlaneFeature> PlaneFeatures;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TSubclassOf<AActor> PlaneBlueprint;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> planes;
	TSharedPtr<IWebSocket> WebSocket;
	APlaneController* PlaneController;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//called when the game ends or when destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
};
