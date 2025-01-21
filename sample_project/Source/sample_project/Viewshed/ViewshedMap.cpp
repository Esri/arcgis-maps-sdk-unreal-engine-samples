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
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapsSDKProjectSettings.h"

AViewshed::AViewshed()
{
	PrimaryActorTick.bCanEverTick = false;
	// Initialize properties
	ViewshedMaterial = nullptr;
}

void AViewshed::BeginPlay()
{
	Super::BeginPlay();
	// Delay the initialization to simulate the Unity coroutine
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &AViewshed::InitializeMap, 3.0f, false);
}

void AViewshed::InitializeMap()
{
	// Find the ArcGISMapComponent
	UArcGISMapComponent* MapComponent = FindComponentByClass<UArcGISMapComponent>();
	if (!MapComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapComponent not found"));
		return;
	}

	// Get the API key
	FString ApiKey = "";
	if (APIKey.IsEmpty())
	{
		if (const UArcGISMapsSDKProjectSettings* Settings = GetDefault<UArcGISMapsSDKProjectSettings>())
		{
			APIKey = Settings->APIKey;
		}
	}

	if (ApiKey.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("An API Key must be set in the project settings for content to load"));
		return;
	}

	// Create the map and set its properties
	auto Map = NewObject<UEsriGameEngineMap>();
	Map->Basemap = NewObject<UEsriGameEngineBasemap>(Map, UEsriGameEngineBasemap::StaticClass(), TEXT("ArcGISImagery"));
	Map->Basemap->Initialize(ApiKey);

	auto ElevationSource = NewObject<UEsriGameEngineElevationSource>(Map, UEsriGameEngineElevationSource::StaticClass(), TEXT("Terrain3D"));
	ElevationSource->Initialize("https://elevation3d.arcgis.com/arcgis/rest/services/WorldElevation3D/Terrain3D/ImageServer", "Terrain 3D", "");

	Map->Elevation = ElevationSource;

	auto BuildingLayer = NewObject<UEsriGameEngineLayer>(Map, UEsriGameEngineLayer::StaticClass(), TEXT("BuildingLayer"));
	BuildingLayer->Initialize("https://tiles.arcgis.com/tiles/z2tnIkrLQ2BRzr6P/arcgis/rest/services/SanFrancisco_Bldgs/SceneServer", "Building Layer", 1.0f, true, "");

	if (ViewshedMaterial)
	{
		BuildingLayer->SetMaterial(ViewshedMaterial);
	}

	Map->Layers.Add(BuildingLayer);

	MapComponent->SetMap(Map);
}
