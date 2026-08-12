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

#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CheckBox.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
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
	virtual void Tick(float deltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	void SetColorModulationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	void SetPointCloudRenderer(EPCLRendererChoice rendererChoice);

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	bool IsPointCloudRendererAvailable(EPCLRendererChoice rendererChoice);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

private:
	TUniquePtr<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudValueFilter> ActiveClassCodeFilter;
	TUniquePtr<Esri::Unreal::ArcGISCollection<double>> ActiveClassCodeValues;
	TUniquePtr<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter>> ActiveFilterCollection;
	TUniquePtr<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnFilter> ActiveReturnsFilter;
	TUniquePtr<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType>> ActiveReturnsValues;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCL|Visualize", meta = (AllowPrivateAccess))
	bool bColorModulationEnabled = false;

	bool bDeferredZoomWhenLoaded = false;
	bool bMapInputBlockedByUI = false;
	bool bPCLUICollapsed = false;
	bool bUpdatingFilterCheckBoxes = false;
	bool bUpdatingRendererCheckBoxes = false;

	TMap<FName, ESlateVisibility> CachedPCLRootChildVisibilities;
	TMap<FName, FVector2D> CachedTabWidgetSizes;

	UPROPERTY()
	TObjectPtr<UCheckBox> ClassAllCheckBox;

	FString ClassAttributeName;

	UPROPERTY()
	TArray<TObjectPtr<UCheckBox>> ClassFilterCheckBoxes;

	TArray<int32> ClassFilterValues;

	UPROPERTY()
	TObjectPtr<UCheckBox> ClassRendererCheckBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCL|Visualize", meta = (AllowPrivateAccess))
	EPCLRendererChoice CurrentRendererChoice = EPCLRendererChoice::RGB;

	EPCLTabLayout CurrentTabLayout = EPCLTabLayout::Default;

	float DeferredPointCloudLayerRetrySeconds = 0.0f;
	FString DeferredPointCloudLayerSource;

	FString ElevationAttributeName;

	UPROPERTY()
	TObjectPtr<UCheckBox> ElevationRendererCheckBox;

	FString IntensityAttributeName;

	UPROPERTY()
	TObjectPtr<UCheckBox> IntensityRendererCheckBox;

	uint64 LayerLoadRequestId = 0;

	UPROPERTY()
	TObjectPtr<UTextBlock> LayerLoadStatusText;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> LegendPanel;

	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> LegendTextures;

	UPROPERTY()
	TObjectPtr<UButton> LoadLayerButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> LoadLayerButtonText;

	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<AArcGISMapActor> MapActor;

	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISMapComponent> MapComponent;

	UPROPERTY()
	TObjectPtr<class UArcGISPointCloudLayer> PendingPointCloudLayer;

	UPROPERTY()
	TObjectPtr<class UArcGISPointCloudLayer> PointCloudLayer;

	int32 PointCloudLayerLoadRetryCount = 0;

	UPROPERTY()
	TObjectPtr<USlider> PointSizeSlider;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointSizeValueText;

	UPROPERTY()
	TObjectPtr<USlider> PointsPerInchSlider;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointsPerInchValueText;

	UPROPERTY()
	TObjectPtr<UCheckBox> ReturnsAllCheckBox;

	FString ReturnsAttributeName;

	UPROPERTY()
	TArray<TObjectPtr<UCheckBox>> ReturnsFilterCheckBoxes;

	FString RGBAttributeName;

	UPROPERTY()
	TObjectPtr<UCheckBox> RGBRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> SourceUrlTextBox;

	UPROPERTY()
	TObjectPtr<UArcGISSpatialReference> SpatialReference;

	UPROPERTY()
	TObjectPtr<UWidget> UIInteractionPanel;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> UIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

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

	UFUNCTION()
	void TogglePCLUICollapse();

	void SetPCLUICollapsed(bool bCollapsed);
	bool IsPCLCollapseToggleUnderCursor() const;
	void ClearActiveFilters();
	void ResetFilterSelections(bool bApplyFilters);
	void SetTabLayout(EPCLTabLayout layout);
	void ApplyTabUIScale();
	void SetNamedWidgetHeightOffset(const FName& widgetName, float heightOffset);
};
