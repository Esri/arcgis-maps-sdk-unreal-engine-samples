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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Json.h"
#include "Http.h"
#include "QueryLocation.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "Geocoder.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AGeocoder : public AActor
{
	GENERATED_BODY()

public:
	AGeocoder();
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable)
	void SendAddressQuery(FString Address);

	UFUNCTION(BlueprintCallable)
	void SelectLocation();

protected:
	virtual void BeginPlay() override;

private:
	void SetupInput();
	void ProcessAddressQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	void ProcessLocationQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	void SendLocationQuery(UArcGISPoint* InPoint);

	AQueryLocation* QueryLocation;
	bool bWaitingForResponse = false;
	bool bShouldSendLocationQuery = false;
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UUserWidget* UIWidget;
	UFunction* WidgetSetInfoFunction;
};
