// /* Copyright 2025 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "GameFramework/Actor.h"
#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "Kismet/GameplayStatics.h"

#include "InputManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInputTrigger);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInputEnd);

UCLASS()
class SAMPLE_PROJECT_API AInputManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInputManager();

	UPROPERTY(BlueprintAssignable)
	FOnInputTrigger OnInputTrigger;

	UPROPERTY(BlueprintAssignable)
	FOnInputEnd OnInputEnd;
	
	UFUNCTION()
	void OnShiftPressed();

	UFUNCTION()
	void OnShiftReleased();

	UFUNCTION()
	void TriggerInputStart(); 

	UFUNCTION()
	void TriggerInputEnd();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

private:
	//void TriggerInput();

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	UInputMappingContext* MappingContext;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	UInputAction* MousePress;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	UInputAction* ShiftModifier;
};
