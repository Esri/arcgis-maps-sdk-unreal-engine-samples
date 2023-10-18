// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "IHeadMountedDisplay.h"
#include "InputCoreTypes.h"
#include "IXRTrackingSystem.h"
#include "InputMappingContext.h"
#include "VRHand.h"
#include "VRCharacterController.generated.h"

UCLASS()
class XR_SAMPLESPROJECT_API AVRCharacterController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacterController();

	TSubclassOf<AVRHand> LeftHandClass;
	TSubclassOf<AVRHand> RightHandClass;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	AVRHand* lefthand;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	AVRHand* rightHand;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UArcGISCameraComponent* VRCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UArcGISLocationComponent* LocationComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USceneComponent* VROrigin;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputMappingContext* MappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/Samples/VRSample/Input/IMC_Default.IMC_Default"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* Move_X = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_MoveX.IA_MoveX"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* Move_Y = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_MoveY.IA_MoveY"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* Move_Z = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_MoveUp.IA_MoveUp"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enhanced Input")
	UInputAction* Turn = LoadObject<UInputAction>(nullptr, TEXT("/Game/Samples/VRSample/Input/IA_Turn.IA_Turn"));
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bUseSmoothTurn = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	bool bMoveInLookDirection = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float MovementDeadzone = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float MoveSpeed = 10000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float RotationDeadzone = 0.2f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float RotationSpeed = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float SnapRotationDegrees = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float UpSpeed = 100000.0f;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void InitializeCapsuleHeight();
	void MoveForward(const FInputActionValue& value);
	void MoveRight(const FInputActionValue& value);
	void MoveUp(const FInputActionValue& value);
	void SmoothTurn(const FInputActionValue& value);
	void SnapTurn(const FInputActionValue& value);
	void ResetDoOnce();
	void SetCapsuleHeight();
	void UpdateRoomScaleMovement();

	bool bDoOnce = true;
	float capsuleHalfHeight;
};
