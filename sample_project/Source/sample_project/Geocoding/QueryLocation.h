// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "QueryLocation.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AQueryLocation : public AActor
{
	GENERATED_BODY()
	
public:	
	AQueryLocation();
	
	virtual void Tick(float DeltaTime) override;

	void SetGeographicLocationXY(UArcGISPoint* Point);

	UPROPERTY(VisibleAnywhere, Category = "ArcGISMapsSDK|SampleDefaultPawn")
	UArcGISLocationComponent* ArcGISLocation;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere)
	FVector3d MeshScale = FVector3d(17.);

protected:
	virtual void BeginPlay() override;

private:
	bool bShouldUpdateElevation = false;
};
