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
#include "EnhancedInputComponent.h"
#include "FeatureItem.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISSurfacePlacementMode.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

AFeatureLayer::AFeatureLayer()
{
	PrimaryActorTick.bCanEverTick = false;
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
					//Add the data recieved into the object and load the object into an array for use later.
					FeatureData.Add(featureLayerProperties);
				}
			}
			else if (bNewLink)
			{
				//Get every feature property and add them to an array
				//These properties show up in the editor and automatically change when the user changes the link, assuming the link is valid
				//The user may click on these properties from the drop down in order to select which ones they would like to get.
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
					//Add the data recieved into the object and load the object into an array for use later.
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

//check for errors that could result in a crash or null return
bool AFeatureLayer::ErrorCheck()
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

	if (WebLink.Link.EndsWith("outfields=*"))
	{
		bButtonActive = true;
	}
	else
	{
		bButtonActive = false;
	}
}

void AFeatureLayer::RefreshProperties(AActor* Feature)
{
	auto FeatureItem = Cast<AFeatureItem>(Feature->StaticClass());

	if(!Feature)
	{
		return;
	}

	FeatureItem->Properties.Empty();
	auto properties = Features[FeatureItem->Index]->AsObject()->GetObjectField(TEXT("Properties"));

	if (bGetAllOutfields)
	{
		for (auto property : properties->Values)
		{
			auto key = property.Key;
			auto value = property.Value->AsString();
			FeatureItem->PropertiesNames.Add(key);
			FeatureItem->Properties.Add(value);
		}
	}
	else
	{
		for (auto OutField : OutFieldsToGet)
		{
			auto propertyOutfield = Features[FeatureItem->Index]->AsObject()->GetObjectField(TEXT("properties"))->GetStringField(OutField);
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
}

void AFeatureLayer::MoveCamera(AActor* Item)
{
	if (!ArcGISPawn)
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
		locationComponent->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
			featureItem->Longitude, featureItem->Latitude, 1000, mapComponent->GetOriginPosition()->GetSpatialReference()));
		locationComponent->SetRotation(UArcGISRotation::CreateArcGISRotation(0, 0, 0));
	}

	if (!mapComponent)
	{
		return;
	}

	mapComponent->SetOriginPosition(
		UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(featureItem->Longitude, featureItem->Latitude, 0,
		                                                       mapComponent->GetOriginPosition()->GetSpatialReference()));
}

void AFeatureLayer::ParseData()
{
	if (FeatureData.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(1, 1, FColor::Red, "Empty");

		return;
	}

	if (bGetAllFeatures)
	{
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

			item->Properties = featureData.FeatureProperties;
			item->Longitude = featureData.GeoProperties[0];
			item->Latitude = featureData.GeoProperties[1];
			featureItem->SetOwner(this);
			item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);
			UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
				item->Longitude, item->Latitude, 0, mapComponent->GetOriginPosition()->GetSpatialReference());
			item->locationComponent->SetPosition(position);
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

			item->Properties = FeatureData[StartValue].FeatureProperties;
			item->Longitude = FeatureData[StartValue].GeoProperties[0];
			item->Latitude = FeatureData[StartValue].GeoProperties[1];
			featureItem->SetOwner(this);
			item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);
			UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
				item->Longitude, item->Latitude, 0, mapComponent->GetOriginPosition()->GetSpatialReference());
			item->locationComponent->SetPosition(position);
			featureItems.Add(featureItem);
			return;
		}

		if (LastValue >= FeatureData.Num())
		{
			for (int index = StartValue; index < FeatureData.Num(); ++index)
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

				item->Properties = FeatureData[index].FeatureProperties;
				item->Longitude = FeatureData[index].GeoProperties[0];
				item->Latitude = FeatureData[index].GeoProperties[1];
				featureItem->SetOwner(this);
				item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);
				UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
					item->Longitude, item->Latitude, 0, mapComponent->GetOriginPosition()->GetSpatialReference());
				item->locationComponent->SetPosition(position);
				featureItems.Add(featureItem);
			}
		}
		else
		{
			for (int index = StartValue; index < LastValue; ++index)
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

				item->Properties = FeatureData[index].FeatureProperties;
				item->Longitude = FeatureData[index].GeoProperties[0];
				item->Latitude = FeatureData[index].GeoProperties[1];
				item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);
				UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
					item->Longitude, item->Latitude, 0, mapComponent->GetOriginPosition()->GetSpatialReference());
				item->locationComponent->SetPosition(position);
				featureItem->SetOwner(this);
				featureItems.Add(featureItem);
			}
		}
	}

	MoveCamera(featureItems[0]);

	if (GetWorldTimerManager().IsTimerActive(startDelayHandle))
	{
		GetWorldTimerManager().ClearTimer(startDelayHandle);
	}
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
			auto featureItem = Cast<AFeatureItem>(HitResult.GetActor());
			
			if (!featureItem)
			{
				return;
			}

			if (bGetAllOutfields)
			{
				for (int index = 0; index < WebLink.OutFields.Num(); index++)
				{
					resultProperties.Add(WebLink.OutFields[index] + ": " + featureItem->Properties[index]);
				}

				FString output = "Properties: \n";
			
				for (auto ResultProperty : resultProperties)
				{
					output += ResultProperty + "\n";
				}
				
				GEngine->AddOnScreenDebugMessage(1, 1, FColor::Red, output);
			}
			else
			{
				for (int index = 0; index < OutFieldsToGet.Num(); index++)
				{
					resultProperties.Add(OutFieldsToGet[index] + ": " + featureItem->Properties[index]);
				}

				FString output = "Properties: \n";
			
				for (auto ResultProperty : resultProperties)
				{
					output += ResultProperty + "\n";
				}
				
				GEngine->AddOnScreenDebugMessage(1, 1, FColor::Red, output);
			}
			
		}
	}
}

//Process the request in order to get the data
void AFeatureLayer::ProcessWebRequest()
{
	FeatureData.Empty();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AFeatureLayer::OnResponseReceived);
	Request->SetURL(WebLink.Link);
	Request->SetVerb("Get");
	Request->ProcessRequest();
}

void AFeatureLayer::BeginPlay()
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
		if (UIWidget)
		{
			UIWidget->AddToViewport();
		}
	}

	mapComponent = UArcGISMapComponent::GetMapComponent(this);
	ArcGISPawn = Cast<AArcGISPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	CreateLink();
	ProcessWebRequest();
}

void AFeatureLayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(mousePress, ETriggerEvent::Started, this, &AFeatureLayer::SelectFeature);
	}
}
