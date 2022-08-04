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
#include "Http.h"
#include "FeatureLayer.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AFeatureLayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFeatureLayer();
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		UFeature* data = NewObject<UFeature>();
private:
	void OnResponseRecieved(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame

};
UCLASS(Blueprintable)
class SAMPLE_PROJECT_API UWebLink : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		FString link;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		FString requestHeaders;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		FString outFieldHeader;
};

UCLASS(Blueprintable)
class SAMPLE_PROJECT_API UFeature : public UObject
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<FString> LEAGUE;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<FString>  TEAM;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<FString>  NAME;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<double>  longitude;
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
		TArray<double>  latitude;
};
