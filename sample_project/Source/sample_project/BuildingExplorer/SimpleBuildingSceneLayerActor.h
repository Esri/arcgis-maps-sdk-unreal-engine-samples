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
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SimpleBuildingSceneLayerActor.generated.h"

class UArcGISMapComponent;
namespace Esri
{
namespace GameEngine
{
namespace Layers
{
class ArcGISBuildingSceneLayer;
namespace BuildingScene
{
class ArcGISBuildingSceneSublayer;
} // namespace BuildingScene
} // namespace Layers
} // namespace GameEngine
} // namespace Esri

USTRUCT(BlueprintType)
struct FCategory
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Name;
};

USTRUCT(BlueprintType)
struct FDiscipline
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FString Name;

	UPROPERTY(BlueprintReadWrite)
	TArray<FCategory> Categories;
};

UCLASS()
class SAMPLE_PROJECT_API ASimpleBuildingSceneLayerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASimpleBuildingSceneLayerActor();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "BuildingSceneLayer")
	void AddDisciplineCategoryData();

	UFUNCTION(BlueprintCallable, Category = "BuildingSceneLayer")
	void GenerateWhereClause(int32 level, int32 phase, bool bClearLevel);

	UFUNCTION(BlueprintCallable, Category = "BuildingSceneLayer")
	void InitializeBuildingSceneLayer();

	UFUNCTION(BlueprintCallable, Category = "BuildingSceneLayer")
	void PopulateSublayerMaps(FString option, bool bVisible);

	UPROPERTY(BlueprintReadWrite, Category = "BuildingSceneLayer")
	TArray<FDiscipline> DisciplineCategoryData;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TWeakObjectPtr<UArcGISMapComponent> ArcGISMapComponent;

	Esri::GameEngine::Layers::ArcGISBuildingSceneLayer* BuildingSceneLayer;

	void SetSublayerVisibility(const Esri::GameEngine::Layers::BuildingScene::ArcGISBuildingSceneSublayer& Sublayer, bool bVisible);
};
