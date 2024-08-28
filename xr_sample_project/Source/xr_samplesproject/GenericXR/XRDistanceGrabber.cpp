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

UXRDistanceGrabber::UXRDistanceGrabber()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UXRDistanceGrabbable* UXRDistanceGrabber::TryGrab()
{
	TArray<FHitResult> traceHits;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypesArray;
	ObjectTypesArray.Add(UEngineTypes::ConvertToObjectType(ECC_Visibility)); 

	if (UKismetSystemLibrary::LineTraceMultiForObjects(GetWorld(), GetComponentLocation(),
		GetComponentLocation() + GetForwardVector() * MaxGrabbableDistance, ObjectTypesArray, false,
		TArray<AActor*>(), EDrawDebugTrace::None, traceHits, true))
	{
		for (auto hit : traceHits)
		{
			if (!hit.GetComponent()->GetAttachParent())
			{
				return nullptr;
			}

			if (auto targetedComponent = Cast<UXRDistanceGrabbable>(hit.GetComponent()->GetAttachParent()))
			{
				auto grabSucceeded = targetedComponent->OnGrabbed(this, hit);

				if (grabSucceeded)
				{
					GrabbedComponent = targetedComponent;
				}
				
				return GrabbedComponent;
			}
		}
	}

	return nullptr;
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
