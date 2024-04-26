// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcGISTabletopPlayerController.h"
#include "ArcGISMapsSDK/API/GameEngine/Extent/ArcGISExtentType.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapExtentShapes.h"
#include "Components/ActorComponent.h"

#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "XRTableTopInteractor.h"

#include "XRTabletopComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XR_SAMPLESPROJECT_API UXRTabletopComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXRTabletopComponent();

// UActorComponent
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
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

	UPROPERTY(EditAnywhere, Category = "ArcGISTabletop")
	AActor* WrapperActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "ArcGISTabletop")
	AXRTableTopInteractor* TabletopController = nullptr;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

private:
	void GetTabletopController();

	void PostUpdateTabletop(FVector3d InAreaMin, FVector3d InAreaMax, Esri::GameEngine::Extent::ArcGISExtentType InType);

	void PreUpdateTabletop();

	void UpdateOffset();

	bool FindArcGISCameraLocationInHierarchy();

	bool SetupReferences();

	UPROPERTY(EditAnywhere, BlueprintGetter = GetExtentCenter, BlueprintSetter = SetExtentCenter, Category = "Tabletop|Extent")
	FGeoPosition CenterPosition;

	UPROPERTY(EditAnywhere, BlueprintGetter = GetShape, BlueprintSetter = SetShape, Category = "Tabletop|Extent")
	EMapExtentShapes Shape = EMapExtentShapes::Circle;

	UPROPERTY(EditAnywhere, BlueprintGetter = GetDimensions, BlueprintSetter = SetDimensions, Category = "Tabletop|Extent", meta = (UIMin = 0.0001))
	FVector2D ExtentDimensions;

	UPROPERTY(EditAnywhere, BlueprintGetter = GetElevationOffset, BlueprintSetter = SetElevationOffset, Category = "Tabletop")
	double ElevationOffset = 0;

	UArcGISLocationComponent* ArcGISCameraLocation = nullptr;

	TWeakObjectPtr<UArcGISMapComponent> MapComponent;

	FDelegateHandle ExtentChangeHandle;

	float ZoomFactor = 0.07;

	bool bNeedsExtentChange = true;
	bool bExtentChanged = false;
	bool bNeedsOffsetChange = true;
};
