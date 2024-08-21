/* Copyright 2024 Esri
*
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "CoreMinimal.h"
#include "Http.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "sample_project/Geocoding/Geocoder.h"
#include "WeatherQuery.generated.h"

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FCoordinates
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Longitude;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Latitude;
};

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FWeatherData
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FCoordinates Coordinates;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Country;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString SkyCondition;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString StationName;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	float Tempurature;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Weather;
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
	UFUNCTION(BlueprintCallable)
	void SendCityQuery(float X, float Y);
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString CityName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FWeatherData> Weather;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* UIWidget;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	void ProcessCityQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	FString webLink = "https://services9.arcgis.com/RHVPKKiFTONKtxq3/ArcGIS/rest/services/NOAA_METAR_current_wind_speed_direction_v1/FeatureServer/0//query?where=COUNTRY+LIKE+%27%25United+States+of+America%27+AND+WEATHER+NOT+IN(%27%2CAutomated+observation+with+no+human+augmentation%3B+there+may+or+may+not+be+significant+weather+present+at+this+time.%27)&outFields=*&f=pgeojson&orderByFields=STATION_NAME";
};
