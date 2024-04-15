// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "Components/SceneComponent.h"
#include "XRGrabComponent.generated.h"

UENUM(BlueprintType)
enum EGrabbleType
{
	None,
	Free,
	Snap,
	Custom
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGrabbed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReleased);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XR_SAMPLESPROJECT_API UXRGrabComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXRGrabComponent();

	EControllerHand GetHeldByHand();
	bool TryGrab(UMotionControllerComponent* MotionController);
	bool TryRelease();
	void SetPrimitiveCompPhysics(bool bSimulate);
	void SetShouldSimulateOnDrop();

	UPROPERTY(BlueprintAssignable)
	FOnGrabbed OnGrabbed;
	UPROPERTY(BlueprintAssignable)
	FOnReleased OnReleased;

	bool bIsHeld;
	bool bSimulateOnDrop;
	bool bIsStationary = false;
	TEnumAsByte<EGrabbleType> GrabType;
	UMotionControllerComponent* MotionControllerRef;
	UXRGrabComponent* PrimaryGrabComponent;
	FRotator PrimaryGrabRelativeRotation;
	UHapticFeedbackEffect_Base* OnGrabHapticFeedback;
	
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
