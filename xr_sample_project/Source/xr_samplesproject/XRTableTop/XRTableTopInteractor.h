/* Copyright 2024 Esri
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
#include "GameFramework/Pawn.h"
#include "InputMappingContext.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "HeadMountedDisplayTypes.h"
#include "xr_samplesproject/GenericXR/XRGrabComponent.h"
#include "XRTableTopInteractor.generated.h"

class AGeocoder;
class UWidgetInteractionComponent;
class UVRHandAnimInstance;
class UXRDistanceGrabComponent;
class UXRTabletopComponent;

UCLASS()
class XR_SAMPLESPROJECT_API AXRTableTopInteractor : public APawn
{
	GENERATED_BODY()

public:
	AXRTableTopInteractor();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	UFUNCTION(BlueprintCallable)
	void StartPanning();
	UFUNCTION(BlueprintCallable)
	void StopPanning();
	virtual void Tick(float DeltaTime) override;

	FHitResult panningHit;
	FVector endPoint;
	TArray<AActor*> IgnoreActors;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UXRTabletopComponent* TabletopComponent{nullptr};
	
protected:
	virtual void BeginPlay() override;

private:
	void OnGrabLeft();
	void OnGrabRight();
	void OnTriggerLeft();
	void OnTriggerRight();
	void OnReleaseLeft();
	void OnReleaseRight();
	void SetLeftGripAxis(const FInputActionValue& value);
	void SetLeftTriggerAxis(const FInputActionValue& value);
	void SetRightGripAxis(const FInputActionValue& value);
	void SetRightTriggerAxis(const FInputActionValue& value);
	void ResetLeftGripAxis();
	void ResetLeftTriggerAxis();
	void ResetRightGripAxis();
	void ResetRightTriggerAxis();
	void SimulateClickLeft();
	void SimulateClickRight();
	void ResetClickLeft();
	void ResetClickRight();
	void SetTabletopComponent();
	void UpdatePanning();
	void ZoomMap(const FInputActionValue& value);
	
	bool bIsPanning;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	bool bIsOverUILeft;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	bool bIsOverUIRight;
	bool bUseRightHand;

	const double MaxEnginePanDistancePerTick = 5.;

	FVector3d PanStartEnginePos{FVector3d::ZeroVector};
	FVector3d PanLastEnginePos{FVector3d::ZeroVector};
	FTransform PanStartWorldTransform{FTransform::Identity};
	UInputMappingContext* inputContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Samples/XRTableTop/Input/IAC_XRTableTop.IAC_XRTableTop'"));
	UInputAction* gripLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Left.IA_Grip_Left'"));
	UInputAction* gripRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Right.IA_Grip_Right'"));
	UInputAction* panLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Left.IA_Pan_Left'"));
	UInputAction* panRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Right.IA_Pan_Right'"));
	UInputAction* zoom = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Zoom.IA_Zoom'"));
	
	
	UMotionControllerComponent* currentPanningController;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UXRDistanceGrabber* distanceGrabLeft;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UXRDistanceGrabber* distanceGrabRight;

	USkeletalMesh* handMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	AGeocoder* geoCoder;
		
	UVRHandAnimInstance* leftAnimInstance;
	UAnimInstance* leftAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* leftHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* leftInteraction;
	
	UVRHandAnimInstance* rightAnimInstance;
	UAnimInstance* rightAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* rightHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Variables", meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* rightInteraction;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	TEnumAsByte<EHMDTrackingOrigin::Type> trackingOrigin;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UCameraComponent* vrCamera;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* leftMotionControllerAim;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* rightMotionControllerAim;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* leftMotionControllerGrip;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* rightMotionControllerGrip;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	USceneComponent* vrOrigin;
};
