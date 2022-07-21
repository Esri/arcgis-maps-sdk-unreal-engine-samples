// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "FeatureLayer.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AFeatureLayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFeatureLayer();
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UFeature* data = NewObject<UFeature>();
private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame

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
