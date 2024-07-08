// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#include "SimpleBuildingSceneLayerActor.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGISBuildingSceneLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/Base/ArcGISLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingAttributeFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingSceneLayerActiveBuildingAttributeFilterChangedEvent.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingSceneSublayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingSceneSublayerDiscipline.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISSolidBuildingFilterDefinition.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASimpleBuildingSceneLayerActor::ASimpleBuildingSceneLayerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASimpleBuildingSceneLayerActor::BeginPlay()
{
	InitializeBuildingSceneLayer();
	Super::BeginPlay();
}

void ASimpleBuildingSceneLayerActor::InitializeBuildingSceneLayer()
{
	//Find the Map Actor in the scene
	for (TActorIterator<AArcGISMapActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
	{
		AArcGISMapActor* ArcGISMapActor = *ActorItr;
		if (ArcGISMapActor)
		{
			ArcGISMapComponent = ArcGISMapActor->FindComponentByClass<UArcGISMapComponent>();
			if (ArcGISMapComponent != nullptr)
			{
				const auto& AllLayers = ArcGISMapComponent->GetLayers();

				for (const auto& Layer : AllLayers)
				{
					UE_LOG(LogTemp, Warning, TEXT("No Layers?"))
					if (Layer.Name == TEXT("Building E"))
					{
						BuildingSceneLayer = static_cast<Esri::GameEngine::Layers::ArcGISBuildingSceneLayer*>(Layer.APIObject->APIObject.Get());
					}
				}
				break;
			}
		}
	}
}

void ASimpleBuildingSceneLayerActor::AddDisciplineCategoryData()
{
	DisciplineCategoryData.Empty();
	if (BuildingSceneLayer)
	{
		const auto& firstLayers = BuildingSceneLayer->GetSublayers();
		FString Tect = BuildingSceneLayer->GetName();
		for (const auto& firstSubLayer : firstLayers)
		{
			if (firstSubLayer.GetName() == TEXT("Full Model"))
			{
				const auto& secondLayers = firstSubLayer.GetSublayers();
				for (const auto& secondSubLayer : secondLayers)
				{
					FDiscipline NewDiscipline;
					NewDiscipline.Name = secondSubLayer.GetName();
					const auto& thirdLayers = secondSubLayer.GetSublayers();
					for (const auto& thirdSubLayer : thirdLayers)
					{
						FCategory SubCategory;
						SubCategory.Name = thirdSubLayer.GetName();
						NewDiscipline.Categories.Add(SubCategory);
					}
					DisciplineCategoryData.Add(NewDiscipline);
				}
			}
		}
	}
}

void ASimpleBuildingSceneLayerActor::ApplyWhereClause(FString level)
{
	Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingAttributeFilter> Filters =
		BuildingSceneLayer->GetBuildingAttributeFilters();
	for (const auto& filter : Filters)
	{
		FString Name = filter.GetName();
		if (Name == level)
		{
			BuildingSceneLayer->SetActiveBuildingAttributeFilter(filter);
		}
	}
}

void ASimpleBuildingSceneLayerActor::PopulateSublayerMaps(FString option, bool bVisible)
{
	// Barebones functionality - We will put them into a map for faster lookup later
	if (BuildingSceneLayer)
	{
		const auto& firstLayers = BuildingSceneLayer->GetSublayers();
		for (const auto& firstSubLayer : firstLayers)
		{
			UE_LOG(LogTemp, Warning, TEXT("Processing Sublayer: %s"), *firstSubLayer.GetName());
			if (firstSubLayer.GetName() == TEXT("Full Model"))
			{
				const auto& secondLayers = firstSubLayer.GetSublayers();
				for (const auto& secondSubLayer : secondLayers)
				{
					UE_LOG(LogTemp, Warning, TEXT("Processing SecondSublayer: %s"), *secondSubLayer.GetName());
					if (secondSubLayer.GetName() == option)
					{
						SetSublayerVisibility(secondSubLayer, bVisible);
						break;
					}
					const auto& thirdLayers = secondSubLayer.GetSublayers();
					for (const auto& thirdSubLayer : thirdLayers)
					{
						UE_LOG(LogTemp, Warning, TEXT("Processing Sublayer: %s"), *thirdSubLayer.GetName());
						if (thirdSubLayer.GetName() == option)
						{
							SetSublayerVisibility(thirdSubLayer, bVisible);
							break;
						}
					}
				}
			}
		}
	}
}

void ASimpleBuildingSceneLayerActor::SetSublayerVisibility(const Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingSceneSublayer& Sublayer,
														   bool bVisible)
{
	// Use const_cast to remove const qualifier if necessary
	const_cast<Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingSceneSublayer&>(Sublayer).SetIsVisible(bVisible);
}

// Called every frame
void ASimpleBuildingSceneLayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
