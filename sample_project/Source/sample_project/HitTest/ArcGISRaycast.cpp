/* Copyright 2024 Esri
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

#include "ArcGISRaycast.h"

#include "ArcGISMapsSDK/API/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "Blueprint/UserWidget.h"
#include "Json.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AArcGISRaycast::AArcGISRaycast()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AArcGISRaycast::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PlayerController->bShowMouseCursor = true;
		PlayerController->bEnableClickEvents = true;

		SetupPlayerInputComponent(PlayerController->InputComponent);
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(MappingContext, 0);
		}
	}

	// Create the UI and add it to the viewport
	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		createProperties = UIWidget->FindFunction(FName("CreateProperties"));
		if (UIWidget)
		{
			UIWidget->AddToViewport();
		}
	}
}

void AArcGISRaycast::CreateLink(FString objectID)
{
	webLink = "https://services.arcgis.com/P3ePLMYs2RVChkJx/ArcGIS/rest/services/Buildings_Boston_USA/FeatureServer/0/"
			  "query?f=geojson&where=1=1&objectids=" +
			  objectID + "&outfields=AREA_SQ_FT,DISTRICT,Height,SUBDISTRIC,ZONE_";

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AArcGISRaycast::OnResponseRecieved);
	Request->SetURL(webLink);
	Request->SetVerb("Get");
	Request->ProcessRequest();
}

void AArcGISRaycast::CreateProperties()
{
	AActor* self = this;

	if (createProperties)
	{
		UIWidget->ProcessEvent(createProperties, &self);
	}
}

void AArcGISRaycast::GetHit()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		resultText.Empty();

		FVector Location, Direction;
		const TArray<AActor*> ActorsToIgnore;
		FHitResult HitResult;
		PlayerController->DeprojectMousePositionToWorld(Location, Direction);

		if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), Location, Location + Direction * 10000000.0, TraceTypeQuery1, false, ActorsToIgnore,
												  EDrawDebugTrace::None, HitResult, true))
		{
			auto mapComponent = UArcGISMapComponent::GetMapComponent(this);

			if (!mapComponent)
			{
				UE_LOG(LogTemp, Error, TEXT("Could not find map component."));
				return;
			}

			auto result = mapComponent->GetArcGISRaycastHit(HitResult);

			if (result.Layer != nullptr && result.FeatureId != -1)
			{
				HitLocation->SetActorLocation(HitResult.ImpactPoint);
				featureID = result.FeatureId;
				auto geoPosition = mapComponent->EngineToGeographic(HitResult.ImpactPoint);
				auto point = Esri::GameEngine::Geometry::ArcGISGeometryEngine::Project(geoPosition,
																					   Esri::GameEngine::Geometry::ArcGISSpatialReference::WGS84());
				auto location = StaticCast<const Esri::GameEngine::Geometry::ArcGISPoint*>(&point);

				if (location)
				{
					position = "- Lat: " + FString::SanitizeFloat(location->GetY()) + ", Long: " + FString::SanitizeFloat(location->GetX());
				}

				CreateLink(FString::FromInt(result.FeatureId));
			}
		}
	}
}

FString AArcGISRaycast::GetObjectIDs(FString response, FString outfield)
{
	TSharedPtr<FJsonObject> ResponseObj;
	auto Reader = TJsonReaderFactory<>::Create(response);
	FString property = "";

	if (FJsonSerializer::Deserialize(Reader, ResponseObj))
	{
		TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");

		for (auto feature : Features)
		{
			property = feature->AsObject()->GetObjectField("properties")->GetStringField(outfield);
		}
	}

	return property;
}

void AArcGISRaycast::OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	if (!bConnectedSucessfully)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not connect to feature server. Please try again."));
		return;
	}

	resultText.Add("- FeatureID: " + FString::FromInt(featureID));

	for (auto outfield : outfields)
	{
		if (GetObjectIDs(Response->GetContentAsString(), outfield) != "")
		{
			resultText.Add("- " + outfield + ": " + GetObjectIDs(Response->GetContentAsString(), outfield));
		}
	}

	resultText.Add(position);
	resultText.Add("\n");
	CreateProperties();
}

void AArcGISRaycast::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(mousePress, ETriggerEvent::Started, this, &AArcGISRaycast::GetHit);
	}
}
