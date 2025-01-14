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

#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/API/GameEngine/MapView/ArcGISDrawStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "QueryLocation.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AQueryLocation : public AActor
{
	GENERATED_BODY()
	
public:	
	AQueryLocation();
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
	void FinalizeAddressQuery();

	const float CameraDistanceToGround{1000};
	UArcGISMapComponent* MapComponent;
	APawn* PawnActor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UStaticMesh* PinMesh = LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Game/SampleViewer/SharedResources/Geometries/Pin.Pin'"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UStaticMesh* PointMesh = LoadObject<UStaticMesh>(nullptr, TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UMaterial* PointMaterial = LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/SampleViewer/SharedResources/Materials/M_PinHead.M_PinHead'"));
};
