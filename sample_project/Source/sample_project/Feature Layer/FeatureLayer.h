/* Copyright 2022 Esri
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
#include "GameFramework/Actor.h"
#include "Http.h"
#include "ArcGISSamples/Public/ArcGISPawn.h"
#include "FeatureLayer.generated.h"

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FWebLink
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString link;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString requestHeaders;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> outFields;
	FString headers;
	FString outFieldHeader;
};

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FProperties
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> featureProperties;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<float> geoProperties;
};

UCLASS()
class SAMPLE_PROJECT_API AFeatureLayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFeatureLayer();
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FWebLink WebLink;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FProperties> FeatureData;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	AArcGISPawn* ArcGisPawn;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bGetAllFeatures = true;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int startValue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int lastValue;
private:
		void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:	
	UFUNCTION(BlueprintCallable)
	void ProcessWebRequest();

};
