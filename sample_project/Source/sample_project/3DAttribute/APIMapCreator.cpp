// COPYRIGHT 1995-2025 ESRI
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
// @@Start(Scripting)
// @@Start(Header)
#include "APIMapCreator.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "ArcGISMapsSDK/API/GameEngine/Elevation/Base/ArcGISElevationSource.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/Base/ArcGISLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISElevationSourceViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISLayerViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewStateMessage.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Elevation/ArcGISImageElevationSource.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Extent/ArcGISExtentCircle.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISImageLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISBasemap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapElevation.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapType.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"

#include "ArcGISPawn.h"
#include "Blueprint/UserWidget.h"
// @@End(Header)

// Set default values
// @@Start(CallTick)
AAPIMapCreator::AAPIMapCreator() : Super()
{
	ViewStateLogging = CreateDefaultSubobject<UArcGISViewStateLoggingComponent>(TEXT("ArcGISViewStateLoggingComponent"));
}

void AAPIMapCreator::HideDirections()
{
	AActor* self = this;
	if (HideInstructions)
	{
		UIWidget->ProcessEvent(HideInstructions, &self);
	}
}

void AAPIMapCreator::SetVisualType(FString type)
{
	if (!AttributeComponent)
	{
		return;
	}
	
	if (type == "Building")
	{
		AttributeComponent->AttributeType = ArcGISVisualizationType::BuildingName;
	}
	else if (type == "Construction")
	{
		AttributeComponent->AttributeType = ArcGISVisualizationType::ConstructionYear;
	}
	else
	{
		AttributeComponent->AttributeType = ArcGISVisualizationType::None;
	}

	CreateArcGISMap();
}

// @@End(CallTick)

// @@Start(SubListener)
void AAPIMapCreator::OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent)
{
	AArcGISActor::OnArcGISMapComponentChanged(InMapComponent);

	if (MapComponent.IsValid())
	{
		CreateArcGISMap();
	}
}
// @@End(SubListener)

#if WITH_EDITOR
void AAPIMapCreator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AAPIMapCreator, APIKey))
	{
		if (MapComponent.IsValid())
		{
			CreateArcGISMap();
		}
	}
}
#endif

// We create our map with this method
// @@Start(CreateMap)
void AAPIMapCreator::CreateArcGISMap()
{
	// @@Start(MapDoc)
	// Create the map document
	auto mapType = EArcGISMapType::Local;
	auto map = UArcGISMap::CreateArcGISMapWithMapType(mapType);
	// @@End(MapDoc)

	// @@Start(Basemap)
	// Add a basemap
	auto basemapType = EArcGISBasemapStyle::ArcGISImagery;
	auto basemap = UArcGISBasemap::CreateArcGISBasemapWithBasemapStyle(basemapType, APIKey);

	// Set the basemap
	map->SetBasemap(basemap);
	// @@End(Basemap)

	// @@Start(Elevation)
	// Set the elevation
	auto elevationSource = UArcGISImageElevationSource::CreateArcGISImageElevationSourceWithName(
		"https://elevation3d.arcgis.com/arcgis/rest/services/WorldElevation3D/Terrain3D/ImageServer", "Terrain 3D", "");
	auto mapElevation = UArcGISMapElevation::CreateArcGISMapElevationWithElevationSource(elevationSource);
	map->SetElevation(mapElevation);
	// @@End(Elevation)

	// @@Start(AddLayer)
	// Create layers
	auto layer_1 = UArcGISImageLayer::CreateArcGISImageLayerWithProperties(
		"https://tiles.arcgis.com/tiles/nGt4QxSblgDfeJn9/arcgis/rest/services/UrbanObservatory_NYC_TransitFrequency/MapServer",
		"NYTransitFrequencyTiles", 1.0f, true, "");
	map->GetLayers()->Add(layer_1);

	auto layer_2 = UArcGISImageLayer::CreateArcGISImageLayerWithProperties(
		"https://tiles.arcgis.com/tiles/nGt4QxSblgDfeJn9/arcgis/rest/services/New_York_Industrial/MapServer", "NYIndustrialTiles", 1.0f, true, "");
	map->GetLayers()->Add(layer_2);

	auto layer_3 = UArcGISImageLayer::CreateArcGISImageLayerWithProperties(
		"https://tiles.arcgis.com/tiles/4yjifSiIG17X0gW4/arcgis/rest/services/NewYorkCity_PopDensity/MapServer", "NYPopDensityTiles", 1.0f, true, "");
	map->GetLayers()->Add(layer_3);

	auto buildingLayer = UArcGIS3DObjectSceneLayer::CreateArcGIS3DObjectSceneLayerWithProperties(
		"https://tiles.arcgis.com/tiles/P3ePLMYs2RVChkJx/arcgis/rest/services/Buildings_NewYork_17/SceneServer", "NYScene", 1.0f, true, "");
	map->GetLayers()->Add(buildingLayer);
	// @@End(AddLayer)

	// This call invokes a function used by the Sample3DAttributesCreator component
	AttributeComponent = Cast<UAttributeComponent>(GetComponentByClass(UAttributeComponent::StaticClass()));

	if (AttributeComponent != nullptr)
	{
		AttributeComponent->Setup3DAttributes(buildingLayer);
	}

	// Remove a layer
	auto index = map->GetLayers()->IndexOf(layer_3);
	map->GetLayers()->Remove(index);
	// Update properties
	layer_1->SetOpacity(0.9f);
	layer_2->SetOpacity(0.6f);

	// @@Start(Extent)
	// Create extent
	if (mapType == EArcGISMapType::Local)
	{
		// Set this to true to enable an extent on the map component
		MapComponent->SetIsExtentEnabled(true);

		auto sr = UArcGISSpatialReference::WGS84();
		auto extentCenter = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(-74.054921, 40.691242, 3000, sr);
		auto extent = UArcGISExtentCircle::CreateArcGISExtentCircle(extentCenter, 10000);
		map->SetClippingArea(extent);
	}
	// @@End(Extent)

	// @@Start(Pawn)
	// Set the initial Pawn location. This can be removed to set the AArcGISPawn`s location using the details panel
	auto pawn = Cast<AArcGISPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISPawn::StaticClass()));
	auto pawnLocationComponent = Cast<UArcGISLocationComponent>(pawn->GetComponentByClass(UArcGISLocationComponent::StaticClass()));

	auto position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(-74.054921, 40.691242, 3000, UArcGISSpatialReference::WGS84());
	pawnLocationComponent->SetPosition(position);

	auto rotation = UArcGISRotation::CreateArcGISRotation(65, 0, 68);
	pawnLocationComponent->SetRotation(rotation);
	// @@End(Pawn)

	// @@Start(ViewOp)
	// Create the view options config struct
	FArcGISViewOptions viewOptions{true};

	// Set the map and view options
	MapComponent->GetView()->SetViewOptions(viewOptions);
	MapComponent->SetMap(map);
	// @@End(ViewOp)
}

void AAPIMapCreator::BeginPlay()
{
	Super::BeginPlay();

	if (UIWidgetClass != nullptr)
	{
		AActor* self = this;
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			UIWidget->AddToViewport();
			HideInstructions = UIWidget->FindFunction(FName("PlayAnim"));
		}
	}
}

// @@End(CreateMap)
// @@End(Scripting)
