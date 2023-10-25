// Copyright 2022 Esri.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
//
#pragma once

#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISElevationSourceViewStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISLayerViewStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewStatus.h"
#include "ArcGISMapsSDK/Components/ArcGISActorComponent.h"
#include "CoreMinimal.h"

#include "ViewStateLoggingComponent.generated.h"

UCLASS(ClassGroup = (ArcGISMapsSDK),
	   meta = (BlueprintSpawnableComponent),
	   Category = "ArcGISMapsSDK|ArcGISViewStateLoggingComponent",
	   hidecategories = (Activation, AssetUserData, Collision, Cooking, LOD, Object, Physics, Rendering, SceneComponent, Tags))
class SAMPLE_PROJECT_API UViewStateLoggingComponent : public UArcGISActorComponent
{
	GENERATED_BODY()

public:
	UViewStateLoggingComponent();

	UPROPERTY(EditAnywhere, Category = "ArcGISMapsSDK|ArcGISViewStateComponent")
	bool EnableLogging = true;

	UPROPERTY(EditAnywhere, Category = "ArcGISMapsSDK|ArcGISViewStateComponent")
	bool DisableWarnings = false;

protected:
	void OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent) override;

#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void SetupViewStateEvents();

	void LogElevationStatus(Esri::GameEngine::View::State::ArcGISElevationSourceViewStatus status,
							FString elevationName,
							FString message,
							FString additionalMessage);
	void LogLayerStatus(Esri::GameEngine::View::State::ArcGISLayerViewStatus status, FString layerName, FString message, FString additionalMessage);
	void LogViewStatus(Esri::GameEngine::View::State::ArcGISViewStatus status, FString message, FString additionalMessage);

	FString ElevationStatusToString(Esri::GameEngine::View::State::ArcGISElevationSourceViewStatus status, bool* warning, bool* error);
	FString LayerStatusToString(Esri::GameEngine::View::State::ArcGISLayerViewStatus status, bool* warning, bool* error);
	FString ViewStatusToString(Esri::GameEngine::View::State::ArcGISViewStatus status, bool* warning, bool* error);
};
