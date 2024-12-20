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

#pragma once

#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Kismet/KismetSystemLibrary.h"
#include "XRDistanceGrabbable.h"
#include "XRDistanceGrabber.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class XR_SAMPLESPROJECT_API UXRDistanceGrabber : public USceneComponent
{
	GENERATED_BODY()

public:	
	UXRDistanceGrabber();
	
	UXRDistanceGrabbable* TryGrab();
	
	bool TryRelease();

	float ClampGrabDistance(const float& Distance);

private:
	bool bIsTracking = false;
	
	const float MaxGrabbableDistance{ 2000. };
	const float MinGrabbableDistance{ 5. };

	UXRDistanceGrabbable* GrabbedComponent; 
};
