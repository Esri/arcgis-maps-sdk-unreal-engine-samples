// Fill out your copyright notice in the Description page of Project Settings.


#include "ThirdPersonCharacter.h"

#include "Blueprint/WidgetBlueprintLibrary.h"

// Sets default values
AThirdPersonCharacter::AThirdPersonCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
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

// Called when the game starts or when spawned
void AThirdPersonCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if(APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if(UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}	
	}
}

void AThirdPersonCharacter::JumpActionEvent(const FInputActionValue& value)
{
	float time = 0.0f;
	bool isFlying = false;
	++jumpCount;
	if(jumpCount == 2)
	{
		if(!isFlying)
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Flying,0);
			isFlying = true;
			jumpCount = 0;
		}
		else
		{
			GetCharacterMovement()->SetMovementMode(MOVE_Walking,0);
			isFlying = false;
			jumpCount = 0;
		}
	}
	else
	{
		Jump();
		time += 0.1f;
		if(time >= 0.5f)
		{
			jumpCount = 0;
		}
	}
}
void AThirdPersonCharacter::StopJumpActionEvent(const FInputActionValue& value)
{
	StopJumping();
}


void AThirdPersonCharacter::Look(const FInputActionValue& value)
{
	FVector2D lookAxisValue = value.Get<FVector2D>();
	if(GetController())
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
	Direction = GetActorForwardVector();
	AddMovementInput(Direction, inputValue);
}

void AThirdPersonCharacter::MoveRight(const FInputActionValue& value)
{
	const float inputValue = value.Get<float>();
	FRotator Rotation = Controller->GetControlRotation();
	FRotator YawRotation(0.0f, Rotation.Yaw, 0.0f);
	FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	Direction = GetActorRightVector();
	AddMovementInput(Direction, inputValue);
}

void AThirdPersonCharacter::MoveUp(const FInputActionValue& value)
{
	if(GetCharacterMovement()->IsFlying())
	{
		AddMovementInput(GetActorUpVector(), value.Get<float>());	
	}
}


void AThirdPersonCharacter::ShowCursor(const FInputActionValue& value)
{
	if(value.Get<bool>())
	{
		ShowCursor(true);	
	}
}


// Called every frame
void AThirdPersonCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AThirdPersonCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if(UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::Look);
		EnhancedInputComponent->BindAction(MoveForwardAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::MoveForward);
		EnhancedInputComponent->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::MoveRight);
		EnhancedInputComponent->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &AThirdPersonCharacter::MoveUp);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AThirdPersonCharacter::JumpActionEvent);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AThirdPersonCharacter::StopJumpActionEvent);
		EnhancedInputComponent->BindAction(ShowMouse, ETriggerEvent::Canceled, this, &AThirdPersonCharacter::ShowCursor);
	}

}

