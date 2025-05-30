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

#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/SplineMeshComponent.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "ArcGISMapsSDK/Utils/GeoCoord/GeoPosition.h"
#include "ArcGISMapsSDK/API/GameEngine/Geometry/ArcGISGeometry.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISAngularUnit.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISAngularUnitId.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeodeticCurveType.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeodeticDistanceResult.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISLinearUnit.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISLinearUnitId.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "sample_project/Routing/Breadcrumb.h"
#include "sample_project/Routing/RouteMarker.h"

#include "Measure.generated.h"

UENUM(BlueprintType)
enum ESelection
{
	isMiles,
	isFeet,
	isMetes,
	isKilometers
};
UCLASS()
class AMeasure : public AActor
{
	GENERATED_BODY()

public:
	AMeasure();

	UFUNCTION(BlueprintCallable)
	void RenderLine();

	UFUNCTION(BlueprintCallable)
	void ClearLine();

	UFUNCTION(BlueprintCallable)
	void SetDistance(float distance);
	UFUNCTION(BlueprintCallable)
	float GetDistance();
	UFUNCTION(BlueprintCallable)
	void SetUnit(UArcGISLinearUnit* unit);
	UFUNCTION(BlueprintCallable)
	UArcGISLinearUnit* GetUnit();

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		TEnumAsByte<ESelection> Selection;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
		float GeodeticDistance;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TObjectPtr<UUserWidget> UIWidget;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	TArray<AActor*> FeaturePoints;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	class AInputManager* inputManager;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	AArcGISMapActor* MapActor;
	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISMapComponent> MapComponent;
	FVector2D RouteCueScale = FVector2D(5);
	UStaticMesh* RouteMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/SampleViewer/SharedResources/Geometries/Cube.Cube"));
	TDoubleLinkedList<USplineMeshComponent*> SplineMeshComponents;
	TArray<ARouteMarker*> Stops;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISLinearUnit> Unit;
	TObjectPtr<UComboBoxString> UnitDropdown;
	FString UnitText;
	UFunction* WidgetFunction;
	UFunction* HideInstructions;
	UFUNCTION()
	void UpdateDistance(float Value);
	double SegmentDistance;
	FActorSpawnParameters SpawnParam = FActorSpawnParameters();
	float MarkerHeight = 7000.0f;
	UArcGISSpatialReference* SpatialRef;

	UFUNCTION()
	void AddStop();
	void Interpolate(AActor* start, AActor* end);
	void SetElevation(AActor* stop);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	bool isHidden = false;
};
