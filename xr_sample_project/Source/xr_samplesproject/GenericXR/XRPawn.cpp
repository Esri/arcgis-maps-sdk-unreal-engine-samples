// Fill out your copyright notice in the Description page of Project Settings.


#include "XRPawn.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/KismetMathLibrary.h"
#include "MotionControllerComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "xr_samplesproject/VRSample/VRHandAnimInstance.h"

// Sets default values
AXRPawn::AXRPawn()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	vrOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	vrOrigin->SetupAttachment(RootComponent);
	vrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	vrCamera->SetupAttachment(vrOrigin);
	vrCamera->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	
	leftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	leftMotionController->SetupAttachment(vrOrigin);
	leftMotionController->SetTrackingSource(EControllerHand::Left);
	leftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
	leftHandMesh->SetupAttachment(leftMotionController);

	rightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	rightMotionController->SetupAttachment(vrOrigin);
	rightMotionController->SetTrackingSource(EControllerHand::Right);
	rightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
	rightHandMesh->SetupAttachment(rightMotionController);

	leftMotionControllerInteractor = CreateDefaultSubobject<UMotionControllerComponent>(
		TEXT("LeftMotionControllerInteractor"));
	leftMotionControllerInteractor->SetupAttachment(vrOrigin);
	leftMotionControllerInteractor->SetTrackingSource(EControllerHand::Left);
	leftInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Left Interactor"));
	leftInteraction->SetupAttachment(leftMotionControllerInteractor);

	rightMotionControllerInteractor = CreateDefaultSubobject<UMotionControllerComponent>(
		TEXT("RightMotionControllerInteractor"));
	rightMotionControllerInteractor->SetupAttachment(vrOrigin);
	rightMotionControllerInteractor->SetTrackingSource(EControllerHand::Right);
	rightInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Right Interactor"));
	rightInteraction->SetupAttachment(rightMotionControllerInteractor);
}

void AXRPawn::BeginPlay()
{
	Super::BeginPlay();

	leftHandMesh->SetSkeletalMesh(handMesh);
	leftHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	leftAnimInstanceBase = leftHandMesh->GetAnimInstance();
	leftAnimInstance = Cast<UVRHandAnimInstance>(leftAnimInstanceBase);
	rightHandMesh->SetSkeletalMesh(handMesh);
	rightHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	rightAnimInstanceBase = rightHandMesh->GetAnimInstance();
	rightAnimInstance = Cast<UVRHandAnimInstance>(rightAnimInstanceBase);
	
	GEngine->XRSystem->SetTrackingOrigin(TrackingOrigin);
	GetCharacterMovement()->SetMovementMode(MOVE_Walking, 0);
	InitializeCapsuleHeight();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(mappingContext, 0);
			Subsystem->AddMappingContext(menuMappingContext, 0);
		}
	}

	FTimerHandle UpdateHeight;
	GetWorldTimerManager().SetTimer(UpdateHeight, this, &AXRPawn::SetCapsuleHeight, 0.35f, true);
	FTimerHandle UpdateCapsuleLocation;
	GetWorldTimerManager().SetTimer(UpdateCapsuleLocation, this, &AXRPawn::UpdateRoomScaleMovement, 0.3f, true);

}

void AXRPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(move_X, ETriggerEvent::Triggered, this, &AXRPawn::MoveRight);
		EnhancedInputComponent->BindAction(move_Y, ETriggerEvent::Triggered, this, &AXRPawn::MoveForward);
		//Update Hand Animations
		EnhancedInputComponent->BindAction(grip_L, ETriggerEvent::Triggered, this, &AXRPawn::SetLeftGripAxis);
		EnhancedInputComponent->BindAction(grip_R, ETriggerEvent::Triggered, this, &AXRPawn::SetRightGripAxis);
		EnhancedInputComponent->BindAction(trigger_L, ETriggerEvent::Triggered, this, &AXRPawn::SetLeftTriggerAxis);
		EnhancedInputComponent->BindAction(trigger_R, ETriggerEvent::Triggered, this, &AXRPawn::SetRightTriggerAxis);
		//Reset Hand Animations on Cancelled Action
		EnhancedInputComponent->BindAction(grip_L, ETriggerEvent::Canceled, this, &AXRPawn::ResetLeftGripAxis);
		EnhancedInputComponent->BindAction(grip_R, ETriggerEvent::Canceled, this, &AXRPawn::ResetRightGripAxis);
		EnhancedInputComponent->BindAction(trigger_L, ETriggerEvent::Canceled, this, &AXRPawn::ResetLeftTriggerAxis);
		EnhancedInputComponent->BindAction(trigger_R, ETriggerEvent::Canceled, this, &AXRPawn::ResetRightTriggerAxis);
		//Reset Hand Animations on Completed Action
		EnhancedInputComponent->BindAction(grip_L, ETriggerEvent::Completed, this, &AXRPawn::ResetLeftGripAxis);
		EnhancedInputComponent->BindAction(grip_R, ETriggerEvent::Completed, this, &AXRPawn::ResetRightGripAxis);
		EnhancedInputComponent->BindAction(trigger_L, ETriggerEvent::Completed, this, &AXRPawn::ResetLeftTriggerAxis);
		EnhancedInputComponent->BindAction(trigger_R, ETriggerEvent::Completed, this, &AXRPawn::ResetRightTriggerAxis);

		//Simulate Mouse Click
		EnhancedInputComponent->BindAction(clickLeft, ETriggerEvent::Started, this, &AXRPawn::SimulateClickLeft);
		EnhancedInputComponent->BindAction(clickLeft, ETriggerEvent::Canceled, this, &AXRPawn::ResetClickLeft);
		EnhancedInputComponent->BindAction(clickLeft, ETriggerEvent::Completed, this, &AXRPawn::ResetClickLeft);
		EnhancedInputComponent->BindAction(clickRight, ETriggerEvent::Started, this, &AXRPawn::SimulateClickRight);
		EnhancedInputComponent->BindAction(clickRight, ETriggerEvent::Canceled, this, &AXRPawn::ResetClickRight);
		EnhancedInputComponent->BindAction(clickRight, ETriggerEvent::Completed, this, &AXRPawn::ResetClickRight);

		EnhancedInputComponent->BindAction(turn, ETriggerEvent::Triggered, this, &AXRPawn::SmoothTurn);
		EnhancedInputComponent->BindAction(turn, ETriggerEvent::Completed, this, &AXRPawn::ResetDoOnce);
	}
}

void AXRPawn::MoveForward(const FInputActionValue& value)
{
	const auto inputValue = value.Get<float>();

	if (abs(inputValue) > MovementDeadzone)
	{
		if (bMoveInLookDirection)
		{
			FVector Direction = vrCamera->GetForwardVector();
			AddMovementInput(Direction, inputValue * MoveSpeed);
		}
		else
		{
			FVector direction = rightMotionController->GetForwardVector();
			AddMovementInput(direction, inputValue * MoveSpeed);
		}
	}
}

void AXRPawn::MoveRight(const FInputActionValue& value)
{
	const auto inputValue = value.Get<float>();

	if (abs(inputValue) > MovementDeadzone)
	{
		if (bMoveInLookDirection)
		{
			FVector Direction = vrCamera->GetRightVector();
			AddMovementInput(Direction, inputValue * MoveSpeed);
		}
		else
		{
			FVector direction = rightMotionController->GetRightVector();
			AddMovementInput(direction, inputValue * MoveSpeed);
		}
	}
}

void AXRPawn::SmoothTurn(const FInputActionValue& value)
{
	auto inputValue = value.Get<float>() * RotationSpeed;
	
	if (bUseSmoothTurn)
	{
		if (abs(inputValue) > RotationDeadzone)
		{
			SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw + inputValue, 0.0f));
		}
	}
	else
	{
		if (bDoOnce && abs(inputValue) > RotationDeadzone)
		{
			auto RotationAngle = 0.0f;
			if (inputValue > 0.0f)
			{
				RotationAngle = SnapRotationDegrees;
				SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw + RotationAngle, 0.0f));
			}
			else
			{
				RotationAngle = SnapRotationDegrees * -1.0f;
				SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw + RotationAngle, 0.0f));
			}
			bDoOnce = false;
		}
	}
}

void AXRPawn::ResetDoOnce()
{
	if (!bDoOnce)
	{
		bDoOnce = true;
	}
}

void AXRPawn::InitializeCapsuleHeight()
{
	capsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void AXRPawn::SetCapsuleHeight()
{
	FVector DevicePosition;
	FQuat DeviceRotation;
	GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, DeviceRotation, DevicePosition);
	GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetScaledCapsuleRadius(), capsuleHalfHeight, true);
	const auto halfHeight = DevicePosition.Z / 2.0f + 10.0f;
	vrOrigin->AddRelativeLocation(FVector(0.0f, 0.0f, capsuleHalfHeight - halfHeight));
	capsuleHalfHeight = halfHeight;
}

void AXRPawn::SetLeftGripAxis(const FInputActionValue& value)
{
	if (leftAnimInstance)
	{
		leftAnimInstance->GripAxis = value.Get<float>();
	}
}

void AXRPawn::SetRightGripAxis(const FInputActionValue& value)
{
	if (rightAnimInstance)
	{
		rightAnimInstance->GripAxis = value.Get<float>();
	}
}

void AXRPawn::SetLeftTriggerAxis(const FInputActionValue& value)
{
	if (leftAnimInstance)
	{
		leftAnimInstance->TriggerAxis = value.Get<float>();
	}
}

void AXRPawn::SetRightTriggerAxis(const FInputActionValue& value)
{
	if (rightAnimInstance)
	{
		rightAnimInstance->TriggerAxis = value.Get<float>();
	}
}

void AXRPawn::ResetLeftGripAxis()
{
	if (leftAnimInstance)
	{
		leftAnimInstance->GripAxis = 0.0f;
	}
}

void AXRPawn::ResetRightGripAxis()
{
	if (rightAnimInstance)
	{
		rightAnimInstance->GripAxis = 0.0f;
	}
}

void AXRPawn::ResetLeftTriggerAxis()
{
	if (leftAnimInstance)
	{
		leftAnimInstance->TriggerAxis = 0.0f;
	}
}

void AXRPawn::ResetRightTriggerAxis()
{
	if (rightAnimInstance)
	{
		rightAnimInstance->TriggerAxis = 0.0f;
	}
}

void AXRPawn::SimulateClickLeft()
{
	leftInteraction->PressPointerKey(EKeys::LeftMouseButton);
}

void AXRPawn::SimulateClickRight()
{
	if (rightInteraction->IsOverFocusableWidget())
	{
		rightInteraction->PressPointerKey(EKeys::LeftMouseButton);
	}
}

void AXRPawn::ResetClickLeft()
{
	leftInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}

void AXRPawn::ResetClickRight()
{
	rightInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}

void AXRPawn::UpdateRoomScaleMovement()
{
	FVector Offset = vrCamera->GetComponentLocation() - GetActorLocation();
	AddActorWorldOffset(FVector(Offset.X, Offset.Y, 0.0f));
	vrOrigin->AddWorldOffset(UKismetMathLibrary::NegateVector(FVector(Offset.X, Offset.Y, 0.0f)));
}
