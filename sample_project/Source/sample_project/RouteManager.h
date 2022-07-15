// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/UObjectGlobals.h"
//#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMesh* MarkerMesh;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:

	AActor* CreateMarker(FVector3d InEnginePosition);


	void SetupInput();
	void AddStop();
};
