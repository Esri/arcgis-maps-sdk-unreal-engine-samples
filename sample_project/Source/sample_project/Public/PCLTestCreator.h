// COPYRIGHT 1995-2026 ESRI
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: https://www.apache.org/licenses/LICENSE-2.0
#pragma once

#include "CoreMinimal.h"
#include "ArcGISMapsSDK/Actors/ArcGISActor.h"
#include "ArcGISViewStateLoggingComponent.h"

#include "PCLTestCreator.generated.h"

class UArcGISPointCloudLayer;

UENUM()
enum class EPointCloudTestRenderer : uint8
{
	ClassBreaks UMETA(DisplayName = "Class Breaks Renderer"),
	Stretch UMETA(DisplayName = "Stretch Renderer"),
	RGB UMETA(DisplayName = "RGB Renderer"),
	UniqueValue UMETA(DisplayName = "Unique Value Renderer")
};

UENUM()
enum class EPointCloudTestFilter : uint8
{
	None UMETA(DisplayName = "No Filter"),
	Bitfield UMETA(DisplayName = "Bitfield Filter"),
	Return UMETA(DisplayName = "Return Filter"),
	Value UMETA(DisplayName = "Value Filter"),
	ValueReturn UMETA(DisplayName = "Value & Return Filter"),
};


UCLASS()
class SAMPLE_PROJECT_API APCLTestCreator : public AArcGISActor
{
	GENERATED_BODY()
	
	public:
	APCLTestCreator();

	UPROPERTY(EditAnywhere, Category = "ArcGISSamples|PointCloudRenderFilterTest")
	FString APIKey;

	UPROPERTY(EditAnywhere, Category = "ArcGISSamples|PointCloudRenderFilterTest")
	FString PointCloudLayerSource = "https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/Moro_Bay_LiDAR/SceneServer";

	UPROPERTY(EditAnywhere, Category = "ArcGISSamples|PointCloudRenderFilterTest")
	EPointCloudTestRenderer Renderer = EPointCloudTestRenderer::RGB;

	UPROPERTY(EditAnywhere, Category = "ArcGISSamples|PointCloudRenderFilterTest")
	EPointCloudTestFilter Filter = EPointCloudTestFilter::None;

	UPROPERTY(EditAnywhere, Category = "ArcGISSamples|PointCloudRenderFilterTest")
	bool bUseColorModulation = false;

	UPROPERTY(EditAnywhere, Category = "ArcGISSamples|PointCloudRenderFilterTest")
	bool bUseFixedSizeDIPs = true;

	UPROPERTY(EditAnywhere,
			  Category = "ArcGISSamples|PointCloudRenderFilterTest",
			  meta = (EditCondition = "bUseFixedSizeDIPs",
					  ClampMin = "1.0",
					  ClampMax = "64.0",
					  UIMin = "1.0",
					  UIMax = "24.0",
					  DisplayName = "Fixed Size DIPs"))
	double FixedSizeDIPs = 5.0;

	UPROPERTY(EditAnywhere,
			  Category = "ArcGISSamples|PointCloudRenderFilterTest",
			  meta = (ClampMin = "1.0", ClampMax = "100.0", UIMin = "1.0", UIMax = "60.0"))
	double PointsPerInch = 10.0;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent) override;

	void CreateArcGISMap();

private:
	UPROPERTY(Category = "ArcGISSamples|PointCloudRenderFilterTest", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UArcGISViewStateLoggingComponent> ViewStateLogging;

	UPROPERTY(Transient)
	TObjectPtr<UArcGISPointCloudLayer> CachedPointCloudLayer;

	void ApplyPointCloudSettings(UArcGISPointCloudLayer* InPointCloudLayer, bool bForceSizeAlgorithmUpdate = false) const;
	void UpdatePointCloudSettings(bool bForceSizeAlgorithmUpdate = false);
};
