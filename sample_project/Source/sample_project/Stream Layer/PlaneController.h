/* Copyright 2023 Esri
*
 * Licensed under the Apache License Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "CoreMinimal.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Actor.h"
#include "PlaneController.generated.h"

USTRUCT(BlueprintType)
struct FPlaneProperties
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FString Name;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Heading;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Speed;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FDateTime DateTimeStamp;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bActive;
};

USTRUCT(BlueprintType)
struct FPlaneGeometry
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double X;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Y;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	double Z;
};

USTRUCT(BlueprintType)
struct FPlaneFeature
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneProperties Attributes;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneGeometry Geometry;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FPlaneGeometry PredictedPoint;

	static FPlaneFeature Create(FString name, double x, double y, double z, float heading, float speed, FDateTime dateTimeStamp);
};

UCLASS()
class SAMPLE_PROJECT_API APlaneController : public AActor
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:
	APlaneController();
	void GetMapComponent();
	void PredictPoint(double intervalMilliseconds);
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	FPlaneFeature FeatureData;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UStaticMeshComponent* Mesh;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UStaticMesh* PlaneModel = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Game/SampleViewer/Samples/StreamLayer/PlaneModel/3D_Model/Boeing_747.Boeing_747"));
	Esri::GameEngine::Geometry::ArcGISPoint PredictedPoint;
	FVector predictedLocation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UWidgetComponent* TextComponent;

	UArcGISMapComponent* MapComponent;
};
