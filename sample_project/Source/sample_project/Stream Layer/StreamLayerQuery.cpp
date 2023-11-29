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
	webSocket = FWebSocketsModule::Get().CreateWebSocket(url);

	webSocket->OnConnected().AddLambda([]() -> void {
	});

	webSocket->OnConnectionError().AddLambda([](const FString& Error) -> void {
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, "Not Connected" + Error);
	});

	webSocket->OnMessage().AddLambda([this](const FString& Message) -> void {
		TryParseAndUpdatePlane(Message);
	});

	webSocket->Connect();
}

void AStreamLayerQuery::TryParseAndUpdatePlane(FString Data)
{
	TSharedPtr<FJsonObject> JsonParsed;
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Data);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed))
	{
		auto attributes = JsonParsed->GetObjectField("attributes");
		auto coordinates = JsonParsed->GetObjectField("geometry");
		auto x = coordinates->GetNumberField("x");
		auto y = coordinates->GetNumberField("y");
		auto z = coordinates->GetNumberField("z");
		auto name = attributes->GetStringField("ACID");
		auto heading = attributes->GetNumberField("Heading");
		auto speed = attributes->GetNumberField("GroundSpeedKnots");
		auto timestampMS = attributes->GetNumberField("DateTimeStamp");
		auto datetimeOffset = FDateTime::FromUnixTimestamp(timestampMS);
		auto dateTimeStamp = datetimeOffset.GetDate();
		auto planeFeature = FPlaneFeature::Create(name, x, y, z, heading, speed, dateTimeStamp);
		
		if (planeData.Contains(name))
		{
			planeData[name]->FeatureData = planeFeature;
			planeData[name]->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		planeData[name]->FeatureData.PredictedPoint.X, planeData[name]->FeatureData.PredictedPoint.Y, planeData[name]->FeatureData.PredictedPoint.Z,
		UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
			planeData[name]->LocationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(planeData[name]->LocationComponent->GetRotation()->GetPitch(),
		planeData[name]->LocationComponent->GetRotation()->GetRoll(),planeData[name]->FeatureData.Attributes.Heading));
		}
		else
		{
			if(planeData.Num() < threshold)
			{
				SpawnPlane(planeFeature);	
			}
		}
	}
}

void AStreamLayerQuery::SpawnPlane(FPlaneFeature PlaneFeature)
{
	FActorSpawnParameters SpawnInfo;
	auto planeActor = GetWorld()->SpawnActor<APlaneController>
		(
			APlaneController::StaticClass(),
			GetActorLocation(),
			GetActorRotation(),
			SpawnInfo
			);
	planeActor->FeatureData = PlaneFeature;
	planeActor->SetActorLabel(*PlaneFeature.Attributes.Name);
	planeActor->LocationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		PlaneFeature.PredictedPoint.X, PlaneFeature.PredictedPoint.Y, PlaneFeature.PredictedPoint.Y,
		UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
	planeActor->LocationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(planeActor->LocationComponent->GetRotation()->GetPitch(),
		planeActor->LocationComponent->GetRotation()->GetRoll(),PlaneFeature.Attributes.Heading));
	planeData.Add(planeActor->FeatureData.Attributes.Name, planeActor);
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
			Plane.Value->LocationComponent->SetPosition(Plane.Value->PredictedPoint);
		}
	}
}

void AStreamLayerQuery::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	webSocket->Close();
}
