// Fill out your copyright notice in the Description page of Project Settings.


#include "Breadcrumb.h"

// Sets default values
ABreadcrumb::ABreadcrumb()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;


	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MeshComponent->SetupAttachment(Root);
	//MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//MeshComponent->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComponent->SetWorldScale3D(FVector3d(2.));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/SampleViewer/Samples/Routing/Geometries/Cube.Cube"));

	if (MeshAsset.Succeeded()) {
		MeshComponent->SetStaticMesh(MeshAsset.Object);
	}

	ArcGISLocation = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("ArcGISLocation"));
	ArcGISLocation->SetupAttachment(Root);

	//SetActorEnableCollision(false);

}

// Called when the game starts or when spawned
void ABreadcrumb::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ABreadcrumb::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

