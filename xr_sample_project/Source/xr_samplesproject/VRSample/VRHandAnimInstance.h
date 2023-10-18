// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VRHandAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class XR_SAMPLESPROJECT_API UVRHandAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GripAxis;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float TriggerAxis;
	
};
