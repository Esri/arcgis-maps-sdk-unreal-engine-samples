// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License. */

#pragma once

#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilter.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/EditableTextBox.h"
#include "Components/PanelWidget.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Templates/UniquePtr.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCLController.generated.h"

class AInputManager;
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
	// Sets default values for this actor's properties
	APCLController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	void SetColorModulationEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	void SetPointCloudRenderer(EPCLRendererChoice RendererChoice);

	UFUNCTION(BlueprintCallable, Category = "PCL|Visualize")
	bool IsPointCloudRendererAvailable(EPCLRendererChoice RendererChoice);

private:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	TObjectPtr<AInputManager> InputManager;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<UUserWidget> UIWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	UPROPERTY()
	TObjectPtr<UComboBoxString> UnitDropdown;

	UPROPERTY()
	TObjectPtr<USlider> PointSizeSlider;

	UPROPERTY()
	TObjectPtr<USlider> PointsPerInchSlider;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointSizeValueText;

	UPROPERTY()
	TObjectPtr<UTextBlock> PointsPerInchValueText;

	UPROPERTY()
	TObjectPtr<UCheckBox> ColorModulationCheckBox;

	UPROPERTY()
	TObjectPtr<UCheckBox> RGBRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UCheckBox> ClassRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UCheckBox> ElevationRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UCheckBox> IntensityRendererCheckBox;

	UPROPERTY()
	TObjectPtr<UButton> CustomizeTabButton;

	UPROPERTY()
	TObjectPtr<UButton> FilterTabButton;

	UPROPERTY()
	TObjectPtr<UButton> VisualizeTabButton;

	UPROPERTY()
	TObjectPtr<UCanvasPanel> LegendPanel;

	UPROPERTY()
	TArray<TObjectPtr<UTexture2D>> LegendTextures;

	UPROPERTY()
	TObjectPtr<UPanelWidget> FilterPanel;

	UPROPERTY()
	TObjectPtr<UCheckBox> ClassAllCheckBox;

	UPROPERTY()
	TArray<TObjectPtr<UCheckBox>> ClassFilterCheckBoxes;

	UPROPERTY()
	TObjectPtr<UCheckBox> ReturnsAllCheckBox;

	UPROPERTY()
	TArray<TObjectPtr<UCheckBox>> ReturnsFilterCheckBoxes;

	UPROPERTY()
	TObjectPtr<UButton> ResetFiltersButton;

	UPROPERTY()
	TObjectPtr<UEditableTextBox> SourceUrlTextBox;

	UPROPERTY()
	TObjectPtr<UButton> LoadLayerButton;

	UPROPERTY()
	TObjectPtr<UTextBlock> LayerLoadStatusText;

	UPROPERTY()
	TObjectPtr<UTextBlock> LoadLayerButtonText;

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
	uint64 LayerLoadRequestId = 0;
	FString DeferredPointCloudLayerSource;
	float DeferredPointCloudLayerRetrySeconds = 0.0f;
	int32 PointCloudLayerLoadRetryCount = 0;
	bool bDeferredZoomWhenLoaded = false;

	UPROPERTY()
	TObjectPtr<UArcGISSpatialReference> SpatialReference;

	UFUNCTION()
	void OnInputTriggered();

	UFUNCTION()
	void OnInputEnded();

	UFUNCTION()
	void OnPointSizeChanged(float Value);

	UFUNCTION()
	void OnPointsPerInchChanged(float Value);

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

	void CreatePointCloudLayer(const FString& Source, bool bZoomWhenLoaded);
	void BuildDataLoaderUI();
	void DeferPointCloudLayerLoad(const FString& Source, bool bZoomWhenLoaded);
	void SetLayerLoadStatus(bool bSucceeded) const;
	void ApplyPointCloudVisualization();
	bool UpdateCurrentRendererSettings();
	void ApplyPointCloudFilters();
	void RefreshAvailablePointCloudAttributes();
	bool IsRendererAvailableFromCachedAttributes(EPCLRendererChoice RendererChoice) const;
	EPCLRendererChoice GetFallbackRendererChoice() const;
	void EnsureAvailableRendererSelected();
	void UpdateRendererCheckBoxes();
	void SetRendererOptionVisibility(EPCLRendererChoice RendererChoice, bool bVisible);
	void UpdateSliderValueTexts() const;
	void BuildFilterTabUI();
	void BuildLegendUI();
	bool AreAllClassOptionsSelected() const;
	bool AreAnyClassOptionsSelected() const;
	bool AreAllReturnsOptionsSelected() const;
	bool AreAnyReturnsOptionsSelected() const;
	void ClearActiveFilters();
	void ResetFilterSelections(bool bApplyFilters);
	void SetTabLayout(EPCLTabLayout Layout);
	void SetNamedWidgetHeightOffset(const FName& WidgetName, float HeightOffset);
};
