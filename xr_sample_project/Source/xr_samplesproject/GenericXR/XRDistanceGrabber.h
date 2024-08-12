// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "XRDistanceGrabbable.h"
#include "XRDistanceGrabber.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
//UCLASS(Blueprintable)
class XR_SAMPLESPROJECT_API UXRDistanceGrabber : public USceneComponent
{
	GENERATED_BODY()

public:	
	UXRDistanceGrabber();
	//~UXRDistanceGrabber();
	
	bool TryGrab();
	
	bool TryRelease();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	//bool bIsDetecting = true;

	virtual void OnComponentCreated() override;

protected:
	virtual void BeginPlay() override;

private:
	bool bIsTracking = false;
	
	const float MaxGrabbableDistance = 5000;

	UXRDistanceGrabbable* GrabbedComponent; 
};
