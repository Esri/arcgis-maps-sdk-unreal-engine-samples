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
		auto departure = attributes->GetNumberField("EstDepartureTime");
		auto arrival = attributes->GetNumberField("EstArrivalTime");
		FDateTime datetimeOffset = FDateTime::FromUnixTimestamp(timestampMS);
		FDateTime departureTime = FDateTime::FromUnixTimestamp(departure);
		FDateTime arrivalTime = FDateTime::FromUnixTimestamp(arrival);
		auto dateTimeStamp = datetimeOffset.GetDate();
		auto planeFeature = FPlaneFeature::Create(Name, x, y, z, heading, speed, dateTimeStamp);
		PlaneFeatures.Add(planeFeature);
	}
}

void AStreamLayerQuery::DisplayPlaneData()
{
	if(PlaneFeatures.IsEmpty())
	{
		return;
	}
	
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
	gObj->LocationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(gObj->LocationComponent->GetRotation()->GetPitch(),
		gObj->LocationComponent->GetRotation()->GetRoll(),PlaneFeatures[i].attributes.heading));
	i++;
}

void AStreamLayerQuery::BeginPlay()
{
	Super::BeginPlay();
	Connect();
	FTimerHandle SpawnPlanes;
	GetWorldTimerManager().SetTimer(SpawnPlanes, this, &AStreamLayerQuery::DisplayPlaneData, 0.5f, true);
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
	TArray<AActor*> result;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlaneController::StaticClass(), result);

	for (auto Actor : result)
	{
		auto Plane = Cast<APlaneController>(Actor);
		
		if (Plane)
		{
			Plane->PredictPoint(GetWorld()->GetDeltaSeconds() * 10);
			Plane->LocationComponent->SetPosition(Plane->predictedPoint);
		}
	}
}

void AStreamLayerQuery::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WebSocket->Close();
	PlaneFeatures.Empty();
}
