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
	virtual void Tick(float DeltaTime) override;

	FHitResult panningHit;
	FVector endPoint;
	TArray<AActor*> IgnoreActors;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UXRTabletopComponent* TabletopComponent{nullptr};


	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess))
	AActor* TestActor;

	UXRDistanceGrabber* distanceGrabLeft;

	UXRDistanceGrabber* distanceGrabRight;

	
protected:
	virtual void BeginPlay() override;

private:
	void OnGrabLeft();
	void OnGrabRight();
	void OnGrabReleaseLeft();
	void OnGrabReleaseRight();
	void HandleWidgetInteraction(bool ButtonPressed);
	void OnTriggerPressedLeft();
	void OnTriggerPressedRight();
	void OnPanLeft();
	void OnPanRight();
	void OnPanReleaseLeft();
	void OnPanReleaseRight();
	void SetGripAxisValue(const float& value);
	void SetTriggerAxisValue(const float& value);
	void SetTabletopComponent();
	void UpdatePanning();
	void UpdateElevationOffsetFromLocation();
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
	UInputAction* grabLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Left.IA_Grip_Left'"));
	UInputAction* grabRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Grip_Right.IA_Grip_Right'"));
	UInputAction* panLeft = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Left.IA_Pan_Left'"));
	UInputAction* panRight = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Pan_Right.IA_Pan_Right'"));
	UInputAction* zoom = LoadObject<UInputAction>(nullptr, TEXT("InputAction'/Game/Samples/XRTableTop/Input/IA_Zoom.IA_Zoom'"));
	
	
	UMotionControllerComponent* currentPanningController;
	

	USkeletalMesh* handMesh = LoadObject<USkeletalMesh>(nullptr, TEXT("SkeletalMesh'/Game/Samples/VRSample/Hands/Meshes/SKM_MannyXR_left.SKM_MannyXR_left'"));

		
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
	USceneComponent* vrOrigin;
	float ZoomLevel;
};
