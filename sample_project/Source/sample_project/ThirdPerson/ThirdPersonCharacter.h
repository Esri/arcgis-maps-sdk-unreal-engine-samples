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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ThirdPersonCharacter.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AThirdPersonCharacter();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UArcGISCameraComponent* FollowCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UArcGISLocationComponent* LocationComponent;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		UInputMappingContext* MappingContext;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		UInputAction* MoveForwardAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		UInputAction* MoveRightAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		 UInputAction* LookAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		UInputAction* MoveUpAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		UInputAction* StartFlyingAction;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
		UInputAction* SprintingAction;
private:
	bool sprinting;
	bool flying;
	
protected:
	virtual void BeginPlay() override;

private:	
	void SetCameraBoomSettings();
	void StartFlying(const FInputActionValue& value);
	void JumpActionEvent(const FInputActionValue& value);
	void StopJumpActionEvent(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	void MoveUp(const FInputActionValue& value);
	void MoveForward(const FInputActionValue& value);
	void MoveRight(const FInputActionValue& value);
	void Sprint(const FInputActionValue& value);

public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
};
