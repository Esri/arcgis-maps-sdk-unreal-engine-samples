// /* Copyright 2025 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FeatureItemBase.generated.h"

class UArcGISLocationComponent;

UCLASS()
class SAMPLE_PROJECT_API AFeatureItemBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFeatureItemBase();

	UPROPERTY(VisibleAnywhere)
	UArcGISLocationComponent* locationComponent;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FString> Properties;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<FString> PropertiesNames;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double Latitude;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	double Longitude;
	UPROPERTY(VisibleAnywhere)
	int Index;

	USceneComponent* Root;
};
