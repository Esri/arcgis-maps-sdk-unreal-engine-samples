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

#pragma once

#include "ArcGISMapsSDK/API/GameEngine/Extent/ArcGISExtentType.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISExtentInstanceData.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapExtentShapes.h"
#include "ArcGISTabletopPlayerController.h"
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "Kismet/GameplayStatics.h"
#include "XRTableTopInteractor.h"
#include "XRTabletopComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XR_SAMPLESPROJECT_API UXRTabletopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UXRTabletopComponent();

	virtual void OnRegister() override;
	virtual void OnUnregister() override;

	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly)
	double GetElevationOffset();
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	void SetElevationOffset(double InValue);

	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly)
	FVector2D GetDimensions();
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	void SetDimensions(FVector2D InValue);

	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly)
	FGeoPosition GetExtentCenter();
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	void SetExtentCenter(FGeoPosition InValue);

	UFUNCTION(BlueprintPure, BlueprintInternalUseOnly)
	EMapExtentShapes GetShape();
	UFUNCTION(BlueprintCallable, BlueprintInternalUseOnly)
	void SetShape(EMapExtentShapes InValue);

	FVector GetMapComponentLocation();

	FTransform GetFromEngineTransform();

	UFUNCTION(BlueprintCallable)
	void ZoomMap(float ZoomValue);

	void MoveExtentCenter(FVector3d WorldPos);

	bool Raycast(FVector InRayOrigin, FVector InRayDirection, OUT FVector& HitLocation);

	UPROPERTY(EditAnywhere, Category = "XRTabletop")
	AActor* WrapperActor = nullptr;

	AXRTableTopInteractor* TabletopController = nullptr;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	bool FindArcGISCameraLocationInHierarchy();
	bool SetupReferences();
	void GetTabletopController();
	void PostUpdateTabletop(FVector3d InAreaMin, FVector3d InAreaMax, Esri::GameEngine::Extent::ArcGISExtentType InType);
	void PreUpdateTabletop();
	void UpdateOffset();

	UPROPERTY(EditAnywhere, BlueprintGetter = GetExtentCenter, BlueprintSetter = SetExtentCenter, Category = "XRTabletop|Extent")
	FGeoPosition CenterPosition;

	UPROPERTY(EditAnywhere, BlueprintGetter = GetShape, BlueprintSetter = SetShape, Category = "XRTabletop|Extent")
	EMapExtentShapes Shape = EMapExtentShapes::Circle;

	UPROPERTY(EditAnywhere, BlueprintGetter = GetDimensions, BlueprintSetter = SetDimensions, Category = "XRTabletop|Extent", meta = (UIMin = 0.0001))
	FVector2D ExtentDimensions;

	UPROPERTY(EditAnywhere, BlueprintGetter = GetElevationOffset, BlueprintSetter = SetElevationOffset, Category = "XRTabletop")
	double ElevationOffset{ 0 };
	
	UArcGISLocationComponent* ArcGISCameraLocation;

	TWeakObjectPtr<UArcGISMapComponent> MapComponent;

	FDelegateHandle ExtentChangeHandle;

	bool bExtentChanged = false;
	bool bNeedsExtentChange = true;
	bool bNeedsOffsetChange = true;
	
	double LocalZOffset{ 0 };

	const float CameraHeightFactor{ 500. };
	const float MaxExtentDimension{ 7500000. };
	const float MinExtentDimension{ 150. };
	const float WrapperScaleFactor{ 0.9 };
	const float ZoomFactor{ 0.07 };


};
