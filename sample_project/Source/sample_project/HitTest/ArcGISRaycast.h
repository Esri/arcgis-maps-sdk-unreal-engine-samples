// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Http.h"
#include "GameFramework/Actor.h"
#include "ArcGISRaycast.generated.h"

class UArcGISMapComponent;

UCLASS()
class SAMPLE_PROJECT_API AArcGISRaycast : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArcGISRaycast();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

private:
	void CreateLink(FString objectID);
	void CreateProperties();
	void GetHit();
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	FString GetObjectIDs(FString response, FString outfield);

	int featureID;
	TArray<FString> outfields = {"AREA_SQ_FT", "DISTRICT", "Height", "SUBDISTRIC", "ZONE_" };
	FString position;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	TArray<FString> resultText;
	UFunction* createProperties;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	AStaticMeshActor* HitLocation;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	UInputMappingContext* MappingContext;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	UInputAction* mousePress;
	UArcGISMapComponent* mapComponent;
	FString webLink;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	UUserWidget* UIWidget;
};
