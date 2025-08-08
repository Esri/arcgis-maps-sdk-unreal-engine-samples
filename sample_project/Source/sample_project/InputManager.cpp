// /* Copyright 2025 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */


#include "InputManager.h"

// Sets default values
AInputManager::AInputManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AInputManager::BeginPlay()
{
	Super::BeginPlay();

	MappingContext = LoadObject<UInputMappingContext>(
		nullptr, TEXT("/Script/EnhancedInput.InputMappingContext'/Game/SampleViewer/SharedResources/Input/IAC_SamplesInput.IAC_SamplesInput'"));

	MousePress = LoadObject<UInputAction>(
		nullptr, TEXT("/Script/EnhancedInput.InputAction'/Game/SampleViewer/SharedResources/Input/IA_LeftMouseClick.IA_LeftMouseClick'"));

	ShiftModifier = LoadObject<UInputAction>(
		nullptr, TEXT("/Script/EnhancedInput.InputAction'/Game/SampleViewer/SharedResources/Input/IA_Shift.IA_Shift'"));

	SpacePress = LoadObject<UInputAction>(
		nullptr, TEXT("/Script/EnhancedInput.InputAction'/Game/SampleViewer/SharedResources/Input/IA_Space.IA_Space'"));

	if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;

		SetupPlayerInputComponent(PlayerController->InputComponent);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		EnableInput(PC);
	}
}

void AInputManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(MappingContext);
		}
	}
}

void AInputManager::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MousePress, ETriggerEvent::Started, this, &AInputManager::TriggerInputStart);
		EnhancedInputComponent->BindAction(MousePress, ETriggerEvent::Completed, this, &AInputManager::TriggerInputEnd);
		EnhancedInputComponent->BindAction(ShiftModifier, ETriggerEvent::Started, this, &AInputManager::OnShiftPressed);
		EnhancedInputComponent->BindAction(ShiftModifier, ETriggerEvent::Completed, this, &AInputManager::OnShiftReleased);
		EnhancedInputComponent->BindAction(SpacePress, ETriggerEvent::Started, this, &AInputManager::TriggerSwitchMode);
	}
}

void AInputManager::TriggerInputStart()
{
	OnInputTrigger.Broadcast();
}

void AInputManager::TriggerInputEnd()
{
	OnInputEnd.Broadcast();
}

void AInputManager::OnShiftPressed()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		PC->GetPawn()->DisableInput(PC);
	}
}

void AInputManager::OnShiftReleased()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC && PC->GetPawn())
	{
		PC->GetPawn()->EnableInput(PC);
	}
}

void AInputManager::TriggerSwitchMode()
{
	OnSwitchMode.Broadcast(); 
}
