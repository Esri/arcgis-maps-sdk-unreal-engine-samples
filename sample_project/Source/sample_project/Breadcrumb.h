// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Breadcrumb.generated.h"

UCLASS()
class SAMPLE_PROJECT_API ABreadcrumb : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ABreadcrumb();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "ArcGISMapsSDK|SampleDefaultPawn")
	UArcGISLocationComponent* ArcGISLocation;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComponent;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	


};
