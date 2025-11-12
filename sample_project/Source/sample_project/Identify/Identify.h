// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "Identify.generated.h"

UCLASS()
class AIdentify : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIdentify();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	FString IdentifyAtMouseClick();

private:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	class AInputManager* InputManager;
	UPROPERTY(meta = (AllowPrivateAccess))
	AArcGISMapActor* MapActor;
	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISMapComponent> MapComponent;
};
