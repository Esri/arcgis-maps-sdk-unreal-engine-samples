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
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Pawn.h"
#include "HeadMountedDisplayTypes.h"
#include "InputMappingContext.h"
#include "IXRTrackingSystem.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MotionControllerComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "xr_samplesproject/VRSample/VRHandAnimInstance.h"
#include "xr_samplesproject/GenericXR/XRDistanceGrabbable.h"
#include "xr_samplesproject/GenericXR/XRDistanceGrabber.h"
#include "XRTableTopInteractor.generated.h"



class AGeocoder;
class UWidgetInteractionComponent;
class UVRHandAnimInstance;
class UXRDistanceGrabComponent;
class UXRTabletopComponent;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
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
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UXRTabletopComponent* TabletopComponent;
	
	UXRDistanceGrabber* distanceGrabLeft;
	UXRDistanceGrabber* distanceGrabRight;

	
protected:
	virtual void BeginPlay() override;

private:
	void HandleWidgetInteraction(bool ButtonPressed);
	void OnGrabLeft();
	void OnGrabReleaseLeft();
	void OnGrabRight();
	void OnGrabReleaseRight();
	void OnPanLeft();
	void OnPanReleaseLeft();
	void OnPanRight();
	void OnPanReleaseRight();
	void OnTriggerPressedLeft();
	void OnTriggerPressedRight();
	void SetGripAxisValue(const float& value);
	void SetTriggerAxisValue(const float& value);
	void SetTabletopComponent();
	void UpdatePanning();
	void OnThumbstickTilted(const FInputActionValue& value);

	UInputMappingContext* inputContext = LoadObject<UInputMappingContext>(nullptr, TEXT("InputMappingContext'/Game/Samples/XRTableTop/Input/IAC_XRTableTop.IAC_XRTableTop'"));
	UInputAction* grabLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Left.IA_Grip_Left'"));
	UInputAction* grabRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Right.IA_Grip_Right'"));
	UInputAction* panLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Left.IA_Pan_Left'"));
	UInputAction* panRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Right.IA_Pan_Right'"));
	UInputAction* zoom = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Zoom.IA_Zoom'"));
	
	UMotionControllerComponent* CurrentPanningController;
	
	UXRDistanceGrabbable* GrabbedComponent;

	USkeletalMesh* handMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));

	UVRHandAnimInstance* leftAnimInstance;
	UAnimInstance* leftAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* leftHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* leftInteraction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* leftMotionControllerAim;
	
	UVRHandAnimInstance* rightAnimInstance;
	UAnimInstance* rightAnimInstanceBase;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* rightHandMesh;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	UWidgetInteractionComponent* rightInteraction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess))
	UMotionControllerComponent* rightMotionControllerAim;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	TEnumAsByte<EHMDTrackingOrigin::Type> TrackingOrigin;

	bool bUseRightHand;
	
	bool bIsPanning;
	const double MaxEnginePanDistancePerTick = 5.;
	FVector3d PanStartEnginePos{FVector3d::ZeroVector};
	FVector3d PanLastEnginePos{FVector3d::ZeroVector};
	FTransform PanStartWorldTransform{FTransform::Identity};

	float ZoomLevel;
	const float MinZoomStep{4.};
};
