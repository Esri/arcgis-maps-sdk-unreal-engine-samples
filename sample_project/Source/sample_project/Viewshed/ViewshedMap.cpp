#include "Viewshed.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISProjectSettingsAsset.h"

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
	if (ApiKey.IsEmpty())
	{
		ApiKey = UArcGISProjectSettingsAsset::GetArcGISProjectSettings()->APIKey;
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
