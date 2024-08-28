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

#include "Camera/CameraComponent.h"
#include "Components/SceneComponent.h"
#include "CoreMinimal.h"
#include "Kismet/KismetMathLibrary.h"
#include "XRDistanceGrabbable.generated.h"


UCLASS(Blueprintable)
class XR_SAMPLESPROJECT_API UXRDistanceGrabbable : public USceneComponent
{
	GENERATED_BODY()

public:	
	UXRDistanceGrabbable();
		
	void AddGrabDistance(float Offset);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool OnGrabbed(UXRDistanceGrabber* Grabber, const FHitResult& Hit);
	virtual bool OnGrabbed_Implementation(UXRDistanceGrabber* Grabber, const FHitResult& Hit);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGrabReleased();
	virtual void OnGrabReleased_Implementation();

private:
	bool bIsGrabbed{ false };
	UXRDistanceGrabber* CurrentGrabber;
	float GrabDistance;
	FVector3d GrabOffset;
};
