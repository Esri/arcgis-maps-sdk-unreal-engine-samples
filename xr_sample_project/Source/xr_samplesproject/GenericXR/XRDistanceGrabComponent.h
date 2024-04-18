// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "Components/SceneComponent.h"
#include "XRDistanceGrabComponent.generated.h"


class UXRGrabComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XR_SAMPLESPROJECT_API UXRDistanceGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXRDistanceGrabComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	UXRGrabComponent* Grab(UMotionControllerComponent* MotionController);

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	bool bIsDetecting = true;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> IgnoreActors;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UXRGrabComponent* TargetedGrabComponent;
};
