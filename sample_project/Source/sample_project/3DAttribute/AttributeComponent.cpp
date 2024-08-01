// Copyright 2022 Esri.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
//
#include "AttributeComponent.h"

#include <assert.h>

#include "Materials/Material.h"

#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISAttribute.h"
#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISAttributeProcessor.h"
#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISVisualizationAttribute.h"
#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISVisualizationAttributeDescription.h"
#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISVisualizationAttributeType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISArrayBuilder.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableArray.h"

#include "APIMapCreator.h"

UAttributeComponent::UAttributeComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;
	bAutoActivate = true;
}

void UAttributeComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAttributeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

#if WITH_EDITOR
void UAttributeComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UAttributeComponent, AttributeType))
	{
		if (auto SampleScene = Cast<AAPIMapCreator>(GetOwner()))
		{
			SampleScene->CreateArcGISMap();
		}
	}
}
#endif

// We create our map with this method
void UAttributeComponent::Setup3DAttributes(UArcGIS3DObjectSceneLayer* Layer)
{
	// To switch the Attribute method, open the AttributeComponent attached to the SampleAPIMapCreator actor and switch the attribute type method
	if (AttributeType == VisualizationType::BuildingName)
	{
		Setup3DAttributesOtherType(Layer);
	}
	else if (AttributeType == VisualizationType::ConstructionYear)
	{
		Setup3DAttributesFloatAndIntegerType(Layer);
	}
}

// This function is an example of how to use attributes WITHOUT the attribute processor
void UAttributeComponent::Setup3DAttributesFloatAndIntegerType(UArcGIS3DObjectSceneLayer* Layer)
{
	// We want to set up an array with the attributes we want to forward to the material
	// Because CNSTRCT_YR is an esriFieldTypeInteger type, the values can be passed directly to the material as an integer
	// esriFieldTypeSingle, esriFieldTypeSmallInteger, esriFieldTypeInteger and esriFieldTypeDouble can be passed directly to the material without processing
	// esriFieldTypeDouble and esriFieldTypeInteger are converted to a float, resulting in a lossy conversion
	// See Setup3DAttributesOtherType below for an example of how to pass non-numeric types to the material
	auto layerAttributes = Esri::Unreal::ArcGISImmutableArray<FString>::CreateBuilder();
	layerAttributes.Add("CNSTRCT_YR");

	auto layerAPIObject = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGIS3DObjectSceneLayer>(Layer->APIObject);
	layerAPIObject->SetAttributesToVisualize(layerAttributes.MoveToArray());

	// We want to set the material we will use to visualize this layer
	// In Unreal Engine, open this material in the Material Editor to view the shader graph
	// In general, you can use this function in other scripts to change the material thats used to render the buildings
	Layer->SetMaterialReference(
		LoadObject<UMaterial>(this, TEXT("Material'/Game/SampleViewer/Samples/MaterialByAttribute/Materials/ConstructionYearRenderer.ConstructionYearRenderer'")));
}

// This function is an example of how to use attributes WITH the attribute processor
void UAttributeComponent::Setup3DAttributesOtherType(UArcGIS3DObjectSceneLayer* Layer)
{
	// We want to set up an array with the attributes we want to forward to the material
	// Because NAME is of type esriFieldTypeString/AttributeType.string, we will need to configure the AttributeProcessor to pass meaningful values to the material
	auto layerAttributes = Esri::Unreal::ArcGISImmutableArray<FString>::CreateBuilder();
	layerAttributes.Add("NAME");

	// The attribute description is the buffer that is output to the material
	// Visualize the label "NAME" in the layer attributes as an input to the attribute processor
	// "IsBuildingOfInterest" describes how we choose to convert "NAME" into a usable type in the material
	// We give the material values we can use to render the models in a way we see fit
	// In this case, we are using "IsBuildingOfInterest" to output either a 0 or a 1 depending on if the buildings "NAME" is a name of interest
	auto attributeDescriptions =
		Esri::Unreal::ArcGISImmutableArray<Esri::GameEngine::Attributes::ArcGISVisualizationAttributeDescription>::CreateBuilder();
	attributeDescriptions.Add(Esri::GameEngine::Attributes::ArcGISVisualizationAttributeDescription(
		"IsBuildingOfInterest", Esri::GameEngine::Attributes::ArcGISVisualizationAttributeType::Int32));

	// The attribute processor does the work on the CPU of converting the attribute into a value that can be used with the material
	// Integers and floats can be processed the same way as other types, although it is not normally necessary
	AttributeProcessor = ::MakeShared<Esri::GameEngine::Attributes::ArcGISAttributeProcessor>();

	AttributeProcessor->SetProcessEvent(
		[this](const Esri::Unreal::ArcGISImmutableArray<Esri::GameEngine::Attributes::ArcGISAttribute>& inputAttributes,
			   const Esri::Unreal::ArcGISImmutableArray<Esri::GameEngine::Attributes::ArcGISVisualizationAttribute>& outputVisualizationAttributes) {
			// Buffers will be provided in the same order they appear in the layer metadata.
			// If layerAttributes contained an additional element, it would be at inputAttributes.At(1)
			const auto nameAttribute = inputAttributes.At(0);

			// The outputVisualizationAttributes array expects that its data is indexed the same way as the attributeDescriptions above.
			const auto isBuildingOfInterestAttribute = outputVisualizationAttributes.At(0);

			const auto isBuildingOfInterestBuffer = isBuildingOfInterestAttribute.GetData();
			const auto isBuildingOfInterestData =
				TArrayView<int32>(reinterpret_cast<int32*>(isBuildingOfInterestBuffer.GetData()), isBuildingOfInterestBuffer.Num() / sizeof(int32));

			// Go over each attribute and if its name is one of the four buildings of interest
			// It sets a "isBuildingOfInterest" value to 1, otherwise it is set to 0
			ForEachString(nameAttribute, [this, &isBuildingOfInterestData](const FAnsiStringView& element, int32 index) {
				isBuildingOfInterestData[index] = IsBuildingOfInterest(element);
			});
		});

	// Pass the layer attributes, attribute descriptions and the attribute processor to the layer
	auto layerAPIObject = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGIS3DObjectSceneLayer>(Layer->APIObject);
	layerAPIObject->SetAttributesToVisualize(layerAttributes.MoveToArray(), attributeDescriptions.MoveToArray(), *AttributeProcessor);

	// In Unreal Engine, open this material in the Material Editor to view the shader graph
	// In general, you can use this function in other scripts to change the material thats used to render the buildings
	Layer->SetMaterialReference(
		LoadObject<UMaterial>(this, TEXT("Material'/Game/SampleViewer/Samples/MaterialByAttribute/Materials/BuildingNameRenderer.BuildingNameRenderer'")));
}

// ForEachString takes care of converting the attribute buffer into a readable string value
void UAttributeComponent::ForEachString(const Esri::GameEngine::Attributes::ArcGISAttribute& attribute,
												 TFunction<void(const FAnsiStringView&, int32)> predicate)
{
	const auto buffer = attribute.GetData();
	const auto metadata = reinterpret_cast<const int*>(buffer.GetData());

	const auto count = metadata[0];
	assert(count);

	const char* string = reinterpret_cast<const char*>(buffer.GetData() + (2 + count) * sizeof(int));

	for (int32 i = 0; i < count; ++i)
	{
		auto element = FAnsiStringView();

		if (metadata[2 + i] > 0)
		{
			// If the length of the string element is 0, it means the element is null
			element = FAnsiStringView(string, metadata[2 + i] - 1);
		}

		predicate(element, i);

		string += metadata[2 + i];
	}
}

// This function checks if the building contains a name we are interested in visualizing
int32 UAttributeComponent::IsBuildingOfInterest(const FAnsiStringView& buildingName)
{
	if (buildingName.Equals("Empire State Building") || buildingName.Equals("Chrysler Building") || buildingName.Equals("Tower 1 World Trade Ctr") ||
		buildingName.Equals("One Chase Manhattan Plaza"))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}
