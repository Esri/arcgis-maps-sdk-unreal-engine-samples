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
#include "DeadReckoning.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/API/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "Kismet/GameplayStatics.h"

APlaneController::APlaneController()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* rootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = rootComponent;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Plane Mesh"));
	Mesh->SetStaticMesh(PlaneModel);
	Mesh->SetWorldRotation(FRotator(0.0f, 90.0f, 0.0f));
	Mesh->SetupAttachment(rootComponent);

	UWidgetBlueprintGeneratedClass* Widget = LoadObject<UWidgetBlueprintGeneratedClass>(nullptr, TEXT("WidgetBlueprint'/Game/SampleViewer/Samples/StreamLayer/UserInterface/wbp_PlaneLabel.wbp_PlaneLabel_c'"));
	TextComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("TextComponent"));
	TextComponent->SetupAttachment(rootComponent);
	TextComponent->SetWidgetClass(Widget);
	TextComponent->SetWorldLocation(FVector(0.0f, 0.0f, 500.0f));
	TextComponent->SetWorldRotation(FRotator(0.0f, 180.0f, 0.0f));
	TextComponent->SetDrawSize(FVector2d(300.0f, 100.0f));
}

void APlaneController::GetMapComponent()
{
	const auto mapComponentActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass());
	MapComponent = Cast<AArcGISMapActor>(mapComponentActor)->GetMapComponent();
}

void APlaneController::PredictPoint(double intervalMilliseconds)
{
	auto cGroundSpeedKnots = FeatureData.Attributes.Speed;
	//0.51444444444 = conversion rate for ground speed knots to meters per second
	auto metersPerSec = cGroundSpeedKnots * 0.51444444444;
	auto simulationSpeedFactor = 0.01;
	auto timespanSec = (intervalMilliseconds / 1000.0) * simulationSpeedFactor;
	TArray<double> currentPoint = {FeatureData.PredictedPoint.X, FeatureData.PredictedPoint.Y, FeatureData.PredictedPoint.Z};
	auto headingDegrees = FeatureData.Attributes.Heading;
	auto drPoint = ADeadReckoning::DeadReckoningPoint(metersPerSec, timespanSec, currentPoint, headingDegrees);
	FeatureData.PredictedPoint.X = drPoint[0];
	FeatureData.PredictedPoint.Y = drPoint[1];
	FeatureData.PredictedPoint.Z = currentPoint[2];
	PredictedPoint = Esri::GameEngine::Geometry::ArcGISPoint(FeatureData.PredictedPoint.X,
		FeatureData.PredictedPoint.Y, FeatureData.PredictedPoint.Z,
		Esri::GameEngine::Geometry::ArcGISSpatialReference(4326));
	auto newLocation = MapComponent->GetView()->APIObject->GeographicToWorld(PredictedPoint);
	predictedLocation = MapComponent->ToEnginePosition(newLocation);
}

FPlaneFeature FPlaneFeature::Create(FString name, double x, double y, double z, float heading, float speed, FDateTime dateTimeStamp)
{
	FPlaneFeature planeFeature;
	planeFeature.Geometry.X = x;
	planeFeature.Geometry.Y = y;
	planeFeature.Geometry.Z = z;
	planeFeature.Attributes.Name = name;
	planeFeature.Attributes.Heading = heading;
	planeFeature.Attributes.Speed = speed;
	planeFeature.Attributes.DateTimeStamp = dateTimeStamp;
	planeFeature.PredictedPoint.X = planeFeature.Geometry.X;
	planeFeature.PredictedPoint.Y = planeFeature.Geometry.Y;
	planeFeature.PredictedPoint.Z = planeFeature.Geometry.Z;
	return planeFeature;
}

void APlaneController::BeginPlay()
{
	Super::BeginPlay();

	GetMapComponent();
}
