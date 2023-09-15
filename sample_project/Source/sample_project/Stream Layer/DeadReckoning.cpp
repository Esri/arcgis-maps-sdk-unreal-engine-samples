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

double ADeadReckoning::InitialBearingTo(TArray<double> orgPoint, TArray<double> dstPoint)
{
	if (orgPoint[0] == dstPoint[0] && orgPoint[1] == dstPoint[1])
	{
		return double.NaN;
	}

	// see mathforum.org/library/drmath/view/55417.html for derivation

	double phi1 = ToRadians(orgPoint[1]);
	double phi2 = ToRadians(dstPoint[1]);
	double deltaLamda = ToRadians(dstPoint[0] - orgPoint[0]);

	double x = FMath::Cos(phi1) * FMath::Sin(phi2) - FMath::Sin(phi1) * FMath::Cos(phi2) * FMath::Cos(deltaLamda);
	double y = FMath::Sin(deltaLamda) * FMath::Cos(phi2);
	double theta = FMath::Atan2(y, x);

	double bearing = ToDegrees(theta);

	return Wrap360(bearing);
}

double ADeadReckoning::Wrap360(double degrees)
{
	if (0 <= degrees && degrees < 360) return degrees; // avoid rounding due to arithmetic ops if within range
	return (degrees % 360 + 360) % 360; // sawtooth wave p:360, a:360
}

TArray<double> ADeadReckoning::MoveTowards(TArray<double> targetLocation, TArray<double> currentLocation, double maxDistanceDelta)
{
	double iBearing = InitialBearingTo(currentLocation, targetLocation);
	double distance = DistanceFromLatLon(currentLocation[0], currentLocation[1], targetLocation[0], targetLocation[1]);
	if (distance >= maxDistanceDelta)
	{
		distance = maxDistanceDelta;
	}
	return MoveByDistanceAndHeading(currentLocation, distance, iBearing);
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

