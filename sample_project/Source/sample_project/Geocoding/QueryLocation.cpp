// Fill out your copyright notice in the Description page of Project Settings.


#include "QueryLocation.h"

// Sets default values
AQueryLocation::AQueryLocation()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set the components 
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetWorldScale3D(MeshScale);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/SampleViewer/Samples/Routing/Geometries/Cube.Cube"));
	if (MeshAsset.Succeeded()) {
		MeshComponent->SetStaticMesh(MeshAsset.Object);
	}

	ArcGISLocation = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("ArcGISLocation"));
	ArcGISLocation->SetupAttachment(Root);
}

// Called when the game starts or when spawned
void AQueryLocation::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AQueryLocation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldUpdateElevation) {

		float TraceLength = 10000000.;
		float HeightOffset = 200.;
		FHitResult TraceHit;
		FVector3d WorldLocation;
		bool bTraceSuccess = false;

		// Do a line trace to determine the height of breadcrumbs
		WorldLocation = FVector3d(
			GetActorLocation().X, GetActorLocation().Y, TraceLength / 2.f);
		bTraceSuccess = GetWorld()->LineTraceSingleByChannel(TraceHit, WorldLocation,
			WorldLocation + TraceLength * FVector3d::DownVector, ECC_Visibility, FCollisionQueryParams());
		
		if (bTraceSuccess) {
			WorldLocation.Z =  TraceHit.ImpactPoint.Z;
			UE_LOG(LogTemp, Warning, TEXT("hit %s"),*TraceHit.GetActor()->GetActorLabel());
		}

		SetActorLocation(WorldLocation);
		RemoveTickPrerequisiteComponent(ArcGISLocation);
		bShouldUpdateElevation = false;

	}
		//UE_LOG(LogTemp, Warning, TEXT("new elevation %f"),GetActorLocation().Z);
}

void AQueryLocation::SetGeographicLocationXY(UArcGISPoint* Point)
{
	ArcGISLocation->SetPosition(Point);
	UE_LOG(LogTemp, Warning, TEXT("heeeeeerrrrreeeee"));
	AddTickPrerequisiteComponent(ArcGISLocation);

	bShouldUpdateElevation = true;
}

