/* Copyright 2024 Esri
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
#include "ArcGISMapsSDK/API/GameEngine/ArcGISLoadStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGISBuildingSceneLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/Base/ArcGISLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingAttributeFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingSceneLayerActiveBuildingAttributeFilterChangedEvent.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingSceneLayerAttributeStatistics.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingSceneSublayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISBuildingSceneSublayerDiscipline.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/BuildingScene/ArcGISSolidBuildingFilterDefinition.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISDictionary.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISFuture.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISBuildingSceneLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/CAPI/GameEngine/Attributes/GEBuildingSceneLayerAttributeStatistics.h"
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
					if (Layer.Type == EArcGISLayerType::ArcGISBuildingSceneLayer)
					{
						BuildingSceneLayer = static_cast<Esri::GameEngine::Layers::ArcGISBuildingSceneLayer*>(Layer.APIObject->APIObject.Get());
					}
				}
				break;
			}
		}
	}
}

UArcGISBuildingSceneLayer* ASimpleBuildingSceneLayerActor::NewBuildingSceneLayer(FString source, FString APIKey)
{
	auto newLayer = UArcGISBuildingSceneLayer::CreateArcGISBuildingSceneLayerWithProperties(source, TEXT("NewBSL"), 1, true, APIKey);

	if (ArcGISMapComponent != nullptr)
	{
		auto NewLayers = ArcGISMapComponent->GetMap()->GetLayers();
		NewLayers->Add(newLayer);
	}
	return newLayer;
}

void ASimpleBuildingSceneLayerActor::ConfigureNewBSL(UArcGISBuildingSceneLayer* newLayer)
{
	// Cast the layer to ArcGISBuildingSceneLayer
	LastActiveBSL = BuildingSceneLayer;
	BuildingSceneLayer = static_cast<Esri::GameEngine::Layers::ArcGISBuildingSceneLayer*>(newLayer->APIObject.Get());
	if (!BuildingSceneLayer)
	{
		// Handle the error: the cast failed
		return;
	}
}

FString ASimpleBuildingSceneLayerActor::LoadStatus()
{
	if (BuildingSceneLayer == nullptr)
	{
		return TEXT("Failed");
	}

	auto LoadStatus = BuildingSceneLayer->GetLoadStatus();

	if (LoadStatus == Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return TEXT("Loaded");
	}
	else if (LoadStatus == Esri::GameEngine::ArcGISLoadStatus::Loading)
	{
		return TEXT("Loading");
	}
	else
	{
		BuildingSceneLayer = LastActiveBSL;
		return TEXT("Failed");
	}
}

void ASimpleBuildingSceneLayerActor::AddDisciplineCategoryData()
{
	DisciplineCategoryData.Empty();

	if (!BuildingSceneLayer)
	{
		return;
	}

	const auto& FirstLayers = BuildingSceneLayer->GetSublayers();
	for (const auto& FirstSubLayer : FirstLayers)
	{
		if (FirstSubLayer.GetName() == TEXT("Full Model"))
		{
			SetSublayerVisibility(FirstSubLayer, true);
			const auto& SecondLayers = FirstSubLayer.GetSublayers();

			for (const auto& SecondSubLayer : SecondLayers)
			{
				FDiscipline NewDiscipline;
				NewDiscipline.Name = SecondSubLayer.GetName();
				const auto& ThirdLayers = SecondSubLayer.GetSublayers();
				SetSublayerVisibility(SecondSubLayer, true);

				for (const auto& ThirdSubLayer : ThirdLayers)
				{
					FCategory SubCategory;
					SubCategory.Name = ThirdSubLayer.GetName();
					NewDiscipline.Categories.Add(SubCategory);
					SetSublayerVisibility(ThirdSubLayer, true);
				}
				DisciplineCategoryData.Add(NewDiscipline);
			}
		}
		else if (FirstSubLayer.GetName() == TEXT("Overview"))
		{
			SetSublayerVisibility(FirstSubLayer, false);
		}
	}

	// Define the order
	TMap<FString, int32> DisciplineOrder;
	DisciplineOrder.Add(TEXT("Architectural"), 0);
	DisciplineOrder.Add(TEXT("Structural"), 1);
	DisciplineOrder.Add(TEXT("Mechanical"), 2);
	DisciplineOrder.Add(TEXT("Electrical"), 3);
	DisciplineOrder.Add(TEXT("Piping"), 4);

	// Sort disciplines based on the defined order
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
void ASimpleBuildingSceneLayerActor::GenerateWhereClause(int32 level, int32 phase, bool bClearLevel, bool bNoLevel)
{
	Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingAttributeFilter> Filters =
		BuildingSceneLayer->GetBuildingAttributeFilters();
	FString BuildingLevels = TEXT("('") + FString::FromInt(level) + TEXT("')");
	FString ConstructionPhases = TEXT("('");

	for (int32 i = 0; i <= phase; ++i)
	{
		auto PhaseNum = FString::FromInt(i);
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
	auto BuildingLevelClause = FString::Printf(TEXT("BldgLevel in %s"), *BuildingLevels);
	auto ConstructionPhaseClause = FString::Printf(TEXT("CreatedPhase in %s"), *ConstructionPhases);
	FString WhereClause = *ConstructionPhaseClause;

	if (!bClearLevel)
	{
		WhereClause = FString::Printf(TEXT("%s and %s"), *BuildingLevelClause, *ConstructionPhaseClause);
	}
	if (bNoLevel)
	{
		WhereClause = *ConstructionPhaseClause;
	}

	Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingAttributeFilter* LevelFilter;
	for (auto Filter : Filters)
	{
		if (Filter.GetName() == TEXT("Filter"))
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
	if (!BuildingSceneLayer)
	{
		return;
	}

	const auto& FirstLayers = BuildingSceneLayer->GetSublayers();
	for (const auto& FirstSubLayer : FirstLayers)
	{
		if (FirstSubLayer.GetName() != TEXT("Full Model"))
		{
			continue;
		}

		const auto& SecondLayers = FirstSubLayer.GetSublayers();
		for (const auto& SecondSubLayer : SecondLayers)
		{
			if (SecondSubLayer.GetName() == option)
			{
				SetSublayerVisibility(SecondSubLayer, bVisible);
				return;
			}

			const auto& ThirdLayers = SecondSubLayer.GetSublayers();
			for (const auto& ThirdSubLayer : ThirdLayers)
			{
				if (ThirdSubLayer.GetName() == option)
				{
					SetSublayerVisibility(ThirdSubLayer, bVisible);
					return;
				}
			}
		}
	}
}

FBuildingStatistics ASimpleBuildingSceneLayerActor::GetStatistics()
{
	FBuildingStatistics buildingStatistics;
	buildingStatistics.BldgLevelMin = INT32_MAX;
	buildingStatistics.BldgLevelMax = INT32_MIN;
	buildingStatistics.CreatedPhaseMin = INT32_MAX;
	buildingStatistics.CreatedPhaseMax = INT32_MIN;

	if (BuildingSceneLayer)
	{
		// Fetch statistics asynchronously
		auto data = BuildingSceneLayer->FetchStatisticsAsync();
		data.Wait();
		auto statistics = data.Get();

		// Process BldgLevel statistics
		auto bldgLevelValue = statistics.At(TEXT("BldgLevel"));
		TArray<FString> bldgLevelMostFrequentValues;
		auto bldgLevelMostFrequentValuesCollection = bldgLevelValue.GetMostFrequentValues();
		TArray<int32> bldgLevelValues;

		for (int32 j = 0; j < bldgLevelMostFrequentValuesCollection.GetSize(); j++)
		{
			auto valueStr = bldgLevelMostFrequentValuesCollection.At(j);
			auto valueInt = FCString::Atoi(*valueStr);
			bldgLevelValues.Add(valueInt);
		}

		// Determine highest and lowest values for BldgLevel
		if (bldgLevelValues.Num() > 0)
		{
			buildingStatistics.BldgLevelMin = FMath::Min(bldgLevelValues);
			buildingStatistics.BldgLevelMax = FMath::Max(bldgLevelValues);
		}

		// Process CreatedPhase statistics
		auto createdPhaseValue = statistics.At(TEXT("CreatedPhase"));
		TArray<FString> createdPhaseMostFrequentValues;
		auto createdPhaseMostFrequentValuesCollection = createdPhaseValue.GetMostFrequentValues();
		TArray<int32> createdPhaseValues;

		for (int32 j = 0; j < createdPhaseMostFrequentValuesCollection.GetSize(); j++)
		{
			auto valueStr = createdPhaseMostFrequentValuesCollection.At(j);
			auto valueInt = FCString::Atoi(*valueStr);
			createdPhaseValues.Add(valueInt);
		}

		// Determine highest and lowest values for CreatedPhase
		if (createdPhaseValues.Num() > 0)
		{
			buildingStatistics.CreatedPhaseMin = FMath::Min(createdPhaseValues);
			buildingStatistics.CreatedPhaseMax = FMath::Max(createdPhaseValues);
		}
	}

	return buildingStatistics;
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
