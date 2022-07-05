// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "editor.h"
#include "Http.h"
#include "FeatureLayer.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AFeatureLayer : public AActor
{
	GENERATED_BODY()
public:
	TArray<FString> names;
private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	
public:	
	// Sets default values for this actor's properties
	AFeatureLayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
class BaseballProperties
{
public:
	FString LEAGUE;
	FString TEAM;
	FString NAME;
};
class Geometry
{
public:
	TArray<double> coordinates;
};
class Feature
{
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		Geometry geometry;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		BaseballProperties properties;
};
class FeatureCollection
{
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<Feature> features;
};
