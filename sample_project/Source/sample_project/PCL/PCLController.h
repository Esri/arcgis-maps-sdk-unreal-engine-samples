/* Copyright 2026 Esri
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
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Templates/UniquePtr.h"

#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilter.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"

#include "PCLController.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class EPCLRendererChoice : uint8
{
	RGB UMETA(DisplayName = "RGB"),
	Class UMETA(DisplayName = "Class"),
	Elevation UMETA(DisplayName = "Elevation"),
	Intensity UMETA(DisplayName = "Intensity")
};

UENUM()
enum class EPCLTabLayout : uint8
{
	Default,
	Visualize,
	Filter
};

UCLASS()
class SAMPLE_PROJECT_API APCLController : public AActor
{
	GENERATED_BODY()

public:

	APCLController();

protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

public:

	virtual void Tick(float deltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	void SetColorModulationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	void SetPointCloudRenderer(EPCLRendererChoice rendererChoice);

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	bool IsPointCloudRendererAvailable(EPCLRendererChoice rendererChoice);

private:
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> UIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY()
	TObjectPtr<USlider> PointSizeSlider;

	UPROPERTY()
	TObjectPtr<USlider> PointsPerInchSlider;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointSizeValueText;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointsPerInchValueText;

	UPROPERTY()
	TObjectPtr<UCheckBox> RGBRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UCheckBox> ClassRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UCheckBox> ElevationRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UCheckBox> IntensityRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> LegendPanel;

	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> LegendTextures;

	UPROPERTY()
	TObjectPtr<UCheckBox> ClassAllCheckBox;

	UPROPERTY()
	TArray<TObjectPtr<UCheckBox>> ClassFilterCheckBoxes;

	UPROPERTY()
	TObjectPtr<UCheckBox> ReturnsAllCheckBox;

	UPROPERTY()
	TArray<TObjectPtr<UCheckBox>> ReturnsFilterCheckBoxes;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> SourceUrlTextBox;

	UPROPERTY()
	TObjectPtr<UButton> LoadLayerButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> LayerLoadStatusText;

	UPROPERTY()
	TObjectPtr<UTextBlock> LoadLayerButtonText;

	UPROPERTY()
	TObjectPtr<UWidget> UIInteractionPanel;

	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<AArcGISMapActor> MapActor;

	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISMapComponent> MapComponent;

	UPROPERTY()
	TObjectPtr<class UArcGISPointCloudLayer> PointCloudLayer;

	UPROPERTY()
	TObjectPtr<class UArcGISPointCloudLayer> PendingPointCloudLayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCL|Visualize", meta = (AllowPrivateAccess))
	EPCLRendererChoice CurrentRendererChoice = EPCLRendererChoice::RGB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCL|Visualize", meta = (AllowPrivateAccess))
	bool bColorModulationEnabled = false;

	EPCLTabLayout CurrentTabLayout = EPCLTabLayout::Default;

	bool bUpdatingRendererCheckBoxes = false;

	FString RGBAttributeName;
	FString ClassAttributeName;
	FString ElevationAttributeName;
	FString IntensityAttributeName;
	FString ReturnsAttributeName;

	TMap<FName, FVector2D> CachedTabWidgetSizes;
	TArray<int32> ClassFilterValues;

	TUniquePtr<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter>> ActiveFilterCollection;
	TUniquePtr<Esri::Unreal::ArcGISCollection<double>> ActiveClassCodeValues;
	TUniquePtr<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType>> ActiveReturnsValues;
	TUniquePtr<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudValueFilter> ActiveClassCodeFilter;
	TUniquePtr<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnFilter> ActiveReturnsFilter;

	bool bUpdatingFilterCheckBoxes = false;
	bool bMapInputBlockedByUI = false;
	bool bPCLUICollapsed = false;
	uint64 LayerLoadRequestId = 0;
	FString DeferredPointCloudLayerSource;
	float DeferredPointCloudLayerRetrySeconds = 0.0f;
	int32 PointCloudLayerLoadRetryCount = 0;
	bool bDeferredZoomWhenLoaded = false;
	TMap<FName, ESlateVisibility> CachedPCLRootChildVisibilities;

	UPROPERTY()
	TObjectPtr<UArcGISSpatialReference> SpatialReference;

	UFUNCTION()
	void OnPointSizeChanged(float value);

	UFUNCTION()
	void OnPointsPerInchChanged(float value);

	UFUNCTION()
	void OnColorModulationCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnRGBRendererCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnClassRendererCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnElevationRendererCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnIntensityRendererCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnCustomizeTabClicked();

	UFUNCTION()
	void OnFilterTabClicked();

	UFUNCTION()
	void OnVisualizeTabClicked();

	UFUNCTION()
	void OnFilterCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnClassAllFilterCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnReturnsAllFilterCheckStateChanged(bool bIsChecked);

	UFUNCTION()
	void OnResetFiltersClicked();

	UFUNCTION()
	void OnLoadPointCloudLayerClicked();

	UFUNCTION()
	void OnCollapseButtonClicked();

	UFUNCTION()
	void OnInfoButtonClicked();

	void SetAllFilterOptionsChecked(const TArray<TObjectPtr<UCheckBox>>& filterCheckBoxes, bool bIsChecked);
	void InitializePCLUI();
	void ShutdownPCLUI();
	void UpdatePCLUI();
	void CreatePointCloudLayer(const FString& source, bool bZoomWhenLoaded);
	void BuildDataLoaderUI();
	void DeferPointCloudLayerLoad(const FString& source, bool bZoomWhenLoaded);
	void SetLayerLoadStatus(bool bSucceeded) const;
	void UpdateMapInputForUIHover();
	void SyncLegendVisibilityWithMainPanel() const;
	void SetMapInputBlockedByUI(bool bBlocked);
	void ApplyPointCloudVisualization();
	void ApplyPointCloudFilters();
	void RefreshAvailablePointCloudAttributes();
	bool IsRendererAvailableFromCachedAttributes(EPCLRendererChoice rendererChoice) const;
	EPCLRendererChoice GetFallbackRendererChoice() const;
	void EnsureAvailableRendererSelected();
	void HandleRendererCheckStateChanged(bool bIsChecked, EPCLRendererChoice rendererChoice);
	void UpdateRendererCheckBoxes();
	void UpdateColorModulationVisibility();
	void SetRendererOptionVisibility(EPCLRendererChoice rendererChoice, bool bVisible);
	void UpdateSliderValueTexts() const;
	void BuildFilterTabUI();
	void BuildLegendUI();
	void ConfigurePCLCollapseInitialState();
	void TogglePCLUICollapse();
	void SetPCLUICollapsed(bool bCollapsed);
	bool IsPCLCollapseToggleUnderCursor() const;
	void ClearActiveFilters();
	void ResetFilterSelections(bool bApplyFilters);
	void SetTabLayout(EPCLTabLayout layout);
	void ApplyTabUIScale();
	void SetNamedWidgetHeightOffset(const FName& widgetName, float heightOffset);
};
