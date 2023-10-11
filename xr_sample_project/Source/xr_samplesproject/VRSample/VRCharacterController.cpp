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
	VRCamera->bUsePawnControlRotation = true;
	
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
	
	if (GetCharacterMovement()->IsFlying())
	{
		AddMovementInput(Direction, inputValue);
	}
	else
	{
		AddMovementInput(Direction, inputValue);
	}
}

void AVRCharacterController::MoveRight(const FInputActionValue& value)
{
	const auto inputValue = value.Get<float>();
	FRotator Rotation = Controller->GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
	FVector Direction = GetActorRightVector();
	
	if (GetCharacterMovement()->IsFlying())
	{
		AddMovementInput(Direction, inputValue);
	}
	else
	{
		AddMovementInput(Direction, inputValue);
	}
}

void AVRCharacterController::MoveUp(const FInputActionValue& value)
{
	auto inputValue = value.Get<float>();
	
	if (GetCharacterMovement()->IsFlying())
	{
		AddMovementInput(GetActorUpVector(), inputValue * 10000.0f);
	}
}

void AVRCharacterController::SmoothTurn(const FInputActionValue& value)
{
	auto InputValue = value.Get<float>();

	if(abs(InputValue) > TurnDeadZone)
	{
		const auto RotationAngle = SnapRotationDegrees;
		SetActorRotation(FRotator(0.0f, 0.0f, GetActorRotation().Yaw * RotationSpeed + RotationAngle));
	}
}  


void AVRCharacterController::SnapTurn(const FInputActionValue& value)
{
	auto InputValue = value.Get<float>();
	auto RotationAngle = 0.0f;
	
	if(abs(InputValue) > TurnDeadZone)
	{
		RotationAngle = SnapRotationDegrees;
		SetActorRotation(FRotator(0.0f, 0.0f, GetActorRotation().Yaw + RotationAngle));
	}
	else
	{
		RotationAngle = SnapRotationDegrees * -1.0f;
		SetActorRotation(FRotator(0.0f, 0.0f, GetActorRotation().Yaw + RotationAngle));
	}
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

