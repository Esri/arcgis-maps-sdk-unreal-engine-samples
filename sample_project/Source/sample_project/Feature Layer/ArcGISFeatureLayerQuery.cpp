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

#include "ArcGISFeatureLayerQuery.h"

#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISPoint.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISSurfacePlacementMode.h"
#include "Engine/Engine.h"
#include "EngineGlobals.h"
#include "FeatureItemBase.h"
#include "Kismet/GameplayStatics.h"

AArcGISFeatureLayerQuery::AArcGISFeatureLayerQuery()
{
	PrimaryActorTick.bCanEverTick = true;
}

//create the link for the user
void AArcGISFeatureLayerQuery::CreateLink()
{
	for (auto header : WebLink.RequestHeaders)
	{
		if (!WebLink.Link.Contains(header))
		{
			WebLink.Link += header;
		}
	}
}

void AArcGISFeatureLayerQuery::GetMapComponent()
{
	const auto mapComponentActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass());
	MapComponent = Cast<AArcGISMapActor>(mapComponentActor)->GetMapComponent();
}

void AArcGISFeatureLayerQuery::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfully)
{
	if (bConnectedSuccessfully)
	{
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
				}

				//Add the data received into the object and load the object into an array for use later.
				FeatureData.Add(featureLayerProperties);
			}
		}
	}
}

void AArcGISFeatureLayerQuery::ParseData(bool GetAllFeatures, int StartValue, int LastValue)
{
	if (FeatureData.IsEmpty())
	{
		GEngine->AddOnScreenDebugMessage(1, 1, FColor::Red, "Empty");
		return;
	}

	if (MapComponent == nullptr)
	{
		GetMapComponent();
	}

	if (!MapComponent)
	{
		return;
	}

	if (GetAllFeatures)
	{
		auto index = 0;
		for (auto featureData : FeatureData)
		{
			auto featureItem = GetWorld()->SpawnActor(FeatureItem);

			if (!featureItem)
			{
				return;
			}

			const auto item = Cast<AFeatureItemBase>(featureItem);

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
			UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(item->Longitude, item->Latitude, 0,
																							MapComponent->GetOriginPosition()->GetSpatialReference());
			item->locationComponent->SetPosition(position);
			UArcGISRotation* rotation = UArcGISRotation::CreateArcGISRotation(90, 0, 90);
			item->locationComponent->SetRotation(rotation);
			index++;
			FeatureItems.Add(featureItem);
		}
	}
	else
	{
		if (StartValue == LastValue)
		{
			auto featureItem = GetWorld()->SpawnActor(FeatureItem);

			if (!featureItem)
			{
				return;
			}

			const auto item = Cast<AFeatureItemBase>(featureItem);

			if (!item)
			{
				return;
			}

			item->Index = StartValue;
			item->Properties = FeatureData[StartValue].FeatureProperties;
			item->Longitude = FeatureData[StartValue].GeoProperties[0];
			item->Latitude = FeatureData[StartValue].GeoProperties[1];
			item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);

			if (MapComponent)
			{
				UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
					item->Longitude, item->Latitude, 0, MapComponent->GetOriginPosition()->GetSpatialReference());
				item->locationComponent->SetPosition(position);
				UArcGISRotation* rotation = UArcGISRotation::CreateArcGISRotation(90, 0, 90);
				item->locationComponent->SetRotation(rotation);
			}

			featureItem->SetOwner(this);
			FeatureItems.Add(featureItem);
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
}

void AArcGISFeatureLayerQuery::ProcessWebRequest()
{
	FeatureData.Empty();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AArcGISFeatureLayerQuery::OnResponseReceived);
	Request->SetURL(WebLink.Link);
	Request->SetVerb("Get");
	Request->ProcessRequest();
}

void AArcGISFeatureLayerQuery::SpawnFeatures(int Start, int Last)
{
	for (int index = Start; index <= Last; ++index)
	{
		auto featureItem = GetWorld()->SpawnActor(FeatureItem);

		if (!featureItem)
		{
			return;
		}

		const auto item = Cast<AFeatureItemBase>(featureItem);

		if (!item)
		{
			return;
		}

		item->Index = index;
		item->Properties = FeatureData[index].FeatureProperties;
		item->Longitude = FeatureData[index].GeoProperties[0];
		item->Latitude = FeatureData[index].GeoProperties[1];
		item->locationComponent->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);

		if (MapComponent)
		{
			UArcGISPoint* position = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(item->Longitude, item->Latitude, 0,
																							MapComponent->GetOriginPosition()->GetSpatialReference());
			item->locationComponent->SetPosition(position);
			UArcGISRotation* rotation = UArcGISRotation::CreateArcGISRotation(90, 0, 90);
			item->locationComponent->SetRotation(rotation);
		}

		featureItem->SetOwner(this);
		FeatureItems.Add(featureItem);
	}
}
