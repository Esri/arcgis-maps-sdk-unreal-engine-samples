// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/Character.h"
#include "IXRTrackingSystem.h"
#include "XRPawn.generated.h"

struct FInputActionValue;
class UVRHandAnimInstance;
class UWidgetInteractionComponent;
class UMotionControllerComponent;
class UCameraComponent;

UCLASS()
class XR_SAMPLESPROJECT_API AXRPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AXRPawn();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bUseSmoothTurn = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bMoveInLookDirection = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float MovementDeadzone = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int MoveSpeed = 10000;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float RotationDeadzone = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float RotationSpeed = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	int SnapRotationDegrees = 30;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void InitializeCapsuleHeight();
	void MoveForward(const FInputActionValue& value);
	void MoveRight(const FInputActionValue& value);
	void SetCapsuleHeight();
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
	void ResetDoOnce();
	void SmoothTurn(const FInputActionValue& value);
	void UpdateRoomScaleMovement();

	bool bDoOnce = true;
	float capsuleHalfHeight;
	UInputAction* clickLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_Menu_Cursor_Left.IA_Menu_Cursor_Left'"));
	UInputAction* clickRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_Menu_Cursor_Right.IA_Menu_Cursor_Right'"));
	USkeletalMesh* handMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));
	UInputAction* grip_L = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_LeftGrip.IA_LeftGrip'"));
	UInputAction* grip_R = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_RightGrip.IA_RightGrip'"));
	UVRHandAnimInstance* leftAnimInstance;
	UAnimInstance* leftAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* leftInteraction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* leftMotionController;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* leftMotionControllerInteractor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* leftHandMesh;
	UInputMappingContext* mappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Samples/VRSample/Input/IMC_Default.IMC_Default'"));
	UInputMappingContext* menuMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Samples/VRSample/Input/IMC_Menu.IMC_Menu'"));
	UInputAction* move_X = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_MoveX.IA_MoveX'"));
	UInputAction* move_Y = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_MoveY.IA_MoveY'"));
	UInputAction* move_Z = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_MoveUp.IA_MoveUp'"));
	UVRHandAnimInstance* rightAnimInstance;
	UAnimInstance* rightAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* rightHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* rightInteraction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* rightMotionController;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* rightMotionControllerInteractor;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	TEnumAsByte<EHMDTrackingOrigin::Type> TrackingOrigin;
	UInputAction* trigger_L = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_LeftTrigger.IA_LeftTrigger'"));
	UInputAction* trigger_R = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_RightTrigger.IA_RightTrigger'"));
	UInputAction* turn = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_Turn.IA_Turn'"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* vrCamera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USceneComponent* vrOrigin;
};
