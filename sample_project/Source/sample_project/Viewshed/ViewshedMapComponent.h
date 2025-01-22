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

#include "CoreMinimal.h"
#include "Materials/MaterialInterface.h"

#include "ArcGISMapsSDK/Components/ArcGISActorComponent.h"

#include "ViewshedMapComponent.generated.h"

UCLASS(ClassGroup = (Viewshed),
	   meta = (BlueprintSpawnableComponent),
	   Category = "Viewshed",
	   hidecategories = (Activation, AssetUserData, Collision, Cooking, LOD, Object, Physics, Rendering, SceneComponent, Tags))
class SAMPLE_PROJECT_API UViewshedMapComponent : public UArcGISActorComponent
{
	GENERATED_BODY()

public:
	UViewshedMapComponent();

	void OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent) override;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* ViewshedMaterial;

	UFUNCTION()
	void InitializeMap();

	FTimerHandle TimerHandle;
};
