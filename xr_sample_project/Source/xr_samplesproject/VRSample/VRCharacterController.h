// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "MotionControllerComponent.h"
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
	USkeletalMesh* LeftMesh = LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_left.SKM_MannyXR_left"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMeshComponent* RightHandMesh;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	USkeletalMesh* RightMesh = LoadObject<USkeletalMesh>(nullptr,TEXT("/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_right.SKM_MannyXR_right"));
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	UMotionControllerComponent* RightMotionController;
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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Variables")
	float RotationSpeed = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float SnapRotationDegrees = 30.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float MoveSpeed = 10000.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables")
	float UpSpeed = 100000.0f;
	float TurnDeadZone = 0.2f;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void MoveForward(const FInputActionValue& value);
	void MoveRight(const FInputActionValue& value);
	void MoveUp(const FInputActionValue& value);
	void SmoothTurn(const FInputActionValue& value);
	void SnapTurn(const FInputActionValue& value);
	void SnapTurning(float value);
	void UpdateRoomScaleMovement();
};
