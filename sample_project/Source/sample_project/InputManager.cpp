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
		EnhancedInputComponent->BindAction(mousePress, ETriggerEvent::Started, this, &AInputManager::TriggerInput);
		//EnhancedInputComponent->BindAction(ShiftModifier, ETriggerEvent::Started, this, &AInputManager::OnShiftPressed);
		//EnhancedInputComponent->BindAction(ShiftModifier, ETriggerEvent::Canceled, this, &AInputManager::OnShiftReleased);
	}
}

void AInputManager::TriggerInput()
{
	OnInputTrigger.Broadcast();
}
/*
void AInputManager::OnShiftPressed()
{
	if (UArcGISCameraComponent* Camera = FindCameraComponent())
	{
		Camera->SetComponentTickEnabled(false);
	}
}

void AInputManager::OnShiftReleased()
{
	if (UArcGISCameraComponent* Camera = FindCameraComponent())
	{
		Camera->SetComponentTickEnabled(true);
	}
}

UArcGISCameraComponent* AInputManager::FindCameraComponent()
{
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors);

	for (AActor* Actor : AllActors)
	{
		if (UArcGISCameraComponent* CameraComp = Actor->FindComponentByClass<UArcGISCameraComponent>())
		{
			return CameraComp;
		}
	}
	return nullptr;
}*/
