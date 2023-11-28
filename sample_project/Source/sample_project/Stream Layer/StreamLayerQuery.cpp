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
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AStreamLayerQuery::AStreamLayerQuery()
{
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UClass> WidgetAsset(TEXT("WidgetBlueprint'/Game/SampleViewer/Samples/StreamLayer/UserInterface/StreamLayer_wbp.StreamLayer_wbp_C'"));
	if (WidgetAsset.Succeeded()) {
		UIWidgetClass = WidgetAsset.Object;
	}
}

void AStreamLayerQuery::Connect()
{
	WebSocket = FWebSocketsModule::Get().CreateWebSocket("wss://geoeventsample1.esri.com:6143/arcgis/ws/services/FAAStream/StreamServer/subscribe");

	WebSocket->OnConnected().AddLambda([]() -> void {
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
		auto timestampMS = attributes->GetNumberField("DateTimeStamp");
		FDateTime datetimeOffset = FDateTime::FromUnixTimestamp(timestampMS);
		auto dateTimeStamp = datetimeOffset.GetDate();
		auto planeFeature = FPlaneFeature::Create(Name, x, y, z, heading, speed, dateTimeStamp);
		
		if (planeData.Contains(Name))
		{
			planeData[Name]->featureData = planeFeature;
			planeData[Name]->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		planeData[Name]->featureData.predictedPoint.x, planeData[Name]->featureData.predictedPoint.y, planeData[Name]->featureData.predictedPoint.z,
		UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
			planeData[Name]->LocationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(planeData[Name]->LocationComponent->GetRotation()->GetPitch(),
		planeData[Name]->LocationComponent->GetRotation()->GetRoll(),planeData[Name]->featureData.attributes.heading));
		}
		else
		{
			if(planeData.Num() < 100)
			{
				SpawnPlane(planeFeature);	
			}
		}
	}
}

void AStreamLayerQuery::SpawnPlane(FPlaneFeature PlaneFeature)
{
	FActorSpawnParameters SpawnInfo;
	auto gObj = GetWorld()->SpawnActor<APlaneController>
		(
			APlaneController::StaticClass(),
			GetActorLocation(),
			GetActorRotation(),
			SpawnInfo
			);
	gObj->featureData = PlaneFeature;
	gObj->SetActorLabel(*PlaneFeature.attributes.Name);
	gObj->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		PlaneFeature.predictedPoint.x, PlaneFeature.predictedPoint.y, PlaneFeature.predictedPoint.z,
		UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
	gObj->LocationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(gObj->LocationComponent->GetRotation()->GetPitch(),
		gObj->LocationComponent->GetRotation()->GetRoll(),PlaneFeature.attributes.heading));
	planeData.Add(gObj->featureData.attributes.Name, gObj);
}

void AStreamLayerQuery::BeginPlay()
{
	Super::BeginPlay();
	Connect();
	
	// Create the UI and add it to the viewport
	if (UIWidgetClass != nullptr)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			UIWidget->AddToViewport();
			UGameplayStatics::GetPlayerController(GetWorld(), 0)->SetInputMode(FInputModeUIOnly());
		}
	}
}

void AStreamLayerQuery::Tick(float DeltaSeconds)
{
	for (auto Plane : planeData)
	{		
		if (Plane.Value)
		{
			Plane.Value->PredictPoint(GetWorld()->GetDeltaSeconds() * 10);
			Plane.Value->LocationComponent->SetPosition(Plane.Value->predictedPoint);
		}
	}
}

void AStreamLayerQuery::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WebSocket->Close();
}
