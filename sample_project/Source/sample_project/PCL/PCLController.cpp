/* Copyright 2026 Esri
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

#include "PCLController.h"

#include "Async/Async.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "ArcGISMapsSDK/API/GameEngine/ArcGISLoadStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudAttribute.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorModulation.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorStop.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorUniqueValue.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFixedSizeAlgorithm.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRGBRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudStretchRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudUniqueValueRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilterMode.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/Symbology/ArcGISSymbolSizeUnits.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISException.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Extent/ArcGISExtentRectangle.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"

namespace PCLControllerPrivate
{
constexpr double DefaultPointSize = 15.0;
constexpr double DefaultPointsPerInch = 40.0;
constexpr double MaxPointSize = 16.0;
constexpr double MinPointSize = 2.0;
constexpr double MinPointsPerInch = 1.0;
constexpr double ElevationLow = -1.5;
constexpr double ElevationMid = 1.5;
constexpr double ElevationHigh = 3.5;
constexpr double IntensityLow = 10385.0;
constexpr double IntensityMid = 38032.0;
constexpr double IntensityHigh = 65680.0;
constexpr float PointCloudLayerLoadRetryInterval = 0.25f;
constexpr int32 MaxPointCloudLayerLoadRetries = 40;
const FString PointCloudLayerSource =
	TEXT("https://tiles.arcgis.com/tiles/V6ZHFr6zdgNZuVG0/arcgis/rest/services/BARNEGAT_BAY_LiDAR_UTM/SceneServer");
const Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType FilterReturnValues[] = {
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::FirstOfMany,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::Last,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::LastOfMany,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::Single};

bool IsValidURL(const FString& source)
{
	return !source.IsEmpty() &&
		   (source.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase) || source.StartsWith(TEXT("http://"), ESearchCase::IgnoreCase));
}

FString NormalizeAttributeName(FString name)
{
	name.ReplaceInline(TEXT("_"), TEXT(""));
	name.ReplaceInline(TEXT("-"), TEXT(""));
	name.ReplaceInline(TEXT(" "), TEXT(""));
	return name.ToUpper();
}

bool MatchesAttributeName(const FString& normalizedName, const TCHAR* candidate)
{
	const FString candidateString(candidate);
	return normalizedName == candidateString || normalizedName.Contains(candidateString);
}

bool IsRGBAttribute(const Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudAttribute& attribute, const FString& normalizedName)
{
	return normalizedName == TEXT("RGB") || normalizedName == TEXT("RGBA") || normalizedName == TEXT("COLOR") || normalizedName == TEXT("COLORRGB") ||
		   attribute.GetValuesPerElement() >= 3;
}

Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudRenderer GetLoadedRenderer(UArcGISPointCloudLayer* pointCloudLayer)
{

	if (!pointCloudLayer || !pointCloudLayer->APIObject)
	{
		return Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudRenderer(nullptr);
	}

	auto layerApi = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(pointCloudLayer->APIObject);

	if (!layerApi || layerApi->GetLoadStatus() != Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudRenderer(nullptr);
	}

	return layerApi->GetRenderer();
}

void ConfigurePointCloudRendererSettings(Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudRenderer& renderer,
										 double pointSize,
										 bool bColorModulationEnabled,
										 const FString& intensityAttributeName)
{
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFixedSizeAlgorithm sizeAlgorithm(
		pointSize, Esri::GameEngine::Map::Symbology::ArcGISSymbolSizeUnits::DIPs);
	renderer.SetSizeAlgorithm(sizeAlgorithm);

	if (bColorModulationEnabled && !intensityAttributeName.IsEmpty())
	{
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorModulation colorModulation(intensityAttributeName, 0.0, 65535.0);
		renderer.SetColorModulation(colorModulation);
	}
	else
	{
		renderer.SetColorModulation(Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorModulation());
	}
}

void AddColorStop(Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop>& stops,
				  double value,
				  FColor&& color,
				  const FString& label)
{
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop stop(color, value);
	stop.SetLabel(label);
	stops.Add(stop);
}

struct FStandardClassInfo
{
	FString Label;
	uint8 Red;
	uint8 Green;
	uint8 Blue;
};

FStandardClassInfo GetStandardClassInfo(int32 classValue)
{
	switch (classValue)
	{
		case 0:
			return {TEXT("Created, never classified"), 128, 128, 128};
		case 1:
			return {TEXT("Unclassified"), 190, 137, 12};
		case 2:
			return {TEXT("Ground"), 219, 255, 104};
		case 3:
			return {TEXT("Low vegetation"), 246, 44, 28};
		case 4:
			return {TEXT("Medium vegetation"), 244, 102, 32};
		case 5:
			return {TEXT("High vegetation"), 199, 24, 255};
		case 6:
			return {TEXT("Building"), 255, 255, 112};
		case 7:
			return {TEXT("Low point (noise)"), 152, 152, 152};
		case 8:
			return {TEXT("Model key-point"), 255, 186, 87};
		case 9:
			return {TEXT("Water"), 246, 244, 22};
		case 10:
			return {TEXT("Rail"), 209, 98, 224};
		case 11:
			return {TEXT("Road surface"), 218, 218, 218};
		case 12:
			return {TEXT("Overlap points"), 84, 167, 255};
		case 13:
			return {TEXT("Wire guard"), 255, 121, 198};
		case 14:
			return {TEXT("Wire conductor"), 255, 160, 67};
		case 15:
			return {TEXT("Transmission tower"), 255, 92, 92};
		case 16:
			return {TEXT("Wire connector"), 136, 255, 218};
		case 17:
			return {TEXT("Bridge deck"), 141, 108, 255};
		case 18:
			return {TEXT("High noise"), 80, 80, 80};
		default:
			return {FString::Printf(TEXT("Class %d"), classValue), 128, 128, 128};
	}
}

void AddClassValue(Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorUniqueValue>& uniqueValues,
				   int32 classValue)
{
	const FStandardClassInfo classInfo = GetStandardClassInfo(classValue);

	Esri::Unreal::ArcGISCollection<FString> valueGroup;
	valueGroup.Add(FString::FromInt(classValue));

	auto color = FColor(classInfo.Red, classInfo.Green, classInfo.Blue, 255);
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorUniqueValue uniqueValue(color, valueGroup);
	uniqueValue.SetLabel(classInfo.Label);
	uniqueValue.SetDescription(classInfo.Label);
	uniqueValues.Add(uniqueValue);
}

int32 CountSelectedOptions(const TArray<TObjectPtr<UCheckBox>>& checkBoxes)
{
	int32 selectedOptionCount = 0;

	for (const TObjectPtr<UCheckBox>& checkBox : checkBoxes)
	{

		if (checkBox && checkBox->IsChecked())
		{
			++selectedOptionCount;
		}
	}

	return selectedOptionCount;
}

template <typename ValueType, typename FilterType, typename ValueGetter, typename FilterFactory>
void AddSelectedPointCloudFilter(const TArray<TObjectPtr<UCheckBox>>& checkBoxes,
								 int32 optionCount,
								 TUniquePtr<Esri::Unreal::ArcGISCollection<ValueType>>& activeValues,
								 TUniquePtr<FilterType>& activeFilter,
								 Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter>& activeFilters,
								 ValueGetter getValue,
								 FilterFactory createFilter)
{
	activeValues = MakeUnique<Esri::Unreal::ArcGISCollection<ValueType>>();
	const int32 optionLimit = FMath::Min(checkBoxes.Num(), optionCount);

	for (int32 index = 0; index < optionLimit; ++index)
	{

		if (checkBoxes[index] && checkBoxes[index]->IsChecked())
		{
			activeValues->Add(getValue(index));
		}
	}

	activeFilter = createFilter(*activeValues);
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter baseFilter(activeFilter->GetHandle());
	activeFilters.Add(baseFilter);
	baseFilter.SetHandle(nullptr);
}

}


APCLController::APCLController()
{
	PrimaryActorTick.bCanEverTick = true;
}

void APCLController::BeginPlay()
{
	Super::BeginPlay();

	MapActor = Cast<AArcGISMapActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass()));

	if (!MapActor)
	{
		return;
	}

	MapComponent = MapActor->GetMapComponent();

	if (!MapComponent)
	{
		return;
	}

	if (UArcGISPoint* originPosition = MapComponent->GetOriginPosition())
	{
		SpatialReference = originPosition->GetSpatialReference();
	}

	InitializePCLUI();
	DeferredPointCloudLayerSource = PCLControllerPrivate::PointCloudLayerSource;
	bDeferredZoomWhenLoaded = true;
	DeferredPointCloudLayerRetrySeconds = PCLControllerPrivate::PointCloudLayerLoadRetryInterval;
	PointCloudLayerLoadRetryCount = 0;
	ApplyPointCloudVisualization();
	ApplyPointCloudFilters();
}

void APCLController::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	ShutdownPCLUI();
	Super::EndPlay(endPlayReason);
}

void APCLController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
	UpdatePCLUI();

	if (!DeferredPointCloudLayerSource.IsEmpty())
	{
		DeferredPointCloudLayerRetrySeconds -= deltaTime;

		if (DeferredPointCloudLayerRetrySeconds <= 0.0f)
		{
			const FString source = DeferredPointCloudLayerSource;
			const bool bZoomWhenLoaded = bDeferredZoomWhenLoaded;
			DeferredPointCloudLayerSource.Reset();
			CreatePointCloudLayer(source, bZoomWhenLoaded);
		}
	}
}

void APCLController::OnPointSizeChanged(float value)
{
	UpdateSliderValueTexts();

	auto renderer = PCLControllerPrivate::GetLoadedRenderer(PointCloudLayer);

	if (renderer)
	{
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFixedSizeAlgorithm sizeAlgorithm(
			FMath::Clamp(static_cast<double>(value), PCLControllerPrivate::MinPointSize, PCLControllerPrivate::MaxPointSize), Esri::GameEngine::Map::Symbology::ArcGISSymbolSizeUnits::DIPs);
		renderer.SetSizeAlgorithm(sizeAlgorithm);
	}
}

void APCLController::OnPointsPerInchChanged(float value)
{
	UpdateSliderValueTexts();

	auto renderer = PCLControllerPrivate::GetLoadedRenderer(PointCloudLayer);

	if (renderer)
	{
		renderer.SetPointsPerInch(FMath::Max(static_cast<double>(value), PCLControllerPrivate::MinPointsPerInch));
	}
}

void APCLController::SetColorModulationEnabled(bool bEnabled)
{

	if (bColorModulationEnabled == bEnabled)
	{
		return;
	}

	bColorModulationEnabled = bEnabled;

	auto renderer = PCLControllerPrivate::GetLoadedRenderer(PointCloudLayer);

	if (renderer)
	{
		renderer.SetColorModulation(bColorModulationEnabled && !IntensityAttributeName.IsEmpty() ?
										Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorModulation(IntensityAttributeName, 0.0, 65535.0) :
										Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorModulation());
	}
}

void APCLController::SetPointCloudRenderer(EPCLRendererChoice rendererChoice)
{
	RefreshAvailablePointCloudAttributes();

	if (!IsRendererAvailableFromCachedAttributes(rendererChoice))
	{
		rendererChoice = GetFallbackRendererChoice();
	}

	if (CurrentRendererChoice == rendererChoice)
	{
		UpdateRendererCheckBoxes();
		BuildLegendUI();
		return;
	}

	CurrentRendererChoice = rendererChoice;
	UpdateRendererCheckBoxes();
	BuildLegendUI();
	ApplyPointCloudVisualization();
}

bool APCLController::IsPointCloudRendererAvailable(EPCLRendererChoice rendererChoice)
{
	RefreshAvailablePointCloudAttributes();
	return IsRendererAvailableFromCachedAttributes(rendererChoice);
}

void APCLController::DeferPointCloudLayerLoad(const FString& source, bool bZoomWhenLoaded)
{
	++PointCloudLayerLoadRetryCount;

	if (PointCloudLayerLoadRetryCount > PCLControllerPrivate::MaxPointCloudLayerLoadRetries)
	{
		DeferredPointCloudLayerSource.Reset();
		SetLayerLoadStatus(false);

		if (LoadLayerButton)
		{
			LoadLayerButton->SetIsEnabled(true);
		}
		return;
	}

	DeferredPointCloudLayerSource = source;
	bDeferredZoomWhenLoaded = bZoomWhenLoaded;
	DeferredPointCloudLayerRetrySeconds = PCLControllerPrivate::PointCloudLayerLoadRetryInterval;

	if (LoadLayerButton)
	{
		LoadLayerButton->SetIsEnabled(false);
	}

	if (LayerLoadStatusText)
	{
		LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void APCLController::CreatePointCloudLayer(const FString& source, bool bZoomWhenLoaded)
{

	if (!PCLControllerPrivate::IsValidURL(source))
	{
		SetLayerLoadStatus(false);
		return;
	}

	if (!MapComponent)
	{
		SetLayerLoadStatus(false);
		return;
	}

	auto* map = MapComponent->GetMap();

	if (!map || !map->APIObject || !static_cast<bool>(*map->APIObject))
	{
		DeferPointCloudLayerLoad(source, bZoomWhenLoaded);
		return;
	}

	UArcGISLayerCollection* mapLayers = nullptr;
	try
	{
		mapLayers = map->GetLayers();
	}
	catch (const Esri::Unreal::ArcGISException&)
	{
		DeferPointCloudLayerLoad(source, bZoomWhenLoaded);
		return;
	}

	if (!mapLayers)
	{
		DeferPointCloudLayerLoad(source, bZoomWhenLoaded);
		return;
	}

	DeferredPointCloudLayerSource.Reset();
	PointCloudLayerLoadRetryCount = 0;

	++LayerLoadRequestId;
	const uint64 requestId = LayerLoadRequestId;

	if (PendingPointCloudLayer)
	{
		const int64 pendingLayerId = PendingPointCloudLayer->GetInstanceId();

		for (int64 index = mapLayers->GetSize() - 1; index >= 0; --index)
		{

			if (UArcGISLayer* existingLayer = mapLayers->At(index); existingLayer && existingLayer->GetInstanceId() == pendingLayerId)
			{
				mapLayers->Remove(index);
				break;
			}
		}
		PendingPointCloudLayer = nullptr;
	}

	UArcGISPointCloudLayer* candidateLayer = nullptr;
	try
	{
		candidateLayer = UArcGISPointCloudLayer::CreateArcGISPointCloudLayer(source, MapComponent->GetAPIKey());
	}
	catch (const Esri::Unreal::ArcGISException&)
	{
		SetLayerLoadStatus(false);
		return;
	}

	if (!candidateLayer || !candidateLayer->APIObject)
	{
		SetLayerLoadStatus(false);
		return;
	}

	PendingPointCloudLayer = candidateLayer;
	candidateLayer->SetOpacity(1.0f);
	candidateLayer->SetIsVisible(true);

	if (LoadLayerButton)
	{
		LoadLayerButton->SetIsEnabled(false);
	}

	if (LayerLoadStatusText)
	{
		LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
	}

	TWeakObjectPtr<APCLController> weakThis(this);
	TWeakObjectPtr<UArcGISPointCloudLayer> weakCandidate(candidateLayer);
	candidateLayer->APIObject->SetDoneLoading([weakThis, weakCandidate, requestId,
											   bZoomWhenLoaded](Esri::Unreal::ArcGISException& loadError) {
		const bool bHadLoadError = static_cast<bool>(loadError);

		AsyncTask(ENamedThreads::GameThread, [weakThis, weakCandidate, requestId, bZoomWhenLoaded, bHadLoadError]() {
			auto* controller = weakThis.Get();
			auto* loadedLayer = weakCandidate.Get();

			if (!controller || !loadedLayer || controller->LayerLoadRequestId != requestId || controller->PendingPointCloudLayer != loadedLayer)
			{
				return;
			}

			controller->PendingPointCloudLayer = nullptr;

			if (controller->LoadLayerButton)
			{
				controller->LoadLayerButton->SetIsEnabled(true);
			}

			auto layerApi = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(loadedLayer->APIObject);
			const bool bLoaded = !bHadLoadError && layerApi && layerApi->GetLoadStatus() == Esri::GameEngine::ArcGISLoadStatus::Loaded;

			auto* currentMap = controller->MapComponent ? controller->MapComponent->GetMap() : nullptr;
			auto* currentLayers = currentMap ? currentMap->GetLayers() : nullptr;

			if (!bLoaded)
			{

				if (currentLayers)
				{
					const int64 loadedLayerId = loadedLayer->GetInstanceId();

					for (int64 index = currentLayers->GetSize() - 1; index >= 0; --index)
					{

						if (UArcGISLayer* existingLayer = currentLayers->At(index); existingLayer && existingLayer->GetInstanceId() == loadedLayerId)
						{
							currentLayers->Remove(index);
							break;
						}
					}
				}

				controller->SetLayerLoadStatus(false);
				return;
			}

			if (currentLayers)
			{
				const int64 loadedLayerId = loadedLayer->GetInstanceId();

				for (int64 index = currentLayers->GetSize() - 1; index >= 0; --index)
				{

					if (auto* existingPointCloudLayer = Cast<UArcGISPointCloudLayer>(currentLayers->At(index));
						existingPointCloudLayer && existingPointCloudLayer->GetInstanceId() != loadedLayerId)
					{
						currentLayers->Remove(index);
					}
				}
			}

			controller->PointCloudLayer = loadedLayer;
			controller->SetLayerLoadStatus(true);
			controller->RefreshAvailablePointCloudAttributes();
			controller->UpdateRendererCheckBoxes();
			controller->ApplyPointCloudVisualization();
			controller->BuildFilterTabUI();
			controller->BuildLegendUI();
			controller->ApplyPointCloudFilters();

			if (bZoomWhenLoaded && controller->MapComponent)
			{
				UArcGISExtentRectangle* layerExtent = loadedLayer->GetExtent();

				if (layerExtent)
				{

					if (UArcGISPoint* extentCenter = layerExtent->GetCenter())
					{
						controller->MapComponent->SetOriginPosition(extentCenter);
						controller->SpatialReference = extentCenter->GetSpatialReference();
					}
				}

				APlayerController* playerController = UGameplayStatics::GetPlayerController(controller->GetWorld(), 0);
				AActor* viewActor = playerController ? playerController->GetViewTarget() : nullptr;

				if (!viewActor && playerController)
				{
					viewActor = playerController->GetPawn();
				}

				if (ACharacter* viewCharacter = Cast<ACharacter>(viewActor))
				{
					viewCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				}

				if (viewActor)
				{
					controller->MapComponent->ZoomToExtent(viewActor, layerExtent);
				}
			}
		});
	});

	mapLayers->Add(candidateLayer);
}

void APCLController::ApplyPointCloudVisualization()
{
	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	auto layerApi = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject);

	if (!layerApi)
	{
		return;
	}

	if (layerApi->GetLoadStatus() != Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return;
	}

	RefreshAvailablePointCloudAttributes();

	const double pointSize =
		FMath::Clamp(PointSizeSlider ? static_cast<double>(PointSizeSlider->GetValue()) : PCLControllerPrivate::DefaultPointSize, PCLControllerPrivate::MinPointSize, PCLControllerPrivate::MaxPointSize);
	const double pointsPerInch =
		FMath::Max(PointsPerInchSlider ? static_cast<double>(PointsPerInchSlider->GetValue()) : PCLControllerPrivate::DefaultPointsPerInch, PCLControllerPrivate::MinPointsPerInch);

	EnsureAvailableRendererSelected();

	auto applyRenderer = [&](auto& renderer) {
		PCLControllerPrivate::ConfigurePointCloudRendererSettings(renderer, pointSize, bColorModulationEnabled, IntensityAttributeName);
		layerApi->SetRenderer(renderer);

		auto attachedRenderer = layerApi->GetRenderer();

		if (attachedRenderer)
		{
			attachedRenderer.SetPointsPerInch(pointsPerInch);
		}
	};

	auto applyRgbRenderer = [&]() {
		const FString attributeName = RGBAttributeName.IsEmpty() ? TEXT("RGB") : RGBAttributeName;
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudRGBRenderer renderer(attributeName);
		applyRenderer(renderer);
	};

	switch (CurrentRendererChoice)
	{
		case EPCLRendererChoice::Class:

			if (!ClassAttributeName.IsEmpty())
			{
				Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorUniqueValue> uniqueValues;

				for (int32 classValue = 0; classValue <= 18; ++classValue)
				{
					PCLControllerPrivate::AddClassValue(uniqueValues, classValue);
				}

				Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudUniqueValueRenderer renderer(ClassAttributeName, uniqueValues);
				applyRenderer(renderer);
				return;
			}
			break;
		case EPCLRendererChoice::Elevation:

			if (!ElevationAttributeName.IsEmpty())
			{
				Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop> stops;
				PCLControllerPrivate::AddColorStop(stops, PCLControllerPrivate::ElevationLow, FColor(42, 43, 238, 255), TEXT("< -1.5"));
				PCLControllerPrivate::AddColorStop(stops, 0.0, FColor(40, 210, 246, 255), TEXT(""));
				PCLControllerPrivate::AddColorStop(stops, PCLControllerPrivate::ElevationMid, FColor(91, 248, 134, 255), TEXT("1.5"));
				PCLControllerPrivate::AddColorStop(stops, 2.5, FColor(250, 244, 73, 255), TEXT(""));
				PCLControllerPrivate::AddColorStop(stops, PCLControllerPrivate::ElevationHigh, FColor(255, 59, 22, 255), TEXT("> 3.5"));

				Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudStretchRenderer renderer(ElevationAttributeName, stops);
				applyRenderer(renderer);
				return;
			}
			break;
		case EPCLRendererChoice::Intensity:

			if (!IntensityAttributeName.IsEmpty())
			{
				Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop> stops;
				PCLControllerPrivate::AddColorStop(stops, PCLControllerPrivate::IntensityLow, FColor(0, 0, 0, 255), TEXT("< 10,385"));
				PCLControllerPrivate::AddColorStop(stops, PCLControllerPrivate::IntensityMid, FColor(128, 128, 128, 255), TEXT("38,032"));
				PCLControllerPrivate::AddColorStop(stops, PCLControllerPrivate::IntensityHigh, FColor(255, 255, 255, 255), TEXT("> 65,680"));

				Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudStretchRenderer renderer(IntensityAttributeName, stops);
				applyRenderer(renderer);
				return;
			}
			break;
		case EPCLRendererChoice::RGB:
		default:
			applyRgbRenderer();
			return;
	}

	applyRgbRenderer();
}

void APCLController::ApplyPointCloudFilters()
{

	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	auto layerApi = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject);

	if (!layerApi || layerApi->GetLoadStatus() != Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return;
	}

	RefreshAvailablePointCloudAttributes();

	ActiveFilterCollection = MakeUnique<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter>>();
	ActiveClassCodeValues.Reset();
	ActiveReturnsValues.Reset();
	ActiveClassCodeFilter.Reset();
	ActiveReturnsFilter.Reset();

	const int32 selectedClassOptionCount = PCLControllerPrivate::CountSelectedOptions(ClassFilterCheckBoxes);
	const bool bUseClassFilter = !ClassAttributeName.IsEmpty() && selectedClassOptionCount > 0 &&
								 selectedClassOptionCount < ClassFilterCheckBoxes.Num();

	if (bUseClassFilter)
	{
		PCLControllerPrivate::AddSelectedPointCloudFilter(
			ClassFilterCheckBoxes,
			ClassFilterValues.Num(),
			ActiveClassCodeValues,
			ActiveClassCodeFilter,
			*ActiveFilterCollection,
			[this](int32 index) { return static_cast<double>(ClassFilterValues[index]); },
			[this](const Esri::Unreal::ArcGISCollection<double>& selectedValues)
			{
				return MakeUnique<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudValueFilter>(
					ClassAttributeName,
					selectedValues,
					Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudValueFilterMode::Include);
			});
	}

	const int32 selectedReturnsOptionCount = PCLControllerPrivate::CountSelectedOptions(ReturnsFilterCheckBoxes);
	const bool bUseReturnsFilter = !ReturnsAttributeName.IsEmpty() && selectedReturnsOptionCount > 0 &&
								   selectedReturnsOptionCount < ReturnsFilterCheckBoxes.Num();

	if (bUseReturnsFilter)
	{
		PCLControllerPrivate::AddSelectedPointCloudFilter(
			ReturnsFilterCheckBoxes,
			static_cast<int32>(UE_ARRAY_COUNT(PCLControllerPrivate::FilterReturnValues)),
			ActiveReturnsValues,
			ActiveReturnsFilter,
			*ActiveFilterCollection,
			[](int32 index) { return PCLControllerPrivate::FilterReturnValues[index]; },
			[this](
				const Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType>& selectedValues)
			{
				return MakeUnique<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnFilter>(
					ReturnsAttributeName, selectedValues);
			});
	}

	layerApi->SetFilters(*ActiveFilterCollection);
}

void APCLController::RefreshAvailablePointCloudAttributes()
{
	RGBAttributeName.Reset();
	ClassAttributeName.Reset();
	ElevationAttributeName.Reset();
	IntensityAttributeName.Reset();
	ReturnsAttributeName.Reset();

	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	auto layerApi = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject);

	if (!layerApi || layerApi->GetLoadStatus() != Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return;
	}

	auto attributes = layerApi->GetAttributes();

	if (!attributes)
	{
		return;
	}

	for (size_t index = 0; index < attributes.GetSize(); ++index)
	{
		auto attribute = attributes.At(index);

		if (!attribute)
		{
			continue;
		}

		const FString name = attribute.GetName();
		const FString normalizedName = PCLControllerPrivate::NormalizeAttributeName(name);

		if (RGBAttributeName.IsEmpty() && PCLControllerPrivate::IsRGBAttribute(attribute, normalizedName))
		{
			RGBAttributeName = name;
		}

		if (ClassAttributeName.IsEmpty() &&
			(PCLControllerPrivate::MatchesAttributeName(normalizedName, TEXT("CLASSCODE")) || PCLControllerPrivate::MatchesAttributeName(normalizedName, TEXT("CLASSIFICATION")) ||
			 PCLControllerPrivate::MatchesAttributeName(normalizedName, TEXT("CLASS"))))
		{
			ClassAttributeName = name;
		}

		if (ElevationAttributeName.IsEmpty() && (PCLControllerPrivate::MatchesAttributeName(normalizedName, TEXT("ELEVATION")) ||
												 PCLControllerPrivate::MatchesAttributeName(normalizedName, TEXT("HEIGHT")) || normalizedName == TEXT("Z")))
		{
			ElevationAttributeName = name;
		}

		if (IntensityAttributeName.IsEmpty() && PCLControllerPrivate::MatchesAttributeName(normalizedName, TEXT("INTENSITY")))
		{
			IntensityAttributeName = name;
		}

		if (ReturnsAttributeName.IsEmpty() && PCLControllerPrivate::MatchesAttributeName(normalizedName, TEXT("RETURNS")))
		{
			ReturnsAttributeName = name;
		}
	}
}

bool APCLController::IsRendererAvailableFromCachedAttributes(EPCLRendererChoice rendererChoice) const
{
	switch (rendererChoice)
	{
		case EPCLRendererChoice::RGB:
			return !RGBAttributeName.IsEmpty();
		case EPCLRendererChoice::Class:
			return !ClassAttributeName.IsEmpty();
		case EPCLRendererChoice::Elevation:
			return !ElevationAttributeName.IsEmpty();
		case EPCLRendererChoice::Intensity:
			return !IntensityAttributeName.IsEmpty();
		default:
			return false;
	}
}

EPCLRendererChoice APCLController::GetFallbackRendererChoice() const
{

	if (IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::RGB))
	{
		return EPCLRendererChoice::RGB;
	}

	if (IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Class))
	{
		return EPCLRendererChoice::Class;
	}

	if (IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Elevation))
	{
		return EPCLRendererChoice::Elevation;
	}

	if (IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Intensity))
	{
		return EPCLRendererChoice::Intensity;
	}

	return EPCLRendererChoice::RGB;
}

void APCLController::EnsureAvailableRendererSelected()
{

	if (!IsRendererAvailableFromCachedAttributes(CurrentRendererChoice))
	{
		CurrentRendererChoice = GetFallbackRendererChoice();
	}
}

void APCLController::ClearActiveFilters()
{

	if (PointCloudLayer && PointCloudLayer->APIObject)
	{

		if (auto layerApi = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject))
		{
			ActiveFilterCollection = MakeUnique<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter>>();
			layerApi->SetFilters(*ActiveFilterCollection);
		}
	}

	ActiveClassCodeValues.Reset();
	ActiveReturnsValues.Reset();
	ActiveClassCodeFilter.Reset();
	ActiveReturnsFilter.Reset();
}
