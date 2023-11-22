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
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Components/WidgetComponent.h"
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
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FTimespan totalTime;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	FTimespan timeLeft;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	bool bActive;
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
	UStaticMeshComponent* mesh;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UStaticMesh* planeModel = LoadObject<UStaticMesh>(
		nullptr, TEXT("/Game/SampleViewer/Samples/StreamLayer/PlaneModel/3D_Model/Boeing_747.Boeing_747"));
	UArcGISPoint* predictedPoint;
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	UWidgetComponent* TextComponent;
};
