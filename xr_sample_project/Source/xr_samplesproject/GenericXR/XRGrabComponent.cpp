// Fill out your copyright notice in the Description page of Project Settings.


#include "XRGrabComponent.h"

// Sets default values for this component's properties
UXRGrabComponent::UXRGrabComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UXRGrabComponent::BeginPlay()
{
	Super::BeginPlay();

	SetShouldSimulateOnDrop();
	if (UPrimitiveComponent* primitive = Cast<UPrimitiveComponent>(GetAttachParent()))
	{
		primitive->SetCollisionProfileName(TEXT("PhysicsActor"), true);
	}
}

EControllerHand UXRGrabComponent::GetHeldByHand()
{
	if (MotionControllerRef->MotionSource == TEXT("LeftGrip"))
	{
		return	EControllerHand::Left;
	}
	else
	{
		return	EControllerHand::Right;
	}
}

void UXRGrabComponent::SetPrimitiveCompPhysics(bool bSimulate)
{
	if (UPrimitiveComponent* primitive = Cast<UPrimitiveComponent>(GetAttachParent()))
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Set Primitive"));	
		primitive->SetSimulatePhysics(bSimulate);
	}
}

void UXRGrabComponent::SetShouldSimulateOnDrop()
{
	if (Cast<UPrimitiveComponent>(GetAttachParent())->IsAnySimulatingPhysics())
	{
		bSimulateOnDrop = true;
	}
}

bool UXRGrabComponent::TryGrab(UMotionControllerComponent* MotionController)
{
	auto location = (GetComponentLocation() - GetAttachParent()->GetComponentLocation()) * -1.0;
	switch (GrabType)
	{
	case None:
		break;
	case Free:
		SetPrimitiveCompPhysics(false);
		if (GetAttachParent()->AttachToComponent(MotionController, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true)))
		{
			bIsHeld = true;
			break;			
		}
		else
		{
			break;
		}
	case Snap:
		SetPrimitiveCompPhysics(false);
		GetAttachParent()->AttachToComponent(MotionController, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
		bIsHeld = true;
		GetAttachParent()->SetRelativeRotation(GetRelativeRotation().GetInverse());
		GetAttachParent()->SetWorldLocation(MotionController->GetComponentLocation() + location);
		break;
	case Custom:
		bIsHeld = true;
		break;
	}
	
	if (bIsHeld)
	{
		MotionControllerRef = MotionController;
		OnGrabbed.Broadcast();
		return true;
	}
	else
	{
		return false;	
	}
}

bool UXRGrabComponent::TryRelease()
{
	switch (GrabType)
	{
	case None:
		break;
		
	case Free:
		if (bSimulateOnDrop)
		{
			SetPrimitiveCompPhysics(true);
			bIsHeld = false;
		}
		else
		{
			GetAttachParent()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
			bIsHeld = false;
		}
		
		break;
		
	case Snap:
		if (bSimulateOnDrop)
		{
			SetPrimitiveCompPhysics(true);
			bIsHeld = false;
		}
		else
		{
			GetAttachParent()->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
			bIsHeld = false;
		}
		
		break;
		
	case Custom:
		bIsHeld = false;
		break;
	}

	if (bIsHeld)
	{
		return false;
	}
	else
	{
		OnReleased.Broadcast();
		return true;
	}
}