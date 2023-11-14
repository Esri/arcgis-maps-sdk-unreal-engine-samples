// Copyright 2022 Esri.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
//
#include "ViewStateLoggingComponent.h"

#include "ArcGISMapsSDK/API/GameEngine/Elevation/Base/ArcGISElevationSource.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/Base/ArcGISLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISElevationSourceViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISLayerViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewStateMessage.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISDictionary.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"

UViewStateLoggingComponent::UViewStateLoggingComponent() : Super()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

void UViewStateLoggingComponent::OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent)
{
	Super::OnArcGISMapComponentChanged(InMapComponent);

	SetupViewStateEvents();
}

#if WITH_EDITOR
void UViewStateLoggingComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UViewStateLoggingComponent, EnableLogging))
	{
		SetupViewStateEvents();
	}
}
#endif

void UViewStateLoggingComponent::SetupViewStateEvents()
{
	if (!MapComponent.IsValid())
	{
		return;
	}

	// Adding callbacks to get state changes from view
	auto viewChanged = [this](Esri::GameEngine::View::State::ArcGISViewState& state) {
		FString messageString = "";
		FString additionalMessage = "";

		auto message = state.GetMessage();
		if (message)
		{
			messageString = message.GetMessage();

			auto additionalInfo = message.GetAdditionalInformation();
			if (additionalInfo.Contains("Additional Message"))
			{
				additionalMessage = additionalInfo.At("Additional Message");
			}
		}

		LogViewStatus(state.GetStatus(), messageString, additionalMessage);
	};

	// Adding callbacks to get state changes from view of ArcGISVisualLayer
	auto layerViewChanged = [this](Esri::GameEngine::Layers::Base::ArcGISLayer& layer, Esri::GameEngine::View::State::ArcGISLayerViewState& state) {
		FString messageString = "";
		FString additionalMessage = "";

		auto message = state.GetMessage();
		if (message)
		{
			messageString = message.GetMessage();

			auto additionalInfo = message.GetAdditionalInformation();
			if (additionalInfo.Contains("Additional Message"))
			{
				additionalMessage = additionalInfo.At("Additional Message");
			}
		}

		LogLayerStatus(state.GetStatus(), layer.GetName(), messageString, additionalMessage);
	};

	// Adding callbacks to get state changes from view of ArcGISElevationSource
	auto elevationSourceViewChanged = [this](Esri::GameEngine::Elevation::Base::ArcGISElevationSource& elevation,
											 Esri::GameEngine::View::State::ArcGISElevationSourceViewState& state) {
		FString messageString = "";
		FString additionalMessage = "";

		auto message = state.GetMessage();
		if (message)
		{
			messageString = message.GetMessage();

			auto additionalInfo = message.GetAdditionalInformation();
			if (additionalInfo.Contains("Additional Message"))
			{
				additionalMessage = additionalInfo.At("Additional Message");
			}
		}

		LogElevationStatus(state.GetStatus(), elevation.GetName(), messageString, additionalMessage);
	};

	// Set event listeners
	MapComponent->GetView()->APIObject->SetViewStateChanged(std::move(viewChanged));
	MapComponent->GetView()->APIObject->SetLayerViewStateChanged(std::move(layerViewChanged));
	MapComponent->GetView()->APIObject->SetElevationSourceViewStateChanged(std::move(elevationSourceViewChanged));
}

void UViewStateLoggingComponent::LogElevationStatus(Esri::GameEngine::View::State::ArcGISElevationSourceViewStatus status,
														  FString elevationName,
														  FString message,
														  FString additionalMessage)
{
	if (!EnableLogging)
	{
		return;
	}

	bool warning = false;
	bool error = false;
	auto statusString = ElevationStatusToString(status, &warning, &error);
	if (!warning && !error)
	{
		UE_LOG(LogTemp, Display, TEXT("ArcGISElevationSource status=%s name='%s'"), *statusString, *elevationName);
	}
	else if (warning && !DisableWarnings)
	{
		UE_LOG(LogTemp, Warning, TEXT("ArcGISElevationSource status=%s name='%s' message='%s' additional-message='%s'"), *statusString,
			   *elevationName, *message, *additionalMessage);
	}
	else if (error)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISElevationSource status=%s name='%s' message='%s' additional-message='%s'"), *statusString, *elevationName,
			   *message, *additionalMessage);
	}
}

void UViewStateLoggingComponent::LogLayerStatus(Esri::GameEngine::View::State::ArcGISLayerViewStatus status,
													  FString layerName,
													  FString message,
													  FString additionalMessage)
{
	if (!EnableLogging)
	{
		return;
	}

	bool warning = false;
	bool error = false;
	auto statusString = LayerStatusToString(status, &warning, &error);
	if (!warning && !error)
	{
		UE_LOG(LogTemp, Display, TEXT("ArcGISLayer status=%s name='%s'."), *statusString, *layerName);
	}
	else if (warning && !DisableWarnings)
	{
		UE_LOG(LogTemp, Warning, TEXT("ArcGISLayer status=%s name='%s' message='%s' additional-message='%s'"), *statusString, *layerName, *message,
			   *additionalMessage);
	}
	else if (error)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISLayer status=%s name='%s' message='%s' additional-message='%s'"), *statusString, *layerName, *message,
			   *additionalMessage);
	}
}

void UViewStateLoggingComponent::LogViewStatus(Esri::GameEngine::View::State::ArcGISViewStatus status,
													 FString message,
													 FString additionalMessage)
{
	if (!EnableLogging)
	{
		return;
	}

	bool warning = false;
	bool error = false;
	auto statusString = ViewStatusToString(status, &warning, &error);
	if (!warning && !error)
	{
		UE_LOG(LogTemp, Display, TEXT("ArcGISRendererView status=%s."), *statusString);
	}
	else if (warning && !DisableWarnings)
	{
		UE_LOG(LogTemp, Warning, TEXT("ArcGISRendererView status=%s message='%s' additional-message='%s'"), *statusString, *message,
			   *additionalMessage);
	}
	else if (error)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISRendererView status=%s message='%s' additional-message='%s'"), *statusString, *message,
			   *additionalMessage);
	}
}

//Active = 1,
//NotEnabled = 2,
//OutOfScale = 4,
//Loading = 8,
//Error = 16,
//Warning = 32
FString UViewStateLoggingComponent::ElevationStatusToString(Esri::GameEngine::View::State::ArcGISElevationSourceViewStatus status,
																  bool* warning,
																  bool* error)
{
	*warning = false;
	*error = false;
	auto bits = static_cast<int>(status);
	FString result = "|";
	if (bits & 1)
	{
		result += "Active|";
	}
	if (bits & 2)
	{
		result += "NotEnabled|";
	}
	if (bits & 4)
	{
		result += "OutOfScale|";
	}
	if (bits & 8)
	{
		result += "Loading|";
	}
	if (bits & 16)
	{
		result += "Error|";
		*error = true;
	}
	if (bits & 32)
	{
		result += "Warning|";
		*warning = true;
	}
	result.TrimCharInline('|', nullptr);
	return result;
}

//Active = 1,
//NotVisible = 2,
//OutOfScale = 4,
//Loading = 8,
//Error = 16,
//Warning = 32
FString UViewStateLoggingComponent::LayerStatusToString(Esri::GameEngine::View::State::ArcGISLayerViewStatus status, bool* warning, bool* error)
{
	*warning = false;
	*error = false;
	auto bits = static_cast<int>(status);
	FString result = "|";
	if (bits & 1)
	{
		result += "Active|";
	}
	if (bits & 2)
	{
		result += "NotEnabled|";
	}
	if (bits & 4)
	{
		result += "OutOfScale|";
	}
	if (bits & 8)
	{
		result += "Loading|";
	}
	if (bits & 16)
	{
		result += "Error|";
		*error = true;
	}
	if (bits & 32)
	{
		result += "Warning|";
		*warning = true;
	}
	result.TrimCharInline('|', nullptr);
	return result;
}

//Active = 1,
//MapNotReady = 2,
//NoViewport = 4,
//Loading = 8,
//Error = 16,
//Warning = 32
FString UViewStateLoggingComponent::ViewStatusToString(Esri::GameEngine::View::State::ArcGISViewStatus status, bool* warning, bool* error)
{
	*warning = false;
	*error = false;
	auto bits = static_cast<int>(status);
	FString result = "|";
	if (bits & 1)
	{
		result += "Active|";
	}
	if (bits & 2)
	{
		result += "MapNotReady|";
	}
	if (bits & 4)
	{
		result += "NoViewport|";
	}
	if (bits & 8)
	{
		result += "Loading|";
	}
	if (bits & 16)
	{
		result += "Error|";
		*error = true;
	}
	if (bits & 32)
	{
		result += "Warning|";
		*warning = true;
	}
	result.TrimCharInline('|', nullptr);
	return result;
}
