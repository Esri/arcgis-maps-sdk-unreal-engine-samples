/* Copyright 2022 Esri
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
#include "Kismet/GameplayStatics.h"
#include "Components/TextRenderComponent.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "QueryLocation.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AQueryLocation : public AActor
{
	GENERATED_BODY()
	
public:	
	AQueryLocation();
	virtual void Tick(float DeltaTime) override;
	void SetupAddressQuery(UArcGISPoint* InPoint, FString InAddress);
	void SetupLocationQuery(FVector3d InPoint);
	void UpdateAddressCue(FString inAddress);
	
	UPROPERTY(VisibleAnywhere)
	UArcGISLocationComponent* ArcGISLocation;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	FVector3d MeshScale = FVector3d(20.);

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* TextComponent;

protected:
	virtual void BeginPlay() override;

private:
	bool bShouldUpdateElevation = false;
	FString Address;
	APawn* PawnActor;
	int StableFramesCounter; // Counting the frames during which the raycast has returned the same hit result
	int FramesToWaitForLoading = 30; // Threshold for comparing the StableFramesCounter against
	int RaycastCounter; // Counting the total number of raycasts performed for this location
	int MaxRaycastAttemts = 200; // Threshold for comparing the RaycastCounter against
	UStaticMesh* PinMesh;
	UStaticMesh* PointMesh;
	UMaterial* PointMaterial;
};
