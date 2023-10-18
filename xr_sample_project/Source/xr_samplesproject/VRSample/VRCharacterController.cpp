// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacterController.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AVRCharacterController::AVRCharacterController()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	vrOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	vrOrigin->SetupAttachment(RootComponent);
	vrCamera = CreateDefaultSubobject<UArcGISCameraComponent>(TEXT("FollowCamera"));
	vrCamera->SetupAttachment(vrOrigin);
	vrCamera->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	
	locationComponent = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("Location Component"));
	locationComponent->SetupAttachment(RootComponent);

	leftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	leftMotionController->SetupAttachment(vrOrigin);
	leftMotionController->SetTrackingSource(EControllerHand::Left);
	leftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
	leftHandMesh->SetupAttachment(leftMotionController);
	leftHandMesh->RegisterComponent();
	leftHandMesh->SetSkeletalMesh(handMesh);
	leftHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	leftHandMesh->SetAnimClass(handAnimBP->GeneratedClass);
	
	rightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	rightMotionController->SetupAttachment(vrOrigin);
	rightMotionController->SetTrackingSource(EControllerHand::Right);
	rightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
	rightHandMesh->SetupAttachment(rightMotionController);
	rightHandMesh->RegisterComponent();
	rightHandMesh->SetSkeletalMesh(handMesh);
	rightHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	rightHandMesh->SetAnimClass(handAnimBP->GeneratedClass);
}

void AVRCharacterController::MoveForward(const FInputActionValue& value)
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
			FRotator Rotation = Controller->GetControlRotation();
			FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
			FVector Direction = GetActorForwardVector();
			AddMovementInput(Direction, inputValue * MoveSpeed);
		}
	}
}

void AVRCharacterController::MoveRight(const FInputActionValue& value)
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
			FRotator Rotation = Controller->GetControlRotation();
			FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
			FVector Direction = GetActorRightVector();
			AddMovementInput(Direction, inputValue * MoveSpeed);
		}
	}
}

void AVRCharacterController::MoveUp(const FInputActionValue& value)
{
	auto inputValue = value.Get<float>();
	
	if (abs(inputValue) > MovementDeadzone)
	{
		AddMovementInput(GetActorUpVector(), inputValue * UpSpeed);
	}
}

void AVRCharacterController::SmoothTurn(const FInputActionValue& value)
{
	auto inputValue = value.Get<float>() * RotationSpeed;

	if (abs(inputValue) > RotationDeadzone)
	{
		SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw + inputValue, 0.0f));
	}
}

void AVRCharacterController::SnapTurn(const FInputActionValue& value)
{
	auto inputValue = value.Get<float>();

	if(bDoOnce && abs(inputValue) > RotationDeadzone)
	{
		auto RotationAngle = 0.0f;
		if(inputValue > 0.0f)
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

void AVRCharacterController::ResetDoOnce()
{
	if(!bDoOnce)
	{
		bDoOnce = true;
	}
}

void AVRCharacterController::InitializeCapsuleHeight() 
{
	capsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

void AVRCharacterController::SetCapsuleHeight() 
{
	FVector DevicePosition;
	FQuat DeviceRotation;
	GEngine->XRSystem->GetCurrentPose(IXRTrackingSystem::HMDDeviceId, DeviceRotation, DevicePosition);
	GetCapsuleComponent()->SetCapsuleSize(GetCapsuleComponent()->GetScaledCapsuleRadius(), capsuleHalfHeight, true);
	auto halfHeight = DevicePosition.Z / 2.0f + 10.0f;
	vrOrigin->AddRelativeLocation(FVector(0.0f, 0.0f, capsuleHalfHeight - halfHeight));
	capsuleHalfHeight = halfHeight;
}

void AVRCharacterController::SetLeftGripAxis(const FInputActionValue& value)
{
	if (leftAnimInstance)
	{
		leftAnimInstance->GripAxis = value.Get<float>();
	}
}

void AVRCharacterController::SetRightGripAxis(const FInputActionValue& value)
{
	if (rightAnimInstance)
	{
		rightAnimInstance->GripAxis = value.Get<float>();
	}
}

void AVRCharacterController::SetLeftTriggerAxis(const FInputActionValue& value)
{
	if (leftAnimInstance)
	{
		leftAnimInstance->TriggerAxis = value.Get<float>();
	}
}

void AVRCharacterController::SetRightTriggerAxis(const FInputActionValue& value)
{	
	if (rightAnimInstance)
	{
		rightAnimInstance->TriggerAxis = value.Get<float>();
	}
}

void AVRCharacterController::UpdateRoomScaleMovement()
{
	FVector Offset = vrCamera->GetComponentLocation() - GetActorLocation();
	AddActorWorldOffset(FVector(Offset.X, Offset.Y, 0.0f));
	vrOrigin->AddWorldOffset(UKismetMathLibrary::NegateVector(FVector(Offset.X, Offset.Y, 0.0f)));
}

// Called when the game starts or when spawned
void AVRCharacterController::BeginPlay()
{
	Super::BeginPlay();

	GEngine->XRSystem->SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying, 0);
	InitializeCapsuleHeight();
	
	leftAnimInstanceBase = leftHandMesh->GetAnimInstance();
	leftAnimInstance = Cast<UVRHandAnimInstance>(leftAnimInstanceBase);
	rightAnimInstanceBase = rightHandMesh->GetAnimInstance();
	rightAnimInstance = Cast<UVRHandAnimInstance>(rightAnimInstanceBase);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(mappingContext, 0);
		}
	}

	auto DeviceType = GEngine->XRSystem->GetHMDDevice()->GetHMDName().ToString();
	UE_LOG(LogTemp, Warning, TEXT("Device Type: %s"), *DeviceType);
	FTimerHandle UpdateHeight;
	GetWorldTimerManager().SetTimer(UpdateHeight, this, &AVRCharacterController::SetCapsuleHeight, 0.35f, true);
	FTimerHandle UpdateCapsuleLocation;
	GetWorldTimerManager().SetTimer(UpdateCapsuleLocation, this, &AVRCharacterController::UpdateRoomScaleMovement, 0.3f, true);
}

// Called every frame
void AVRCharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetCharacterMovement()->Velocity.Size() > 1000)
	{
		vrCamera->PostProcessSettings.WeightedBlendables.Array[0].Weight = 1.0;
	}
	else 
	{
		vrCamera->PostProcessSettings.WeightedBlendables.Array[0].Weight = 0.0;
	}

}

// Called to bind functionality to input
void AVRCharacterController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(move_X, ETriggerEvent::Triggered, this, &AVRCharacterController::MoveRight);
		EnhancedInputComponent->BindAction(move_Y, ETriggerEvent::Triggered, this, &AVRCharacterController::MoveForward);
		EnhancedInputComponent->BindAction(move_Z, ETriggerEvent::Triggered, this, &AVRCharacterController::MoveUp);
		EnhancedInputComponent->BindAction(grip_L, ETriggerEvent::Triggered, this, &AVRCharacterController::SetLeftGripAxis);
		EnhancedInputComponent->BindAction(grip_R, ETriggerEvent::Triggered, this, &AVRCharacterController::SetRightGripAxis);
		EnhancedInputComponent->BindAction(trigger_L, ETriggerEvent::Triggered, this, &AVRCharacterController::SetLeftTriggerAxis);
		EnhancedInputComponent->BindAction(trigger_R, ETriggerEvent::Triggered, this, &AVRCharacterController::SetRightTriggerAxis);

		if(bUseSmoothTurn)
		{
			EnhancedInputComponent->BindAction(turn, ETriggerEvent::Triggered, this, &AVRCharacterController::SmoothTurn);
		}
		else
		{
			EnhancedInputComponent->BindAction(turn, ETriggerEvent::Triggered, this, &AVRCharacterController::SnapTurn);
			EnhancedInputComponent->BindAction(turn, ETriggerEvent::Completed, this, &AVRCharacterController::ResetDoOnce);
		}
	}
}