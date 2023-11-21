// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "PlaneController.generated.h"

USTRUCT(BlueprintType)
struct FPlaneProperties
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float heading;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float speed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDateTime dateTimeStamp;
};

USTRUCT(BlueprintType)
struct FPlaneGeometry
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double x;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double y;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double z;
};

USTRUCT(BlueprintType)
struct FPlaneFeature
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneProperties attributes;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneGeometry Geometry;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneGeometry predictedPoint;

	static FPlaneFeature Create(FString name, double x, double y, double z, float heading, float speed, FDateTime dateTimeStamp);
};

UCLASS()
class SAMPLE_PROJECT_API APlaneController : public AActor
{
	GENERATED_BODY()
	
public:	
	APlaneController();
	void PredictPoint(double intervalMilliseconds);
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FPlaneFeature featureData;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UArcGISLocationComponent* LocationComponent;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UStaticMesh* planeModel = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/SampleViewer/Samples/StreamLayer/PlaneModel/3D_Model/Boeing_747.Boeing_747"));
	UArcGISPoint* predictedPoint;
};