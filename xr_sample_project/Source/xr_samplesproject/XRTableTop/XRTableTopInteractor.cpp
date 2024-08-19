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

#include "XRTableTopInteractor.h"
#include "XRTabletopComponent.h"

AXRTableTopInteractor::AXRTableTopInteractor()
{
	PrimaryActorTick.bCanEverTick = true;

	auto vrOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	vrOrigin->SetupAttachment(RootComponent);
	auto vrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	vrCamera->SetupAttachment(vrOrigin);

	leftMotionControllerAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionControllerAim"));
	leftMotionControllerAim->SetupAttachment(vrOrigin);
	leftMotionControllerAim->SetTrackingSource(EControllerHand::Left);
	leftMotionControllerAim->MotionSource = TEXT("LeftAim");
	leftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
	leftHandMesh->SetupAttachment(leftMotionControllerAim);
	distanceGrabLeft = CreateDefaultSubobject<UXRDistanceGrabber>(TEXT("DistanceGrabLeft"));
	distanceGrabLeft->SetupAttachment(leftMotionControllerAim);
	leftInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Left Interactor"));
	leftInteraction->SetupAttachment(leftMotionControllerAim);

	rightMotionControllerAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionControllerAim"));
	rightMotionControllerAim->SetupAttachment(vrOrigin);
	rightMotionControllerAim->SetTrackingSource(EControllerHand::Right);
	rightMotionControllerAim->MotionSource = TEXT("RightAim");
	rightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
	rightHandMesh->SetupAttachment(rightMotionControllerAim);
	distanceGrabRight = CreateDefaultSubobject<UXRDistanceGrabber>(TEXT("DistanceGrabRight"));
	distanceGrabRight->SetupAttachment(rightMotionControllerAim);
	rightInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Right Interactor"));
	rightInteraction->SetupAttachment(rightMotionControllerAim);
}

void AXRTableTopInteractor::BeginPlay()
{
	Super::BeginPlay();

	leftHandMesh->SetSkeletalMesh(handMesh);
	leftHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	leftAnimInstanceBase = leftHandMesh->GetAnimInstance();
	leftAnimInstance = Cast<UVRHandAnimInstance>(leftAnimInstanceBase);
	rightHandMesh->SetSkeletalMesh(handMesh);
	rightHandMesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	rightAnimInstanceBase = rightHandMesh->GetAnimInstance();
	rightAnimInstance = Cast<UVRHandAnimInstance>(rightAnimInstanceBase);
	
	SetTabletopComponent();
	
	GEngine->XRSystem->SetTrackingOrigin(TrackingOrigin);

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(inputContext, 0);
		}
	}
}

void AXRTableTopInteractor::SetTabletopComponent()
{
	// Find the tabletop component attached to the map actor
	auto mapComponent = UArcGISMapComponent::GetMapComponent(this);
	if (mapComponent)
	{
		TabletopComponent = mapComponent->GetOwner()->FindComponentByClass<UXRTabletopComponent>();
	}
}

void AXRTableTopInteractor::StartPanning()
{	
	auto hitLocation = FVector3d::ZeroVector;

	CurrentPanningController = bUseRightHand ? rightMotionControllerAim : leftMotionControllerAim;

	auto hitIsInExtent = TabletopComponent->Raycast(CurrentPanningController->GetComponentLocation(),
		CurrentPanningController->GetForwardVector(), hitLocation);

	// Start panning if controller is pointing to a location withing the extent
	if (hitIsInExtent)
	{
		bIsPanning = true;
		auto mapTranslation = FTransform(TabletopComponent->GetMapComponentLocation());
		auto toWorldTransform = TabletopComponent->GetFromEngineTransform();

		FTransform::Multiply(&PanStartWorldTransform, &mapTranslation, &toWorldTransform);
		PanStartEnginePos = hitLocation;
	}
}

void AXRTableTopInteractor::StopPanning()
{
	bIsPanning = false;
}

void AXRTableTopInteractor::UpdatePanning()
{
	auto hitLocation = PanStartEnginePos;

	if (TabletopComponent)
	{
		TabletopComponent->Raycast(CurrentPanningController->GetComponentLocation(), 
			CurrentPanningController->GetForwardVector(), hitLocation);

		auto panDeltaEngine = PanStartEnginePos - hitLocation;
		
		// Limit panning amount per frame
		if (FVector3d::Dist(hitLocation, PanLastEnginePos) > MaxEnginePanDistancePerTick) {
			panDeltaEngine *= (panDeltaEngine.Length() + MaxEnginePanDistancePerTick) / panDeltaEngine.Length();
		}
		PanLastEnginePos = hitLocation;

		auto panDeltaWorld = PanStartWorldTransform.TransformPosition(panDeltaEngine);
		TabletopComponent->MoveExtentCenter(panDeltaWorld);
	}
}

void AXRTableTopInteractor::OnThumbstickTilted(const FInputActionValue& value)
{
	if (bIsPanning)
	{
		return;
	}

	if (GrabbedComponent) // If grabbing the table, move is closer or farther
	{
		GrabbedComponent->AddGrabDistance(value.Get<float>()*4);
	}
	else // Zoom the map in discrete steps
	{
		ZoomLevel += value.Get<float>();

		if (FMath::Abs(ZoomLevel) > MinZoomStep)
		{
			TabletopComponent->ZoomMap(ZoomLevel);
			ZoomLevel = 0;
		}
	}
}

void AXRTableTopInteractor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(zoom, ETriggerEvent::Triggered, this, &AXRTableTopInteractor::OnThumbstickTilted);
		
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Triggered, this, &AXRTableTopInteractor::OnPanLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Triggered, this, &AXRTableTopInteractor::OnPanRight);
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Started, this, &AXRTableTopInteractor::OnTriggerPressedLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Started, this, &AXRTableTopInteractor::OnTriggerPressedRight);
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Completed , this, &AXRTableTopInteractor::OnPanReleaseLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Completed, this, &AXRTableTopInteractor::OnPanReleaseRight);
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Canceled , this, &AXRTableTopInteractor::OnPanReleaseLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Canceled, this, &AXRTableTopInteractor::OnPanReleaseRight);

		EnhancedInputComponent->BindAction(grabLeft, ETriggerEvent::Started , this, &AXRTableTopInteractor::OnGrabLeft);
		EnhancedInputComponent->BindAction(grabRight, ETriggerEvent::Started, this, &AXRTableTopInteractor::OnGrabRight);
		EnhancedInputComponent->BindAction(grabLeft, ETriggerEvent::Completed , this, &AXRTableTopInteractor::OnGrabReleaseLeft);
		EnhancedInputComponent->BindAction(grabRight, ETriggerEvent::Completed, this, &AXRTableTopInteractor::OnGrabReleaseRight);
		EnhancedInputComponent->BindAction(grabLeft, ETriggerEvent::Canceled , this, &AXRTableTopInteractor::OnGrabReleaseLeft);
		EnhancedInputComponent->BindAction(grabRight, ETriggerEvent::Canceled, this, &AXRTableTopInteractor::OnGrabReleaseRight);
	}
}

void AXRTableTopInteractor::OnGrabLeft()
{
	bUseRightHand = false;

	GrabbedComponent = distanceGrabLeft->TryGrab();

	SetGripAxisValue(1);
}

void AXRTableTopInteractor::OnGrabRight()
{
	bUseRightHand = true;
	
	GrabbedComponent = distanceGrabRight->TryGrab();

	SetGripAxisValue(1);
}

void AXRTableTopInteractor::OnGrabReleaseLeft()
{
	distanceGrabLeft->TryRelease();

	GrabbedComponent = nullptr;

	SetGripAxisValue(0);
}

void AXRTableTopInteractor::OnGrabReleaseRight()
{
	distanceGrabRight->TryRelease();

	GrabbedComponent = nullptr;

	SetGripAxisValue(0);
}

void AXRTableTopInteractor::HandleWidgetInteraction(bool ButtonPressed)
{
	auto currentInteraction = bUseRightHand ? rightInteraction : leftInteraction;

	if (ButtonPressed) 
	{
		if (currentInteraction->IsOverInteractableWidget())
		{
			currentInteraction->PressPointerKey(EKeys::LeftMouseButton);
		}
	}
	else
	{
		currentInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
	}
}

void AXRTableTopInteractor::OnTriggerPressedLeft()
{
	bUseRightHand = false;
	
	HandleWidgetInteraction(true);

	SetTriggerAxisValue(1);
}

void AXRTableTopInteractor::OnTriggerPressedRight()
{
	bUseRightHand = true;

	HandleWidgetInteraction(true);

	SetTriggerAxisValue(1);
}

void AXRTableTopInteractor::OnPanLeft()
{
	if (!bIsPanning) 
	{
		bUseRightHand = false;
		StartPanning();
	}
	else 
	{
		UpdatePanning();
	}
}

void AXRTableTopInteractor::OnPanRight()
{
	if (!bIsPanning)
	{
		bUseRightHand = true;
		StartPanning();
	}
	else
	{
		UpdatePanning();
	}
}

void AXRTableTopInteractor::OnPanReleaseLeft()
{
	bUseRightHand = false;
	
	StopPanning();

	HandleWidgetInteraction(false);

	SetTriggerAxisValue(0);
}

void AXRTableTopInteractor::OnPanReleaseRight()
{
	bUseRightHand = true;

	StopPanning();

	HandleWidgetInteraction(false);

	SetTriggerAxisValue(0);
}

void AXRTableTopInteractor::SetGripAxisValue(const float& value)
{
	auto currentInstance = bUseRightHand ? rightAnimInstance : leftAnimInstance;

	if (currentInstance)
	{
		currentInstance->GripAxis = value;
	}
}

void AXRTableTopInteractor::SetTriggerAxisValue(const float& value)
{
	auto currentInstance = bUseRightHand ? rightAnimInstance : leftAnimInstance;

	if (currentInstance)
	{
		currentInstance->TriggerAxis = value;
	}
}