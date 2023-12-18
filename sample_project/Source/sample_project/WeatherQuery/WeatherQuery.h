// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "GameFramework/Actor.h"
#include "sample_project/Geocoding/Geocoder.h"
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
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float tempurature;
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
	AGeocoder* Geocoder;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FWeatherData> weather;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* UIWidget;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FString> CityNames;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void GetCityNames();
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	void SendCityQuery(float X, float Y);
	void ProcessCityQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	FString webLink = "https://services9.arcgis.com/RHVPKKiFTONKtxq3/ArcGIS/rest/services/NOAA_METAR_current_wind_speed_direction_v1/FeatureServer/0//query?where=COUNTRY+LIKE+%27%25United+States+of+America%27+AND+WEATHER+NOT+IN(%27No+significant+weather+present+at+this+time.%27%2C+%27Automated+observation+with+no+human+augmentation%3B+there+may+or+may+not+be+significant+weather+present+at+this+time.%27)&outFields=*&f=pgeojson";
};
