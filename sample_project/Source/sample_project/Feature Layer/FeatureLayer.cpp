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
#include "Json.h"

// Sets default values
AFeatureLayer::AFeatureLayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

}

void AFeatureLayer::OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	if(bConnectedSucessfully)
	{
		bLinkReturnError = false;
		TSharedPtr<FJsonObject> ResponseObj;
		const auto ResponseBody = Response->GetContentAsString();
		auto Reader = TJsonReaderFactory<>::Create(ResponseBody);
	
		if (FJsonSerializer::Deserialize(Reader, ResponseObj))
		{
			TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");

			for (int i = 0; i != Features.Num(); i++)
			{
				FFeatureLayerProperties testProperties;
				auto feature = Features[i]->AsObject();
				auto properties = feature->GetObjectField("properties");
			
				for (auto outfield : WebLink.OutFields)
				{
					testProperties.FeatureProperties.Add(feature->GetObjectField("properties")->GetStringField(outfield));
				}
				TArray<TSharedPtr<FJsonValue>> coordinates = feature->GetObjectField("geometry")->GetArrayField("coordinates");
				if(!coordinates.IsEmpty())
				{
					for (const auto Coordinate : coordinates)
					{
						testProperties.GeoProperties.Add(Coordinate->AsNumber());
					}
				}
				else
				{
					bCoordinatesErrorReturn = true;
				}
				FeatureData.Add(testProperties);
			}
		}	
	}
	else
	{
		bLinkReturnError = true;
	}
}

bool AFeatureLayer::ErrorCheck()
{
	if(FeatureData.IsEmpty())
	{
		bLinkReturnError = true;
		return false;
	}
	for (const auto Data : FeatureData)
	{
		FFeatureLayerProperties feature = Data;
		
		for (const auto property : feature.FeatureProperties)
		{
			if(property.Len() == 0)
			{
				return true;
			}
		}
	}
	return false;
}

void AFeatureLayer::CreateLink()
{
	for (auto header : WebLink.RequestHeaders)
	{
		if(!WebLink.Link.Contains(header))
		{
			WebLink.Link += header;
		}
		else
		{
			continue;
		}
	}

	if(WebLink.Link.EndsWith("outfields=*"))
	{
		bButtonActive = true;	
	}
	else
	{
		bButtonActive = false;
	}
}


void AFeatureLayer::ProcessWebRequest()
{
	FeatureData.Empty();
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AFeatureLayer::OnResponseRecieved);
	Request->SetURL(WebLink.Link);
	Request->SetVerb("Get");
	Request->ProcessRequest();
}

void AFeatureLayer::BeginPlay()
{
	Super::BeginPlay();

	CreateLink();
}
