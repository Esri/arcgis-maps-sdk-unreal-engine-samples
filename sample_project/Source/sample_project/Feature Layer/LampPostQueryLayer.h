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

#include "ArcGISSamples/Public/ArcGISPawn.h"
#include "ArcGISFeatureLayerQuery.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "Engine/DataTable.h"
#include "LampPostQueryLayer.generated.h"

class ALampPostItem;
class AFeatureItem;
class UInputAction;
class UInputMappingContext;

UCLASS()
class SAMPLE_PROJECT_API ALampPostQueryLayer : public AArcGISFeatureLayerQuery
{
	GENERATED_BODY()

public:
	ALampPostQueryLayer();

	UFUNCTION(BlueprintCallable)
	void CreateLink();
	UFUNCTION(BlueprintCallable)
	bool HasErrors();
	UFUNCTION(BlueprintCallable)
	void ParseData();
	UFUNCTION(BlueprintCallable)
	void ProcessWebRequest();
	void RefreshProperties(ALampPostItem* Item);
	UFUNCTION()
	void SelectFeature();
	void SpawnFeatures(int Start, int Last);

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	AArcGISPawn* ArcGISPawn;
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

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	ALampPostItem* currentFeature;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FProperties> FeatureData;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> featureItems;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int LastValue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> OutFieldsToGet;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TArray<FString> PropertiesToGet;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FString> resultProperties;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int StartValue;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FLink WebLink;

private:
	static void AddAdditionalMaterial(const ALampPostItem* Item, UMaterialInstance* Material);
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully);
	void GetMapComponent();
	static void RemoveAdditionalMaterial(const ALampPostItem* Item);

	//UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	///class AInputManager* inputManager;
	
	UFunction* clearProperties;
	UFunction* createProperties;
	TArray<TSharedPtr<FJsonValue>> Features;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	UMaterialInstance* highlightMaterial;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	UArcGISMapComponent* mapComponent;
	FTimerHandle startDelayHandle;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	UUserWidget* UIWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
