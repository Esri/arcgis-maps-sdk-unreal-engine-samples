// COPYRIGHT 1995-2022 ESRI
// TRADE SECRETS: ESRI PROPRIETARY AND CONFIDENTIAL
// Unpublished material - all rights reserved under the
// Copyright Laws of the United States and applicable international
// laws, treaties, and conventions.
//
// For additional information, contact:
// Attn: Contracts and Legal Department
// Environmental Systems Research Institute, Inc.
// 380 New York Street
// Redlands, California 92373
// USA
//
// email: legal@esri.com

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Components/ComboBoxString.h"
#include "Components/SplineMeshComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "sample_project/Routing/RouteMarker.h"
#include "sample_project/Routing/Breadcrumb.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISLinearUnitId.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISLinearUnit.h" 
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISAngularUnit.h" 
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISAngularUnitId.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeodeticCurveType.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeodeticDistanceResult.h"
#include "ArcGISMapsSDK/API/GameEngine/Geometry/ArcGISGeometry.h"
#include "Measure.generated.h"


UENUM()
enum UnitType
{
	m = 0,
	km = 1,
	mi = 2,
	ft = 3
};


UCLASS()
class AMeasure : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMeasure();
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
		void ClearLine();
	UFUNCTION(BlueprintCallable)
		void UnitChanged();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	void SetupInput();
	void AddStop();
	void SetElevation(AActor* stop);


	TSubclassOf<class UUserWidget> UIWidgetClass;
	UUserWidget* UIWidget;
	UFunction* WidgetFunction;
	UComboBoxString* UnitDropdown;
	UArcGISMapComponent* MapComponent;
	TDoubleLinkedList < USplineMeshComponent*> SplineMeshComponents;
	UStaticMesh* RouteMesh;
	TArray<AActor*> featurePoints;
	TArray<ARouteMarker*> stops;
	FVector2D RouteCueScale = FVector2D(5.);
	double geodeticDistance = 0;
	double InterpolationInterval = 100;
	FString unitTxt;
	FString geodeticDistanceText;
	double ConvertUnits(double units, UnitType from, UnitType to);
	UArcGISLinearUnit* unit;
	UnitType currentUnit;
};
