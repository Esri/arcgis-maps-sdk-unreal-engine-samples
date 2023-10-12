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
	FRotator Rotation = Controller->GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
	FVector Direction = GetActorForwardVector();

	AddMovementInput(Direction, inputValue * 1000.0f);
}

void AVRCharacterController::MoveRight(const FInputActionValue& value)
{
	const auto inputValue = value.Get<float>();
	FRotator Rotation = Controller->GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
	FVector Direction = GetActorRightVector();
	AddMovementInput(Direction, inputValue * MoveSpeed);
}

void AVRCharacterController::MoveUp(const FInputActionValue& value)
{
	auto inputValue = value.Get<float>();
	if (abs(inputValue) > 0.2f) 
	{
		AddMovementInput(GetActorUpVector(), inputValue * UpSpeed);
	}
}

void AVRCharacterController::SmoothTurn(const FInputActionValue& value)
{
	auto InputValue = value.Get<float>() * RotationSpeed;
	
	if(abs(InputValue) > TurnDeadZone)
	{
		SetActorRotation(FRotator(0.0f, GetActorRotation().Yaw + InputValue, 0.0f));
	}
}  

void AVRCharacterController::SnapTurn(const FInputActionValue& value)
{
	auto InputValue = value.Get<float>();
	auto RotationAngle = 0.0f;
	FTimerHandle RotateHandle;

	if(abs(InputValue) > TurnDeadZone)
	{
		RotationAngle = SnapRotationDegrees;
		FTimerDelegate TimerDel;
		TimerDel.BindLambda(this, SnapTurning, RotationAngle);
		GetWorldTimerManager().SetTimer(RotateHandle, TimerDel, 0.2f, true);
	}
	else
	{
		RotationAngle = SnapRotationDegrees * -1.0f;
		FTimerDelegate TimerDel;
		TimerDel.BindLambda(this, &AVRCharacterController::SnapTurning, RotationAngle);
		GetWorldTimerManager().SetTimer(RotateHandle, TimerDel, 0.2f, true);
	}
}

void AVRCharacterController::SnapTurning(float value)
{
	AddActorLocalRotation(FRotator(0.0f, GetActorRotation().Yaw + value, 0.0f));
}

void AVRCharacterController::UpdateRoomScaleMovement()
{
	FVector Offset = VRCamera->GetComponentLocation() - VROrigin->GetComponentLocation();
	AddActorWorldOffset(FVector(Offset.X, Offset.Y, 0.0f));
	VROrigin->AddWorldOffset(UKismetMathLibrary::NegateVector(FVector(Offset.X, Offset.Y, 0.0f)));
}


// Called when the game starts or when spawned
void AVRCharacterController::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->SetMovementMode(MOVE_Flying, 0);
	
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	FTimerHandle UpdateHeight;
	GetWorldTimerManager().SetTimer(UpdateHeight, this, &AVRCharacterController::UpdateRoomScaleMovement, 0.3f, true);
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
		}
	}

}

