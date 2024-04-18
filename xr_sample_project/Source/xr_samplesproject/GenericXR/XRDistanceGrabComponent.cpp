// Fill out your copyright notice in the Description page of Project Settings.


#include "XRDistanceGrabComponent.h"
#include "XRGrabComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values for this component's properties
UXRDistanceGrabComponent::UXRDistanceGrabComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

// Called every frame
void UXRDistanceGrabComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FVector forwardDirection = GetForwardVector() * 5000.0f;
	
	if (bIsDetecting)
	{
		FHitResult outHit;
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
		ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECC_PhysicsBody));
		IgnoreActors.Empty();
		TargetedGrabComponent = nullptr;
		
		if(UKismetSystemLibrary::SphereTraceSingleForObjects(GetWorld(),GetComponentLocation(), GetComponentLocation() + forwardDirection, 50.0f, ObjectTypesArray,false, IgnoreActors, EDrawDebugTrace::None, outHit, true))
		{
			if (outHit.bBlockingHit)
			{
				IgnoreActors.Add(outHit.GetActor());
				auto grabComponent = outHit.GetActor()->GetComponentByClass(UXRGrabComponent::StaticClass());

				if (UXRGrabComponent* grabbable = Cast<UXRGrabComponent>(grabComponent))
				{
					if (FVector::Distance(grabbable->GetComponentLocation(), GetComponentLocation()) >= 50.0f)
					{
						if (!TargetedGrabComponent)
						{
							TargetedGrabComponent = grabbable;
						}
						else
						{
							auto rot1 = UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(), TargetedGrabComponent->GetComponentLocation());
							auto rot2 = UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(), grabbable->GetComponentLocation());

							if (FVector::Distance(UKismetMathLibrary::GetForwardVector(rot1), GetForwardVector()) > FVector::Distance(UKismetMathLibrary::GetForwardVector(rot2), GetForwardVector()))
							{
								TargetedGrabComponent = grabbable;
							}
						}
					}
				}
			}
		}
	}
}

UXRGrabComponent* UXRDistanceGrabComponent::Grab(UMotionControllerComponent* MotionController)
{
	if (TargetedGrabComponent)
	{
		auto grabComp = TargetedGrabComponent;
		bIsDetecting = false;

		switch (grabComp->GrabType)
		{
			case None:
				break;
			case Free:
				grabComp->GetOwner()->SetActorLocationAndRotation(GetComponentTransform().GetLocation(), GetComponentTransform().GetRotation(), false, new FHitResult, ETeleportType::TeleportPhysics);
				break;
			case Snap:
				grabComp->GetOwner()->SetActorLocationAndRotation(GetComponentTransform().GetLocation(), GetComponentTransform().GetRotation(), false, new FHitResult, ETeleportType::TeleportPhysics);
				break;
			case Custom:
				break;
		}
		
		grabComp->TryGrab(MotionController);
		TargetedGrabComponent = nullptr;
		return grabComp;
	}
	
	return nullptr;
}

