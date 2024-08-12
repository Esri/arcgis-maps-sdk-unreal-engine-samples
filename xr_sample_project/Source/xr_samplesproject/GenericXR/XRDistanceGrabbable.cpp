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

#include "XRDistanceGrabbable.h"
#include "XRDistanceGrabber.h"


// Sets default values for this component's properties
UXRDistanceGrabbable::UXRDistanceGrabbable()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UXRDistanceGrabbable::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UXRDistanceGrabbable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsGrabbed)
	{
		auto newLocation = CurrentGrabber->GetComponentLocation() + CurrentGrabber->GetForwardVector() * GrabDistance + GrabOffset;
		//auto newRotation= UKismetMathLibrary::FindLookAtRotation(CurrentGrabber->GetOwner()->GetActorLocation(), newLocation);
		
		auto playerLookDirection = newLocation - CurrentGrabber->GetOwner()->GetActorLocation();
		playerLookDirection.Z = 0;

		auto newRotation = FRotationMatrix::MakeFromX(playerLookDirection).Rotator();
		//GetOwner()->SetActorLocation(newLocation);

		GetOwner()->SetActorLocationAndRotation(newLocation, newRotation);
	}
}

void UXRDistanceGrabbable::OnTargetted_Implementation(UXRDistanceGrabber* Grabber)
{
	UE_LOG(LogTemp, Warning, TEXT("+++++++++++++++++++ c++ impl grab"));
}

bool UXRDistanceGrabbable::OnGrabbed_Implementation(UXRDistanceGrabber* Grabber, const FHitResult& Hit)
{
	//UE_LOG(LogTemp, Warning, TEXT("+++++++++++++++++++ c++ impl target"));

	if (bIsGrabbed) 
	{
		return false;
	}
	//UE_LOG(LogTemp, Error, TEXT("location %s"), *Hit.ImpactPoint.ToString());

	bIsGrabbed = true;
	CurrentGrabber = Grabber;
	GrabOffset = GetOwner()->GetActorLocation() - Hit.ImpactPoint;
	GrabDistance = FVector3d::Dist(Grabber->GetComponentLocation(), GrabOffset);
	return true;
}

void UXRDistanceGrabbable::OnGrabReleased_Implementation()
{
	bIsGrabbed = false;
	//CurrentGrabber = nullptr;
	//GrabDistance = 0;
	//GrabOffset = FVector3d();
}

