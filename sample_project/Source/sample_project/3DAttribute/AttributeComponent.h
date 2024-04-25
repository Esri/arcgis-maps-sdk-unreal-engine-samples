// Copyright 2022 Esri.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
//
#pragma once

#include <functional>
#include "Components/ActorComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"

#include "AttributeComponent.generated.h"

namespace Esri
{
namespace GameEngine
{
namespace Attributes
{
class ArcGISAttribute;
class ArcGISAttributeProcessor;
} // namespace Attributes
} // namespace GameEngine
} // namespace Esri

UENUM()
enum class VisualizationType : uint8
{
	None = 0,
	ConstructionYear = 1,
	BuildingName = 2
};

UCLASS(ClassGroup = (ArcGISSamples), meta = (BlueprintSpawnableComponent), Category = "ArcGISSamples|AttributeComponent")
class SAMPLE_PROJECT_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAttributeComponent();

	UPROPERTY(EditAnywhere, Category = "ArcGISSamples|SampleAttributeComponent")
	VisualizationType AttributeType;

	// UObject
#if WITH_EDITOR
	void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void Setup3DAttributes(UArcGIS3DObjectSceneLayer* Layer);

protected:
	// UActorComponent
	void BeginPlay() override;
	void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	TSharedPtr<Esri::GameEngine::Attributes::ArcGISAttributeProcessor> AttributeProcessor;
	void Setup3DAttributesFloatAndIntegerType(UArcGIS3DObjectSceneLayer* Layer);
	void Setup3DAttributesOtherType(UArcGIS3DObjectSceneLayer* Layer);
	int32 IsBuildingOfInterest(const FAnsiStringView& buildingName);
	void ForEachString(const Esri::GameEngine::Attributes::ArcGISAttribute& attribute, std::function<void(const FAnsiStringView&, int32)> predicate);
};
