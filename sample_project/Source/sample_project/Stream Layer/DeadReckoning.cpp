// Fill out your copyright notice in the Description page of Project Settings.


#include "DeadReckoning.h"

// Sets default values
ADeadReckoning::ADeadReckoning()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

double ADeadReckoning::ToRadians(double degrees)
{
	return degrees * PI / 180.0;
}

double ADeadReckoning::ToDegrees(double radians)
{
	return radians * 180.0 / PI;
}

double ADeadReckoning::DistanceFromLatLon(double lon1, double lat1, double lon2, double lat2)
{
	auto radLon1 = ToRadians(lon1);
	auto radLat1 = ToRadians(lat1);
	auto radLon2 = ToRadians(lon2);
	auto radLat2 = ToRadians(lat2);
	return FMath::Acos(FMath::Sin(radLat1) * FMath::Sin(radLat2) + FMath::Cos(radLat1) * FMath::Cos(radLat2) * FMath::Cos(radLon2 - radLon1)) * earthRadiusMeters;
	
}

TArray<double> ADeadReckoning::DeadReckoningPoint(double speed, double timespan, TArray<double> currentPoint, double headingDegrees)
{
	auto predictiveDistance = speed * timespan;
	return MoveByDistanceAndHeading(currentPoint, predictiveDistance, headingDegrees);
}

TArray<double> ADeadReckoning::MoveByDistanceAndHeading(TArray<double> currentPoint, double distanceMeters, double headingDegrees)
{
	auto distRatio = distanceMeters / earthRadiusMeters;
	auto distRatioSine = FMath::Sin(distRatio);
	auto distRatioCosine = FMath::Cos(distRatio);

	auto startLonRad = ToRadians(currentPoint[0]);
	auto startLatRad = ToRadians(currentPoint[1]);

	auto startLatCos = FMath::Cos(startLatRad);
	auto startLatSin = FMath::Sin(startLatRad);

	auto endLatRads = FMath::Asin((startLatSin * distRatioCosine) + (startLatCos * distRatioSine * FMath::Cos(ToRadians(headingDegrees))));
	auto endLonRads = startLonRad + FMath::Atan2(FMath::Sin(ToRadians(headingDegrees)) * distRatioSine * startLatCos, distRatioCosine - startLatSin * FMath::Sin(endLatRads));

	auto newLong = ToDegrees(endLonRads);
	auto newLat = ToDegrees(endLatRads);
	auto newPoint = {newLong, newLat};
	return newPoint;
}


// Called when the game starts or when spawned
void ADeadReckoning::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADeadReckoning::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

