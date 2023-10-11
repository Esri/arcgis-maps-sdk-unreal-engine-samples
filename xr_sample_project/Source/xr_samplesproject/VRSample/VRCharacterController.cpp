// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacterController.h"

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
	LeftHandMesh->CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
	LeftHandMesh->SetupAttachment(LeftMotionController);
	
	RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	RightMotionController->SetupAttachment(VROrigin);
	RightMotionController->SetTrackingSource(EControllerHand::Right);
	RightHandMesh->CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
	RightHandMesh->SetupAttachment(RightMotionController);

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

// Called when the game starts or when spawned
void AVRCharacterController::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}
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
		//EnhancedInputComponent->BindAction();
	}

}

