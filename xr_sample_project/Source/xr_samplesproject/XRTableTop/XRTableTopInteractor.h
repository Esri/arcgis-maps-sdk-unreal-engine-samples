// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "HeadMountedDisplayTypes.h"
#include "xr_samplesproject/GenericXR/XRGrabComponent.h"
#include "XRTableTopInteractor.generated.h"

class UWidgetInteractionComponent;
class UVRHandAnimInstance;
class UXRDistanceGrabComponent;
class UXRTabletopComponent;

UCLASS()
class XR_SAMPLESPROJECT_API AXRTableTopInteractor : public APawn
{
	GENERATED_BODY()

public:
	AXRTableTopInteractor();

	UFUNCTION(BlueprintCallable)
	void StartPanning();
	UFUNCTION(BlueprintCallable)
	void StopPanning();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UXRGrabComponent* GetGrabComponentNearMotionController(UMotionControllerComponent* MotionController);
	void OnGrabLeft();
	void OnGrabRight();
	void OnTriggerLeft();
	void OnTriggerRight();
	void OnReleaseLeft();
	void OnReleaseRight();
	void SetLeftGripAxis(const FInputActionValue& value);
	void SetLeftTriggerAxis(const FInputActionValue& value);
	void SetRightGripAxis(const FInputActionValue& value);
	void SetRightTriggerAxis(const FInputActionValue& value);
	void ResetLeftGripAxis();
	void ResetLeftTriggerAxis();
	void ResetRightGripAxis();
	void ResetRightTriggerAxis();
	void SimulateClickLeft();
	void SimulateClickRight();
	void ResetClickLeft();
	void ResetClickRight();
	void SetTabletopComponent();
	void UpdatePointDrag();
	void ZoomMap(const FInputActionValue& value);
	
	bool bIsDragging;
	bool bUseRightHand;
	FVector3d DragStartEnginePos{FVector3d::ZeroVector};
	FTransform DragStartWorldTransform{FTransform::Identity};
	FHitResult panningHit;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputMappingContext* inputContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Samples/XRTableTop/Input/IAC_XRTableTop.IAC_XRTableTop'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* gripLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Left.IA_Grip_Left'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* gripRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Right.IA_Grip_Right'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* panLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Left.IA_Pan_Left'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* panRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Right.IA_Pan_Right'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* zoom = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Zoom.IA_Zoom'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* grip_L = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_LeftGrip.IA_LeftGrip'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* grip_R = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_RightGrip.IA_RightGrip'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* trigger_L = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_LeftTrigger.IA_LeftTrigger'"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UInputAction* trigger_R = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_RightTrigger.IA_RightTrigger'"));
	
	UXRGrabComponent* heldComponentLeft = nullptr;
	UXRGrabComponent* heldComponentRight = nullptr;
	
	UMotionControllerComponent* currentPanningController;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UXRDistanceGrabComponent* distanceGrabLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UXRDistanceGrabComponent* distanceGrabRight;

	USkeletalMesh* handMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));

	UVRHandAnimInstance* leftAnimInstance;
	UAnimInstance* leftAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* leftHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* leftInteraction;
	
	UVRHandAnimInstance* rightAnimInstance;
	UAnimInstance* rightAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* rightHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* rightInteraction;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UXRTabletopComponent* TabletopComponent{nullptr};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	TEnumAsByte<EHMDTrackingOrigin::Type> trackingOrigin;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UCameraComponent* vrCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* leftMotionControllerAim;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* rightMotionControllerAim;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* leftMotionControllerGrip;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* rightMotionControllerGrip;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	USceneComponent* vrOrigin;
};
