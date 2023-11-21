// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "GameFramework/Actor.h"
#include "WeatherQuery.generated.h"

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FCoordinates
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float longitude;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float latitude;
};

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FWeatherData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString country;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString skyCondition;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString stationName;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString weather;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCoordinates coordinates;
};

UCLASS()
class SAMPLE_PROJECT_API AWeatherQuery : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeatherQuery();

	UFUNCTION(BlueprintCallable)
	void ProcessWebRequest();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDataUpdate;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FWeatherData> weather;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* UIWidget;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);

	FString webLink = "https://services9.arcgis.com/RHVPKKiFTONKtxq3/ArcGIS/rest/services/NOAA_METAR_current_wind_speed_direction_v1/FeatureServer/0/query?f=geojson&where=1=1&outfields=*";
};
