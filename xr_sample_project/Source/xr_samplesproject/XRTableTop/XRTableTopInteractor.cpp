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
#include "xr_samplesproject/Geocoding/Geocoder.h"
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
	distanceGrabLeft = CreateDefaultSubobject<UXRDistanceGrabComponent>(TEXT("DistanceGrabLeft"));
	distanceGrabLeft->SetupAttachment(leftMotionControllerAim);
	leftInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Left Interactor"));
	leftInteraction->SetupAttachment(leftMotionControllerAim);

	rightMotionControllerAim = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionControllerAim"));
	rightMotionControllerAim->SetupAttachment(vrOrigin);
	rightMotionControllerAim->SetTrackingSource(EControllerHand::Right);
	rightMotionControllerAim->MotionSource = TEXT("RightAim");
	rightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
	rightHandMesh->SetupAttachment(rightMotionControllerAim);
	distanceGrabRight = CreateDefaultSubobject<UXRDistanceGrabComponent>(TEXT("DistanceGrabRight"));
	distanceGrabRight->SetupAttachment(rightMotionControllerAim);
	rightInteraction = CreateDefaultSubobject<UWidgetInteractionComponent>(TEXT("Right Interactor"));
	rightInteraction->SetupAttachment(rightMotionControllerAim);
	
	leftMotionControllerGrip = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionControllerGrip"));
	leftMotionControllerGrip->SetupAttachment(vrOrigin);
	leftMotionControllerGrip->SetTrackingSource(EControllerHand::Left);
	leftMotionControllerGrip->MotionSource = TEXT("LeftGrip");

	rightMotionControllerGrip = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionControllerGrip"));
	rightMotionControllerGrip->SetupAttachment(vrOrigin);
	rightMotionControllerGrip->SetTrackingSource(EControllerHand::Right);
	rightMotionControllerGrip->MotionSource = TEXT("RightGrip");
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
	geoCoder = Cast<AGeocoder>(UGameplayStatics::GetActorOfClass(GetWorld(), AGeocoder::StaticClass()));
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

	
	if(bIsDragging)
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
}

{

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

void AXRTableTopInteractor::ZoomMap(const FInputActionValue& value)
{
	TabletopComponent->ZoomMap(value.Get<float>());
}

void AXRTableTopInteractor::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(zoom, ETriggerEvent::Triggered, this, &AXRTableTopInteractor::ZoomMap);
		
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Triggered, this, &AXRTableTopInteractor::OnTriggerLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Triggered, this, &AXRTableTopInteractor::OnTriggerRight);
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Completed , this, &AXRTableTopInteractor::OnReleaseLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Completed, this, &AXRTableTopInteractor::OnReleaseRight);
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Canceled , this, &AXRTableTopInteractor::OnReleaseLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Canceled, this, &AXRTableTopInteractor::OnReleaseRight);
	}
}


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

	if (leftInteraction->IsOverInteractableWidget())
	{
		leftInteraction->PressPointerKey(EKeys::LeftMouseButton);	
	}
	else
	{
	}
}

void AXRTableTopInteractor::OnTriggerRight()
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

		if (rightInteraction->IsOverInteractableWidget())
		{
			rightInteraction->PressPointerKey(EKeys::LeftMouseButton);
		}
		else
		{
}

void AXRTableTopInteractor::OnReleaseLeft()
{
	StopPanning();

	leftInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}

void AXRTableTopInteractor::OnReleaseRight()
{
	StopPanning();

	rightInteraction->ReleasePointerKey(EKeys::LeftMouseButton);
}

void AXRTableTopInteractor::SetLeftGripAxis(const FInputActionValue& value)
{
	if (leftAnimInstance)
	{
		leftAnimInstance->GripAxis = value.Get<float>();
	}
}

void AXRTableTopInteractor::SetRightGripAxis(const FInputActionValue& value)
{
	if (rightAnimInstance)
	{
		rightAnimInstance->GripAxis = value.Get<float>();
	}
}

void AXRTableTopInteractor::SetLeftTriggerAxis(const FInputActionValue& value)
{
	if (leftAnimInstance)
	{
		leftAnimInstance->TriggerAxis = value.Get<float>();
	}
}

void AXRTableTopInteractor::SetRightTriggerAxis(const FInputActionValue& value)
{	
	if (rightAnimInstance)
	{
		rightAnimInstance->TriggerAxis = value.Get<float>();
	}
}

void AXRTableTopInteractor::ResetLeftGripAxis()
{
	if (leftAnimInstance)
	{
		leftAnimInstance->GripAxis = 0.0f;
	}
}

void AXRTableTopInteractor::ResetRightGripAxis()
{
	if (rightAnimInstance)
	{
		rightAnimInstance->GripAxis = 0.0f;
	}
}

void AXRTableTopInteractor::ResetLeftTriggerAxis()
{
	if (leftAnimInstance)
	{
		leftAnimInstance->TriggerAxis = 0.0f;
	}
}

void AXRTableTopInteractor::ResetRightTriggerAxis()
{
	if (rightAnimInstance)
	{
		rightAnimInstance->TriggerAxis = 0.0f;
	}
}
