// Fill out your copyright notice in the Description page of Project Settings.


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
	{
		UpdatePointDrag();
	}
}

void AXRTableTopInteractor::SetTabletopComponent()
{
	auto mapComponent = UArcGISMapComponent::GetMapComponent(this);
	if (mapComponent)
	{
		TabletopComponent = mapComponent->GetOwner()->FindComponentByClass<UXRTabletopComponent>();
	}
}

void AXRTableTopInteractor::StartPanning(UMotionControllerComponent* MotionController)
{
	currentPanningController = MotionController;
	auto location = MotionController->GetComponentLocation();
	auto direction = MotionController->GetForwardVector();
	auto hitLocation = FVector3d::ZeroVector;
	TabletopComponent->Raycast(location, direction, hitLocation);
	bIsDragging = true;
	auto mapTranslation = FTransform(TabletopComponent->GetMapComponentLocation());
	auto toWorldTransform = TabletopComponent->GetFromEngineTransform();
	FTransform::Multiply(&DragStartWorldTransform, &mapTranslation, &toWorldTransform);
	DragStartEnginePos = hitLocation;
}

void AXRTableTopInteractor::StopPanning()
{
	bIsDragging = false;
}

void AXRTableTopInteractor::UpdatePointDrag()
{
	FVector3d engineLocation = currentPanningController->GetComponentLocation();
	FVector engineDirection = currentPanningController->GetForwardVector();
	auto hitLocation = DragStartEnginePos;

	if (TabletopComponent)
	{
		TabletopComponent->Raycast(engineLocation, engineDirection, hitLocation);
		auto dragDeltaEngine = DragStartEnginePos - hitLocation;
		auto dragDeltaWorld = DragStartWorldTransform.TransformPosition(dragDeltaEngine);

		TabletopComponent->MoveExtentCenter(dragDeltaWorld);
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
		
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Started, this, &AXRTableTopInteractor::OnTriggerLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Started, this, &AXRTableTopInteractor::OnTriggerRight);
		EnhancedInputComponent->BindAction(panLeft, ETriggerEvent::Completed, this, &AXRTableTopInteractor::OnReleaseLeft);
		EnhancedInputComponent->BindAction(panRight, ETriggerEvent::Completed, this, &AXRTableTopInteractor::OnReleaseRight);
		
		EnhancedInputComponent->BindAction(gripLeft, ETriggerEvent::Started, this, &AXRTableTopInteractor::OnGrabLeft);
		EnhancedInputComponent->BindAction(gripRight, ETriggerEvent::Started, this, &AXRTableTopInteractor::OnGrabRight);
		EnhancedInputComponent->BindAction(gripLeft, ETriggerEvent::Completed, this, &AXRTableTopInteractor::OnReleaseLeft);
		EnhancedInputComponent->BindAction(gripRight, ETriggerEvent::Completed, this, &AXRTableTopInteractor::OnReleaseRight);

	}
}

UXRGrabComponent* AXRTableTopInteractor::GetGrabComponentNearMotionController(UMotionControllerComponent* MotionController)
{
	auto localNearestComponentDistance = 9999999.0;
	UXRGrabComponent* localNearestGrabComponent = nullptr;
	TArray<AActor*> ignoredActors;
	FHitResult outHit;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
	auto localGripPosition = MotionController->GetComponentLocation();
	auto bHasHit = UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),
		localGripPosition, localGripPosition, 6.0f, ObjectTypesArray,
		false, ignoredActors, EDrawDebugTrace::None, outHit, true);
	
	if (bHasHit)
	{
		auto grabComponents = outHit.GetActor()->K2_GetComponentsByClass(UXRGrabComponent::StaticClass());

		if (grabComponents.Max() > 0)
		{
			for (auto Component : grabComponents)
			{
				auto grabComponent = Cast<UXRGrabComponent>(Component);
				if (grabComponent)
				{
					auto gripPosition= grabComponent->GetComponentLocation() - localGripPosition;
					if (FMath::Square(gripPosition.Length()) <= localNearestComponentDistance)
					{
						localNearestComponentDistance = FMath::Square(gripPosition.Length());
						localNearestGrabComponent = grabComponent;
					}
				}
			}
		}
	}

	return localNearestGrabComponent;
}

void AXRTableTopInteractor::OnGrabLeft()
{
	if (UXRGrabComponent* grabComponent = distanceGrabLeft->Grab(leftMotionControllerGrip))
	{
		heldComponentLeft = grabComponent;
	}

	if (UXRGrabComponent* grabComponent = GetGrabComponentNearMotionController(leftMotionControllerGrip))
	{
		heldComponentLeft = grabComponent;
		distanceGrabRight->bIsDetecting = false;
		
		if (heldComponentLeft == heldComponentRight)
		{
			heldComponentRight = nullptr;
		}
	}
}

void AXRTableTopInteractor::OnGrabRight()
{
	if (UXRGrabComponent* grabComponent = distanceGrabRight->Grab(rightMotionControllerGrip))
	{
		heldComponentRight = grabComponent;
	}

	if (UXRGrabComponent* grabComponent = GetGrabComponentNearMotionController(rightMotionControllerGrip))
	{
		if (grabComponent->TryGrab(rightMotionControllerGrip))
		{
			heldComponentRight = grabComponent;
			distanceGrabRight->bIsDetecting = false;
			
			if (heldComponentRight == heldComponentLeft)
			{
				heldComponentLeft = nullptr;
			}
		}	
	}
}

void AXRTableTopInteractor::OnTriggerLeft()
{
	if (UXRGrabComponent* grabComponent = distanceGrabLeft->Grab(leftMotionControllerGrip))
	{
		heldComponentLeft = grabComponent;
	}

	if (UXRGrabComponent* grabComponent = GetGrabComponentNearMotionController(leftMotionControllerGrip))
	{
		if (grabComponent->TryGrab(leftMotionControllerGrip))
		{
			heldComponentLeft = grabComponent;
			distanceGrabRight->bIsDetecting = false;
			
			if (heldComponentLeft == heldComponentRight)
			{
				heldComponentRight = nullptr;
			}
		}	
	}
}

void AXRTableTopInteractor::OnTriggerRight()
{
	if (UXRGrabComponent* grabComponent = distanceGrabRight->Grab(rightMotionControllerGrip))
	{
		heldComponentRight = grabComponent;
	}

	if (UXRGrabComponent* grabComponent = GetGrabComponentNearMotionController(rightMotionControllerGrip))
	{
		if (grabComponent->TryGrab(rightMotionControllerGrip))
		{
			heldComponentRight = grabComponent;
			distanceGrabRight->bIsDetecting = false;
			
			if (heldComponentRight == heldComponentLeft)
			{
				heldComponentLeft = nullptr;
			}
		}	
	}
}

void AXRTableTopInteractor::OnReleaseLeft()
{
	distanceGrabRight->bIsDetecting = true;
	if (heldComponentLeft)
	{
		if (heldComponentLeft->TryRelease())
		{
			distanceGrabLeft->bIsDistanceGrab = false;
			heldComponentLeft = nullptr;
		}
	}
}

void AXRTableTopInteractor::OnReleaseRight()
{
	distanceGrabRight->bIsDetecting = true;
	if (heldComponentRight)
	{
		if (heldComponentRight->TryRelease())
		{
			distanceGrabRight->bIsDistanceGrab = false;
			heldComponentRight = nullptr;
		}
	}
}
