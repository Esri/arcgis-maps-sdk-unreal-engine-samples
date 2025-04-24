// COPYRIGHT 1995-2025 ESRI
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
#pragma once

#include "CoreMinimal.h"

#include "ArcGISMapsSDK/Actors/ArcGISActor.h"

#include "sample_project/3DAttribute/ViewStateLoggingComponent.h"

class UMaterialInterface;

#include "ViewshedMapCreator.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AViewshedMapCreator : public AArcGISActor
{
	GENERATED_BODY()

public:
	AViewshedMapCreator();

	UFUNCTION(BlueprintCallable)
	void HideDirections();
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ArcGISSamples|ViewshedMapCreator")
	FString APIKey;

	#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent) override;

	UFUNCTION(BlueprintCallable)
	void CreateArcGISMap();

private:
	UPROPERTY(Category = "ArcGISSamples|ViewshedMapCreator", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UViewStateLoggingComponent> ViewStateLogging;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	TSubclassOf<class UUserWidget> UIWidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UUserWidget* UIWidget;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UMaterialInterface> ViewshedMaterial;

	UFunction* HideInstructions;

protected:
	virtual void BeginPlay() override;
};
