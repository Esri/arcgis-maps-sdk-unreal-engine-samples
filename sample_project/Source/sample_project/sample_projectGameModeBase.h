// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "editor.h"
#include "Http.h"
#include "sample_projectGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SAMPLE_PROJECT_API Asample_projectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void StartPlay() override;
	TArray<FString> names;
private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	void GetFeatures(FString Response);
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

