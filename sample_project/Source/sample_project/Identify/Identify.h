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

#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISAttributeValue.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/ListView.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Styling/SlateTypes.h"
#include "Identify.generated.h"

class UListView;
class UScrollBox;
class UHorizontalBox;
class UHorizontalBoxSlot;
class UCheckBox;
class UWidget;
class UTextBlock;
class UArcGIS3DObjectSceneLayer;
class UMaterial;
class UMaterialParameterCollection;
class UButton;

USTRUCT(BlueprintType)
struct FAttributeRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Identify")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "Identify")
	FString Value;
};

USTRUCT(BlueprintType)
struct FFeatureAttributeSet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FAttributeRow> Attributes;
};

UCLASS()
class AIdentify : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AIdentify();

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UUserWidget> UIWidget;
	UPROPERTY(BlueprintReadOnly)
	TArray<FAttributeRow> LastAttributes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UObject> PropertyRowClass;
	UPROPERTY(BlueprintReadOnly)
	TArray<FFeatureAttributeSet> AllFeaturesAttributes;

	UFUNCTION()
	void OnInputTriggered();
	UFUNCTION(BlueprintCallable)
	void SelectFeatureByIndex(int32 Index);
	UFUNCTION(BlueprintCallable)
	void SetupHighlightAttributesOnMap();
	int64 GetCurrentFeatureID() const;
	void ApplySelectionToMaterial();
	FString IdentifyAtMouseClick();
	UFUNCTION()
	void RefreshListViewFromAttributes();
	UFUNCTION()
	void UpdatePageTexts();
	UFUNCTION()
	void ApplyCurrentFeature();
	UFUNCTION()
	void UpdateBuildingListUI();
	virtual void Tick(float DeltaTime) override;
	UFUNCTION()
	void OnBuildingSelected(bool bIsChecked);
	UFUNCTION(BlueprintCallable)

	void ShowFeature(bool bPrevious = false);
	void SyncBuildingCheckStates();
	bool IsInvalidDate(const Esri::GameEngine::Attributes::ArcGISAttributeValue& AttributeValue);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	class AInputManager* InputManager;
	UPROPERTY(EditAnywhere)
	AArcGISMapActor* MapActor;
	UPROPERTY(EditAnywhere)
	UArcGISMapComponent* MapComponent = nullptr;
	UPROPERTY()
	UListView* PropertyListView = nullptr;
	UPROPERTY()
	UScrollBox* BuildingList = nullptr;
	UPROPERTY()
	TArray<UCheckBox*> BuildingCheckBoxes;
	UPROPERTY()
	class UTextBlock* CurrentPageText = nullptr;
	UPROPERTY()
	class UTextBlock* TotalPageText = nullptr;
	UPROPERTY(EditAnywhere)
	FString HighlightIdFieldName = TEXT("OBJECTID");
	UPROPERTY(EditAnywhere)
	UMaterialParameterCollection* BuildingSelectionCollection = nullptr;
	UPROPERTY(EditAnywhere)
	UMaterial* HighlightMaterial = nullptr;
	UPROPERTY(EditAnywhere)
	FString HighlightLayerName;
	UPROPERTY()
	UCheckBox* RadioButton;
	UPROPERTY()
	UTextBlock* BuildingLabelTemplate;
	UPROPERTY()
	UTextBlock* TotalNumText;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	FSlateFontInfo BuildingRowFontInfo;
	bool bHasBuildingRowFont = false;
	FCheckBoxStyle CachedRadioStyle;
	bool bHasRadioStyle = false;
	UWidget* BuildingInfoPanel;
	FString LastIdentifyOutput;
	float Length = 10000000.0f;
	int32 CurrentFeatureIndex = 0;
};
