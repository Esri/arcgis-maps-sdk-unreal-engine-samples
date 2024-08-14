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
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "IXRTrackingSystem.h"
#include "XRTabletopComponent.h"
#include "Components/WidgetInteractionComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "xr_samplesproject/GenericXR/XRDistanceGrabComponent.h"
#include "xr_samplesproject/GenericXR/XRGrabComponent.h"
#include "xr_samplesproject/VRSample/VRHandAnimInstance.h"

// Sets default values
AXRTableTopInteractor::AXRTableTopInteractor()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	vrOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	vrOrigin->SetupAttachment(RootComponent);
	vrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
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

// Called when the game starts or when spawned
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
	GEngine->XRSystem->SetTrackingOrigin(trackingOrigin);
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem
			<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(inputContext, 0);
		}
	}
}

// Called every frame
void AXRTableTopInteractor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	
}

void AXRTableTopInteractor::SetTabletopComponent()
{
	auto mapComponent = UArcGISMapComponent::GetMapComponent(this);
	if (mapComponent)
	{
		TabletopComponent = mapComponent->GetOwner()->FindComponentByClass<UXRTabletopComponent>();
	}
}

void AXRTableTopInteractor::StartPanning()
{	
	auto hitLocation = FVector3d::ZeroVector;

	currentPanningController = bUseRightHand ? rightMotionControllerAim : leftMotionControllerAim;

	auto hitIsInExtent = TabletopComponent->Raycast(currentPanningController->GetComponentLocation(),
		currentPanningController->GetForwardVector(), hitLocation);

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
		//currentPanningController = bUseRightHand ? rightMotionControllerAim : leftMotionControllerAim;

		//auto start = currentPanningController->GetComponentLocation();
		//auto end = currentPanningController->GetComponentLocation() + 10000. * currentPanningController->GetForwardVector();
		auto inExtent = TabletopComponent->Raycast(
			currentPanningController->GetComponentLocation(), 
			currentPanningController->GetForwardVector(), hitLocation);

		//auto inExtent = TabletopComponent->Raycast(panningHit.TraceStart, panningHit.TraceEnd - panningHit.TraceStart, hitLocation);

		//if (!inExtent) {
		//	return;
		//}


		auto panDeltaEngine = PanStartEnginePos - hitLocation;
		if (FVector3d::Dist(hitLocation, PanLastEnginePos) > MaxEnginePanDistancePerTick) {
			panDeltaEngine *= (panDeltaEngine.Length() + MaxEnginePanDistancePerTick) / panDeltaEngine.Length();
			//UE_LOG(LogTemp, Warning, TEXT("%f"), (hitLocation-PanLastEnginePos).Length());
		}
		PanLastEnginePos = hitLocation;

		auto panDeltaWorld = PanStartWorldTransform.TransformPosition(panDeltaEngine);
		//auto panDeltaWorld = PanStartWorldTransform.TransformPosition(hitLocation);

		//DrawDebugLine(GetWorld(), PanStartEnginePos, hitLocation, FColor::Black, false, -1.f, 0, 3);
		DrawDebugSphere(GetWorld(), PanStartEnginePos, 10, 5, FColor::Blue);
		DrawDebugSphere(GetWorld(), hitLocation, 10, 5, FColor::Red);

		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, inExtent? FColor::Green : FColor::Red, hitLocation.ToString());


		//GEngine->AddOnScreenDebugMessage(-1, 15.0f, inExtent? FColor::Green : FColor::Red, hitLocation.ToString());
		//auto temp = PanStartWorldTransform.TransformPosition(PanStartEnginePos);
		//UE_LOG(LogTemp, Warning, TEXT("%f:   %s"), (temp-panDeltaWorld).Length(), *(temp-panDeltaWorld).ToString());
	
		TabletopComponent->MoveExtentCenter(panDeltaWorld);

		//bIsPanning = false;
	}
}



void AXRTableTopInteractor::UpdateElevationOffsetFromLocation() 
{
	auto wrapper = TabletopComponent->WrapperActor;
	TabletopComponent->SetElevationOffset(wrapper->GetActorLocation().Z / wrapper->GetActorScale3D().Z);
}

void AXRTableTopInteractor::ZoomMap(const FInputActionValue& value)
{
	ZoomLevel += value.Get<float>();

	if (ZoomLevel < -4 || ZoomLevel > 4)
	{
		TabletopComponent->ZoomMap(ZoomLevel);
		ZoomLevel = 0;

		UpdateElevationOffsetFromLocation();

	}

}

void AXRTableTopInteractor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(zoom, ETriggerEvent::Triggered, this, &AXRTableTopInteractor::ZoomMap);
		
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

	distanceGrabLeft->TryGrab();

	SetGripAxisValue(1);
}

void AXRTableTopInteractor::OnGrabRight()
{
	bUseRightHand = true;
	
	distanceGrabRight->TryGrab();

	SetGripAxisValue(1);
}

void AXRTableTopInteractor::OnGrabReleaseLeft()
{
	distanceGrabLeft->TryRelease();

	UpdateElevationOffsetFromLocation();

	SetGripAxisValue(0);
}

void AXRTableTopInteractor::OnGrabReleaseRight()
{
	distanceGrabRight->TryRelease();

	UpdateElevationOffsetFromLocation();

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

