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

void AFeatureLayer::GetMapComponent()
{
	const auto mapComponentActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass());
	mapComponent = Cast<AArcGISMapActor>(mapComponentActor)->GetMapComponent();
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

void AFeatureLayer::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (bConnectedSuccessfully)
	{
		bLinkReturnError = false;
		TSharedPtr<FJsonObject> ResponseObj;
		const auto ResponseBody = Response->GetContentAsString();
		auto Reader = TJsonReaderFactory<>::Create(ResponseBody);

		//deserialize the json data received in the http request
		if (FJsonSerializer::Deserialize(Reader, ResponseObj))
		{
			//get the array field features
			Features = ResponseObj->GetArrayField(TEXT("features"));

			if (!bNewLink && !bGetAll)
			{
				//parse through the features in order to get individual properties associated with the features
				for (int i = 0; i != Features.Num(); i++)
				{
					//create a new feature object to store the data received associated with this feature iteration
					FFeatureLayerProperties featureLayerProperties;
					auto feature = Features[i]->AsObject();
					//get the properties field associated with the individual feature

					//outfield can be set in the scene on bp_feature
					//this loop will take each outfield set in the scene and check to see if the outfield exists
					//if it does exist, it will return the result of the outfield associated with this feature
					//if it does not exist, it will return and error message in the scene
					if (bGetAllOutfields)
					{
						for (auto outfield : WebLink.OutFields)
						{
							auto propertyOutfield = feature->GetObjectField(TEXT("properties"))->GetStringField(outfield);
							if (propertyOutfield.IsEmpty())
							{
								featureLayerProperties.FeatureProperties.Add(
									FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
							}
							else
							{
								featureLayerProperties.FeatureProperties.Add(feature->GetObjectField(TEXT("properties"))->GetStringField(outfield));
							}
						}
					}
					else
					{
						for (auto outfield : OutFieldsToGet)
						{
							auto propertyOutfield = feature->GetObjectField(TEXT("properties"))->GetStringField(outfield);
							if (propertyOutfield.IsEmpty())
							{
								featureLayerProperties.FeatureProperties.Add(
									FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
							}
							else
							{
								featureLayerProperties.FeatureProperties.Add(feature->GetObjectField(TEXT("properties"))->GetStringField(outfield));
							}
						}
					}
					//this will get the type of feature
					auto type = feature->GetObjectField(TEXT("geometry"))->GetStringField(TEXT("type"));
					//this will get the geometry or coordinates of the feature
					auto coordinates = feature->GetObjectField(TEXT("geometry"))->GetArrayField(TEXT("coordinates"));

					//To avoid crashes, this checks to see if the type of feature is Point, if so it will get the geometry
					//if not, it will return an error
					//current the only type of data supported by this sample is Point Layers, but more will be added in the future.
					if (type.ToLower() == "point")
					{
						for (auto Coordinate : coordinates)
						{
							featureLayerProperties.GeoProperties.Add(Coordinate->AsNumber());
						}
						bCoordinatesErrorReturn = false;
					}
					else
					{
						bCoordinatesErrorReturn = true;
					}
					//Add the data received into the object and load the object into an array for use later.
					FeatureData.Add(featureLayerProperties);
				}
			}
			else if (bNewLink)
			{
				//Get every feature property and add them to an array
				//These properties show up in the editor and automatically change when the user changes the link, assuming the link is valid
				//The user may click on these properties from the drop-down in order to select which ones they would like to get.
				auto properties = Features[0]->AsObject()->GetObjectField(TEXT("properties"));
				auto propertyFields = properties->Values;

				for (auto key : propertyFields)
				{
					WebLink.OutFields.Add(key.Key);
				}
			}
			else if (bGetAll)
			{
				auto properties = Features[0]->AsObject()->GetObjectField(TEXT("properties"));
				auto propertyFields = properties->Values;

				for (auto key : propertyFields)
				{
					WebLink.OutFields.Add(key.Key);
				}
				//parse through the features in order to get individual properties associated with the features
				for (int i = 0; i != Features.Num(); i++)
				{
					//create a new feature object to store the data received associated with this feature iteration
					FFeatureLayerProperties featureLayerProperties;
					auto feature = Features[i]->AsObject();
					//get the properties field associated with the individual feature

					//outfield can be set in the scene on bp_feature
					//this loop will take each outfield set in the scene and check to see if the outfield exists
					//if it does exist, it will return the result of the outfield associated with this feature
					//if it does not exist, it will return and error message in the scene
					if (bGetAllOutfields)
					{
						for (auto outfield : WebLink.OutFields)
						{
							auto propertyOutfield = feature->GetObjectField(TEXT("properties"))->GetStringField(outfield);
							if (propertyOutfield.IsEmpty())
							{
								featureLayerProperties.FeatureProperties.Add(
									FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
							}
							else
							{
								featureLayerProperties.FeatureProperties.Add(feature->GetObjectField(TEXT("properties"))->GetStringField(outfield));
							}
						}
					}
					else
					{
						for (auto outfield : OutFieldsToGet)
						{
							auto propertyOutfield = feature->GetObjectField(TEXT("properties"))->GetStringField(outfield);

							if (propertyOutfield.IsEmpty())
							{
								featureLayerProperties.FeatureProperties.Add(
									FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
							}
							else
							{
								featureLayerProperties.FeatureProperties.Add(feature->GetObjectField(TEXT("properties"))->GetStringField(outfield));
							}
						}
					}
					//this will get the type of feature
					auto type = feature->GetObjectField(TEXT("geometry"))->GetStringField(TEXT("type"));
					//this will get the geometry or coordinates of the feature
					auto coordinates = feature->GetObjectField(TEXT("geometry"))->GetArrayField(TEXT("coordinates"));

					//To avoid crashes, this checks to see if the type of feature is Point, if so it will get the geometry
					//if not, it will return an error
					//current the only type of data supported by this sample is Point Layers, but more will be added in the future.
					if (type.ToLower() == "point")
					{
						for (auto Coordinate : coordinates)
						{
							featureLayerProperties.GeoProperties.Add(Coordinate->AsNumber());
						}
						bCoordinatesErrorReturn = false;
					}
					else
					{
						bCoordinatesErrorReturn = true;
					}
					//Add the data received into the object and load the object into an array for use later.
					FeatureData.Add(featureLayerProperties);
				}
			}
		}
		else
		{
			bLinkReturnError = true;
		}
	}
}

void AFeatureLayer::ParseData()
{
	if (FeatureData.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(1, 1, FColor::Red, "Empty");
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

	if (bGetAllFeatures)
	{
		auto index = 0;
		for (auto featureData : FeatureData)
		{
			auto featureItem = GetWorld()->SpawnActor(AFeatureItem::StaticClass());

			if (!featureItem)
			{
				return;
			}

			const auto item = Cast<AFeatureItem>(featureItem);

			if (!item)
			{
				return;
			}

			item->Index = index;
			item->Properties = featureData.FeatureProperties;
			item->Longitude = featureData.GeoProperties[0];
			item->Latitude = featureData.GeoProperties[1];
			featureItem->SetOwner(this);
			item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);
			UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
				item->Longitude, item->Latitude, 0, mapComponent->GetOriginPosition()->GetSpatialReference());
			item->locationComponent->SetPosition(position);
			index++;
			featureItems.Add(featureItem);
		}
	}
	else
	{
		if (StartValue == LastValue)
		{
			auto featureItem = GetWorld()->SpawnActor(AFeatureItem::StaticClass());

			if (!featureItem)
			{
				return;
			}

			const auto item = Cast<AFeatureItem>(featureItem);

			if (!item)
			{
				return;
			}

			item->Index = StartValue;
			item->Properties = FeatureData[StartValue].FeatureProperties;
			item->Longitude = FeatureData[StartValue].GeoProperties[0];
			item->Latitude = FeatureData[StartValue].GeoProperties[1];
			item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);

			if (mapComponent)
			{
				UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
					item->Longitude, item->Latitude, 0, mapComponent->GetOriginPosition()->GetSpatialReference());
				item->locationComponent->SetPosition(position);
			}

			featureItem->SetOwner(this);
			featureItems.Add(featureItem);
		}

		if (LastValue >= FeatureData.Num())
		{
			SpawnFeatures(StartValue, FeatureData.Num());
		}
		else
		{
			SpawnFeatures(StartValue, LastValue);
		}
	}

	MoveCamera(featureItems[0]);

	if (GetWorldTimerManager().IsTimerActive(startDelayHandle))
	{
		GetWorldTimerManager().ClearTimer(startDelayHandle);
	}
}

void AFeatureLayer::ProcessWebRequest()
{
	FeatureData.Empty();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AFeatureLayer::OnResponseReceived);
	Request->SetURL(WebLink.Link);
	Request->SetVerb("Get");
	Request->ProcessRequest();
	currentFeature = nullptr;
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

void AFeatureLayer::SpawnFeatures(int Start, int Last)
{
	for (int index = Start; index <= Last; ++index)
	{
		auto featureItem = GetWorld()->SpawnActor(AFeatureItem::StaticClass());

		if (!featureItem)
		{
			return;
		}

		const auto item = Cast<AFeatureItem>(featureItem);

		if (!item)
		{
			return;
		}

		item->Index = index;
		item->Properties = FeatureData[index].FeatureProperties;
		item->Longitude = FeatureData[index].GeoProperties[0];
		item->Latitude = FeatureData[index].GeoProperties[1];
		item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);

		if (mapComponent)
		{
			UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
				item->Longitude, item->Latitude, 0, mapComponent->GetOriginPosition()->GetSpatialReference());
			item->locationComponent->SetPosition(position);
		}

		featureItem->SetOwner(this);
		featureItems.Add(featureItem);
	}
}
