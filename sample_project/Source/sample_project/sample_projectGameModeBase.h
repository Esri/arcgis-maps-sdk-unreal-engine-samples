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
UCLASS(Blueprintable)
class SAMPLE_PROJECT_API Asample_projectGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	virtual void StartPlay() override;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UFeature* data = NewObject<UFeature>();
private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
};

UCLASS(Blueprintable)
class SAMPLE_PROJECT_API UFeature : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<FString> LEAGUE;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<FString>  TEAM;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<FString>  NAME;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<double>  longitude;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<double>  latitude;
};

