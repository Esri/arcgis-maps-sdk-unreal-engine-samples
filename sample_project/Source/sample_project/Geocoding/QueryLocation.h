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

	void SetupAddressQuery(UArcGISPoint* InPoint, FString InAddress);
	void SetupLocationQuery(FVector3d InPoint);
	void UpdateAddressCue(FString inAddress);
	
	bool bIsAddressQuery = false;
	
	UPROPERTY(VisibleAnywhere, Category = "ArcGISMapsSDK|SampleDefaultPawn")
	UArcGISLocationComponent* ArcGISLocation;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	FVector3d MeshScale = FVector3d(12.);

	UPROPERTY(VisibleAnywhere)
	UTextRenderComponent* TextComponent;



	AArcGISMapActor* Map;

	UArcGISMapComponent* MapComp;




protected:
	virtual void BeginPlay() override;

private:
	bool bShouldUpdateElevation = false;
	FString Address;
	APawn* PawnActor;
	UINT16 StableFramesCounter; // Counting the frames during which the raycast has returned the same hit result
	UINT16 FramesToWaitForLoading = 30; // Threshold for comparing the StableFramesCounter against
	UINT16 RaycastCounter; // Counting the total number of raycasts performed for this location
	UINT16 MaxRaycastAttemts = 200; // Threshold for comparing the RaycastCounter against

	UStaticMesh* PinMesh;
	UStaticMesh* PointMesh;
	UMaterial* PointMaterial;
};
