// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaneController.h"
#include "DeadReckoning.h"

// Sets default values
APlaneController::APlaneController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UStaticMeshComponent* mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane Mesh"));
	mesh->SetStaticMesh(planeModel);
	mesh->SetupAttachment(RootComponent);
	LocationComponent = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("Location Component"));
}

void APlaneController::PredictPoint(double intervalMilliseconds)
{
	auto cGroundSpeedKnots = featureData.attributes.speed;
	auto metersPerSec = cGroundSpeedKnots * 0.51444444444;
	auto simulationSpeedFactor = 1.5;
	auto timespanSec = (intervalMilliseconds / 1000.0) * simulationSpeedFactor;
	TArray<double> currentPoint = { featureData.predictedPoint.x, featureData.predictedPoint.y, featureData.predictedPoint.z };
	auto headingDegrees = featureData.attributes.heading;
	auto drPoint = ADeadReckoning::DeadReckoningPoint(metersPerSec, timespanSec, currentPoint, headingDegrees);
	featureData.predictedPoint.x = drPoint[0];
	featureData.predictedPoint.y = drPoint[1];
	featureData.predictedPoint.z = currentPoint[2];
}

FPlaneFeature FPlaneFeature::Create(FString name, double x, double y, double z, float heading, float speed, FDateTime dateTimeStamp)
{
	FPlaneFeature planeFeature;
	planeFeature.Geometry.x = x;
	planeFeature.Geometry.y = y;
	planeFeature.Geometry.z = z;
	planeFeature.attributes.Name = name;
	planeFeature.attributes.heading = heading;
	planeFeature.attributes.speed = speed;
	planeFeature.attributes.dateTimeStamp = dateTimeStamp;
	planeFeature.predictedPoint.x = planeFeature.Geometry.x;
	planeFeature.predictedPoint.y = planeFeature.Geometry.y;
	planeFeature.predictedPoint.z = planeFeature.Geometry.z;
	return planeFeature;
}

// Called when the game starts or when spawned
void APlaneController::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void APlaneController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

