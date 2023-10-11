// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "MotionControllerComponent.h"
#include "GameFramework/Character.h"
#include "VRCharacterController.generated.h"

UCLASS()
class XR_SAMPLESPROJECT_API AVRCharacterController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacterController();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UArcGISCameraComponent* VRCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UArcGISLocationComponent* LocationComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UMotionControllerComponent* LeftMotionController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* LeftHandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* RightHandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UMotionControllerComponent* RightMotionController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* VROrigin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputMappingContext* MappingContext;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void MoveForward(const FInputActionValue& value);
	void MoveRight(const FInputActionValue& value);
	void MoveUp(const FInputActionValue& value);
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
