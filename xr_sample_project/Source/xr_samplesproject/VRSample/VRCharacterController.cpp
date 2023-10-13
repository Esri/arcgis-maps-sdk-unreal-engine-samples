// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacterController.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
AVRCharacterController::AVRCharacterController()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	VROrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	VROrigin->SetupAttachment(RootComponent);
	VRCamera = CreateDefaultSubobject<UArcGISCameraComponent>(TEXT("FollowCamera"));
	VRCamera->SetupAttachment(VROrigin);
	VRCamera->SetRelativeLocation(FVector(0.0f, 0.0f, -90.0f));
	
	LocationComponent = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("Location Component"));
	LocationComponent->SetupAttachment(RootComponent);

	LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	LeftMotionController->SetupAttachment(VROrigin);
	LeftMotionController->SetTrackingSource(EControllerHand::Left);
	LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
	LeftHandMesh->SetupAttachment(LeftMotionController);
	LeftHandMesh->RegisterComponent();
	LeftHandMesh->SetSkeletalMesh(LeftMesh);
	LeftHandMesh->SetRelativeRotation(FRotator(-90.0f, -90.0f, 0.0f));
	
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	RightMotionController->SetupAttachment(VROrigin);
	RightMotionController->SetTrackingSource(EControllerHand::Right);
	RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
	RightHandMesh->SetupAttachment(RightMotionController);
	RightHandMesh->RegisterComponent();
	RightHandMesh->SetSkeletalMesh(RightMesh);
	RightHandMesh->SetRelativeRotation(FRotator(90.0f, -90.0f, 0.0f));  
}

void AVRCharacterController::MoveForward(const FInputActionValue& value)
{
	const auto inputValue = value.Get<float>();
	
	if (abs(inputValue) > MovementDeadzone)
	{
		if (bMoveInLookDirection)
		{
			FVector Direction = VRCamera->GetForwardVector();
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
			FVector Direction = VRCamera->GetRightVector();
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
	auto InputValue = value.Get<float>() * RotationSpeed;
	
	if(abs(InputValue) > RotationDeadzone)
	{
		SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw + InputValue, 0.0f));
	}
}  

void AVRCharacterController::SnapTurn(const FInputActionValue& value)
{
	auto InputValue = value.Get<float>();

	if(bDoOnce && abs(InputValue) > RotationDeadzone)
	{
		auto RotationAngle = 0.0f;
		if(InputValue > 0.0f)
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
	VROrigin->AddRelativeLocation(FVector(0.0f, 0.0f, capsuleHalfHeight - halfHeight));
	capsuleHalfHeight = halfHeight;
}

void AVRCharacterController::UpdateRoomScaleMovement()
{
	FVector Offset = VRCamera->GetComponentLocation() - GetActorLocation();
	AddActorWorldOffset(FVector(Offset.X, Offset.Y, 0.0f));
	VROrigin->AddWorldOffset(UKismetMathLibrary::NegateVector(FVector(Offset.X, Offset.Y, 0.0f)));
}

// Called when the game starts or when spawned
void AVRCharacterController::BeginPlay()
{
	Super::BeginPlay();

	GEngine->XRSystem->SetTrackingOrigin(EHMDTrackingOrigin::Floor);
	GetCharacterMovement()->SetMovementMode(MOVE_Flying, 0);
	InitializeCapsuleHeight();
	FTimerHandle UpdateHeight;
	GetWorldTimerManager().SetTimer(UpdateHeight, this, &AVRCharacterController::SetCapsuleHeight, 0.35f, true);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	auto DeviceType = GEngine->XRSystem->GetHMDDevice()->GetHMDName().ToString();
	GEngine->AddOnScreenDebugMessage(1, 5.0f, FColor::Green, "Type: " + DeviceType);

	FTimerHandle UpdateCapsuleLocation;
	GetWorldTimerManager().SetTimer(UpdateCapsuleLocation, this, &AVRCharacterController::UpdateRoomScaleMovement, 0.3f, true);
}

// Called every frame
void AVRCharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AVRCharacterController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(Move_X, ETriggerEvent::Triggered, this, &AVRCharacterController::MoveRight);
		EnhancedInputComponent->BindAction(Move_Y, ETriggerEvent::Triggered, this, &AVRCharacterController::MoveForward);
		EnhancedInputComponent->BindAction(Move_Z, ETriggerEvent::Triggered, this, &AVRCharacterController::MoveUp);

		if(bUseSmoothTurn)
		{
			EnhancedInputComponent->BindAction(Turn, ETriggerEvent::Triggered, this, &AVRCharacterController::SmoothTurn);
		}
		else
		{
			EnhancedInputComponent->BindAction(Turn, ETriggerEvent::Triggered, this, &AVRCharacterController::SnapTurn);
			EnhancedInputComponent->BindAction(Turn, ETriggerEvent::Completed, this, &AVRCharacterController::ResetDoOnce);
		}
	}
}