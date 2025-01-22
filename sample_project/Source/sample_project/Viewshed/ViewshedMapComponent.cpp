/* Copyright 2025 Esri
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
#include "ViewshedMapComponent.h"

#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

#include "ArcGISMapsSDK/API/GameEngine/Elevation/Base/ArcGISElevationSource.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Elevation/ArcGISImageElevationSource.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISBasemap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapElevation.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapType.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapsSDKProjectSettings.h"

UViewshedMapComponent::UViewshedMapComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UViewshedMapComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UViewshedMapComponent::OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent)
{
	UArcGISActorComponent::OnArcGISMapComponentChanged(InMapComponent);

	if (MapComponent.IsValid())
	{
		UViewshedMapComponent::InitializeMap();
	}
}

void UViewshedMapComponent::InitializeMap()
{
	FString APIKey = "";

	if (APIKey.IsEmpty())
	{
		if (const UArcGISMapsSDKProjectSettings* Settings = GetDefault<UArcGISMapsSDKProjectSettings>())
		{
			APIKey = Settings->APIKey;
		}
	}

	if (APIKey.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("An API Key must be set in the project settings for content to load"));
		return;
	}

	auto spatialReference = UArcGISSpatialReference::WebMercator();

	auto mapType = EArcGISMapType::Local;
	auto map = UArcGISMap::CreateArcGISMapWithSpatialReferenceAndMapType(spatialReference, mapType);

	auto basemapType = EArcGISBasemapStyle::ArcGISImagery;
	auto basemap = UArcGISBasemap::CreateArcGISBasemapWithBasemapStyle(basemapType, APIKey);

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

	MapComponent->SetMap(map);
}
