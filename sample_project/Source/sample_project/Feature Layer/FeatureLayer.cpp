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
			TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField(TEXT("features"));


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
								featureLayerProperties.FeatureProperties.Add(FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
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
								featureLayerProperties.FeatureProperties.Add(FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
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
								featureLayerProperties.FeatureProperties.Add(FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
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
								featureLayerProperties.FeatureProperties.Add(FString::FromInt(feature->GetObjectField(TEXT("properties"))->GetIntegerField(outfield)));
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
			if(property.Len() == 0)
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

	CreateLink();
}
