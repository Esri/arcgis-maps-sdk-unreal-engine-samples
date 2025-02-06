/* Copyright 2024 Esri
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
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "ArcGISRaycast.generated.h"

class UArcGISMapComponent;

UCLASS()
class SAMPLE_PROJECT_API AArcGISRaycast : public AActor
{
	GENERATED_BODY()

public:
	AArcGISRaycast();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
private:
	void CreateLink(FString objectID);
	void CreateProperties();
	UFUNCTION()
	void GetHit();
	FString GetObjectIDs(FString response, FString outfield);
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);

	UFunction* createProperties;
	int featureID;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	AStaticMeshActor* HitLocation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	class AInputManager* inputManager;
	TArray<FString> outfields = {"AREA_SQ_FT", "DISTRICT", "Height", "SUBDISTRIC", "ZONE_"};
	FString position;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta = (AllowPrivateAccess))
	TArray<FString> resultText;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	UUserWidget* UIWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> UIWidgetClass;
	FString webLink;
};
