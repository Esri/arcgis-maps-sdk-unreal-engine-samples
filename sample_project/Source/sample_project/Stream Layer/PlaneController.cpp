// Fill out your copyright notice in the Description page of Project Settings.


#include "PlaneController.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "DeadReckoning.h"

APlaneController::APlaneController()
{
	PrimaryActorTick.bCanEverTick = true;

	UStaticMeshComponent* mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane Mesh"));
	mesh->SetStaticMesh(planeModel);
	RootComponent = mesh;
	LocationComponent = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("Location Component"));
	LocationComponent->SetupAttachment(RootComponent);
}

void APlaneController::PredictPoint(double intervalMilliseconds)
{
	auto cGroundSpeedKnots = featureData.attributes.speed;
	//0.541 m/s = conversion rate for Ground speed Knots to meters per second
	auto metersPerSec = cGroundSpeedKnots * 0.51444444444;
	auto simulationSpeedFactor = 1.5;
	auto timespanSec = (intervalMilliseconds / 1000.0) * simulationSpeedFactor;
	TArray<double> currentPoint = {featureData.predictedPoint.x, featureData.predictedPoint.y, featureData.predictedPoint.z};
	auto headingDegrees = featureData.attributes.heading;
	auto drPoint = ADeadReckoning::DeadReckoningPoint(metersPerSec, timespanSec, currentPoint, headingDegrees);
	featureData.predictedPoint.x = drPoint[0];
	featureData.predictedPoint.y = drPoint[1];
	featureData.predictedPoint.z = currentPoint[2];
	predictedPoint = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		featureData.predictedPoint.x, featureData.predictedPoint.y, featureData.predictedPoint.z,
		UArcGISSpatialReference::CreateArcGISSpatialReference(4326));
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
