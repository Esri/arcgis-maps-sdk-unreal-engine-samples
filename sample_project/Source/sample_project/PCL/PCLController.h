// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License. */

#pragma once

#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/CheckBox.h"
#include "Components/ComboBoxString.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PCLController.generated.h"

class AInputManager;

UENUM(BlueprintType)
enum class EPCLRendererChoice : uint8
{
	RGB UMETA(DisplayName = "RGB"),
	Class UMETA(DisplayName = "Class"),
	Elevation UMETA(DisplayName = "Elevation"),
	Intensity UMETA(DisplayName = "Intensity")
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

	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<AArcGISMapActor> MapActor;

	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISMapComponent> MapComponent;

	UPROPERTY()
	TObjectPtr<class UArcGISPointCloudLayer> PointCloudLayer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCL|Visualize", meta = (AllowPrivateAccess))
	EPCLRendererChoice CurrentRendererChoice = EPCLRendererChoice::RGB;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PCL|Visualize", meta = (AllowPrivateAccess))
	bool bColorModulationEnabled = false;

	bool bUpdatingRendererCheckBoxes = false;

	FString RGBAttributeName;
	FString ClassAttributeName;
	FString ElevationAttributeName;
	FString IntensityAttributeName;

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

	void CreatePointCloudLayer();
	void ApplyPointCloudVisualization();
	void RefreshAvailablePointCloudAttributes();
	void UpdateRendererCheckBoxes();
	void UpdateSliderValueTexts() const;
};
