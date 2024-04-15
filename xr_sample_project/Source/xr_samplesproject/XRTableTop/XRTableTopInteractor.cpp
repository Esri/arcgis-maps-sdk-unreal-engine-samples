// Fill out your copyright notice in the Description page of Project Settings.


#include "XRTableTopInteractor.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "IXRTrackingSystem.h"
#include "XRTabletopComponent.h"

// Sets default values
AXRTableTopInteractor::AXRTableTopInteractor()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	vrOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("VROrigin"));
	vrOrigin->SetupAttachment(RootComponent);
	vrCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VRCamera"));
	vrCamera->SetupAttachment(vrOrigin);

	leftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
	leftMotionController->SetupAttachment(vrOrigin);
	leftMotionController->SetTrackingSource(EControllerHand::Left);

	rightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
	rightMotionController->SetupAttachment(vrOrigin);
	rightMotionController->SetTrackingSource(EControllerHand::Right);
}

// Called when the game starts or when spawned
void AXRTableTopInteractor::BeginPlay()
{
	Super::BeginPlay();

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

void AXRTableTopInteractor::StartPanning()
{
	auto location = leftMotionController->GetRelativeLocation();
	auto direction = leftMotionController->GetForwardVector();
	auto hitLocation = FVector3d::ZeroVector;

	if(TabletopComponent->Raycast(location, direction, hitLocation))
	{
		bIsDragging = true;
		auto mapTranslation = FTransform(TabletopComponent->GetMapComponentLocation());
		auto toWorldTransform = TabletopComponent->GetFromEngineTransform();
		FTransform::Multiply(&DragStartWorldTransform, &mapTranslation, &toWorldTransform);
		DragStartEnginePos = hitLocation;	
	}
}

void AXRTableTopInteractor::StopPanning()
{
	bIsDragging = false;
}

void AXRTableTopInteractor::UpdatePointDrag()
{
	FVector3d engineLocation, engineDirection;
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
	}
}

