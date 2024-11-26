// Copyright 2024 Esri.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0

#include "ArcGISXRAttributionComponent.h"

#include "Delegates/Delegate.h"

#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/Actors/ArcGISActor.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"

UArcGISXRAttributionComponent::UArcGISXRAttributionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;

	OnAttributionChanged.AddDynamic(this, &UArcGISXRAttributionComponent::BroadcastAttributionChange);
}

void UArcGISXRAttributionComponent::OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent)
{
	if (MapComponent.IsValid())
	{
		// Set the attribution callback to null when the map component changed.
		MapComponent->GetView()->APIObject->SetAttributionChanged(nullptr);
	}

	Super::OnArcGISMapComponentChanged(InMapComponent);

	if (!MapComponent.IsValid())
	{
		return;
	}

	MapComponent->GetView()->APIObject->SetAttributionChanged([this]() {
			AsyncTask(ENamedThreads::GameThread, [this]() {
				UE_LOG(LogTemp, Display, TEXT("%s"), *MapComponent->GetView()->GetAttributionText());
				BroadcastAttributionChange();
			});
		}
	);
}

void UArcGISXRAttributionComponent::BroadcastAttributionChange()
{
	if (MapComponent.IsValid())
	{
		OnAttributionChanged.Broadcast();
	}
}
