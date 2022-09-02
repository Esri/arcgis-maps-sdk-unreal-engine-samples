// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Components/SplineMeshComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Routing/RouteMarker.h"
#include "Routing/Breadcrumb.h"
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


UCLASS()
class AMeasure : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMeasure();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	void SetupInput();
	void AddStop();
	void SetElevation(AActor* stop);
	
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UUserWidget* UIWidget;
	UArcGISMapComponent* MapComponent;
	TDoubleLinkedList < USplineMeshComponent*> SplineMeshComponents;
	UStaticMesh* RouteMesh;
	TArray<ARouteMarker*> stops;
	TArray<AActor*> featurePoints;
	FVector2D RouteCueScale = FVector2D(5.);
	double geodeticDistance=0;
	double InterpolationInterval = 100;
	FString unitTxt;
	FString GeodeticDistanceText;
	
};
