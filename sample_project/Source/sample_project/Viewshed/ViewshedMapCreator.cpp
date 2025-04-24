// COPYRIGHT 1995-2025 ESRI
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
#include "ViewshedMapCreator.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

#include "ArcGISMapsSDK/API/GameEngine/Elevation/Base/ArcGISElevationSource.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/Base/ArcGISLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISElevationSourceViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISLayerViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewStateMessage.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Elevation/ArcGISImageElevationSource.h"
//#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Extent/ArcGISExtentCircle.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"
//#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISImageLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISBasemap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapElevation.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapType.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapsSDKProjectSettings.h"

#include "Blueprint/UserWidget.h"

AViewshedMapCreator::AViewshedMapCreator() : Super()
{
	ViewStateLogging = CreateDefaultSubobject<UViewStateLoggingComponent>(TEXT("ArcGISViewStateLoggingComponent"));
}

void AViewshedMapCreator::HideDirections()
{
	AActor* self = this;
	if (HideInstructions)
	{
		UIWidget->ProcessEvent(HideInstructions, &self);
	}
}

void AViewshedMapCreator::OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent)
{
	AArcGISActor::OnArcGISMapComponentChanged(InMapComponent);

	if (MapComponent.IsValid())
	{
		CreateArcGISMap();
	}
}

#if WITH_EDITOR
void AViewshedMapCreator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(AViewshedMapCreator, APIKey))
	{
		if (MapComponent.IsValid())
		{
			CreateArcGISMap();
		}
	}
}
#endif

void AViewshedMapCreator::CreateArcGISMap()
{
	auto mapType = EArcGISMapType::Local;
	auto map = UArcGISMap::CreateArcGISMapWithMapType(mapType);
	
	if (APIKey.IsEmpty())
	{
		if (const UArcGISMapsSDKProjectSettings* Settings = GetDefault<UArcGISMapsSDKProjectSettings>())
		{
			APIKey = Settings->APIKey;
		}
	}
	auto basemapType = EArcGISBasemapStyle::ArcGISImagery;
	auto basemap = UArcGISBasemap::CreateArcGISBasemapWithBasemapStyle(basemapType, APIKey);

	// Set thebasemap
	map->SetBasemap(basemap);
	auto elevationSource = UArcGISImageElevationSource::CreateArcGISImageElevationSourceWithName(
		"https://elevation3d.arcgis.com/arcgis/rest/services/WorldElevation3D/Terrain3D/ImageServer", "Terrain 3D", "");
	auto mapElevation = UArcGISMapElevation::CreateArcGISMapElevationWithElevationSource(elevationSource);
	map->SetElevation(mapElevation);

	auto buildingLayer = UArcGIS3DObjectSceneLayer::CreateArcGIS3DObjectSceneLayerWithProperties("https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/SanFrancisco_Bldgs/SceneServer", "SanFrancisco", 1.0f, true, APIKey);

	if (ViewshedMaterial)
	{
		buildingLayer->SetMaterialReference(ViewshedMaterial);
	}

	map->GetLayers()->Add(buildingLayer);

	FArcGISViewOptions viewOptions{true};

	MapComponent->GetView()->SetViewOptions(viewOptions);
	MapComponent->SetMap(map);
}

void AViewshedMapCreator::BeginPlay()
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
