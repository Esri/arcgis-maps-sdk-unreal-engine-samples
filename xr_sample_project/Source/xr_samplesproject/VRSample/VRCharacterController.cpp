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
	vrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	vrCamera->SetupAttachment(vrOrigin);
	vrCamera->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	vrCamera->AddOrUpdateBlendable(LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/Samples/VRSample/M_vignette.M_vignette'")), 0.0f);
	springArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	springArmComponent->SetupAttachment(RootComponent);
	vrWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HUD"));
	vrWidget->SetupAttachment(springArmComponent);
	
	locationComponent = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("Location Component"));
	locationComponent->SetupAttachment(RootComponent);

	leftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	leftMotionController->SetupAttachment(vrOrigin);
	leftMotionController->SetTrackingSource(EControllerHand::Left);
	leftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
	leftHandMesh->SetupAttachment(leftMotionController);
	widgetLeft = CreateDefaultSubobject<UWidgetComponent>(TEXT("Left Helper"));
	widgetLeft->SetupAttachment(leftHandMesh);
	
	rightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	rightMotionController->SetupAttachment(vrOrigin);
	rightMotionController->SetTrackingSource(EControllerHand::Right);
	rightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
	rightHandMesh->SetupAttachment(rightMotionController);
	widgetRight = CreateDefaultSubobject<UWidgetComponent>(TEXT("Right Helper"));
	widgetRight->SetupAttachment(rightHandMesh);

	leftMotionControllerInteractor = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionControllerInteractor"));
	leftMotionControllerInteractor->SetupAttachment(vrOrigin);
	leftMotionControllerInteractor->SetTrackingSource(EControllerHand::Left);
	leftInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Left Interactor"));
	leftInteraction->SetupAttachment(leftMotionControllerInteractor);
	
	rightMotionControllerInteractor = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionControllerInteractor"));
	rightMotionControllerInteractor->SetupAttachment(vrOrigin);
	rightMotionControllerInteractor->SetTrackingSource(EControllerHand::Right);
	rightInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Right Interactor"));
	rightInteraction->SetupAttachment(rightMotionControllerInteractor);
}

void AVRCharacterController::ActivateMenu()
{
	if(vrWidget->bHiddenInGame)
	{
		vrWidget->SetHiddenInGame(false);
		vrWidget->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	else
	{
		vrWidget->SetHiddenInGame(true);
		vrWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AVRCharacterController::HideLeftMenu()
{
	if (widgetLeft->IsVisible()) 
	{
		widgetLeft->SetHiddenInGame(true);
	}
	else
	{
		widgetLeft->SetHiddenInGame(false);
	}
}

void AVRCharacterController::HideRightMenu()
{
	if (widgetRight->IsVisible())
	{
		widgetRight->SetHiddenInGame(true);
	}
	else
	{
		widgetRight->SetHiddenInGame(false);
	}
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
			FVector direction = rightMotionController->GetForwardVector();
			AddMovementInput(direction, inputValue * MoveSpeed);
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
			FVector direction = rightMotionController->GetRightVector();
			AddMovementInput(direction, inputValue * MoveSpeed);
		}
	}
}

void AVRCharacterController::MoveUp(const FInputActionValue& value)
{
	auto inputValue = value.Get<float>();
	
	if (abs(inputValue) > 0.5f)
	{
		AddMovementInput(GetActorUpVector(), inputValue * UpSpeed);
	}
}

void AVRCharacterController::SmoothTurn(const FInputActionValue& value)
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

void AVRCharacterController::ResetLeftGripAxis()
{
	if (leftAnimInstance)
	{
		leftAnimInstance->GripAxis = 0.0f;
	}
}

void AVRCharacterController::ResetRightGripAxis()
{
	if (rightAnimInstance)
	{
		rightAnimInstance->GripAxis = 0.0f;
	}
}

void AVRCharacterController::ResetLeftTriggerAxis()
{
	if (leftAnimInstance)
	{
		leftAnimInstance->TriggerAxis = 0.0f;
	}
}

void AVRCharacterController::ResetRightTriggerAxis()
{
	if (rightAnimInstance)
	{
		rightAnimInstance->TriggerAxis = 0.0f;
	}
}

void AVRCharacterController::SimulateClickLeft()
{
	leftInteraction->PressPointerKey(EKeys::LeftMouseButton);
}

void AVRCharacterController::SimulateClickRight()
{
	if(rightInteraction->IsOverInteractableWidget())
	{
		rightInteraction->PressPointerKey(EKeys::LeftMouseButton);	
	}
	else
	{
		if(GeoCoder != nullptr)
		{
			GeoCoder->SelectLocation(rightInteraction->GetComponentLocation(), rightInteraction->GetForwardVector());	
		}
	}
}

void AVRCharacterController::ResetClickLeft()
{
	leftInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}

void AVRCharacterController::ResetClickRight()
{
	rightInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
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
	
	leftHandMesh->SetSkeletalMesh(handMesh);
	leftHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	leftAnimInstanceBase = leftHandMesh->GetAnimInstance();
	leftAnimInstance = Cast<UVRHandAnimInstance>(leftAnimInstanceBase);
	rightHandMesh->SetSkeletalMesh(handMesh);
	rightHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	rightAnimInstanceBase = rightHandMesh->GetAnimInstance();
	rightAnimInstance = Cast<UVRHandAnimInstance>(rightAnimInstanceBase);
	
	GEngine->XRSystem->SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying, 0);
	InitializeCapsuleHeight();
	GeoCoder = Cast<AGeocoder>(UGameplayStatics::GetActorOfClass(GetWorld(), AGeocoder::StaticClass()));
	vrWidget->SetHiddenInGame(false);
	
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
	GetWorldTimerManager().SetTimer(UpdateHeight, this, &AVRCharacterController::SetCapsuleHeight, 0.35f, true);
	FTimerHandle UpdateCapsuleLocation;
	GetWorldTimerManager().SetTimer(UpdateCapsuleLocation, this, &AVRCharacterController::UpdateRoomScaleMovement, 0.3f, true);
}

// Called every frame
void AVRCharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetCharacterMovement()->Velocity.Size() > 100)
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
		//Update Hand Animations
		EnhancedInputComponent->BindAction(grip_L, ETriggerEvent::Triggered, this, &AVRCharacterController::SetLeftGripAxis);
		EnhancedInputComponent->BindAction(grip_R, ETriggerEvent::Triggered, this, &AVRCharacterController::SetRightGripAxis);
		EnhancedInputComponent->BindAction(trigger_L, ETriggerEvent::Triggered, this, &AVRCharacterController::SetLeftTriggerAxis);
		EnhancedInputComponent->BindAction(trigger_R, ETriggerEvent::Triggered, this, &AVRCharacterController::SetRightTriggerAxis);
		//Reset Hand Animations on Cancelled Action
		EnhancedInputComponent->BindAction(grip_L, ETriggerEvent::Canceled, this, &AVRCharacterController::ResetLeftGripAxis);
		EnhancedInputComponent->BindAction(grip_R, ETriggerEvent::Canceled, this, &AVRCharacterController::ResetRightGripAxis);
		EnhancedInputComponent->BindAction(trigger_L, ETriggerEvent::Canceled, this, &AVRCharacterController::ResetLeftTriggerAxis);
		EnhancedInputComponent->BindAction(trigger_R, ETriggerEvent::Canceled, this, &AVRCharacterController::ResetRightTriggerAxis);
		//Reset Hand Animations on Completed Action
		EnhancedInputComponent->BindAction(grip_L, ETriggerEvent::Completed, this, &AVRCharacterController::ResetLeftGripAxis);
		EnhancedInputComponent->BindAction(grip_R, ETriggerEvent::Completed, this, &AVRCharacterController::ResetRightGripAxis);
		EnhancedInputComponent->BindAction(trigger_L, ETriggerEvent::Completed, this, &AVRCharacterController::ResetLeftTriggerAxis);
		EnhancedInputComponent->BindAction(trigger_R, ETriggerEvent::Completed, this, &AVRCharacterController::ResetRightTriggerAxis);

		//Menu Activation
		EnhancedInputComponent->BindAction(menu_Left, ETriggerEvent::Started, this, &AVRCharacterController::ActivateMenu);
		EnhancedInputComponent->BindAction(menu_Right, ETriggerEvent::Started, this, &AVRCharacterController::ActivateMenu);

		//Simulate Mouse Click
		EnhancedInputComponent->BindAction(clickLeft, ETriggerEvent::Started, this, &AVRCharacterController::SimulateClickLeft);
		EnhancedInputComponent->BindAction(clickLeft, ETriggerEvent::Canceled, this, &AVRCharacterController::ResetClickLeft);
		EnhancedInputComponent->BindAction(clickLeft, ETriggerEvent::Completed, this, &AVRCharacterController::ResetClickLeft);
		EnhancedInputComponent->BindAction(clickRight, ETriggerEvent::Started, this, &AVRCharacterController::SimulateClickRight);
		EnhancedInputComponent->BindAction(clickRight, ETriggerEvent::Canceled, this, &AVRCharacterController::ResetClickRight);
		EnhancedInputComponent->BindAction(clickRight, ETriggerEvent::Completed, this, &AVRCharacterController::ResetClickRight);

		EnhancedInputComponent->BindAction(turn, ETriggerEvent::Triggered, this, &AVRCharacterController::SmoothTurn);
		EnhancedInputComponent->BindAction(turn, ETriggerEvent::Completed, this, &AVRCharacterController::ResetDoOnce);

		EnhancedInputComponent->BindAction(hideLeftMenu, ETriggerEvent::Started, this, &AVRCharacterController::HideLeftMenu);
		EnhancedInputComponent->BindAction(hideRightMenu, ETriggerEvent::Started, this, &AVRCharacterController::HideRightMenu);
	}
}