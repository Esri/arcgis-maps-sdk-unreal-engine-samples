// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "IHeadMountedDisplay.h"
#include "InputCoreTypes.h"
#include "IXRTrackingSystem.h"
#include "InputMappingContext.h"
#include "MotionControllerComponent.h"
#include "VRHandAnimInstance.h"
#include "VRCharacterController.generated.h"

UCLASS()
class XR_SAMPLESPROJECT_API AVRCharacterController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacterController();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bUseSmoothTurn = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bMoveInLookDirection = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float MovementDeadzone = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float MoveSpeed = 10000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float RotationDeadzone = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float RotationSpeed = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float SnapRotationDegrees = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float UpSpeed = 100000.0f;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void InitializeCapsuleHeight();
	void MoveForward(const FInputActionValue& value);
	void MoveRight(const FInputActionValue& value);
	void MoveUp(const FInputActionValue& value);
	void ResetDoOnce();
	void SetCapsuleHeight();
	void SetLeftGripAxis(const FInputActionValue& value);
	void SetLeftTriggerAxis(const FInputActionValue& value);
	void SetRightGripAxis(const FInputActionValue& value);
	void SetRightTriggerAxis(const FInputActionValue& value);
	void ResetLeftGripAxis();
	void ResetLeftTriggerAxis();
	void ResetRightGripAxis();
	void ResetRightTriggerAxis();
	void SmoothTurn(const FInputActionValue& value);
	void SnapTurn(const FInputActionValue& value);
	void UpdateRoomScaleMovement();

	bool bDoOnce = true;
	float capsuleHalfHeight;
	UAnimBlueprint* handAnimBP = LoadObject<UAnimBlueprint>(nullptr, TEXT("/Game/Samples/VRSample/Hands/Animations/ABP_Hand.ABP_Hand"));
	USkeletalMesh* handMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_left.SKM_MannyXR_left"));
	UInputAction* grip_L = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_LeftGrip.IA_LeftGrip"));
	UInputAction* grip_R = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_RightGrip.IA_RightGrip"));
	UVRHandAnimInstance* leftAnimInstance;
	UAnimInstance* leftAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* leftMotionController;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* leftHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UArcGISLocationComponent* locationComponent;
	UInputMappingContext* mappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Samples/VRSample/Input/IMC_Default.IMC_Default"));
	UInputAction* move_X = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_MoveX.IA_MoveX"));
	UInputAction* move_Y = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_MoveY.IA_MoveY"));
	UInputAction* move_Z = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_MoveUp.IA_MoveUp"));
	UVRHandAnimInstance* rightAnimInstance;
	UAnimInstance* rightAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* rightHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UMotionControllerComponent* rightMotionController;
	UInputAction* trigger_L = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_LeftTrigger.IA_LeftTrigger"));
	UInputAction* trigger_R = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_RightTrigger.IA_RightTrigger"));
	UInputAction* turn = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/VRSample/Input/IA_Turn.IA_Turn'"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* vrCamera;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USceneComponent* vrOrigin;
};
