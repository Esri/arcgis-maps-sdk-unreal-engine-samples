/* Copyright 2022 Esri
*
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "ThirdPersonCharacter.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/CapsuleComponent.h"


AThirdPersonCharacter::AThirdPersonCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 0.0f, 360.0f);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UArcGISCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
}

void AThirdPersonCharacter::BeginPlay()
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

void AThirdPersonCharacter::JumpActionEvent(const FInputActionValue& value)
{
	Jump();
}

void AThirdPersonCharacter::StopJumpActionEvent(const FInputActionValue& value)
{
	StopJumping();
}

void AThirdPersonCharacter::SetCameraBoomSettings()
{
	if (GetCharacterMovement()->IsFlying())
	{
		CameraBoom->SocketOffset.Z = 75.0f;
		CameraBoom->bEnableCameraLag = true;
	}
	else
	{
		CameraBoom->SocketOffset.Z = 0.0f;
		CameraBoom->bEnableCameraLag = false;
	}
}


void AThirdPersonCharacter::StartFlying(const FInputActionValue& value)
{
	if (!GetCharacterMovement()->IsFlying())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Flying, 0);
	}
	SetCameraBoomSettings();
}

void AThirdPersonCharacter::StopFlying(const FInputActionValue& value)
{
	if (GetCharacterMovement()->IsFlying())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking, 0);
	}
	SetCameraBoomSettings();
}

void AThirdPersonCharacter::Look(const FInputActionValue& value)
{
	FVector2D lookAxisValue = value.Get<FVector2D>();
	
	if (GetController())
	{
		AddControllerYawInput(lookAxisValue.X);
		AddControllerPitchInput(-lookAxisValue.Y);
	}
}

void AThirdPersonCharacter::MoveForward(const FInputActionValue& value)
{
	const float inputValue = value.Get<float>();
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

void AThirdPersonCharacter::MoveRight(const FInputActionValue& value)
{
	float inputValue = value.Get<float>();
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

void AThirdPersonCharacter::MoveUp(const FInputActionValue& value)
{
	float inputValue = value.Get<float>();
	
	if (GetCharacterMovement()->IsFlying())
	{
		AddMovementInput(GetActorUpVector(), inputValue * 10000.0f);
	}
}

void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::Look);
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::MoveForward);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::MoveRight);
		EnhancedInputComponent->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::MoveUp);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AThirdPersonCharacter::JumpActionEvent);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AThirdPersonCharacter::StopJumpActionEvent);
		EnhancedInputComponent->BindAction(StartFlyingAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::StartFlying);
		EnhancedInputComponent->BindAction(StopFlyingAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::StopFlying);
	}
}
