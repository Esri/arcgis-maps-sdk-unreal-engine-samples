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
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAssetPin(TEXT("/Game/SampleViewer/Samples/Routing/Geometries/Marker.Marker"));

	if (MeshAssetPin.Succeeded()) {
		PinMesh = MeshAssetPin.Object;
		//MeshComponent->SetStaticMesh(MeshAsset.Object);
	}

	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAssetPoint(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	if (MeshAssetPoint.Succeeded()) {
		PointMesh = MeshAssetPoint.Object;
	}
	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));
	TextComponent->SetupAttachment(Root);

	TextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextComponent->SetTextRenderColor(FColor::Black);
	
	static ConstructorHelpers::FObjectFinder<UMaterial> MaterialAssetPoint(TEXT("Material'/Game/SampleViewer/Samples/Routing/Materials/M_MarkerHead.M_MarkerHead'"));
	if (MaterialAssetPoint.Succeeded()) {
		PointMaterial = MaterialAssetPoint.Object;
	}	
	static ConstructorHelpers::FObjectFinder<UMaterial> TextMaterialAsset(TEXT("Material'/Game/SampleViewer/SharedResources/Materials/TextMaterialWithBackground.TextMaterialWithBackground'"));
	if (TextMaterialAsset.Succeeded()) {
		TextComponent->SetMaterial(0, TextMaterialAsset.Object);
	}

	ArcGISLocation = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("ArcGISLocation"));
	ArcGISLocation->SetupAttachment(Root);





}

// Called when the game starts or when spawned
void AQueryLocation::BeginPlay()
{
	Super::BeginPlay();

	PawnActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);






	Map = Cast<AArcGISMapActor>(UGameplayStatics::GetActorOfClass(this, AArcGISMapActor::StaticClass()));

	MapComp= Cast<UArcGISMapComponent>(Map->FindComponentByClass(UArcGISMapComponent::StaticClass()));




}

// Called every frame
void AQueryLocation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);



	if (bShouldUpdateElevation) {

		if (RaycastCounter >= MaxRaycastAttemts) {
			bShouldUpdateElevation = false;
			UE_LOG(LogTemp, Warning, TEXT("no raycast hit detected after %d attempts"), MaxRaycastAttemts);
			return;
		}

		bool bTraceSuccess = false;
		float TraceLength = 1000000.;
		FVector3d TraceDirection = GetActorUpVector()*-1.;
		FVector3d WorldLocation = PawnActor->GetActorLocation();
		FHitResult TraceHit;
		RaycastCounter++;

		bTraceSuccess = GetWorld()->LineTraceSingleByChannel(TraceHit, WorldLocation,
			WorldLocation + TraceDirection*TraceLength, ECC_Visibility, FCollisionQueryParams());

		if (bTraceSuccess && TraceHit.GetActor()->GetClass()==AArcGISMapActor::StaticClass()) {
UE_LOG(LogTemp, Warning, TEXT("hit : %s    impact point:  %s  elevation: %f"),
	*TraceHit.GetActor()->GetActorLabel(),*TraceHit.ImpactPoint.ToString(),  ArcGISLocation->GetPosition()->GetZ() - TraceHit.Distance/100.);
		

		if (StableFramesCounter < FramesToWaitForLoading && TraceHit.ImpactPoint != GetActorLocation() ) {

UE_LOG(LogTemp, Warning, TEXT("updating the hit %d: %s   vs %s "), StableFramesCounter, *TraceHit.ImpactPoint.ToString(), *GetActorLocation().ToString());

			StableFramesCounter = 0;
					SetActorLocation(TraceHit.ImpactPoint);
		}
		else if (StableFramesCounter >= FramesToWaitForLoading) {
UE_LOG(LogTemp, Warning, TEXT("is stable. finishing up "));

					SetActorLocation(TraceHit.ImpactPoint);

					RemoveTickPrerequisiteComponent(ArcGISLocation);
					RemoveTickPrerequisiteActor(PawnActor);
					bShouldUpdateElevation = false;		
		
		
					PawnActor->SetActorLocation(TraceHit.ImpactPoint - TraceDirection * 100000);
					PawnActor->SetActorRotation(TraceDirection.Rotation());

					TextComponent->SetText(FText::FromString(Address));
					TextComponent->SetWorldRotation(PawnActor->GetActorRotation() + FRotator3d(180, 0, 180));

		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("ok %d "), StableFramesCounter);
			StableFramesCounter++;
		}


			//DrawDebugDirectionalArrow
			//(
			//	PawnActor->GetWorld(),
			//	WorldLocation,
			//	WorldLocation + TraceDirection * 100000.,
			//	50,
			//	FColor::Blue,
			//	true,
			//	1000,
			//	10,
			//	1
			//);

			//USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			//SplineMesh->SetMobility(EComponentMobility::Movable);
			//SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			//SplineMesh->RegisterComponent();
			//SplineMesh->SetMobility(EComponentMobility::Movable);

			//SplineMesh->SetStartAndEnd(
			//	WorldLocation,
			//	TraceDirection,
			//	WorldLocation + TraceDirection * TraceLength,
			//	TraceDirection);

			//SplineMesh->SetStartScale(FVector2d(1.));
			//SplineMesh->SetEndScale(FVector2d(1.));

			//SplineMesh->SetStaticMesh(RouteMesh);
		}
	}
}

void AQueryLocation::SetupAddressQuery(UArcGISPoint* InPoint, FString InAddress)
{
	ArcGISLocation->SetPosition(InPoint);
	ArcGISLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(90., 0., 0.));

	UArcGISLocationComponent* PawnLocation = PawnActor->FindComponentByClass<UArcGISLocationComponent>();
	
	PawnLocation->SetPosition(InPoint);
	PawnLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(0.,0.,0.));

	AddTickPrerequisiteActor(PawnActor);
	AddTickPrerequisiteComponent(ArcGISLocation);
	bIsAddressQuery = true;
	Address = InAddress;

	StableFramesCounter = 0;
	RaycastCounter = 0;
	bShouldUpdateElevation = true;

	MeshComponent->SetStaticMesh(PinMesh);

	TextComponent->SetWorldScale3D(FVector3d(200.));
	TextComponent->SetRelativeLocation(FVector3d(2000, 0, GetActorScale3D().Z * 9000));
}

void AQueryLocation::SetupLocationQuery(FVector3d InLocation)
{
	
	
	
	SetActorLocation(InLocation);
	//ArcGISLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(90., 0., 0.));
	UArcGISRotation* PawnRotation = PawnActor->FindComponentByClass<UArcGISLocationComponent>()->GetRotation();

	ArcGISLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(
		PawnRotation->GetPitch(), PawnRotation->GetRoll(), PawnRotation->GetHeading()));
	



	AddTickPrerequisiteComponent(ArcGISLocation);
	bIsAddressQuery = false;
	Address = TEXT("");

	StableFramesCounter = 0;
	RaycastCounter = 0;
	bShouldUpdateElevation = false;

	MeshComponent->SetStaticMesh(PointMesh);
	MeshComponent->SetMaterial(0, PointMaterial);
	TextComponent->SetWorldScale3D(FVector3d(100.));
	TextComponent->SetRelativeLocation(FVector3d(-1000, 0, GetActorScale3D().Z * 1000));
	TextComponent->SetRelativeRotation(FRotator3d(0, 0, 0));

}

void AQueryLocation::UpdateAddressCue(FString InAddress)
{
	//TextComponent->SetWorldRotation(FRotator3d(
	//	-PawnActor->GetActorRotation().Pitch,
	//	180.+ PawnActor->GetActorRotation().Yaw,
	//	-90+ PawnActor->GetActorRotation().Roll));
	//RemoveTickPrerequisiteComponent(ArcGISLocation);

	TextComponent->SetRelativeRotation(FRotator3d(0, 180, 0));
//TextComponent->SetRelativeRotation( PawnActor->GetActorRotation()+
	//MapComp->GetENUAtLocationToViewENUTransform(GetActorLocation()).Rotator()* FRotator3d(0));
	RemoveTickPrerequisiteComponent(ArcGISLocation);
		
	


	UE_LOG(LogTemp, Warning, TEXT("pawn: %s    soll: %s    ist: %s  "),
		*PawnActor->GetActorRotation().ToString(),
		*FRotator3d(-PawnActor->GetActorRotation().Pitch,180. + PawnActor->GetActorRotation().Yaw,PawnActor->GetActorRotation().Roll).ToString(),
		*(TextComponent->GetRelativeRotation() + GetActorRotation()).ToString()
	);
		



	TextComponent->SetText(FText::FromString(InAddress));
}
