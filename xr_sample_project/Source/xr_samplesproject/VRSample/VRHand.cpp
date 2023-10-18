// Fill out your copyright notice in the Description page of Project Settings.


#include "VRHand.h"

// Sets default values
AVRHand::AVRHand()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	if(bIsLeft)
	{
		LeftMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftMotionController"));
		LeftMotionController->SetupAttachment(RootComponent);
		LeftMotionController->SetTrackingSource(EControllerHand::Left);
		LeftHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Left Hand Mesh"));
		LeftHandMesh->SetupAttachment(LeftMotionController);
		LeftHandMesh->RegisterComponent();
		LeftHandMesh->SetSkeletalMesh(LeftMesh);
		LeftHandMesh->SetRelativeRotation(FRotator(-90.0f, -90.0f, 0.0f));	
	}
	else
	{
		RightMotionController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightMotionController"));
		RightMotionController->SetupAttachment(RootComponent);
		RightMotionController->SetTrackingSource(EControllerHand::Right);
		RightHandMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Right Hand Mesh"));
		RightHandMesh->SetupAttachment(RightMotionController);
		RightHandMesh->RegisterComponent();
		RightHandMesh->SetSkeletalMesh(RightMesh);
		RightHandMesh->SetRelativeRotation(FRotator(90.0f, -90.0f, 0.0f));	
	}
}

// Called when the game starts or when spawned
void AVRHand::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AVRHand::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

