/* Copyright 2025 Esri
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
#include "ArcGISFeatureLayerQuery.generated.h"

class AFeatureItemBase;
class AFeatureItem;
class UArcGISMapComponent;

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FLink : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Link;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> RequestHeaders;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FString> OutFields;
	FString Headers;
	FString OutFieldHeader;
};

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FProperties
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> FeatureProperties;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<float> GeoProperties;
};

UCLASS()
class SAMPLE_PROJECT_API AArcGISFeatureLayerQuery : public AActor
{
	GENERATED_BODY()

public:
	AArcGISFeatureLayerQuery();

	UFUNCTION(BlueprintCallable)
	virtual void CreateLink();
	virtual void GetMapComponent();
	UFUNCTION(BlueprintCallable)
	void ParseData(bool GetAllFeatures, int StartValue, int LastValue);
	UFUNCTION(BlueprintCallable)
	virtual void ProcessWebRequest();
	virtual void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void SpawnFeatures(int Start, int Last);
	
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FProperties> FeatureData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	const TSubclassOf<AFeatureItemBase> FeatureItem;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> FeatureItems;
	TArray<TSharedPtr<FJsonValue>> Features;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	UArcGISMapComponent* MapComponent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLink WebLink;
};
