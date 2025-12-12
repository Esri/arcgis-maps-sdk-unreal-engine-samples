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
#include "Components/WidgetSwitcher.h"
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
class UWidgetSwitcher;

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

	UPROPERTY(BlueprintReadOnly)
	TArray<FFeatureAttributeSet> AllFeaturesAttributes;

	UPROPERTY(BlueprintReadOnly)
	TArray<FAttributeRow> LastAttributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UObject> PropertyRowClass;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<UUserWidget> UIWidget;

	UFUNCTION(BlueprintCallable)
	void SelectFeatureByIndex(int32 Index);

	UFUNCTION(BlueprintCallable)
	void SetupHighlightAttributesOnMap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	FSlateFontInfo BuildingRowFontInfo;
	FCheckBoxStyle CachedRadioStyle;
	int32 CurrentFeatureIndex = 0;
	int64 GetCurrentFeatureID() const;
	bool bHasBuildingRowFont = false;
	bool bHasRadioStyle = false;
	float Length = 10000000.0f;

	UPROPERTY()
	TArray<UCheckBox*> BuildingCheckBoxes;

	UPROPERTY()
	UWidget* BuildingInfoPanel;

	UPROPERTY()
	UTextBlock* BuildingLabelTemplate;

	UPROPERTY()
	UScrollBox* BuildingList = nullptr;

	UPROPERTY(EditAnywhere)
	UMaterialParameterCollection* BuildingSelectionCollection = nullptr;

	UPROPERTY()
	class UTextBlock* CurrentPageText = nullptr;

	UPROPERTY(EditAnywhere)
	FString HighlightIdFieldName = TEXT("OBJECTID");

	UPROPERTY(EditAnywhere)
	FString HighlightLayerName;

	UPROPERTY(EditAnywhere)
	UMaterial* HighlightMaterial = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	class AInputManager* InputManager;

	UPROPERTY(EditAnywhere)
	AArcGISMapActor* MapActor;

	UPROPERTY(EditAnywhere)
	UArcGISMapComponent* MapComponent = nullptr;

	UPROPERTY()
	UListView* PropertyListView = nullptr;

	UPROPERTY()
	UCheckBox* RadioButton;

	UPROPERTY()
	UButton* ResetButton;

	UPROPERTY()
	UTextBlock* TotalNumText;

	UPROPERTY()
	class UTextBlock* TotalPageText = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY()
	UWidgetSwitcher* WidgetSwitcher;

	UFUNCTION()
	void RefreshListViewFromAttributes();

	UFUNCTION()
	void UpdatePageTexts();

	UFUNCTION()
	void ApplyCurrentFeature();

	UFUNCTION()
	void UpdateBuildingListUI();

	UFUNCTION()
	void OnBuildingSelected(bool bIsChecked);

	UFUNCTION(BlueprintCallable)
	void ShowFeature(bool bPrevious = false);

	UFUNCTION()
	void OnResetClicked();

	UFUNCTION()
	void OnInputTriggered();

	void SyncBuildingCheckStates();
	bool IsInvalidDate(const Esri::GameEngine::Attributes::ArcGISAttributeValue& AttributeValue);
	void ClearSelectionAndUI();
	void ApplySelectionToMaterial();
	void IdentifyAtMouseClick();
	AIdentify();
};