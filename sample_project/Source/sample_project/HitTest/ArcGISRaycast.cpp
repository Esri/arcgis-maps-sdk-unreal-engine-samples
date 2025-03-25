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
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "sample_project/InputManager.h"

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

	inputManager->OnInputTrigger.AddDynamic(this, &AArcGISRaycast::GetHit);

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


void AArcGISRaycast::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	inputManager->OnInputTrigger.RemoveDynamic(this, &AArcGISRaycast::GetHit);
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
			const auto mapComponentActor = UGameplayStatics::GetActorOfClass(GetWorld(), UArcGISMapComponent::StaticClass());
			const auto mapComponent = Cast<UArcGISMapComponent>(mapComponentActor);

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
				auto geoPosition = mapComponent->TransformEnginePositionToPoint(HitResult.ImpactPoint);
				
				if (geoPosition)
				{
					position = "- Lat: " + FString::SanitizeFloat(geoPosition->GetY()) + ", Long: " + FString::SanitizeFloat(geoPosition->GetX());
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
