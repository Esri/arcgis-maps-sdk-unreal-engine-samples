// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IWebSocket.h"
#include "StreamLayerQuery.generated.h"

USTRUCT(BlueprintType)
struct FPlaneProperties
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float heading;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float speed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDateTime dateTimeStamp;
};

USTRUCT(BlueprintType)
struct FPlaneGeometry
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double x;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double y;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double z;
};


USTRUCT(BlueprintType)
struct FPlaneFeature
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneProperties attributes;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneGeometry Geometry;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneGeometry predictedPoint;
};

UCLASS()
class SAMPLE_PROJECT_API AStreamLayerQuery : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStreamLayerQuery();
	void Connect();
	void TryParseAndUpdatePlane(FString data);
	void PredictLocation(double intervalMilliseconds);
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FPlaneFeature> PlaneFeatures;
	TSharedPtr<IWebSocket> WebSocket;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	//called when the game ends or when destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
