// Fill out your copyright notice in the Description page of Project Settings.

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
		BroadcastAttributionChange();
		});
}

void UArcGISXRAttributionComponent::BroadcastAttributionChange()
{
	if (MapComponent.IsValid())
	{
		//MapComponent->GetView()->GetAttributionText()
		OnAttributionChanged.Broadcast();
	}
}
