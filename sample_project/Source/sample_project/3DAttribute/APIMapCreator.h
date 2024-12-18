// Copyright 2022 Esri.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
// @@Start(Script)
// @@Start(Header)
#pragma once

#include "CoreMinimal.h"

#include "ArcGISMapsSDK/Actors/ArcGISActor.h"

#include "AttributeComponent.h"
#include "ViewStateLoggingComponent.h"

#include "APIMapCreator.generated.h"
// @@End(Header)

// @@Start(Class)
// The declaration of our Actor class
UCLASS()
class SAMPLE_PROJECT_API AAPIMapCreator : public AArcGISActor
{
	GENERATED_BODY()

	// @@Start(mapcreator)
	public:
	AAPIMapCreator();
	
	UFUNCTION(BlueprintCallable)
	void HideDirections();
	UFUNCTION(BlueprintCallable)
	void SetVisualType(FString type);
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "ArcGISSamples|SampleAPIMapCreator")
	FString APIKey;
	// @@End(mapcreator)

	// UObject
	#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// @@Start(SubListener)
	// IArcGISMapsSDKSubsystemListener
	void OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent) override;
	// @@End(SubListener)

	// @@Start(createmap)
	UFUNCTION(BlueprintCallable)
	void CreateArcGISMap();
	// @@End(createmap)

	private:
	UPROPERTY(Category = "ArcGISSamples|SampleAPIMapCreator", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UAttributeComponent> AttributeComponent;
	UPROPERTY(Category = "ArcGISSamples|SampleAPIMapCreator", VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UViewStateLoggingComponent> ViewStateLogging;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(AllowPrivateAccess))
	UUserWidget* UIWidget;
	UFunction* HideInstructions;
	
protected:
	virtual void BeginPlay() override;
};
// @@End(Class)
// @@End(Script)