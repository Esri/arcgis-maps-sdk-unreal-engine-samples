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
#include "Components/PointLightComponent.h"
#include "GameFramework/Actor.h"
#include "LampPostItem.generated.h"

class UArcGISLocationComponent;

UCLASS()
class SAMPLE_PROJECT_API ALampPostItem : public AActor
{
	GENERATED_BODY()

public:
	ALampPostItem();

	UPROPERTY(VisibleAnywhere)
	UArcGISLocationComponent* locationComponent;
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess))
	UStaticMeshComponent* lamp;
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess))
	UStaticMeshComponent* glass;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FString> Properties;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FString> PropertiesNames;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double Latitude;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double Longitude;
	UPROPERTY(VisibleAnywhere)
	int Index;

private:
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess))
	UStaticMesh* lampPostMesh = LoadObject<UStaticMesh>(
		nullptr, TEXT(
			"/Script/Engine.StaticMesh'/Game/SampleViewer/Samples/ThirdPersonCharacter/Meshes/City_lantern_FBX_City_lantern.City_lantern_FBX_City_lantern'"));
	UPROPERTY(VisibleAnywhere, meta=(AllowPrivateAccess))
	UStaticMesh* glassMesh = LoadObject<UStaticMesh>(
		nullptr, TEXT(
			"/Script/Engine.StaticMesh'/Game/SampleViewer/Samples/ThirdPersonCharacter/Meshes/City_lantern_FBX_Glass.City_lantern_FBX_Glass'"));

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UPointLightComponent* pointLight;
};
