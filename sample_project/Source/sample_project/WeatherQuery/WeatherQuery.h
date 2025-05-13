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
#include "sample_project/Feature Layer/ArcGISFeatureLayerQuery.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Tempurature;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Weather;
};

UCLASS()
class SAMPLE_PROJECT_API AWeatherQuery : public AArcGISFeatureLayerQuery
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeatherQuery();
	virtual void ProcessWebRequest() override;	
	
	UFUNCTION(BlueprintCallable)
	void SendCityQuery(float X, float Y);
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FString CityName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FWeatherData> Weather;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UUserWidget* UIWidget;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully) override;
	void ProcessCityQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
};
