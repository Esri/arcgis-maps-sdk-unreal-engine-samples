/* Copyright 2025 Esri
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


#include "LampPostQueryLayer.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISSurfacePlacementMode.h"
#include "ArcGISPawn.h"
#include "Blueprint/UserWidget.h"
#include "LampPostItem.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "Kismet/GameplayStatics.h"

ALampPostQueryLayer::ALampPostQueryLayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALampPostQueryLayer::BeginPlay()
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

	CreateLink();
	ProcessWebRequest();
	ArcGISPawn = Cast<AArcGISPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

//create the link for the user
void ALampPostQueryLayer::CreateLink()
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

void ALampPostQueryLayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

//check for errors that could result in a crash or null return
bool ALampPostQueryLayer::HasErrors()
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

void ALampPostQueryLayer::GetMapComponent()
{
	const auto mapComponentActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass());
	mapComponent = Cast<AArcGISMapActor>(mapComponentActor)->GetMapComponent();
}

void ALampPostQueryLayer::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
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
				FProperties featureLayerProperties;
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

void ALampPostQueryLayer::ParseData()
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

	auto index = 0;
	
	for (auto featureData : FeatureData)
	{
		auto featureItem = GetWorld()->SpawnActor(ALampPostItem::StaticClass());

		if (!featureItem)
		{
			return;
		}

		const auto item = Cast<ALampPostItem>(featureItem);

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
			item->Longitude, item->Latitude, 0, UArcGISSpatialReference::WGS84());
		item->locationComponent->SetPosition(position);
		UArcGISRotation* rotation = UArcGISRotation::CreateArcGISRotation(90, 0, 0);
		item->locationComponent->SetRotation(rotation);
		index++;
		featureItems.Add(featureItem);
	}
}

void ALampPostQueryLayer::ProcessWebRequest()
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &ALampPostQueryLayer::OnResponseReceived);
	Request->SetURL(WebLink.Link);
	Request->SetVerb("Get");
	Request->ProcessRequest();
	currentFeature = nullptr;
}
