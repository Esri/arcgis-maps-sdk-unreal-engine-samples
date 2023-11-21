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


#include "StreamLayerQuery.h"
#include "WebSocketsModule.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"

AStreamLayerQuery::AStreamLayerQuery()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AStreamLayerQuery::Connect()
{
	WebSocket = FWebSocketsModule::Get().CreateWebSocket("wss://geoeventsample1.esri.com:6143/arcgis/ws/services/FAAStream/StreamServer/subscribe");

	WebSocket->OnConnected().AddLambda([]() -> void {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Connected");
	});

	WebSocket->OnConnectionError().AddLambda([](const FString& Error) -> void {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Not Connected" + Error);
	});

	WebSocket->OnMessage().AddLambda([this](const FString& Message) -> void {
		TryParseAndUpdatePlane(Message);
	});

	WebSocket->Connect();
}

void AStreamLayerQuery::TryParseAndUpdatePlane(FString data)
{
	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(data);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		auto attributes = JsonParsed->GetObjectField("attributes");
		auto coordinates = JsonParsed->GetObjectField("geometry");
		auto x = coordinates->GetNumberField("x");
		auto y = coordinates->GetNumberField("y");
		auto z = coordinates->GetNumberField("z");
		auto Name = attributes->GetStringField("ACID");
		auto heading = attributes->GetNumberField("Heading");
		auto speed = attributes->GetNumberField("GroundSpeedKnots");
		long timestampMS = attributes->GetNumberField("DateTimeStamp");
		FDateTime datetimeOffset = FDateTime::FromUnixTimestamp(timestampMS);
		auto dateTimeStamp = datetimeOffset.GetDate();
		auto planeFeature = FPlaneFeature::Create(Name, x, y, z, heading, speed, dateTimeStamp);
		PlaneFeatures.Add(planeFeature);
	}
}

void AStreamLayerQuery::DisplayPlaneData()
{
	for (auto i = 0; i < PlaneFeatures.Num(); i++)
	{
		FString name = "PlaneController_" + FString::FromInt(i);
		if (auto Plane = FindObject<APlaneController>(ANY_PACKAGE, *name, true))
		{
			//GetDeltaSeconds() * 1000 converts miliseconds to seconds
			Plane->PredictPoint(GetWorld()->GetDeltaSeconds() * 1000);
			Plane->LocationComponent->SetPosition(Plane->predictedPoint);
		}
		else
		{
			FActorSpawnParameters SpawnInfo;
			auto gObj = GetWorld()->SpawnActor<APlaneController>
				(
					APlaneController::StaticClass(),
					GetActorLocation(),
					GetActorRotation(),
					SpawnInfo
					);
			gObj->featureData = PlaneFeatures[i];
			planes.Add(gObj);
			gObj->SetActorLabel(*PlaneFeatures[i].attributes.Name);
			gObj->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
				PlaneFeatures[i].predictedPoint.x, PlaneFeatures[i].predictedPoint.y, PlaneFeatures[i].predictedPoint.z,
				UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
			gObj->LocationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(gObj->LocationComponent->GetRotation()->GetPitch(),gObj->LocationComponent->GetRotation()->GetRoll(),PlaneFeatures[i].attributes.heading));
		}
	}
}

void AStreamLayerQuery::BeginPlay()
{
	Super::BeginPlay();
	Connect();
	FTimerHandle SpawnPlanes;
	GetWorldTimerManager().SetTimer(SpawnPlanes, this, &AStreamLayerQuery::DisplayPlaneData, 0.5f, true);
}

void AStreamLayerQuery::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WebSocket->Close();
	PlaneFeatures.Empty();
}
