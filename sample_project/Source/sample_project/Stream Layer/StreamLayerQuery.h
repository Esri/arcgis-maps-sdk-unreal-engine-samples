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
	AStreamLayerQuery();
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FPlaneFeature> PlaneFeatures;

private:
	void Connect();
	void TryParseAndUpdatePlane(FString data);
	void DisplayPlaneData();

	TArray<AActor*> planes;
	float timeToLive = 3.0f;
	TSharedPtr<IWebSocket> WebSocket;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
