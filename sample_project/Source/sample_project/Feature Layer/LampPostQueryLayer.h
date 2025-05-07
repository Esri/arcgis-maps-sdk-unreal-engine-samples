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

#pragma once

#include "ArcGISSamples/Public/ArcGISPawn.h"
#include "ArcGISFeatureLayerQuery.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "Engine/DataTable.h"
#include "LampPostQueryLayer.generated.h"

class ALampPostItem;
class AFeatureItem;
class UInputAction;
class UInputMappingContext;

UCLASS()
class SAMPLE_PROJECT_API ALampPostQueryLayer : public AArcGISFeatureLayerQuery
{
	GENERATED_BODY()

public:
	ALampPostQueryLayer();
	

	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	AArcGISPawn* ArcGISPawn;

private:
	
	TArray<TSharedPtr<FJsonValue>> Features;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
