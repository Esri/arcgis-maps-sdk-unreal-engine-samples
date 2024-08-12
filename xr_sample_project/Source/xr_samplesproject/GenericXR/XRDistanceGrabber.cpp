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

#include "XRDistanceGrabber.h"

// Sets default values for this component's properties
UXRDistanceGrabber::UXRDistanceGrabber()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	//bAutoActivate = true;
	//Activate();
	//RegisterComponent();

	// ...
}

//UXRDistanceGrabber::~UXRDistanceGrabber()
//{
//	UE_LOG(LogTemp, Error, TEXT("Deeeeeeeeeeee"));
//}

bool UXRDistanceGrabber::TryGrab()
{
	TArray<FHitResult> traceHits;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // TODO change type

	//if (this)
	//{
	//			//UE_LOG(LogTemp, Error, TEXT("location %s"), *GetName());
	//			UE_LOG(LogTemp, Error, TEXT("location %s"), *GetComponentLocation().ToString());


	//}

	//if(UKismetSystemLibrary::BoxTraceMultiByProfile(GetWorld(),GetComponentLocation(), GetComponentLocation() + forwardDirection, EOrientation() 50.0f, ObjectTypesArray,false, IgnoreActors, EDrawDebugTrace::None, outHit, true))
	//if (UKismetSystemLibrary::BoxTraceMultiForObjects( GetWorld(), GetComponentLocation(), 
	//if (UKismetSystemLibrary::SphereTraceSingleForObjects(
	//	GetWorld(), GetComponentLocation(), GetComponentLocation() + forwardDirection, 
	//	1.0f, ObjectTypesArray, false, IgnoreActors, EDrawDebugTrace::ForOneFrame, outHit, false, FColor::Cyan))
	if (UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), GetComponentLocation(),
		GetComponentLocation() + GetForwardVector() * MaxGrabbableDistance, ObjectTypesArray, false,
		TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, traceHits, true, FColor::Cyan))
	{
		for (auto hit : traceHits)
		{
			if (!hit.GetComponent()->GetAttachParent())
			{
				return false;
			}

			if (auto targetedComponent = Cast<UXRDistanceGrabbable>(hit.GetComponent()->GetAttachParent()))
			{
				//UE_LOG(LogTemp, Error, TEXT("hit %s"), *targetedComponent->GetName());


				//auto temp = Cast<UXRDistanceGrabbable>(targetedComponent);
			
				//UE_LOG(LogTemp, Error, TEXT("class %s"), *targetedComponent->GetClass()->GetName());

				targetedComponent->OnTargetted(this);
				
				auto success = targetedComponent->OnGrabbed(this, hit);

				if (success)
				{
					GrabbedComponent = targetedComponent;
				}
				
				return success;
				//temp->OnGrabbed();
				//temp->OnGrabberDetected();
			}
		}

	}

	return false;
}

bool UXRDistanceGrabber::TryRelease()
{
	if (GrabbedComponent) 
	{
		GrabbedComponent->OnGrabReleased();
		GrabbedComponent = nullptr;
		return true;
	} 

	return false;
}


// Called when the game starts
void UXRDistanceGrabber::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UXRDistanceGrabber::OnComponentCreated()
{
	Super::OnComponentCreated();

	UE_LOG(LogTemp, Log, TEXT("UMyCustomComponent::OnComponentCreated called. %s"), *this->GetName());

}


// Called every frame
void UXRDistanceGrabber::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//FVector forwardDirection = GetForwardVector() * 5000.0f;

	//if (bIsTracking)
	//{
	//	//FHitResult outHit;
	//	TArray<FHitResult> outHits;
	//	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	//	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	//	//IgnoreActors.Empty();
	//	//TargetedGrabComponent = nullptr;

	//	//if(UKismetSystemLibrary::BoxTraceMultiByProfile(GetWorld(),GetComponentLocation(), GetComponentLocation() + forwardDirection, EOrientation() 50.0f, ObjectTypesArray,false, IgnoreActors, EDrawDebugTrace::None, outHit, true))
	//	if (UKismetSystemLibrary::BoxTraceMultiForObjects(
	//		GetWorld(), GetComponentLocation(), GetComponentLocation() + forwardDirection, FVector3d(1),
	//		FRotator(), ObjectTypesArray, false, TArray<AActor*>(), EDrawDebugTrace::ForOneFrame, outHits, true, FColor::Cyan))
	//		//if (UKismetSystemLibrary::SphereTraceSingleForObjects(
	//		//	GetWorld(), GetComponentLocation(), GetComponentLocation() + forwardDirection, 
	//		//	1.0f, ObjectTypesArray, false, IgnoreActors, EDrawDebugTrace::ForOneFrame, outHit, false, FColor::Cyan))
	//	{
	//		for (auto outHit : outHits) {
	//			auto x = outHit.GetComponent()->GetAttachParent();
	//			if (x)
	//			{
	//				UE_LOG(LogTemp, Error, TEXT("hit %s"), *x->GetName());


	//				auto temp = Cast<UXRDistanceGrabbable>(x);
	//				if (temp)
	//				{

	//				UE_LOG(LogTemp, Error, TEXT("class %s"), *x->GetClass()->GetName());
	//					temp->OnTargetted(this);
	//					//temp->OnGrabbed(this);

	//					//temp->OnGrabbed();
	//					//temp->OnGrabberDetected();
	//				}
	//			}
	//		}
	//		//if (UKismetSystemLibrary::DoesImplementInterface(outHit.GetComponent(), IGrabbableInterface::StaticClass()); outHit.Component.inter)
	//		//impactPoint = outHit.ImpactPoint;
	//		//if (outHit.bBlockingHit)
	//		//{
	//		//	IgnoreActors.Add(outHit.GetActor());
	//		//	auto grabComponent = outHit.GetActor()->GetComponentByClass(UXRGrabComponent::StaticClass());

	//		//	if (UXRGrabComponent* grabbable = Cast<UXRGrabComponent>(grabComponent))
	//		//	{
	//		//		bIsValidGrabbable = true;
	//		//		if (FVector::Distance(grabbable->GetComponentLocation(), GetComponentLocation()) >= 50.0f)
	//		//		{
	//		//			if (!TargetedGrabComponent)
	//		//			{
	//		//				TargetedGrabComponent = grabbable;
	//		//			}
	//		//			else
	//		//			{
	//		//				auto rot1 = UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(), TargetedGrabComponent->GetComponentLocation());
	//		//				auto rot2 = UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(), grabbable->GetComponentLocation());

	//		//				if (FVector::Distance(UKismetMathLibrary::GetForwardVector(rot1), GetForwardVector()) > FVector::Distance(UKismetMathLibrary::GetForwardVector(rot2), GetForwardVector()))
	//		//				{
	//		//					TargetedGrabComponent = grabbable;
	//		//				}
	//		//			}
	//		//		}
	//		//	}
	//		//}
	//	}
	//	else
	//	{
	//		//bIsTracking = false;
	//	}
	//}
}

