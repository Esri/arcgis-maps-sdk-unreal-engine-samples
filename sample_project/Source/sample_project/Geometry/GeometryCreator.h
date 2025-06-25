// /* Copyright 2025 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#pragma once

#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISAngularUnit.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISAngularUnitId.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISAreaUnit.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISEnvelopeBuilder.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeodeticDistanceResult.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISLinearUnit.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISPoint.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISPolygonBuilder.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/ComboBoxString.h"
#include "Components/SplineMeshComponent.h"
#include "Components/TextBlock.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "sample_project/Routing/Breadcrumb.h"
#include "sample_project/Routing/RouteMarker.h"

#include "GeometryCreator.generated.h"

class UArcGISCameraControllerComponent;
class UArcGISLocationComponent;
class UArcGISMapComponent;
class ULineBatchComponent;
class UStaticMeshComponent;

UCLASS()
class AGeometryCreator : public AActor
{
	GENERATED_BODY()

public:
	AGeometryCreator();

	UPROPERTY(BlueprintReadWrite)
	float Calculation = 0.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> UIWidget;
	UPROPERTY()
	TObjectPtr<UComboBoxString> UnitDropdown;

	UFUNCTION(BlueprintCallable)
	void SetCalculation(float InCalculation);

	UFUNCTION(BlueprintCallable)
	float GetCalculation();

	UFUNCTION(BlueprintCallable)
	void SetLinearUnit(UArcGISLinearUnit* LU);

	UFUNCTION(BlueprintCallable)
	void SetAreaUnit(UArcGISAreaUnit* AU);

	UFUNCTION(BlueprintCallable)
	UArcGISLinearUnit* GetLinearUnit();

	UFUNCTION(BlueprintCallable)
	UArcGISAreaUnit* GetAreaUnit();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

private:
	bool bIsDragging = false;
	float InterpolationInterval = 10000;
	FActorSpawnParameters SpawnParam = FActorSpawnParameters();
	FString UnitText;
	FVector2D RouteCueScale = FVector2D(5);
	TDoubleLinkedList<USplineMeshComponent*> SplineMeshComponents;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	bool bIsEnvelopeMode = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	bool bIsPolygonMode = false;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	bool bIsPolylineMode = true;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISAreaUnit> CurrentAreaUnit;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISLinearUnit> CurrentLinearUnit;
	UPROPERTY()
	TArray<AActor*> FeaturePoints;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	class AInputManager* InputManager;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> InterpolationMarkerPrefab;
	UPROPERTY()
	FVector LastRootPosition;
	UPROPERTY()
	TArray<AActor*> lastToStartInterpolationPoints;
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> LineMarkerPrefab;
	UPROPERTY(meta = (AllowPrivateAccess))
	AArcGISMapActor* MapActor;
	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISMapComponent> MapComponent;
	UPROPERTY(EditAnywhere)
	UStaticMesh* RouteMesh;
	UPROPERTY()
	UArcGISSpatialReference* SpatialReference;
	UPROPERTY()
	TObjectPtr<UArcGISPoint> StartPoint;
	UPROPERTY()
	TArray<ARouteMarker*> Stops;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;

	void CreateAndCalculatePolygon();
	void CreateAndCalculateEnvelope(UArcGISPoint* Start, UArcGISPoint* End);
	void Interpolate(AActor* Start, AActor* End, TArray<AActor*>& Points);
	void VisualizeEnvelope(double MinX, double MinY, double MaxX, double MaxY, UArcGISSpatialReference* SpatialRef);
	void UpdateDraggingVisualization();

	UFUNCTION()
	void StartGeometry();

	UFUNCTION()
	void EndGeometry();

	UFUNCTION(BlueprintCallable)
	void RenderLine(TArray<AActor*>& Points);

	UFUNCTION(BlueprintCallable)
	void ClearLine();

	UFUNCTION()
	void UpdateDisplay(float Value);
};
