// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextRenderComponent.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"

#include "DrawDebugHelpers.h"
#include "Components/SplineMeshComponent.h"

#include "QueryLocation.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AQueryLocation : public AActor
{
	GENERATED_BODY()
	
public:	
	AQueryLocation();
	
	virtual void Tick(float DeltaTime) override;

	void ApplyQueryResults(UArcGISPoint* Point, FString InAddress);

	UPROPERTY(VisibleAnywhere, Category = "ArcGISMapsSDK|SampleDefaultPawn")
	UArcGISLocationComponent* ArcGISLocation;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	FVector3d MeshScale = FVector3d(17.);

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* TextComponent;




	UStaticMesh* RouteMesh;

	AArcGISMapActor* Map;

	UArcGISMapComponent* MapComp;




protected:
	virtual void BeginPlay() override;

private:
	bool bShouldUpdateElevation = false;
	FString Address;
	APawn* PawnActor;
	UINT16 FrameCounter;
	UINT16 FramesToWait = 10;
};
