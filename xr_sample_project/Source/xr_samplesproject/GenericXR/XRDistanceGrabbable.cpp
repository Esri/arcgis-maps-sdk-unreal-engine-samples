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

UXRDistanceGrabbable::UXRDistanceGrabbable()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UXRDistanceGrabbable::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsGrabbed)
	{
		auto newLocation = CurrentGrabber->GetComponentLocation() + CurrentGrabber->GetForwardVector() * GrabDistance + GrabOffset;
		
		auto playerLookDirection = newLocation - CurrentGrabber->GetOwner()->GetActorLocation();
		playerLookDirection.Z = 0;

		auto newRotation = FRotationMatrix::MakeFromX(playerLookDirection).Rotator();

		GetOwner()->SetActorLocationAndRotation(newLocation, newRotation);
	}
}

bool UXRDistanceGrabbable::OnGrabbed_Implementation(UXRDistanceGrabber* Grabber, const FHitResult& Hit)
{
	if (bIsGrabbed) 
	{
		return false;
	}

	bIsGrabbed = true;
	CurrentGrabber = Grabber;
	GrabOffset = GetOwner()->GetActorLocation() - Hit.ImpactPoint;
	GrabDistance = FVector3d::Dist(Grabber->GetComponentLocation(), Hit.ImpactPoint);
	return true;
}

void UXRDistanceGrabbable::OnGrabReleased_Implementation()
{
	bIsGrabbed = false;
}

void UXRDistanceGrabbable::AddGrabDistance(float Offset)
{
	GrabDistance += Offset;
}
