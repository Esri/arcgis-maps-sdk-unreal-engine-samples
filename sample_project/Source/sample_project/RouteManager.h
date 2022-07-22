// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/UObjectGlobals.h"
#include "Containers/List.h"
#include "Components/SplineMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "Json.h"
#include "Http.h"
#include "RouteMarker.h"
#include "Breadcrumb.h"
 #include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
 #include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "RouteManager.generated.h"
UCLASS()
class SAMPLE_PROJECT_API ARouteManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Sets default values for this actor's properties
	ARouteManager();

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	void PostRoutingRequest();
	void ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	void SetupInput();
	void AddStop();

	TSubclassOf<class UUserWidget> UIWidgetClass;
	UUserWidget* UIWidget;
	UArcGISMapComponent* MapComponent;
	TDoubleLinkedList < USplineMeshComponent*> SplineMeshComponents;
	UStaticMesh* RouteMesh;
	TDoubleLinkedList<ARouteMarker*> Stops;
	TDoubleLinkedList<ABreadcrumb*> Breadcrumbs;
	bool bIsRouting = false;
	bool bShouldUpdateBreadcrums = false;
	FVector2D RouteCueScale = FVector2D(5.);
	int StopCount = 2;
};
