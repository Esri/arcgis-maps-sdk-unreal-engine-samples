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
	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Game/SampleViewer/Samples/Routing/Geometries/Marker.Marker"));

	if (MeshAsset.Succeeded()) {
		MeshComponent->SetStaticMesh(MeshAsset.Object);
	}


	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));
	TextComponent->SetupAttachment(Root);
	TextComponent->SetWorldScale3D(FVector3d(200.));
	TextComponent->SetRelativeLocation(FVector3d(2000, 0, GetActorScale3D().Z * 9000));
	TextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextComponent->SetTextRenderColor(FColor::Black);
	
	static ConstructorHelpers::FObjectFinder<UMaterial> TextMaterialAsset(TEXT("Material'/Game/SampleViewer/SharedResources/Materials/TextMaterialWithBackground.TextMaterialWithBackground'"));
	if (TextMaterialAsset.Succeeded()) {
		TextComponent->SetMaterial(0, TextMaterialAsset.Object);
	}

	ArcGISLocation = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("ArcGISLocation"));
	ArcGISLocation->SetupAttachment(Root);




	static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAssetxxx(TEXT("/Game/SampleViewer/Samples/Routing/Geometries/Cube.Cube"));
	if (MeshAssetxxx.Succeeded()) {
		RouteMesh = MeshAssetxxx.Object;
	}
}

// Called when the game starts or when spawned
void AQueryLocation::BeginPlay()
{
	Super::BeginPlay();

	PawnActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

	FrameCounter = FramesToWait;





	Map = Cast<AArcGISMapActor>(UGameplayStatics::GetActorOfClass(this, AArcGISMapActor::StaticClass()));

	MapComp= Cast<UArcGISMapComponent>(Map->FindComponentByClass(UArcGISMapComponent::StaticClass()));




}

// Called every frame
void AQueryLocation::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);






	if (bShouldUpdateElevation) {

		float TraceLength = 1000000.;
		FVector3d TraceDirection;
		FHitResult TraceHit;
		FVector3d WorldLocation;
		bool bTraceSuccess = false;

	//	if (PawnActor->GetActorLocation().Equals(GetActorLocation())) {

	//UE_LOG(LogTemp, Warning, TEXT("/*/*/*/*/*/*/*/*/*/*/*/*/*: %s"), *(PawnActor->GetActorLocation().ToString()));
	//	}

		WorldLocation =  PawnActor->GetActorLocation();
		//WorldLocation = GetActorLocation() + GetActorUpVector() * (TraceLength / 2.);
			
			//FVector3d(GetActorLocation().X, GetActorLocation().Y, TraceLength / 2.f);
		TraceDirection =  GetActorUpVector()*-1.;
		bTraceSuccess = GetWorld()->LineTraceSingleByChannel(TraceHit, WorldLocation,
			WorldLocation + TraceDirection*TraceLength, ECC_Visibility, FCollisionQueryParams());
		
//UE_LOG(LogTemp, Warning, TEXT("start: %s"),*WorldLocation.ToString());
//UE_LOG(LogTemp, Warning, TEXT("down: %s"),*TraceDirection.ToString());



////////////////////////////////////////////////////////
//FVector3d TraceDirection2;
//FHitResult TraceHit2;
//FVector3d WorldLocation2;
//bool bTraceSuccess2 = false;
//
//
//WorldLocation2 = GetActorLocation() - GetActorUpVector() * (TraceLength / 2.);
//
////FVector3d(GetActorLocation().X, GetActorLocation().Y, TraceLength / 2.f);
//TraceDirection2 = GetActorUpVector() ;
//bTraceSuccess = GetWorld()->LineTraceSingleByChannel(TraceHit2, WorldLocation2,
//	WorldLocation2 + TraceDirection2 * TraceLength, ECC_Visibility, FCollisionQueryParams());
//
//float dist = FVector3d::Distance(WorldLocation, TraceHit.ImpactPoint);
//float dist2 = FVector3d::Distance(WorldLocation2, TraceHit2.ImpactPoint);
//
//if (dist > dist2) {
//
//UE_LOG(LogTemp, Warning, TEXT("**************upside down: %f  %f"),dist, dist2);
//WorldLocation = WorldLocation2;
//TraceDirection = TraceDirection2;
//TraceHit = TraceHit2;
//bTraceSuccess = bTraceSuccess2;
//}
////////////////////////////


		if (bTraceSuccess) {
			//WorldLocation.Z =  TraceHit.ImpactPoint.Z;
UE_LOG(LogTemp, Warning, TEXT("hit : %s    impact point:  %s  elevation: %f"),
	*TraceHit.GetActor()->GetActorLabel(),*TraceHit.ImpactPoint.ToString(),  ArcGISLocation->GetPosition()->GetZ() - TraceHit.Distance/100.);
		

if (FrameCounter>0 && TraceHit.ImpactPoint != GetActorLocation() ) {

	UE_LOG(LogTemp, Warning, TEXT("updating the hit %d: %s   vs %s "), FrameCounter, *TraceHit.ImpactPoint.ToString(), *GetActorLocation().ToString());


	FrameCounter = FramesToWait;
			SetActorLocation(TraceHit.ImpactPoint);




}
else if (FrameCounter<=0) {
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
	UE_LOG(LogTemp, Warning, TEXT("ok %d "), FrameCounter);

	FrameCounter--;

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
		else {
			UE_LOG(LogTemp, Warning, TEXT("no hit detected"));

		}

		//SetActorLocation(WorldLocation);



		//FTransform t = MapComp->GetENUAtLocationToViewENUTransform(WorldLocation);
		//UE_LOG(LogTemp, Warning, TEXT("%f   %f   %f"), t.Rotator().Pitch, t.Rotator().Yaw, t.Rotator().Roll);
		//PawnActor->SetActorLocationAndRotation(WorldLocation+FVector3d(0,0,50000), FRotator3d( - 90, 0, 0));
		//PawnActor->SetActorLocationAndRotation(WorldLocation + FVector3d(0, 0, 50000), GetActorRotation()+t.Rotator()+ FRotator3d(-90, 0, 0));
		
		

		//UArcGISLocationComponent* PawnLocation = PawnActor->FindComponentByClass<UArcGISLocationComponent>();
		//if (PawnLocation == nullptr) {
		//	UE_LOG(LogTemp, Warning, TEXT("############# pawn location is null") );

		//	return;
		//}

		
		

		//UE_LOG(LogTemp, Warning, TEXT("before: %f"), PawnActor->GetActorLocation().Z);
		//PawnLocation->SetPosition(
		//	UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		//		PawnLocation->GetPosition()->GetX(),
		//		PawnLocation->GetPosition()->GetY(),
		//		PawnLocation->GetPosition()->GetZ() + 50000,
		//		UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
		//UE_LOG(LogTemp, Warning, TEXT("after: %f"), PawnActor->GetActorLocation().Z);
		///////////////////
		//UE_LOG(LogTemp, Warning, TEXT("before: %f"), PawnLocation->GetPosition()->GetZ());
		//PawnActor->SetActorLocation(PawnActor->GetActorLocation() + FVector3d(0., 0., 1000.));
		//UE_LOG(LogTemp, Warning, TEXT("after: %f"), PawnLocation->GetPosition()->GetZ());



		//PawnLocation->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		//	ArcGISLocation->GetPosition()->GetX(),
		//	ArcGISLocation->GetPosition()->GetY(),
		//	ArcGISLocation->GetPosition()->GetZ() + 1000,
		//	UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));


		//PawnLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(0.,0.,0.));

		//UE_LOG(LogTemp, Warning, TEXT("new rotation %f  %f  %f"), 
		//	PawnActor->GetActorRotation().Yaw,
		//	PawnActor->GetActorRotation().Pitch,
		//	PawnActor->GetActorRotation().Roll
		//);

		

		//UArcGISLocationComponent* ArcGISLocation = PawnActor->FindComponentByClass<UArcGISLocationComponent>();


		//if (ArcGISLocation != nullptr) {
		//	ArcGISLocation->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		//		
		//		UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
		//}
		//else {
		//	UE_LOG(LogTemp, Warning, TEXT("component not found"));
		//}


	}
}

void AQueryLocation::ApplyQueryResults(UArcGISPoint* Point, FString InAddress)
{
	ArcGISLocation->SetPosition(Point);
	ArcGISLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(90., 0., 0.));

	UArcGISLocationComponent* PawnLocation = PawnActor->FindComponentByClass<UArcGISLocationComponent>();
	
	PawnLocation->SetPosition(Point);
	PawnLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(0.,0.,0.));

	AddTickPrerequisiteActor(PawnActor);
	AddTickPrerequisiteComponent(ArcGISLocation);
	Address = InAddress;
	FrameCounter = FramesToWait;
	bShouldUpdateElevation = true;
}

//https://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer/reverseGeocode?f=pjson&featureTypes=&location=-74,40.7