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

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "XRDistanceGrabbable.generated.h"


UCLASS(Blueprintable)
class XR_SAMPLESPROJECT_API UXRDistanceGrabbable : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UXRDistanceGrabbable();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnTargetted(UXRDistanceGrabber* Grabber);
	virtual void OnTargetted_Implementation(UXRDistanceGrabber* Grabber);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	bool OnGrabbed(UXRDistanceGrabber* Grabber, const FHitResult& Hit);
	virtual bool OnGrabbed_Implementation(UXRDistanceGrabber* Grabber, const FHitResult& Hit);
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	void OnGrabReleased();
	virtual void OnGrabReleased_Implementation();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool bIsGrabbed{ false };
	UXRDistanceGrabber* CurrentGrabber;
	float GrabDistance;
	FVector3d GrabOffset;
};
