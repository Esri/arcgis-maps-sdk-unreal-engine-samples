// /* Copyright 2025 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "Blueprint/UserWidget.h"
#include "Components/ListView.h"
#include "Identify.generated.h"
class UListView; 
class UWidget;
class UTextBlock; 

USTRUCT(BlueprintType)
struct FAttributeRow
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "Identify")
	FString Key;

	UPROPERTY(BlueprintReadWrite, Category = "Identify")
	FString Value;
};

USTRUCT(BlueprintType)
struct FFeatureAttributeSet
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TArray<FAttributeRow> Attributes;
};


UCLASS()
class AIdentify : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AIdentify();
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess))
	TSubclassOf<UUserWidget> UIWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UUserWidget> UIWidget;
	UPROPERTY(BlueprintReadOnly, Category = "Identify")
	TArray<FAttributeRow> LastAttributes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identify|UI")
	TSubclassOf<UObject> PropertyRowClass; 
	UPROPERTY(BlueprintReadOnly)
	TArray<FFeatureAttributeSet> AllFeaturesAttributes;
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentFeatureIndex = 0;
	virtual void Tick(float DeltaTime) override;
	FString IdentifyAtMouseClick();
	UFUNCTION()
	void OnInputTriggered();
	void RefreshListViewFromAttributes();
	void UpdatePageTexts();

	UFUNCTION(BlueprintCallable)
	void ShowNextFeature();

	UFUNCTION(BlueprintCallable)
	void ShowPreviousFeature();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	class AInputManager* InputManager;
	UPROPERTY(meta = (AllowPrivateAccess))
	AArcGISMapActor* MapActor;
	UPROPERTY(meta = (AllowPrivateAccess))
	TObjectPtr<UArcGISMapComponent> MapComponent;
	UWidget* BuildingInfoPanel;

	FString LastIdentifyOutput;
	UPROPERTY()
	UListView* PropertyListView = nullptr;
	UPROPERTY()
	class UTextBlock* CurrentPageText = nullptr;
	UPROPERTY()
	class UTextBlock* TotalPageText = nullptr;
};
