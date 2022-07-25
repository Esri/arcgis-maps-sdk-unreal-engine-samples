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
	TSharedPtr<FJsonObject> ResponseObj;
	const FString ResponseBody = Response->GetContentAsString();
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
	if (FJsonSerializer::Deserialize(Reader, ResponseObj))
	{
		TArray<TSharedPtr<FJsonValue>> Features = ResponseObj->GetArrayField("features");
		for (int i = 0; i != Features.Num(); i++)
		{
			TArray<double> geoCoordinates = {};
			TSharedPtr<FJsonObject> feature = Features[i]->AsObject();
			TSharedPtr<FJsonObject> properties = feature->GetObjectField("properties");
			TSharedPtr<FJsonObject> Geometry = feature->GetObjectField("geometry");
			TArray<TSharedPtr<FJsonValue>> coordinates = Geometry->GetArrayField("coordinates");
			data->NAME.Add(properties->GetStringField("NAME"));
			data->LEAGUE.Add(properties->GetStringField("LEAGUE"));
			data->TEAM.Add(properties->GetStringField("TEAM"));
			data->longitude.Add(coordinates[0]->AsNumber());
			data->latitude.Add(coordinates[1]->AsNumber());
		}
	}
}

// Called when the game starts or when spawned
void AFeatureLayer::BeginPlay()
{
	Super::BeginPlay();
	UWebLink* url = NewObject<UWebLink>();
	url->link = "https://services.arcgis.com/P3ePLMYs2RVChkJx/ArcGIS/rest/services/Major_League_Baseball_Stadiums/FeatureServer/0";
	url->requestHeaders = "f=geojson&where=1=1";
	url->outFieldHeader = "outfields=TEAM,NAME,LEAGUE";

	url->requestHeaders += "&" + url->outFieldHeader;
	url->link += "/Query?" + url->requestHeaders;
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AFeatureLayer::OnResponseRecieved);
	Request->SetURL(url->link);
	Request->SetVerb("Get");
	Request->ProcessRequest();
}
