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
#include "ViewshedMap.h"

#include "Engine/World.h"
#include "TimerManager.h"

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
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISBasemap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapElevation.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapType.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapsSDKProjectSettings.h"

AViewshedMap::AViewshedMap()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AViewshedMap::BeginPlay()
{
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AViewshedMap::InitializeMap, 3.0f, false);
}

void AViewshedMap::OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent)
{
	AArcGISActor::OnArcGISMapComponentChanged(InMapComponent);

	if (MapComponent)
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AViewshedMap::InitializeMap, 3.0f, false);
	}
}

void AViewshedMap::InitializeMap()
{
	if (!MapComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapComponent not found"));
		return;
	}

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

	auto map = MapComponent->GetMap();
	
	auto buildingLayer = UArcGIS3DObjectSceneLayer::CreateArcGIS3DObjectSceneLayerWithProperties("https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/SanFrancisco_Bldgs/SceneServer", "NYScene", 1.0f, true, "");
	
	map->GetLayers()->Add(buildingLayer);

	if (ViewshedMaterial)
	{
		buildingLayer->SetMaterialReference(ViewshedMaterial);
	}

	map->GetLayers()->RemoveAll();

	map->GetLayers()->Add(buildingLayer);

	MapComponent->SetMap(map);
}
