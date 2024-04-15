// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "HeadMountedDisplayTypes.h"
#include "XRTableTopInteractor.generated.h"

class UXRTabletopComponent;

UCLASS()
class XR_SAMPLESPROJECT_API AXRTableTopInteractor : public APawn
{
	GENERATED_BODY()

public:
	AXRTableTopInteractor();

	UFUNCTION(BlueprintCallable)
	void StartPanning();
	UFUNCTION(BlueprintCallable)
	void StopPanning();
	
protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	void SetTabletopComponent();
	void UpdatePointDrag();
	void ZoomMap(const FInputActionValue& value);
	
	bool bIsDragging;
	FVector3d DragStartEnginePos{FVector3d::ZeroVector};
	FTransform DragStartWorldTransform{FTransform::Identity};

	UInputMappingContext* inputContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Samples/XRTableTop/Input/IAC_XRTableTop.IAC_XRTableTop'"));
	UInputAction* gripLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Left.IA_Grip_Left'"));
	UInputAction* gripRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Right.IA_Grip_Right'"));
	UInputAction* panLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Left.IA_Pan_Left'"));
	UInputAction* panRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Right.IA_Pan_Right'"));
	UInputAction* zoom = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Zoom.IA_Zoom'"));

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UXRTabletopComponent* TabletopComponent{nullptr};
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	TEnumAsByte<EHMDTrackingOrigin::Type> trackingOrigin;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UCameraComponent* vrCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* leftMotionController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* rightMotionController;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	USceneComponent* vrOrigin;
};
