/* Copyright 2023 Esri
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

#include "SimpleBuildingSceneLayerActor.h"
#include <Kismet/GameplayStatics.h>
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

// Sets default values
ASimpleBuildingSceneLayerActor::ASimpleBuildingSceneLayerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ASimpleBuildingSceneLayerActor::BeginPlay()
{
	APlayerController* const PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	PlayerController->SetInputMode(FInputModeGameAndUI());
	PlayerController->SetShowMouseCursor(true);
	Super::BeginPlay();
}

void ASimpleBuildingSceneLayerActor::InitializeBuildingSceneLayer()
{
	// Find the Map Actor in the scene
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
					if (Layer.Type == 7)
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
		const auto& FirstLayers = BuildingSceneLayer->GetSublayers();
		for (const auto& FirstSubLayer : FirstLayers)
		{
			if (FirstSubLayer.GetName() == TEXT("Full Model"))
			{
				const auto& SecondLayers = FirstSubLayer.GetSublayers();
				for (const auto& SecondSubLayer : SecondLayers)
				{
					FDiscipline NewDiscipline;
					NewDiscipline.Name = SecondSubLayer.GetName();
					const auto& ThirdLayers = SecondSubLayer.GetSublayers();
					for (const auto& ThirdSubLayer : ThirdLayers)
					{
						FCategory SubCategory;
						SubCategory.Name = ThirdSubLayer.GetName();
						NewDiscipline.Categories.Add(SubCategory);
					}
					DisciplineCategoryData.Add(NewDiscipline);
				}
			}
		}
	}

	// Define the order
	TMap<FString, int32> DisciplineOrder;
	DisciplineOrder.Add(TEXT("Architectural"), 0);
	DisciplineOrder.Add(TEXT("Structural"), 1);
	DisciplineOrder.Add(TEXT("Electrical"), 2);

	DisciplineCategoryData.Sort([&DisciplineOrder](const FDiscipline& A, const FDiscipline& B) {
		int32 OrderA = DisciplineOrder.Contains(A.Name) ? DisciplineOrder[A.Name] : MAX_int32;
		int32 OrderB = DisciplineOrder.Contains(B.Name) ? DisciplineOrder[B.Name] : MAX_int32;
		return OrderA < OrderB;
	});

	// Sort categories within each discipline alphabetically
	for (FDiscipline& Discipline : DisciplineCategoryData)
	{
		Discipline.Categories.Sort([](const FCategory& A, const FCategory& B) {
			return A.Name.Compare(B.Name, ESearchCase::IgnoreCase) < 0;
		});
	}
}
// Creating where clauses to filter desired levels/construction phases
void ASimpleBuildingSceneLayerActor::GenerateWhereClause(int32 level, int32 phase, bool bClearLevel)
{
	Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingAttributeFilter> Filters =
		BuildingSceneLayer->GetBuildingAttributeFilters();
	FString BuildingLevels = TEXT("('");
	FString ConstructionPhases = TEXT("('");

	for (int32 i = 0; i <= level; ++i)
	{
		FString LevelNum = FString::FromInt(i);
		BuildingLevels += LevelNum;
		if (i != level)
		{
			BuildingLevels += TEXT("', '");
		}
		else
		{
			BuildingLevels += TEXT("')");
		}
	}
	for (int32 i = 1; i <= phase; ++i)
	{
		FString PhaseNum = FString::FromInt(i);
		ConstructionPhases += PhaseNum;
		if (i != phase)
		{
			ConstructionPhases += TEXT("', '");
		}
		else
		{
			ConstructionPhases += TEXT("')");
		}
	}

	// Create the where clauses
	FString BuildingLevelClause = FString::Printf(TEXT("BldgLevel in %s"), *BuildingLevels);
	FString ConstructionPhaseClause = FString::Printf(TEXT("CreatedPhase in %s"), *ConstructionPhases);
	FString WhereClause = *ConstructionPhaseClause;

	if (!bClearLevel)
	{
		WhereClause = FString::Printf(TEXT("%s and %s"), *BuildingLevelClause, *ConstructionPhaseClause);
	}

	Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingAttributeFilter* LevelFilter;
	for (auto Filter : Filters)
	{
		FString Name = Filter.GetName();
		if (Name == TEXT("Filter"))
		{
			LevelFilter = &Filter;
			auto solidDefinition = Filter.GetSolidFilterDefinition();
			solidDefinition.SetWhereClause(WhereClause);
			BuildingSceneLayer->SetActiveBuildingAttributeFilter(Filter);
		}
	}
}

void ASimpleBuildingSceneLayerActor::PopulateSublayerMaps(FString option, bool bVisible)
{
	// Search for given discipline/category
	if (BuildingSceneLayer)
	{
		const auto& FirstLayers = BuildingSceneLayer->GetSublayers();
		for (const auto& FirstSubLayer : FirstLayers)
		{
			if (FirstSubLayer.GetName() == TEXT("Full Model"))
			{
				const auto& SecondLayers = FirstSubLayer.GetSublayers();
				for (const auto& SecondSubLayer : SecondLayers)
				{
					if (SecondSubLayer.GetName() == option)
					{
						SetSublayerVisibility(SecondSubLayer, bVisible);
						break;
					}
					const auto& ThirdLayers = SecondSubLayer.GetSublayers();
					for (const auto& ThirdSubLayer : ThirdLayers)
					{
						if (ThirdSubLayer.GetName() == option)
						{
							SetSublayerVisibility(ThirdSubLayer, bVisible);
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
	const_cast<Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingSceneSublayer&>(Sublayer).SetIsVisible(bVisible);
}

// Called every frame
void ASimpleBuildingSceneLayerActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
