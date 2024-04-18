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

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bIsHeld;
	bool bSimulateOnDrop;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	TEnumAsByte<EGrabbleType> GrabType;
	UMotionControllerComponent* MotionControllerRef;
	UXRGrabComponent* PrimaryGrabComponent;
	FRotator PrimaryGrabRelativeRotation;
	UHapticFeedbackEffect_Base* OnGrabHapticFeedback;
	
	
protected:
	virtual void BeginPlay() override;
};
