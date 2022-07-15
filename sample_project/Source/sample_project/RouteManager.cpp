// Fill out your copyright notice in the Description page of Project Settings.


#include "RouteManager.h"

// Sets default values
ARouteManager::ARouteManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARouteManager::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();
	
}

// Called every frame
void ARouteManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

AActor* ARouteManager::CreateMarker(FVector3d InEnginePosition)
{
	AActor* Marker = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), InEnginePosition, FRotator(0.f));
	//
	
	Marker->AddComponentByClass(USceneComponent::StaticClass(),false, FTransform3d(),false);
	
	UActorComponent* MeshComponent = Marker->AddComponentByClass(UStaticMeshComponent::StaticClass(),false, FTransform3d(),false);
	
	
	
	/*USceneComponent* Root = NewObject<USceneComponent>(Marker, USceneComponent::StaticClass(), TEXT("RootComponent"));
	Marker->SetRootComponent(Root);

	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(Marker, UStaticMeshComponent::StaticClass(), TEXT("MarkerMesh"));
	Marker->addcompo
	MeshComponent->SetupAttachment(Root);

	*/
	if (MarkerMesh != nullptr) {

		Cast<UStaticMeshComponent>(MeshComponent)->SetStaticMesh(MarkerMesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("No mesh"));

	}

	//UArcGISLocationComponent* ArcGISLocation = Marker->CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("ArcGISLocation"));
	//ArcGISLocation->SetupAttachment(Root);

	return Marker;
}

void ARouteManager::SetupInput()
{
	InputComponent = NewObject<UInputComponent>(this);
	InputComponent->RegisterComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &ARouteManager::AddStop);
		EnableInput(GetWorld()->GetFirstPlayerController());
	}

}

void ARouteManager::AddStop()
{


	////using namespace Esri::ArcGISMapsSDK::Utils::GeoCoord;
	//using namespace Esri::GameEngine::Geometry;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FVector WorldLocation;
	FVector WorldDirection;
	float DistanceAboveGround = 5000.0f;
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FVector PlaneOrigin(0.0f, 0.0f, DistanceAboveGround);
	FVector ActorWorldLocation = FMath::LinePlaneIntersection(
		WorldLocation,
		WorldLocation + WorldDirection,
		PlaneOrigin,
		FVector::UpVector);

	FHitResult TraceHit;
	if (GetWorld()->LineTraceSingleByChannel(TraceHit, WorldLocation, WorldLocation + 100000.f * WorldDirection,
		ECC_Visibility, FCollisionQueryParams()))
	{
		if (TraceHit.bBlockingHit)
		{
			if (GEngine) {

				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *TraceHit.GetActor()->GetName()));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Impact Point: %s"), *TraceHit.ImpactPoint.ToString()));
				GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Normal Point: %s"), *TraceHit.ImpactNormal.ToString()));
			}
			UE_LOG(LogTemp, Warning, TEXT("You are hitting: %s"), *TraceHit.GetActor()->GetName());
			UE_LOG(LogTemp, Warning, TEXT("Impact Point: %s"), *TraceHit.ImpactPoint.ToString());
		}
		DrawDebugLine(GetWorld(), WorldLocation, WorldLocation + 100000.f * WorldDirection, FColor::Green, false, 10, 0, 1);
	}

	////UE_LOG(LogTemp, Warning, TEXT("Mouse Location: %f, %f, %f"), ActorWorldLocation.X, ActorWorldLocation.Y, ActorWorldLocation.Z);
	CreateMarker(TraceHit.ImpactPoint);

	//AStaticMeshActor* MyNewActor = GetWorld()->SpawnActor<AStaticMeshActor>();
	//MyNewActor->SetMobility(EComponentMobility::Movable);
	//MyNewActor->SetActorLocation(TraceHit.ImpactPoint);
	//if (MarkerMesh != nullptr) {

	//	MyNewActor->FindComponentByClass<UStaticMeshComponent>()->SetStaticMesh(MyMesh);
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("No mesh"));

	//}

	//Marker = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), TraceHit.ImpactPoint, FRotator(0.f));

	//UArcGISPoint* pos = Marker->ArcGISLocation->GetPosition();

	//ArcGISPoint* pt = new ArcGISPoint(
	//	pos->GetX(), pos->GetY(), pos->GetZ(),
	//	ArcGISSpatialReference(pos->GetSpatialReference()));


	//ArcGISGeometry newgeom = ArcGISGeometryEngine::Project(*pt, ArcGISSpatialReference::WGS84());

	//ArcGISPoint* projpt = new ArcGISPoint(newgeom.GetHandle());

	////FGeoPosition ProjPos = GeoUtils::WebMercatorToWGS84LonLatAlt(FVector3d(pos->GetX(), pos->GetY(), pos->GetZ()));
	//////FGeoPosition ProjPos(0, 0, 0, 4326);
	////UE_LOG(LogTemp, Warning, TEXT("Position: (%f,%f,%f):%d -> (%f,%f,%f):4326"), 
	////	pos->GetX(), pos->GetY(), pos->GetZ(), pos->GetSpatialReference()->GetWKID(),
	////	ProjPos.X, ProjPos.Y, ProjPos.Z );

	//UE_LOG(LogTemp, Warning, TEXT("**Position: (%f,%f,%f):%d -> (%f,%f,%f):4326"),
	//	pos->GetX(), pos->GetY(), pos->GetZ(), *pos->GetSpatialReference()->GetWKText(),
	//	projpt->GetX(), projpt->GetY(), projpt->GetZ());


	//newgeom = ArcGISGeometryEngine::Project(*projpt,
	//	ArcGISSpatialReference(pos->GetSpatialReference()));


	//pt = new ArcGISPoint(newgeom.GetHandle());
	//UE_LOG(LogTemp, Warning, TEXT("converted back to %d: (%f,%f,%f)"), pt->GetSpatialReference().GetWKID(),
	//	pt->GetX(), pt->GetY(), pt->GetZ());


	////Marker->FindComponentByClass<UStaticMeshComponent>()->SetStaticMesh(MyMesh);
	////UStaticMeshComponent* MeshComponent;
	////MeshComponent = MyNewActor->GetStaticMeshComponent();
	////if (MyMesh)
	////	MeshComponent->SetStaticMesh(MyMesh);


}

