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
