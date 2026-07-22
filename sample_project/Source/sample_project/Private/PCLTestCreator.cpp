// COPYRIGHT 1995-2026 ESRI
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: https://www.apache.org/licenses/LICENSE-2.0
#include "PCLTestCreator.h"

#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudBitfieldFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudClassBreaksRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorClassBreak.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorModulation.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorStop.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorUniqueValue.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFilterInternal.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFilterType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFixedSizeAlgorithm.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRGBRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRendererInternal.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRendererType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudSizeAlgorithmInternal.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudSizeAlgorithmType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudSplatAlgorithm.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudStretchRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudUniqueValueRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilterMode.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/Symbology/ArcGISSymbolSizeUnits.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Elevation/ArcGISImageElevationSource.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISBasemap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapElevation.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapType.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapsSDKProjectSettings.h"


namespace
{
using namespace Esri::GameEngine::Layers;
using namespace Esri::GameEngine::Layers::PointCloud;
using namespace Esri::GameEngine::Map::Symbology;
using namespace Esri::Unreal;

ArcGISPointCloudRendererType GetRendererType(EPointCloudTestRenderer Renderer)
{
	switch (Renderer)
	{
		case EPointCloudTestRenderer::ClassBreaks:
			return ArcGISPointCloudRendererType::PointCloudClassBreaksRenderer;
		case EPointCloudTestRenderer::Stretch:
			return ArcGISPointCloudRendererType::PointCloudStretchRenderer;
		case EPointCloudTestRenderer::UniqueValue:
			return ArcGISPointCloudRendererType::PointCloudUniqueValueRenderer;
		case EPointCloudTestRenderer::RGB:
		default:
			return ArcGISPointCloudRendererType::PointCloudRGBRenderer;
	}
}

TArray<ArcGISPointCloudFilterType> GetFilterTypes(EPointCloudTestFilter Filter)
{
	TArray<ArcGISPointCloudFilterType> FilterTypes;

	switch (Filter)
	{
		case EPointCloudTestFilter::Bitfield:
			FilterTypes.Add(ArcGISPointCloudFilterType::PointCloudBitfieldFilter);
			break;
		case EPointCloudTestFilter::Return:
			FilterTypes.Add(ArcGISPointCloudFilterType::PointCloudReturnFilter);
			break;
		case EPointCloudTestFilter::Value:
			FilterTypes.Add(ArcGISPointCloudFilterType::PointCloudValueFilter);
			break;
		case EPointCloudTestFilter::ValueReturn:
			FilterTypes.Add(ArcGISPointCloudFilterType::PointCloudValueFilter);
			FilterTypes.Add(ArcGISPointCloudFilterType::PointCloudReturnFilter);
			break;
		default:
			break;
	}

	return FilterTypes;
}

ArcGISPointCloudSizeAlgorithmType GetSizeAlgorithmType(bool bUseFixedSizeDIPs)
{
	return bUseFixedSizeDIPs ? ArcGISPointCloudSizeAlgorithmType::PointCloudFixedSizeAlgorithm :
							   ArcGISPointCloudSizeAlgorithmType::PointCloudSplatAlgorithm;
}

bool HasRendererType(ArcGISPointCloudRenderer& Renderer, EPointCloudTestRenderer DesiredRenderer)
{
	return Renderer && ArcGISPointCloudRendererInternal::GetObjectType(&Renderer) == GetRendererType(DesiredRenderer);
}

bool HasSizeAlgorithmType(ArcGISPointCloudRenderer& Renderer, bool bUseFixedSizeDIPs)
{
	auto SizeAlgorithm = Renderer.GetSizeAlgorithm();

	return SizeAlgorithm && ArcGISPointCloudSizeAlgorithmInternal::GetObjectType(&SizeAlgorithm) == GetSizeAlgorithmType(bUseFixedSizeDIPs);
}

bool HasFilterType(const ArcGISCollection<ArcGISPointCloudFilter>& Filters, EPointCloudTestFilter DesiredFilter)
{
	if (!Filters)
	{
		return DesiredFilter == EPointCloudTestFilter::None;
	}

	if (DesiredFilter == EPointCloudTestFilter::None)
	{
		return Filters.GetSize() == 0;
	}

	auto FilterTypes = GetFilterTypes(DesiredFilter);

	if (FilterTypes.Num() != Filters.GetSize())
	{
		return false;
	}

	for (size_t i = 0; i < Filters.GetSize(); ++i)
	{
		auto Filter = Filters.At(i);
		if (!Filter || ArcGISPointCloudFilterInternal::GetObjectType(&Filter) != FilterTypes[i])
		{
			return false;
		}
	}

	return true;
}
void ApplyRendererSettings(ArcGISPointCloudRenderer& Renderer,
						   double PointsPerInch,
						   bool bUseColorModulation,
						   bool bUseFixedSizeDIPs,
						   double FixedSizeDIPs,
						   bool bForceSizeAlgorithmUpdate)
{
	Renderer.SetPointsPerInch(PointsPerInch);
	Renderer.SetColorModulation(bUseColorModulation ? ArcGISPointCloudColorModulation(TEXT("INTENSITY"), 50.0, 250.0) :
													  ArcGISPointCloudColorModulation(nullptr));

	if (!HasSizeAlgorithmType(Renderer, bUseFixedSizeDIPs) || bForceSizeAlgorithmUpdate)
	{
		if (bUseFixedSizeDIPs)
		{
			ArcGISPointCloudFixedSizeAlgorithm FixedSizeAlgorithm(FixedSizeDIPs, ArcGISSymbolSizeUnits::DIPs);
			Renderer.SetSizeAlgorithm(FixedSizeAlgorithm);
		}
		else
		{
			ArcGISPointCloudSplatAlgorithm SplatAlgorithm(1.0);
			Renderer.SetSizeAlgorithm(SplatAlgorithm);
		}
	}
}

void ApplyNewRenderer(ArcGISPointCloudLayer& Layer,
					  ArcGISPointCloudRenderer& Renderer,
					  double PointsPerInch,
					  bool bUseColorModulation,
					  bool bUseFixedSizeDIPs,
					  double FixedSizeDIPs)
{
	ApplyRendererSettings(Renderer, PointsPerInch, bUseColorModulation, bUseFixedSizeDIPs, FixedSizeDIPs, true);

	Layer.SetRenderer(Renderer);
}

ArcGISCollection<ArcGISPointCloudColorClassBreak> CreateClassBreaks()
{
	ArcGISCollection<ArcGISPointCloudColorClassBreak> ClassBreaks;
	ClassBreaks.Add(ArcGISPointCloudColorClassBreak(FColor(200, 120, 30, 255), 0.0, 50.0));
	ClassBreaks.Add(ArcGISPointCloudColorClassBreak(FColor(20, 200, 30, 255), 50.0, 100.0));
	ClassBreaks.Add(ArcGISPointCloudColorClassBreak(FColor(200, 10, 230, 255), 100.0, 250.0));

	return ClassBreaks;
}

ArcGISCollection<ArcGISPointCloudColorStop> CreateColorStops()
{
	ArcGISCollection<ArcGISPointCloudColorStop> Stops;
	Stops.Add(ArcGISPointCloudColorStop(FColor(87, 0, 255, 255), -40.0));
	Stops.Add(ArcGISPointCloudColorStop(FColor(13, 255, 25, 255), 150.0));
	Stops.Add(ArcGISPointCloudColorStop(FColor(240, 255, 23, 255), 250.0));

	return Stops;
}

ArcGISCollection<FString> CreateStringValues(std::initializer_list<const TCHAR*> Values)
{
	ArcGISCollection<FString> Collection;

	for (const auto Value : Values)
	{
		Collection.Add(FString(Value));
	}

	return Collection;
}

ArcGISCollection<ArcGISPointCloudColorUniqueValue> CreateUniqueValues()
{
	ArcGISCollection<ArcGISPointCloudColorUniqueValue> Values;
	Values.Add(ArcGISPointCloudColorUniqueValue(FColor(0, 5, 240, 255), CreateStringValues({TEXT("3"), TEXT("4"), TEXT("5")})));
	Values.Add(ArcGISPointCloudColorUniqueValue(FColor(150, 80, 0, 255), CreateStringValues({TEXT("2")})));
	Values.Add(ArcGISPointCloudColorUniqueValue(FColor(0, 28, 229, 255), CreateStringValues({TEXT("9")})));
	Values.Add(ArcGISPointCloudColorUniqueValue(FColor(120, 129, 129, 255), CreateStringValues({TEXT("6")})));
	Values.Add(ArcGISPointCloudColorUniqueValue(FColor(0, 0, 0, 255), CreateStringValues({TEXT("0"), TEXT("1"), TEXT("11"), TEXT("12")})));

	return Values;
}

ArcGISCollection<ArcGISPointCloudFilter> CreateFilters(EPointCloudTestFilter Filter)
{
	ArcGISCollection<ArcGISPointCloudFilter> Filters;

	switch (Filter)
	{
		case EPointCloudTestFilter::Bitfield:
		{
			ArcGISCollection<uint32_t> RequiredClearBits;
			RequiredClearBits.Add(static_cast<uint32_t>(7));

			ArcGISCollection<uint32_t> RequiredSetBits;
			RequiredSetBits.Add(static_cast<uint32_t>(6));

			ArcGISPointCloudBitfieldFilter BitfieldFilter(TEXT("FLAGS"), RequiredClearBits, RequiredSetBits);
			const ArcGISPointCloudFilter& BaseFilter = BitfieldFilter;
			Filters.Add(BaseFilter);
			break;
		}
		case EPointCloudTestFilter::Return:
		{
			ArcGISCollection<ArcGISPointCloudReturnType> ReturnTypes;
			ReturnTypes.Add(ArcGISPointCloudReturnType::FirstOfMany);

			ArcGISPointCloudReturnFilter ReturnFilter(TEXT("RETURNS"), ReturnTypes);
			const ArcGISPointCloudFilter& BaseFilter = ReturnFilter;
			Filters.Add(BaseFilter);
			break;
		}
		case EPointCloudTestFilter::Value:
		{
			ArcGISCollection<double> Values;
			Values.Add(0.0);
			Values.Add(1.0);
			Values.Add(2.0);
			Values.Add(3.0);
			Values.Add(4.0);
			Values.Add(5.0);

			ArcGISPointCloudValueFilter ValueFilter(TEXT("CLASS_CODE"), Values, ArcGISPointCloudValueFilterMode::Exclude);
			const ArcGISPointCloudFilter& BaseFilter = ValueFilter;
			Filters.Add(BaseFilter);
			break;
		}
		case EPointCloudTestFilter::ValueReturn:
		{
			ArcGISCollection<double> Values;
			Values.Add(2.0);
			Values.Add(7.0);

			ArcGISPointCloudValueFilter ValueFilter(TEXT("CLASS_CODE"), Values, ArcGISPointCloudValueFilterMode::Exclude);
			const ArcGISPointCloudFilter& BaseValueFilter = ValueFilter;
			Filters.Add(BaseValueFilter);

			ArcGISCollection<ArcGISPointCloudReturnType> ReturnTypes;
			ReturnTypes.Add(ArcGISPointCloudReturnType::LastOfMany);

			ArcGISPointCloudReturnFilter ReturnFilter(TEXT("RETURNS"), ReturnTypes);
			const ArcGISPointCloudFilter& BaseReturnFilter = ReturnFilter;
			Filters.Add(BaseReturnFilter);
			break;
		}
		case EPointCloudTestFilter::None:
		default:
			break;
	}

	return Filters;
}
} // namespace

APCLTestCreator::APCLTestCreator() : Super()
{
	ViewStateLogging = CreateDefaultSubobject<UArcGISViewStateLoggingComponent>(TEXT("ArcGISViewStateLoggingComponent"));
}

void APCLTestCreator::OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent)
{
	AArcGISActor::OnArcGISMapComponentChanged(InMapComponent);

	if (MapComponent.IsValid())
	{
		CreateArcGISMap();
	}
}

#if WITH_EDITOR
void APCLTestCreator::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const auto PropertyName = PropertyChangedEvent.MemberProperty ? PropertyChangedEvent.MemberProperty->GetFName() : NAME_None;

	if (PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, APIKey) ||
		PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, PointCloudLayerSource))
	{
		if (MapComponent.IsValid())
		{
			CreateArcGISMap();
		}
	}
	else if (PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, Renderer) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, Filter) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, bUseColorModulation) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, bUseFixedSizeDIPs) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, FixedSizeDIPs) ||
			 PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, PointsPerInch))
	{
		UpdatePointCloudSettings(PropertyName == GET_MEMBER_NAME_CHECKED(APCLTestCreator, FixedSizeDIPs));
	}
}
#endif

void APCLTestCreator::CreateArcGISMap()
{
	CachedPointCloudLayer = nullptr;

	if (PointCloudLayerSource.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("PointCloudLayerSource is empty."));
		return;
	}

	if (APIKey.IsEmpty())
	{
		if (const UArcGISMapsSDKProjectSettings* Settings = GetDefault<UArcGISMapsSDKProjectSettings>())
		{
			APIKey = Settings->APIKey;
		}
	}

	auto map = UArcGISMap::CreateArcGISMapWithMapType(EArcGISMapType::Local);

	auto basemap = UArcGISBasemap::CreateArcGISBasemapWithBasemapStyle(EArcGISBasemapStyle::ArcGISImagery, APIKey);
	map->SetBasemap(basemap);

	auto elevationSource = UArcGISImageElevationSource::CreateArcGISImageElevationSourceWithName(
		"https://elevation3d.arcgis.com/arcgis/rest/services/WorldElevation3D/Terrain3D/ImageServer", "Terrain 3D", APIKey);
	auto mapElevation = UArcGISMapElevation::CreateArcGISMapElevationWithElevationSource(elevationSource);
	map->SetElevation(mapElevation);

	CachedPointCloudLayer =
		UArcGISPointCloudLayer::CreateArcGISPointCloudLayerWithProperties(PointCloudLayerSource, "Moro Bay LiDAR", 1.0f, true, APIKey);
	ApplyPointCloudSettings(CachedPointCloudLayer);

	map->GetLayers()->Add(CachedPointCloudLayer);

	FArcGISViewOptions viewOptions{true};
	MapComponent->GetView()->SetViewOptions(viewOptions);
	MapComponent->SetMap(map);
}

void APCLTestCreator::UpdatePointCloudSettings(bool bForceSizeAlgorithmUpdate)
{
	if (CachedPointCloudLayer)
	{
		ApplyPointCloudSettings(CachedPointCloudLayer, bForceSizeAlgorithmUpdate);
	}
	else if (MapComponent.IsValid())
	{
		CreateArcGISMap();
	}
}

void APCLTestCreator::ApplyPointCloudSettings(UArcGISPointCloudLayer* InPointCloudLayer, bool bForceSizeAlgorithmUpdate) const
{
	if (!InPointCloudLayer || !InPointCloudLayer->APIObject)
	{
		return;
	}

	auto pointCloudLayerAPIObject = StaticCastSharedPtr<ArcGISPointCloudLayer>(InPointCloudLayer->APIObject);
	auto currentRenderer = pointCloudLayerAPIObject->GetRenderer();

	if (HasRendererType(currentRenderer, Renderer))
	{
		ApplyRendererSettings(currentRenderer, PointsPerInch, bUseColorModulation, bUseFixedSizeDIPs, FixedSizeDIPs, bForceSizeAlgorithmUpdate);
	}
	else
	{
		switch (Renderer)
		{
			case EPointCloudTestRenderer::ClassBreaks:
			{
				ArcGISPointCloudClassBreaksRenderer ClassBreaksRenderer(TEXT("INTENSITY"), CreateClassBreaks());
				ApplyNewRenderer(*pointCloudLayerAPIObject, ClassBreaksRenderer, PointsPerInch, bUseColorModulation, bUseFixedSizeDIPs,
								 FixedSizeDIPs);
				break;
			}
			case EPointCloudTestRenderer::Stretch:
			{
				ArcGISPointCloudStretchRenderer StretchRenderer(TEXT("ELEVATION"), CreateColorStops());
				ApplyNewRenderer(*pointCloudLayerAPIObject, StretchRenderer, PointsPerInch, bUseColorModulation, bUseFixedSizeDIPs, FixedSizeDIPs);
				break;
			}
			case EPointCloudTestRenderer::UniqueValue:
			{
				ArcGISPointCloudUniqueValueRenderer UniqueValueRenderer(TEXT("CLASS_CODE"), CreateUniqueValues());
				ApplyNewRenderer(*pointCloudLayerAPIObject, UniqueValueRenderer, PointsPerInch, bUseColorModulation, bUseFixedSizeDIPs,
								 FixedSizeDIPs);
				break;
			}
			case EPointCloudTestRenderer::RGB:
			default:
			{
				ArcGISPointCloudRGBRenderer RGBRenderer(TEXT("RGB"));
				ApplyNewRenderer(*pointCloudLayerAPIObject, RGBRenderer, PointsPerInch, bUseColorModulation, bUseFixedSizeDIPs, FixedSizeDIPs);
				break;
			}
		}
	}

	auto currentFilters = pointCloudLayerAPIObject->GetFilters();

	if (!HasFilterType(currentFilters, Filter))
	{
		pointCloudLayerAPIObject->SetFilters(CreateFilters(Filter));
	}
}

