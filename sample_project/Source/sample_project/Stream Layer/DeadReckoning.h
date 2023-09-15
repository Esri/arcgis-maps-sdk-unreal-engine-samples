// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeadReckoning.generated.h"

UCLASS()
class SAMPLE_PROJECT_API ADeadReckoning : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ADeadReckoning();
	static double ToRadians(double degrees);
	static double ToDegrees(double radians);
	static double DistanceFromLatLon(double lon1, double lat1, double lon2, double lat2);
	static TArray<double> DeadReckoningPoint(double speed, double timespan, TArray<double> currentPoint, double headingDegrees);
	static TArray<double> MoveByDistanceAndHeading(TArray<double> currentPoint, double distanceMeters, double headingDegrees);
	static double InitialBearingTo(TArray<double> orgPoint, TArray<double> dstPoint);
	static double Wrap360(double degrees);
	static TArray<double> MoveTowards(TArray<double> targetLocation, TArray<double> currentLocation, double maxDistanceDelta);
	
	inline static double earthRadiusMeters = 6356752.3142;
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
