/* Copyright 2022 Esri
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


#include "FeatureLayer.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISSurfacePlacementMode.h"
#include "Blueprint/UserWidget.h"
#include "FeatureItem.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "Kismet/GameplayStatics.h"
#include "sample_project/InputManager.h"

AFeatureLayer::AFeatureLayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AFeatureLayer::AddAdditionalMaterial(const AFeatureItem* Item, UMaterialInstance* Material)
{
	if (!Material)
	{
		return;
	}

	Item->pin->SetOverlayMaterial(Material);
}

void AFeatureLayer::BeginPlay()
{
	Super::BeginPlay();

	// Create the UI and add it to the viewport
	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (UIWidget)
		{
			createProperties = UIWidget->FindFunction(FName("AddPropertiesToList"));
			clearProperties = UIWidget->FindFunction(FName("Clear Properties"));
			UIWidget->AddToViewport();
		}
	}

	ArcGISPawn = Cast<AArcGISPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	if (!inputManager)
	{
		return;
	}
	
	inputManager->OnInputTrigger.AddDynamic(this, &AFeatureLayer::SelectFeature);
	
	CreateLink();
	ProcessWebRequest();
}

//create the link for the user
void AFeatureLayer::CreateLink()
{
	for (auto header : WebLink.RequestHeaders)
	{
		if (!WebLink.Link.Contains(header))
		{
			WebLink.Link += header;
		}
		else
		{
			continue;
		}
	}

	bButtonActive = WebLink.Link.EndsWith("outfields=*");
}

void AFeatureLayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	inputManager->OnInputTrigger.RemoveDynamic(this, &AFeatureLayer::SelectFeature);
}

//check for errors that could result in a crash or null return
bool AFeatureLayer::HasErrors()
{
	if (FeatureData.IsEmpty())
	{
		bLinkReturnError = true;
		return false;
	}

	for (auto feature : FeatureData)
	{
		for (auto property : feature.FeatureProperties)
		{
			if (property.Len() == 0)
			{
				return true;
			}
		}
	}
	return false;
}

void AFeatureLayer::MoveCamera(AActor* Item)
{
	if (!ArcGISPawn)
	{
		return;
	}

	if (mapComponent == nullptr)
	{
		GetMapComponent();
	}

	if (!mapComponent)
	{
		return;
	}

	const auto featureItem = Cast<AFeatureItem>(Item);

	if (!featureItem)
	{
		return;
	}

	if (const auto locationComponent = Cast<UArcGISLocationComponent>(ArcGISPawn->GetComponentByClass(UArcGISLocationComponent::StaticClass())))
	{
		const auto position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
			featureItem->Longitude, featureItem->Latitude, 250, mapComponent->GetOriginPosition()->GetSpatialReference());
		locationComponent->SetPosition(position);
		locationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(0, 0, 0));
	}

	const auto originPosition = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(featureItem->Longitude, featureItem->Latitude, 0,
	                                                                                   mapComponent->GetOriginPosition()->GetSpatialReference());
	mapComponent->SetOriginPosition(originPosition);
}

void AFeatureLayer::RefreshProperties(AFeatureItem* Item)
{
	Item->Properties.Empty();
	Item->PropertiesNames.Empty();
	resultProperties.Empty();
	const auto properties = Features[Item->Index]->AsObject()->GetObjectField(TEXT("Properties"));

	if (bGetAllOutfields)
	{
		for (auto property : properties->Values)
		{
			auto key = property.Key;
			auto value = property.Value->AsString();
			Item->PropertiesNames.Add(key);
			Item->Properties.Add(value);
			resultProperties.Add(key + ": " + value);
		}

		FString output = "Properties: \n";

		for (auto ResultProperty : resultProperties)
		{
			output += ResultProperty + "\n";
		}
	}
	else
	{
		const auto feature = Features[Item->Index]->AsObject();

		for (auto outfield : OutFieldsToGet)
		{
			auto propertyOutfield = feature->GetObjectField(TEXT("properties"))->GetStringField(outfield);

			if (propertyOutfield.IsEmpty())
			{
				Item->Properties.Add(
					FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
			}
			else
			{
				Item->Properties.Add(feature->GetObjectField(TEXT("properties"))->GetStringField(outfield));
			}

			resultProperties.Add(outfield + ": " + propertyOutfield);
		}

		FString output = "Properties: \n";

		for (auto ResultProperty : resultProperties)
		{
			output += ResultProperty + "\n";
		}
	}
}

void AFeatureLayer::RemoveAdditionalMaterial(const AFeatureItem* Item)
{
	Item->pin->SetOverlayMaterial(nullptr);
}

void AFeatureLayer::SelectFeature()
{
	if (APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		FVector Location, Direction;
		const TArray<AActor*> ActorsToIgnore;
		FHitResult HitResult;
		PlayerController->DeprojectMousePositionToWorld(Location, Direction);

		if (UKismetSystemLibrary::LineTraceSingle(GetWorld(), Location, Location + Direction * 10000000.0, TraceTypeQuery1, false, ActorsToIgnore,
		                                          EDrawDebugTrace::None, HitResult, true))
		{
			if (currentFeature)
			{
				RemoveAdditionalMaterial(currentFeature);
			}

			currentFeature = Cast<AFeatureItem>(HitResult.GetActor());

			if (!currentFeature)
			{
				return;
			}

			RefreshProperties(currentFeature);
			AddAdditionalMaterial(currentFeature, highlightMaterial);

			if (clearProperties)
			{
				AActor* self = this;
				UIWidget->ProcessEvent(clearProperties, &self);
			}

			if (createProperties)
			{
				AActor* self = this;
				UIWidget->ProcessEvent(createProperties, &self);
			}
		}
	}
}
