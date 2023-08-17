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
		const FString ResponseBody = Response->GetContentAsString();
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	
		if (FJsonSerializer::Deserialize(Reader, ResponseObj))
		{
			TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");

			for (int i = 0; i != Features.Num(); i++)
			{
				FProperties testProperties;
				TSharedPtr<FJsonObject> feature = Features[i]->AsObject();
				TSharedPtr<FJsonObject> properties = feature->GetObjectField("properties");
			
				for(int j = 0; j != WebLink.outFields.Max(); j++)
				{
					testProperties.featureProperties.Add(feature->GetObjectField("properties")->GetStringField(WebLink.outFields[j]));
				}
				TArray<TSharedPtr<FJsonValue>> coordinates = feature->GetObjectField("geometry")->GetArrayField("coordinates");
				testProperties.geoProperties.Add(coordinates[0]->AsNumber());
				testProperties.geoProperties.Add(coordinates[1]->AsNumber());	
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
	for (auto Element : FeatureData)
	{
		FProperties feature = Element;
		for (auto property : feature.featureProperties)
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
	if(!WebLink.link.Contains("f=geojson&where=1=1"))
	{
		WebLink.requestHeaders += "&outfields=*";
		WebLink.link += WebLink.requestHeaders;
		bButtonActive = true;
	}
	else if(WebLink.link.EndsWith("where=1=1"))
	{
		WebLink.link += "&outfields=*";
		bButtonActive = true;
	}
	else if(WebLink.link.EndsWith("outfields=*"))
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
	Request->SetURL(WebLink.link);
	Request->SetVerb("Get");
	Request->ProcessRequest();
}

// Called when the game starts or when spawned
void AFeatureLayer::BeginPlay()
{
	Super::BeginPlay();

	CreateLink();
	/*
	WebLink.link = "https://services.arcgis.com/P3ePLMYs2RVChkJx/ArcGIS/rest/services/Major_League_Baseball_Stadiums/FeatureServer/0/Query?";
	WebLink.requestHeaders = "f=geojson&where=1=1";*/
}
