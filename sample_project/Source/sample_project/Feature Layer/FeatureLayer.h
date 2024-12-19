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
#include "Engine/DataTable.h"
#include "EnhancedInputSubsystems.h"
#include "FeatureLayer.generated.h"

class UInputAction;
class UInputMappingContext;

USTRUCT(BlueprintType)
struct SAMPLE_PROJECT_API FWebLink : public FTableRowBase
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
	void RefreshProperties(AActor* Feature);
	UFUNCTION(BlueprintCallable)
	void MoveCamera(AActor* Item);
	UFUNCTION(BlueprintCallable)
	void ParseData();
	void SelectFeature();
	UFUNCTION(BlueprintCallable)
	void ProcessWebRequest();
	UFUNCTION(BlueprintCallable)
	bool ErrorCheck();
	UFUNCTION(BlueprintCallable)
	void CreateLink();
	AFeatureLayer();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bButtonActive;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bCoordinatesErrorReturn;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bGetAllFeatures;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bGetAllOutfields = true;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bLinkReturnError;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bNewLink;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bGetAll;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	AArcGISPawn* ArcGISPawn;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FFeatureLayerProperties> FeatureData;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> featureItems;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int LastValue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> OutFieldsToGet;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> PropertiesToGet;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int StartValue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FWebLink WebLink;

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> resultProperties;

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);

	TArray<TSharedPtr<FJsonValue>> Features;
	UArcGISMapComponent* mapComponent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	UInputMappingContext* MappingContext;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	UInputAction* mousePress;
	FTimerHandle startDelayHandle;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	UUserWidget* UIWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);
};
