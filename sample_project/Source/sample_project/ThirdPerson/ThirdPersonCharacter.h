// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputSubsystems.h"
#include "ThirdPersonCharacter.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AThirdPersonCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AThirdPersonCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		USpringArmComponent* CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		UArcGISCameraComponent* FollowCamera;
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
		UInputAction* ShowMouse;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int jumpCount;

	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void JumpActionEvent(const FInputActionValue& value);
	void StopJumpActionEvent(const FInputActionValue& value);
	void Look(const FInputActionValue& value);
	void MoveUp(const FInputActionValue& value);
	void MoveForward(const FInputActionValue& value);
	void MoveRight(const FInputActionValue& value);
	void ShowCursor(const FInputActionValue& value);
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
