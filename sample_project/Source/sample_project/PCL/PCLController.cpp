// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License. */


#include "PCLController.h"

#include "ArcGISMapsSDK/API/GameEngine/ArcGISLoadStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudAttribute.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorModulation.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorStop.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorUniqueValue.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFixedSizeAlgorithm.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRGBRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudStretchRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudUniqueValueRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/Symbology/ArcGISSymbolSizeUnits.h"
#include "ArcGISMapsSDK/API/Standard/ArcGISRGBColor.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "sample_project/InputManager.h"

namespace
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
const FString PointCloudLayerSource = TEXT("https://www.arcgis.com/home/item.html?id=93c83277e8c34ea2ab38f2e1eb1e0d63");

FText FormatSliderValue(float Value)
{
	return FText::FromString(FString::Printf(TEXT("%.0f"), Value));
}

FString NormalizeAttributeName(FString Name)
{
	Name.ReplaceInline(TEXT("_"), TEXT(""));
	Name.ReplaceInline(TEXT("-"), TEXT(""));
	Name.ReplaceInline(TEXT(" "), TEXT(""));
	return Name.ToUpper();
}

bool MatchesAttributeName(const FString& NormalizedName, const TCHAR* Candidate)
{
	const FString CandidateString(Candidate);
	return NormalizedName == CandidateString || NormalizedName.Contains(CandidateString);
}

bool IsRGBAttribute(const Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudAttribute& Attribute, const FString& NormalizedName)
{
	return NormalizedName == TEXT("RGB") || NormalizedName == TEXT("RGBA") || NormalizedName == TEXT("COLOR") ||
		   NormalizedName == TEXT("COLORRGB") || Attribute.GetValuesPerElement() >= 3;
}

Esri::Standard::ArcGISRGBColor MakeColor(uint8 Red, uint8 Green, uint8 Blue)
{
	return Esri::Standard::ArcGISRGBColor(Red, Green, Blue, 255);
}

void AddColorStop(Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop>& Stops,
				  double Value,
				  Esri::Standard::ArcGISRGBColor&& Color,
				  const FString& Label)
{
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop Stop(Color, Value);
	Stop.SetLabel(Label);
	Stops.Add(Stop);
}

void GetStandardClassInfo(int32 ClassValue, FString& Label, uint8& Red, uint8& Green, uint8& Blue)
{
	switch (ClassValue)
	{
	case 0:
		Label = TEXT("Created, never classified");
		Red = 128;
		Green = 128;
		Blue = 128;
		return;
	case 1:
		Label = TEXT("Unclassified");
		Red = 190;
		Green = 137;
		Blue = 12;
		return;
	case 2:
		Label = TEXT("Ground");
		Red = 219;
		Green = 255;
		Blue = 104;
		return;
	case 3:
		Label = TEXT("Low vegetation");
		Red = 246;
		Green = 44;
		Blue = 28;
		return;
	case 4:
		Label = TEXT("Medium vegetation");
		Red = 244;
		Green = 102;
		Blue = 32;
		return;
	case 5:
		Label = TEXT("High vegetation");
		Red = 199;
		Green = 24;
		Blue = 255;
		return;
	case 6:
		Label = TEXT("Building");
		Red = 255;
		Green = 255;
		Blue = 112;
		return;
	case 7:
		Label = TEXT("Low point (noise)");
		Red = 152;
		Green = 152;
		Blue = 152;
		return;
	case 8:
		Label = TEXT("Model key-point");
		Red = 255;
		Green = 186;
		Blue = 87;
		return;
	case 9:
		Label = TEXT("Water");
		Red = 246;
		Green = 244;
		Blue = 22;
		return;
	case 10:
		Label = TEXT("Rail");
		Red = 209;
		Green = 98;
		Blue = 224;
		return;
	case 11:
		Label = TEXT("Road surface");
		Red = 218;
		Green = 218;
		Blue = 218;
		return;
	case 12:
		Label = TEXT("Overlap points");
		Red = 84;
		Green = 167;
		Blue = 255;
		return;
	case 13:
		Label = TEXT("Wire guard");
		Red = 255;
		Green = 121;
		Blue = 198;
		return;
	case 14:
		Label = TEXT("Wire conductor");
		Red = 255;
		Green = 160;
		Blue = 67;
		return;
	case 15:
		Label = TEXT("Transmission tower");
		Red = 255;
		Green = 92;
		Blue = 92;
		return;
	case 16:
		Label = TEXT("Wire connector");
		Red = 136;
		Green = 255;
		Blue = 218;
		return;
	case 17:
		Label = TEXT("Bridge deck");
		Red = 141;
		Green = 108;
		Blue = 255;
		return;
	case 18:
		Label = TEXT("High noise");
		Red = 80;
		Green = 80;
		Blue = 80;
		return;
	default:
		Label = FString::Printf(TEXT("Class %d"), ClassValue);
		Red = 128;
		Green = 128;
		Blue = 128;
		return;
	}
}

void AddClassValue(Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorUniqueValue>& UniqueValues,
				   int32 ClassValue)
{
	FString Label;
	uint8 Red = 128;
	uint8 Green = 128;
	uint8 Blue = 128;
	GetStandardClassInfo(ClassValue, Label, Red, Green, Blue);

	Esri::Unreal::ArcGISCollection<FString> ValueGroup;
	ValueGroup.Add(FString::FromInt(ClassValue));

	auto Color = MakeColor(Red, Green, Blue);
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorUniqueValue UniqueValue(Color, ValueGroup);
	UniqueValue.SetLabel(Label);
	UniqueValue.SetDescription(Label);
	UniqueValues.Add(UniqueValue);
}
} // namespace

// Sets default values
APCLController::APCLController()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APCLController::BeginPlay()
{
	Super::BeginPlay();

	MapActor = Cast<AArcGISMapActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass()));

	if (!MapActor)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapActor not found in the level!"));
		return;
	}

	MapComponent = MapActor->GetMapComponent();
	if (!MapComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapComponent not found on ArcGISMapActor!"));
		return;
	}

	if (UArcGISPoint* OriginPosition = MapComponent->GetOriginPosition())
	{
		SpatialReference = OriginPosition->GetSpatialReference();
	}

	if (!InputManager)
	{
		InputManager = Cast<AInputManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AInputManager::StaticClass()));
	}

	if (InputManager)
	{
		InputManager->OnInputTrigger.AddDynamic(this, &APCLController::OnInputTriggered);
		InputManager->OnInputEnd.AddDynamic(this, &APCLController::OnInputEnded);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InputManager not found in the level."));
	}

	auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (playerController)
	{
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
	}

	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (!UIWidget)
		{
			return;
		}

		UIWidget->AddToViewport();
		UnitDropdown = Cast<UComboBoxString>(UIWidget->GetWidgetFromName(TEXT("UnitDropDown")));
		PointSizeSlider = Cast<USlider>(UIWidget->GetWidgetFromName(TEXT("Slider_PointsPerInch_1")));
		PointsPerInchSlider = Cast<USlider>(UIWidget->GetWidgetFromName(TEXT("Slider_PointsPerInch")));
		PointSizeValueText = Cast<UTextBlock>(UIWidget->GetWidgetFromName(TEXT("Text_PointSizeValue")));
		PointsPerInchValueText = Cast<UTextBlock>(UIWidget->GetWidgetFromName(TEXT("Text_PointsPerInchValue")));
		ColorModulationCheckBox = Cast<UCheckBox>(UIWidget->GetWidgetFromName(TEXT("Checkbox_ColorModulation")));
		RGBRendererCheckBox = Cast<UCheckBox>(UIWidget->GetWidgetFromName(TEXT("Checkbox_Renderer_RGB")));
		ClassRendererCheckBox = Cast<UCheckBox>(UIWidget->GetWidgetFromName(TEXT("Checkbox_Renderer_Class")));
		ElevationRendererCheckBox = Cast<UCheckBox>(UIWidget->GetWidgetFromName(TEXT("Checkbox_Renderer_Elevation")));
		IntensityRendererCheckBox = Cast<UCheckBox>(UIWidget->GetWidgetFromName(TEXT("Checkbox_Renderer_Intensity")));

		if (PointSizeSlider)
		{
			PointSizeSlider->SetMinValue(MinPointSize);
			PointSizeSlider->SetMaxValue(MaxPointSize);
			PointSizeSlider->SetStepSize(1.0f / static_cast<float>(MaxPointSize - MinPointSize));
			PointSizeSlider->SetValue(FMath::Clamp(PointSizeSlider->GetValue(), static_cast<float>(MinPointSize), static_cast<float>(MaxPointSize)));
			PointSizeSlider->OnValueChanged.AddDynamic(this, &APCLController::OnPointSizeChanged);
		}

		if (PointsPerInchSlider)
		{
			PointsPerInchSlider->SetMinValue(MinPointsPerInch);
			PointsPerInchSlider->SetValue(FMath::Max(PointsPerInchSlider->GetValue(), static_cast<float>(MinPointsPerInch)));
			PointsPerInchSlider->OnValueChanged.AddDynamic(this, &APCLController::OnPointsPerInchChanged);
		}

		if (ColorModulationCheckBox)
		{
			ColorModulationCheckBox->SetIsChecked(bColorModulationEnabled);
			ColorModulationCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnColorModulationCheckStateChanged);
		}

		if (RGBRendererCheckBox)
		{
			RGBRendererCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnRGBRendererCheckStateChanged);
		}

		if (ClassRendererCheckBox)
		{
			ClassRendererCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnClassRendererCheckStateChanged);
		}

		if (ElevationRendererCheckBox)
		{
			ElevationRendererCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnElevationRendererCheckStateChanged);
		}

		if (IntensityRendererCheckBox)
		{
			IntensityRendererCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnIntensityRendererCheckStateChanged);
		}

		UpdateSliderValueTexts();
		UpdateRendererCheckBoxes();
		CreatePointCloudLayer();
		ApplyPointCloudVisualization();

		if (UIWidget->FindFunction("ShowInstruction"))
		{
			UIWidget->ProcessEvent(UIWidget->FindFunction("ShowInstruction"), nullptr);
		}
	}
}

void APCLController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (InputManager)
	{
		InputManager->OnInputTrigger.RemoveDynamic(this, &APCLController::OnInputTriggered);
		InputManager->OnInputEnd.RemoveDynamic(this, &APCLController::OnInputEnded);
	}

	Super::EndPlay(EndPlayReason);
}

// Called every frame
void APCLController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APCLController::OnInputTriggered()
{
}

void APCLController::OnInputEnded()
{
}

void APCLController::OnPointSizeChanged(float Value)
{
	UpdateSliderValueTexts();
	ApplyPointCloudVisualization();
}

void APCLController::OnPointsPerInchChanged(float Value)
{
	UpdateSliderValueTexts();
	ApplyPointCloudVisualization();
}

void APCLController::SetColorModulationEnabled(bool bEnabled)
{
	if (bColorModulationEnabled == bEnabled)
	{
		return;
	}

	bColorModulationEnabled = bEnabled;
	ApplyPointCloudVisualization();
}

void APCLController::SetPointCloudRenderer(EPCLRendererChoice RendererChoice)
{
	if (CurrentRendererChoice == RendererChoice)
	{
		return;
	}

	CurrentRendererChoice = RendererChoice;
	UpdateRendererCheckBoxes();
	ApplyPointCloudVisualization();
}

bool APCLController::IsPointCloudRendererAvailable(EPCLRendererChoice RendererChoice)
{
	RefreshAvailablePointCloudAttributes();

	switch (RendererChoice)
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

void APCLController::OnColorModulationCheckStateChanged(bool bIsChecked)
{
	SetColorModulationEnabled(bIsChecked);
}

void APCLController::OnRGBRendererCheckStateChanged(bool bIsChecked)
{
	if (bIsChecked && !bUpdatingRendererCheckBoxes)
	{
		SetPointCloudRenderer(EPCLRendererChoice::RGB);
	}
}

void APCLController::OnClassRendererCheckStateChanged(bool bIsChecked)
{
	if (bIsChecked && !bUpdatingRendererCheckBoxes)
	{
		SetPointCloudRenderer(EPCLRendererChoice::Class);
	}
}

void APCLController::OnElevationRendererCheckStateChanged(bool bIsChecked)
{
	if (bIsChecked && !bUpdatingRendererCheckBoxes)
	{
		SetPointCloudRenderer(EPCLRendererChoice::Elevation);
	}
}

void APCLController::OnIntensityRendererCheckStateChanged(bool bIsChecked)
{
	if (bIsChecked && !bUpdatingRendererCheckBoxes)
	{
		SetPointCloudRenderer(EPCLRendererChoice::Intensity);
	}
}

void APCLController::CreatePointCloudLayer()
{
	PointCloudLayer = nullptr;

	if (!MapComponent)
	{
		return;
	}

	auto* Map = MapComponent->GetMap();
	if (!Map)
	{
		return;
	}

	auto* MapLayers = Map->GetLayers();
	if (!MapLayers)
	{
		return;
	}

	for (int64 Index = MapLayers->GetSize() - 1; Index >= 0; --Index)
	{
		if (Cast<UArcGISPointCloudLayer>(MapLayers->At(Index)))
		{
			MapLayers->Remove(Index);
		}
	}

	PointCloudLayer = UArcGISPointCloudLayer::CreateArcGISPointCloudLayerWithProperties(
		PointCloudLayerSource, TEXT("Point Cloud Scene Layer"), 1.0f, true, MapComponent->GetAPIKey());

	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	TWeakObjectPtr<APCLController> WeakThis(this);
	PointCloudLayer->APIObject->SetDoneLoading([WeakThis](auto& LoadError) {
		if (auto* Controller = WeakThis.Get())
		{
			Controller->ApplyPointCloudVisualization();
		}
	});

	MapLayers->Add(PointCloudLayer);
}

void APCLController::ApplyPointCloudVisualization()
{
	if (!PointCloudLayer)
	{
		CreatePointCloudLayer();
	}

	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	auto LayerAPI = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject);
	if (!LayerAPI)
	{
		return;
	}

	if (LayerAPI->GetLoadStatus() != Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return;
	}

	RefreshAvailablePointCloudAttributes();

	const double PointSize =
		FMath::Clamp(PointSizeSlider ? static_cast<double>(PointSizeSlider->GetValue()) : DefaultPointSize, MinPointSize, MaxPointSize);
	const double PointsPerInch = FMath::Max(PointsPerInchSlider ? static_cast<double>(PointsPerInchSlider->GetValue()) : DefaultPointsPerInch,
											MinPointsPerInch);

	auto ConfigureRenderer = [this, PointSize, PointsPerInch](auto& Renderer) {
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFixedSizeAlgorithm SizeAlgorithm(
			PointSize, Esri::GameEngine::Map::Symbology::ArcGISSymbolSizeUnits::DIPs);
		Renderer.SetSizeAlgorithm(SizeAlgorithm);
		Renderer.SetPointsPerInch(PointsPerInch);

		if (bColorModulationEnabled && !IntensityAttributeName.IsEmpty())
		{
			Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorModulation ColorModulation(IntensityAttributeName, 0.0, 65535.0);
			Renderer.SetColorModulation(ColorModulation);
		}
	};

	auto ApplyRGBRenderer = [&]() {
		const FString AttributeName = RGBAttributeName.IsEmpty() ? TEXT("RGB") : RGBAttributeName;
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudRGBRenderer Renderer(AttributeName);
		ConfigureRenderer(Renderer);
		LayerAPI->SetRenderer(Renderer);
	};

	switch (CurrentRendererChoice)
	{
	case EPCLRendererChoice::Class:
		if (!ClassAttributeName.IsEmpty())
		{
			Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorUniqueValue> UniqueValues;
			for (int32 ClassValue = 0; ClassValue <= 18; ++ClassValue)
			{
				AddClassValue(UniqueValues, ClassValue);
			}

			Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudUniqueValueRenderer Renderer(ClassAttributeName, UniqueValues);
			ConfigureRenderer(Renderer);
			LayerAPI->SetRenderer(Renderer);
			return;
		}
		break;
	case EPCLRendererChoice::Elevation:
		if (!ElevationAttributeName.IsEmpty())
		{
			Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop> Stops;
			AddColorStop(Stops, ElevationLow, MakeColor(42, 43, 238), TEXT("< -1.5"));
			AddColorStop(Stops, 0.0, MakeColor(40, 210, 246), TEXT(""));
			AddColorStop(Stops, ElevationMid, MakeColor(91, 248, 134), TEXT("1.5"));
			AddColorStop(Stops, 2.5, MakeColor(250, 244, 73), TEXT(""));
			AddColorStop(Stops, ElevationHigh, MakeColor(255, 59, 22), TEXT("> 3.5"));

			Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudStretchRenderer Renderer(ElevationAttributeName, Stops);
			ConfigureRenderer(Renderer);
			LayerAPI->SetRenderer(Renderer);
			return;
		}
		break;
	case EPCLRendererChoice::Intensity:
		if (!IntensityAttributeName.IsEmpty())
		{
			Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop> Stops;
			AddColorStop(Stops, IntensityLow, MakeColor(0, 0, 0), TEXT("< 10,385"));
			AddColorStop(Stops, IntensityMid, MakeColor(128, 128, 128), TEXT("38,032"));
			AddColorStop(Stops, IntensityHigh, MakeColor(255, 255, 255), TEXT("> 65,680"));

			Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudStretchRenderer Renderer(IntensityAttributeName, Stops);
			ConfigureRenderer(Renderer);
			LayerAPI->SetRenderer(Renderer);
			return;
		}
		break;
	case EPCLRendererChoice::RGB:
	default:
		ApplyRGBRenderer();
		return;
	}

	ApplyRGBRenderer();
}

void APCLController::RefreshAvailablePointCloudAttributes()
{
	RGBAttributeName.Reset();
	ClassAttributeName.Reset();
	ElevationAttributeName.Reset();
	IntensityAttributeName.Reset();

	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	auto LayerAPI = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject);
	if (!LayerAPI || LayerAPI->GetLoadStatus() != Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return;
	}

	auto Attributes = LayerAPI->GetAttributes();
	if (!Attributes)
	{
		return;
	}

	for (size_t Index = 0; Index < Attributes.GetSize(); ++Index)
	{
		auto Attribute = Attributes.At(Index);
		if (!Attribute)
		{
			continue;
		}

		const FString Name = Attribute.GetName();
		const FString NormalizedName = NormalizeAttributeName(Name);

		if (RGBAttributeName.IsEmpty() && IsRGBAttribute(Attribute, NormalizedName))
		{
			RGBAttributeName = Name;
		}

		if (ClassAttributeName.IsEmpty() && (MatchesAttributeName(NormalizedName, TEXT("CLASSCODE")) ||
											 MatchesAttributeName(NormalizedName, TEXT("CLASSIFICATION")) ||
											 MatchesAttributeName(NormalizedName, TEXT("CLASS"))))
		{
			ClassAttributeName = Name;
		}

		if (ElevationAttributeName.IsEmpty() &&
			(MatchesAttributeName(NormalizedName, TEXT("ELEVATION")) || MatchesAttributeName(NormalizedName, TEXT("HEIGHT")) ||
			 NormalizedName == TEXT("Z")))
		{
			ElevationAttributeName = Name;
		}

		if (IntensityAttributeName.IsEmpty() && MatchesAttributeName(NormalizedName, TEXT("INTENSITY")))
		{
			IntensityAttributeName = Name;
		}
	}
}

void APCLController::UpdateRendererCheckBoxes()
{
	TGuardValue<bool> UpdatingGuard(bUpdatingRendererCheckBoxes, true);

	if (RGBRendererCheckBox)
	{
		RGBRendererCheckBox->SetIsChecked(CurrentRendererChoice == EPCLRendererChoice::RGB);
	}

	if (ClassRendererCheckBox)
	{
		ClassRendererCheckBox->SetIsChecked(CurrentRendererChoice == EPCLRendererChoice::Class);
	}

	if (ElevationRendererCheckBox)
	{
		ElevationRendererCheckBox->SetIsChecked(CurrentRendererChoice == EPCLRendererChoice::Elevation);
	}

	if (IntensityRendererCheckBox)
	{
		IntensityRendererCheckBox->SetIsChecked(CurrentRendererChoice == EPCLRendererChoice::Intensity);
	}
}

void APCLController::UpdateSliderValueTexts() const
{
	if (PointSizeValueText && PointSizeSlider)
	{
		PointSizeValueText->SetText(FormatSliderValue(PointSizeSlider->GetValue()));
	}

	if (PointsPerInchValueText && PointsPerInchSlider)
	{
		PointsPerInchValueText->SetText(FormatSliderValue(PointsPerInchSlider->GetValue()));
	}
}
