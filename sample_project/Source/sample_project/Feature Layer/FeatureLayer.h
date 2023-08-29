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
	FString Link;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> RequestHeaders;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> OutFields;
	FString Headers;
	FString OutFieldHeader;
};

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FFeatureLayerProperties
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> FeatureProperties;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<float> GeoProperties;
};

UCLASS()
class SAMPLE_PROJECT_API AFeatureLayer : public AActor
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void ProcessWebRequest();
	UFUNCTION(BlueprintCallable)
	bool ErrorCheck();
	UFUNCTION(BlueprintCallable)
	void CreateLink();
	AFeatureLayer();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bButtonActive;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool bCoordinatesErrorReturn;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bGetAllFeatures = true;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bLinkReturnError;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	AArcGISPawn* ArcGISPawn;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FFeatureLayerProperties> FeatureData;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int LastValue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int StartValue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FWebLink WebLink;
	
private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	
protected:
	virtual void BeginPlay() override;
	
};
