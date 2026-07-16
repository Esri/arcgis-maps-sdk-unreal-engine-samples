// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the specific language governing permissions and limitations under the License. */


#include "PCLController.h"

#include "ArcGISMapsSDK/API/GameEngine/ArcGISLoadStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudAttribute.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorModulation.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorStop.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorUniqueValue.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFixedSizeAlgorithm.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRGBRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudStretchRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudUniqueValueRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilterMode.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/Symbology/ArcGISSymbolSizeUnits.h"
#include "ArcGISMapsSDK/API/Standard/ArcGISRGBColor.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "Async/Async.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/ButtonSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Components/SizeBox.h"
#include "Components/Spacer.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Engine/Texture2D.h"
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
constexpr float LegendCompactWidth = 345.0f;
constexpr float LegendCompactHeight = 76.0f;
constexpr float LegendExpandedWidth = 360.0f;
constexpr float LegendExpandedHeight = 320.0f;
constexpr float VisualizeTabHeightOffset = 150.0f;
constexpr float FilterTabHeightOffset = 430.0f;
const FString PointCloudLayerSource = TEXT("https://www.arcgis.com/home/item.html?id=93c83277e8c34ea2ab38f2e1eb1e0d63");
const FName ExpandableTabWidgetNames[] = {
	TEXT("CanvasPanel_37"),
	TEXT("Background"),
	TEXT("Switcher_PCLTabs"),
	TEXT("Panel_Visualize"),
	TEXT("Panel_VisualizeContent"),
	TEXT("Panel_Filter")
};
const Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType FilterReturnValues[] = {
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::FirstOfMany,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::Last,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::LastOfMany,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::Single
};
const TCHAR* FilterReturnLabels[] = {TEXT("First of many"), TEXT("Last"), TEXT("Last of many"), TEXT("Single")};

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

FString GetClassCodeLabel(int32 ClassCode)
{
	FString Label;
	uint8 Red = 0;
	uint8 Green = 0;
	uint8 Blue = 0;
	GetStandardClassInfo(ClassCode, Label, Red, Green, Blue);
	return Label;
}

FSlateColor MakeSlateColor(float Red, float Green, float Blue, float Alpha = 1.0f)
{
	return FSlateColor(FLinearColor(Red, Green, Blue, Alpha));
}

void ConfigureTextBlock(UTextBlock* TextBlock, int32 FontSize, const FSlateColor& Color)
{
	if (!TextBlock)
	{
		return;
	}

	FSlateFontInfo Font = TextBlock->GetFont();
	Font.Size = FontSize;
	TextBlock->SetFont(Font);
	TextBlock->SetColorAndOpacity(Color);
}

UTextBlock* CreateText(UObject* Outer, const FString& Text, int32 FontSize, const FSlateColor& Color)
{
	UTextBlock* TextBlock = NewObject<UTextBlock>(Outer);
	TextBlock->SetText(FText::FromString(Text));
	ConfigureTextBlock(TextBlock, FontSize, Color);
	return TextBlock;
}

UBorder* CreateColorBlock(UObject* Outer, const FLinearColor& Color)
{
	UBorder* ColorBlock = NewObject<UBorder>(Outer);
	ColorBlock->SetBrushColor(Color);
	return ColorBlock;
}

UHorizontalBox* AddLegendRow(UObject* Outer, UPanelWidget* Parent, const FString& Label, const FLinearColor& Color)
{
	UHorizontalBox* Row = NewObject<UHorizontalBox>(Outer);
	Parent->AddChild(Row);
	if (auto* VerticalSlot = Cast<UVerticalBoxSlot>(Row->Slot))
	{
		VerticalSlot->SetPadding(FMargin(0.0f, 1.0f, 0.0f, 1.0f));
	}
	else if (auto* ScrollSlot = Cast<UScrollBoxSlot>(Row->Slot))
	{
		ScrollSlot->SetPadding(FMargin(0.0f, 1.0f, 0.0f, 1.0f));
	}

	USizeBox* SwatchBox = NewObject<USizeBox>(Outer);
	SwatchBox->SetWidthOverride(20.0f);
	SwatchBox->SetHeightOverride(20.0f);
	Row->AddChild(SwatchBox);
	if (auto* SwatchSlot = Cast<UHorizontalBoxSlot>(SwatchBox->Slot))
	{
		SwatchSlot->SetPadding(FMargin(0.0f, 3.0f, 12.0f, 3.0f));
		SwatchSlot->SetVerticalAlignment(VAlign_Center);
		SwatchSlot->SetHorizontalAlignment(HAlign_Center);
	}

	UBorder* Swatch = CreateColorBlock(Outer, Color);
	SwatchBox->AddChild(Swatch);

	UTextBlock* LabelText = CreateText(Outer, Label, 22, MakeSlateColor(1.0f, 1.0f, 1.0f));
	Row->AddChild(LabelText);
	if (auto* LabelSlot = Cast<UHorizontalBoxSlot>(LabelText->Slot))
	{
		LabelSlot->SetPadding(FMargin(0.0f));
		LabelSlot->SetVerticalAlignment(VAlign_Center);
	}

	return Row;
}

void AddGradientStep(UObject* Outer, UVerticalBox* GradientBox, const FLinearColor& Color)
{
	UBorder* Step = CreateColorBlock(Outer, Color);
	GradientBox->AddChild(Step);
	if (auto* StepSlot = Cast<UVerticalBoxSlot>(Step->Slot))
	{
		StepSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
}

FLinearColor EvaluateGradientColor(const TArray<FLinearColor>& Colors, float T)
{
	if (Colors.IsEmpty())
	{
		return FLinearColor::White;
	}

	if (Colors.Num() == 1)
	{
		return Colors[0];
	}

	const float Scaled = FMath::Clamp(T, 0.0f, 1.0f) * static_cast<float>(Colors.Num() - 1);
	const int32 Index = FMath::Min(FMath::FloorToInt(Scaled), Colors.Num() - 2);
	const float LocalT = Scaled - static_cast<float>(Index);
	return FMath::Lerp(Colors[Index], Colors[Index + 1], LocalT);
}

UTexture2D* CreateLegendGradientTexture(UObject* Outer, const TArray<FLinearColor>& TopToBottomColors)
{
	constexpr int32 TextureWidth = 16;
	constexpr int32 TextureHeight = 128;

	UTexture2D* Texture = UTexture2D::CreateTransient(TextureWidth, TextureHeight, PF_B8G8R8A8);
	if (!Texture)
	{
		return nullptr;
	}

	Texture->SRGB = true;
	Texture->CompressionSettings = TC_VectorDisplacementmap;
	Texture->MipGenSettings = TMGS_NoMipmaps;

	FTexture2DMipMap& Mip = Texture->GetPlatformData()->Mips[0];
	void* Data = Mip.BulkData.Lock(LOCK_READ_WRITE);
	FColor* Pixels = static_cast<FColor*>(Data);

	for (int32 Y = 0; Y < TextureHeight; ++Y)
	{
		const float T = static_cast<float>(Y) / static_cast<float>(TextureHeight - 1);
		const FColor Color = EvaluateGradientColor(TopToBottomColors, T).ToFColor(true);

		for (int32 X = 0; X < TextureWidth; ++X)
		{
			Pixels[Y * TextureWidth + X] = Color;
		}
	}

	Mip.BulkData.Unlock();
	Texture->UpdateResource();
	return Texture;
}

void SetVerticalSlotPadding(UWidget* Widget, const FMargin& Padding)
{
	if (auto* VerticalSlot = Cast<UVerticalBoxSlot>(Widget ? Widget->Slot : nullptr))
	{
		VerticalSlot->SetPadding(Padding);
	}
	else if (auto* ScrollSlot = Cast<UScrollBoxSlot>(Widget ? Widget->Slot : nullptr))
	{
		ScrollSlot->SetPadding(Padding);
	}
}

void SetHorizontalSlotPadding(UWidget* Widget, const FMargin& Padding)
{
	if (auto* Slot = Cast<UHorizontalBoxSlot>(Widget ? Widget->Slot : nullptr))
	{
		Slot->SetPadding(Padding);
		Slot->SetVerticalAlignment(VAlign_Center);
	}
}

UCheckBox* AddCheckBoxRow(UObject* Outer, UPanelWidget* Parent, const FString& Label, bool bChecked)
{
	UHorizontalBox* Row = NewObject<UHorizontalBox>(Outer);
	Parent->AddChild(Row);
	SetVerticalSlotPadding(Row, FMargin(0.0f, 1.0f, 0.0f, 1.0f));

	UCheckBox* CheckBox = NewObject<UCheckBox>(Outer);
	CheckBox->SetIsChecked(bChecked);
	Row->AddChild(CheckBox);
	SetHorizontalSlotPadding(CheckBox, FMargin(0.0f, 0.0f, 10.0f, 0.0f));

	UTextBlock* LabelText = CreateText(Outer, Label, 22, MakeSlateColor(1.0f, 1.0f, 1.0f));
	Row->AddChild(LabelText);
	SetHorizontalSlotPadding(LabelText, FMargin(0.0f));

	return CheckBox;
}

UScrollBox* AddFilterScrollSection(UObject* Outer, UVerticalBox* Parent, float Height)
{
	USizeBox* SectionBox = NewObject<USizeBox>(Outer);
	SectionBox->SetHeightOverride(Height);
	Parent->AddChild(SectionBox);
	SetVerticalSlotPadding(SectionBox, FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	UScrollBox* ScrollBox = NewObject<UScrollBox>(Outer);
	ScrollBox->SetOrientation(EOrientation::Orient_Vertical);
	ScrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	ScrollBox->SetAlwaysShowScrollbar(true);
	ScrollBox->SetAlwaysShowScrollbarTrack(true);
	ScrollBox->SetScrollbarThickness(FVector2D(8.0f, 8.0f));
	SectionBox->AddChild(ScrollBox);

	return ScrollBox;
}

template <typename WidgetType>
WidgetType* FindNamedWidget(UUserWidget* Widget, const TCHAR* WidgetName, bool bWarnIfMissing = true)
{
	WidgetType* NamedWidget = Widget ? Cast<WidgetType>(Widget->GetWidgetFromName(WidgetName)) : nullptr;
	if (!NamedWidget && bWarnIfMissing)
	{
		UE_LOG(LogTemp, Warning, TEXT("UI_PCL widget binding failed: %s"), WidgetName);
	}

	return NamedWidget;
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
		UnitDropdown = FindNamedWidget<UComboBoxString>(UIWidget, TEXT("UnitDropDown"), false);
		PointSizeSlider = FindNamedWidget<USlider>(UIWidget, TEXT("Slider_PointsSize"));
		PointsPerInchSlider = FindNamedWidget<USlider>(UIWidget, TEXT("Slider_PointsPerInch"));
		PointSizeValueText = FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_PointSizeValue"));
		PointsPerInchValueText = FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_PointsPerInchValue"));
		ColorModulationCheckBox = FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_ColorModulation"));
		RGBRendererCheckBox = FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_RGB"));
		ClassRendererCheckBox = FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Class"));
		ElevationRendererCheckBox = FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Elevation"));
		IntensityRendererCheckBox = FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Intensity"));
		CustomizeTabButton = FindNamedWidget<UButton>(UIWidget, TEXT("Button_CustomizeTab"));
		FilterTabButton = FindNamedWidget<UButton>(UIWidget, TEXT("Button_FilterTab"));
		VisualizeTabButton = FindNamedWidget<UButton>(UIWidget, TEXT("Button_VisualizeTab"));
		FilterPanel = FindNamedWidget<UPanelWidget>(UIWidget, TEXT("Panel_Filter"));

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

		if (CustomizeTabButton)
		{
			CustomizeTabButton->OnClicked.AddDynamic(this, &APCLController::OnCustomizeTabClicked);
		}

		if (FilterTabButton)
		{
			FilterTabButton->OnClicked.AddDynamic(this, &APCLController::OnFilterTabClicked);
		}

		if (VisualizeTabButton)
		{
			VisualizeTabButton->OnClicked.AddDynamic(this, &APCLController::OnVisualizeTabClicked);
		}

		UpdateSliderValueTexts();
		UpdateRendererCheckBoxes();
		BuildFilterTabUI();
		BuildLegendUI();
		CreatePointCloudLayer();
		ApplyPointCloudVisualization();
		ApplyPointCloudFilters();

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
	RefreshAvailablePointCloudAttributes();
	if (!IsRendererAvailableFromCachedAttributes(RendererChoice))
	{
		RendererChoice = GetFallbackRendererChoice();
	}

	if (CurrentRendererChoice == RendererChoice)
	{
		UpdateRendererCheckBoxes();
		BuildLegendUI();
		return;
	}

	CurrentRendererChoice = RendererChoice;
	UpdateRendererCheckBoxes();
	BuildLegendUI();
	ApplyPointCloudVisualization();
}

bool APCLController::IsPointCloudRendererAvailable(EPCLRendererChoice RendererChoice)
{
	RefreshAvailablePointCloudAttributes();
	return IsRendererAvailableFromCachedAttributes(RendererChoice);
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

void APCLController::OnCustomizeTabClicked()
{
	SetTabLayout(EPCLTabLayout::Default);
}

void APCLController::OnFilterTabClicked()
{
	SetTabLayout(EPCLTabLayout::Filter);
}

void APCLController::OnVisualizeTabClicked()
{
	SetTabLayout(EPCLTabLayout::Visualize);
}

void APCLController::OnFilterCheckStateChanged(bool bIsChecked)
{
	if (bUpdatingFilterCheckBoxes)
	{
		return;
	}

	TGuardValue<bool> UpdatingGuard(bUpdatingFilterCheckBoxes, true);

	if (ClassAllCheckBox)
	{
		bool bAllClassesChecked = true;
		for (TObjectPtr<UCheckBox> CheckBox : ClassFilterCheckBoxes)
		{
			bAllClassesChecked = bAllClassesChecked && CheckBox && CheckBox->IsChecked();
		}
		ClassAllCheckBox->SetIsChecked(bAllClassesChecked);
	}

	if (ReturnsAllCheckBox)
	{
		bool bAllReturnsChecked = true;
		for (TObjectPtr<UCheckBox> CheckBox : ReturnsFilterCheckBoxes)
		{
			bAllReturnsChecked = bAllReturnsChecked && CheckBox && CheckBox->IsChecked();
		}
		ReturnsAllCheckBox->SetIsChecked(bAllReturnsChecked);
	}

	ApplyPointCloudFilters();
}

void APCLController::OnClassAllFilterCheckStateChanged(bool bIsChecked)
{
	if (bUpdatingFilterCheckBoxes)
	{
		return;
	}

	TGuardValue<bool> UpdatingGuard(bUpdatingFilterCheckBoxes, true);

	for (TObjectPtr<UCheckBox> CheckBox : ClassFilterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->SetIsChecked(bIsChecked);
		}
	}

	ApplyPointCloudFilters();
}

void APCLController::OnReturnsAllFilterCheckStateChanged(bool bIsChecked)
{
	if (bUpdatingFilterCheckBoxes)
	{
		return;
	}

	TGuardValue<bool> UpdatingGuard(bUpdatingFilterCheckBoxes, true);

	for (TObjectPtr<UCheckBox> CheckBox : ReturnsFilterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->SetIsChecked(bIsChecked);
		}
	}

	ApplyPointCloudFilters();
}

void APCLController::OnResetFiltersClicked()
{
	ResetFilterSelections(true);
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

	PointCloudLayer = UArcGISPointCloudLayer::CreateArcGISPointCloudLayer(PointCloudLayerSource, MapComponent->GetAPIKey());

	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	PointCloudLayer->SetOpacity(1.0f);
	PointCloudLayer->SetIsVisible(true);

	TWeakObjectPtr<APCLController> WeakThis(this);
	PointCloudLayer->APIObject->SetDoneLoading([WeakThis](auto& LoadError) {
		AsyncTask(ENamedThreads::GameThread, [WeakThis]() {
			if (auto* Controller = WeakThis.Get())
			{
				Controller->RefreshAvailablePointCloudAttributes();
				Controller->UpdateRendererCheckBoxes();
				Controller->ApplyPointCloudVisualization();
				Controller->BuildFilterTabUI();
				Controller->BuildLegendUI();
				Controller->ApplyPointCloudFilters();
			}
		});
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

	EnsureAvailableRendererSelected();

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

void APCLController::ApplyPointCloudFilters()
{
	if (!PointCloudLayer || !PointCloudLayer->APIObject)
	{
		return;
	}

	auto LayerAPI = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject);
	if (!LayerAPI || LayerAPI->GetLoadStatus() != Esri::GameEngine::ArcGISLoadStatus::Loaded)
	{
		return;
	}

	RefreshAvailablePointCloudAttributes();

	ActiveFilterCollection = MakeUnique<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter>>();
	ActiveClassCodeValues.Reset();
	ActiveReturnsValues.Reset();
	ActiveClassCodeFilter.Reset();
	ActiveReturnsFilter.Reset();

	const bool bUseClassFilter = !ClassAttributeName.IsEmpty() && !AreAllClassOptionsSelected() && AreAnyClassOptionsSelected();
	if (bUseClassFilter)
	{
		ActiveClassCodeValues = MakeUnique<Esri::Unreal::ArcGISCollection<double>>();
		for (int32 Index = 0; Index < ClassFilterCheckBoxes.Num() && Index < ClassFilterValues.Num(); ++Index)
		{
			if (ClassFilterCheckBoxes[Index] && ClassFilterCheckBoxes[Index]->IsChecked())
			{
				ActiveClassCodeValues->Add(static_cast<double>(ClassFilterValues[Index]));
			}
		}

		ActiveClassCodeFilter = MakeUnique<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudValueFilter>(
			ClassAttributeName, *ActiveClassCodeValues, Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudValueFilterMode::Include);
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter BaseClassFilter(ActiveClassCodeFilter->GetHandle());
		ActiveFilterCollection->Add(BaseClassFilter);
		BaseClassFilter.SetHandle(nullptr);
	}

	const bool bUseReturnsFilter = !ReturnsAttributeName.IsEmpty() && !AreAllReturnsOptionsSelected() && AreAnyReturnsOptionsSelected();
	if (bUseReturnsFilter)
	{
		ActiveReturnsValues = MakeUnique<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType>>();
		for (int32 Index = 0; Index < ReturnsFilterCheckBoxes.Num() && Index < UE_ARRAY_COUNT(FilterReturnValues); ++Index)
		{
			if (ReturnsFilterCheckBoxes[Index] && ReturnsFilterCheckBoxes[Index]->IsChecked())
			{
				ActiveReturnsValues->Add(FilterReturnValues[Index]);
			}
		}

		ActiveReturnsFilter =
			MakeUnique<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnFilter>(ReturnsAttributeName, *ActiveReturnsValues);
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter BaseReturnsFilter(ActiveReturnsFilter->GetHandle());
		ActiveFilterCollection->Add(BaseReturnsFilter);
		BaseReturnsFilter.SetHandle(nullptr);
	}

	LayerAPI->SetFilters(*ActiveFilterCollection);
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

		if (ReturnsAttributeName.IsEmpty() && MatchesAttributeName(NormalizedName, TEXT("RETURNS")))
		{
			ReturnsAttributeName = Name;
		}
	}
}

bool APCLController::IsRendererAvailableFromCachedAttributes(EPCLRendererChoice RendererChoice) const
{
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

void APCLController::UpdateRendererCheckBoxes()
{
	TGuardValue<bool> UpdatingGuard(bUpdatingRendererCheckBoxes, true);
	const bool bHasLoadedRendererAttributes =
		!RGBAttributeName.IsEmpty() || !ClassAttributeName.IsEmpty() || !ElevationAttributeName.IsEmpty() || !IntensityAttributeName.IsEmpty();

	if (bHasLoadedRendererAttributes)
	{
		EnsureAvailableRendererSelected();
		SetRendererOptionVisibility(EPCLRendererChoice::RGB, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::RGB));
		SetRendererOptionVisibility(EPCLRendererChoice::Class, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Class));
		SetRendererOptionVisibility(EPCLRendererChoice::Elevation, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Elevation));
		SetRendererOptionVisibility(EPCLRendererChoice::Intensity, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Intensity));
	}

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

void APCLController::SetRendererOptionVisibility(EPCLRendererChoice RendererChoice, bool bVisible)
{
	const ESlateVisibility Visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	const TCHAR* RowName = nullptr;
	const TCHAR* CheckBoxName = nullptr;
	const TCHAR* TextName = nullptr;

	switch (RendererChoice)
	{
	case EPCLRendererChoice::RGB:
		RowName = TEXT("Row_Checkbox_Renderer_RGB");
		CheckBoxName = TEXT("Checkbox_Renderer_RGB");
		TextName = TEXT("Text_Renderer_RGB");
		break;
	case EPCLRendererChoice::Class:
		RowName = TEXT("Row_Checkbox_Renderer_Class");
		CheckBoxName = TEXT("Checkbox_Renderer_Class");
		TextName = TEXT("Text_Renderer_Class");
		break;
	case EPCLRendererChoice::Elevation:
		RowName = TEXT("Row_Checkbox_Renderer_Elevation");
		CheckBoxName = TEXT("Checkbox_Renderer_Elevation");
		TextName = TEXT("Text_Renderer_Elevation");
		break;
	case EPCLRendererChoice::Intensity:
		RowName = TEXT("Row_Checkbox_Renderer_Intensity");
		CheckBoxName = TEXT("Checkbox_Renderer_Intensity");
		TextName = TEXT("Text_Renderer_Intensity");
		break;
	default:
		return;
	}

	if (UWidget* Row = UIWidget ? UIWidget->GetWidgetFromName(RowName) : nullptr)
	{
		Row->SetVisibility(Visibility);
		return;
	}

	if (UWidget* CheckBox = UIWidget ? UIWidget->GetWidgetFromName(CheckBoxName) : nullptr)
	{
		CheckBox->SetVisibility(Visibility);
		CheckBox->SetIsEnabled(bVisible);
	}
	if (UWidget* Text = UIWidget ? UIWidget->GetWidgetFromName(TextName) : nullptr)
	{
		Text->SetVisibility(Visibility);
		Text->SetIsEnabled(bVisible);
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

void APCLController::BuildFilterTabUI()
{
	if (!FilterPanel || !UIWidget)
	{
		return;
	}

	FilterPanel->ClearChildren();
	ClassFilterCheckBoxes.Reset();
	ClassFilterValues.Reset();
	ReturnsFilterCheckBoxes.Reset();
	ClassAllCheckBox = nullptr;
	ReturnsAllCheckBox = nullptr;
	ResetFiltersButton = nullptr;

	RefreshAvailablePointCloudAttributes();
	const bool bHasClassCodeFilter = !ClassAttributeName.IsEmpty();
	const bool bHasReturnsFilter = !ReturnsAttributeName.IsEmpty();

	if (!bHasClassCodeFilter && !bHasReturnsFilter)
	{
		ClearActiveFilters();
		return;
	}

	UVerticalBox* Content = NewObject<UVerticalBox>(UIWidget);
	FilterPanel->AddChild(Content);

	if (auto* CanvasSlot = Cast<UCanvasPanelSlot>(Content->Slot))
	{
		CanvasSlot->SetPosition(FVector2D(0.0f, 0.0f));
		CanvasSlot->SetSize(FVector2D(430.0f, 660.0f));
	}

	if (bHasClassCodeFilter)
	{
		UTextBlock* ClassHeading = CreateText(UIWidget, TEXT("Class Code"), 24, MakeSlateColor(0.78f, 0.78f, 0.82f));
		Content->AddChild(ClassHeading);
		SetVerticalSlotPadding(ClassHeading, FMargin(0.0f, 0.0f, 0.0f, 10.0f));

		UScrollBox* ClassScrollBox = AddFilterScrollSection(UIWidget, Content, 206.0f);

		ClassAllCheckBox = AddCheckBoxRow(UIWidget, ClassScrollBox, TEXT("<all>"), true);
		ClassAllCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnClassAllFilterCheckStateChanged);

		for (int32 ClassCode = 0; ClassCode <= 18; ++ClassCode)
		{
			UCheckBox* CheckBox = AddCheckBoxRow(UIWidget, ClassScrollBox, GetClassCodeLabel(ClassCode), true);
			CheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnFilterCheckStateChanged);
			ClassFilterCheckBoxes.Add(CheckBox);
			ClassFilterValues.Add(ClassCode);
		}
	}

	if (bHasClassCodeFilter && bHasReturnsFilter)
	{
		USpacer* SectionSpacer = NewObject<USpacer>(UIWidget);
		SectionSpacer->SetSize(FVector2D(1.0f, 4.0f));
		Content->AddChild(SectionSpacer);
	}

	if (bHasReturnsFilter)
	{
		UTextBlock* ReturnsHeading = CreateText(UIWidget, TEXT("Returns"), 24, MakeSlateColor(0.78f, 0.78f, 0.82f));
		Content->AddChild(ReturnsHeading);
		SetVerticalSlotPadding(ReturnsHeading, FMargin(0.0f, 0.0f, 0.0f, 10.0f));

		UScrollBox* ReturnsScrollBox = AddFilterScrollSection(UIWidget, Content, 148.0f);

		ReturnsAllCheckBox = AddCheckBoxRow(UIWidget, ReturnsScrollBox, TEXT("<all>"), true);
		ReturnsAllCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnReturnsAllFilterCheckStateChanged);

		for (int32 Index = 0; Index < UE_ARRAY_COUNT(FilterReturnLabels); ++Index)
		{
			UCheckBox* CheckBox = AddCheckBoxRow(UIWidget, ReturnsScrollBox, FilterReturnLabels[Index], true);
			CheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnFilterCheckStateChanged);
			ReturnsFilterCheckBoxes.Add(CheckBox);
		}
	}

	USpacer* InfoSpacer = NewObject<USpacer>(UIWidget);
	InfoSpacer->SetSize(FVector2D(1.0f, 10.0f));
	Content->AddChild(InfoSpacer);

	UBorder* InfoBorder = NewObject<UBorder>(UIWidget);
	InfoBorder->SetBrushColor(FLinearColor(0.05f, 0.05f, 0.06f, 0.58f));
	Content->AddChild(InfoBorder);
	SetVerticalSlotPadding(InfoBorder, FMargin(0.0f, 0.0f, 0.0f, 22.0f));

	UHorizontalBox* InfoRow = NewObject<UHorizontalBox>(UIWidget);
	InfoBorder->SetContent(InfoRow);

	UTextBlock* InfoIcon = CreateText(UIWidget, TEXT("i"), 24, MakeSlateColor(0.67f, 0.27f, 1.0f));
	InfoRow->AddChild(InfoIcon);
	SetHorizontalSlotPadding(InfoIcon, FMargin(16.0f, 8.0f, 14.0f, 8.0f));

	UTextBlock* InfoText = CreateText(UIWidget, TEXT("Filtering allows you to include or exclude points\nbased on classification codes, returns and etc."),
									  14, MakeSlateColor(0.78f, 0.78f, 0.82f));
	InfoRow->AddChild(InfoText);
	SetHorizontalSlotPadding(InfoText, FMargin(0.0f, 8.0f, 16.0f, 8.0f));

	ResetFiltersButton = NewObject<UButton>(UIWidget);
	Content->AddChild(ResetFiltersButton);

	UTextBlock* ResetText = CreateText(UIWidget, TEXT("Reset Filters"), 17, MakeSlateColor(1.0f, 1.0f, 1.0f));
	ResetFiltersButton->AddChild(ResetText);
	if (auto* ButtonSlot = Cast<UButtonSlot>(ResetText->Slot))
	{
		ButtonSlot->SetPadding(FMargin(16.0f, 6.0f, 16.0f, 6.0f));
	}

	ResetFiltersButton->OnClicked.AddDynamic(this, &APCLController::OnResetFiltersClicked);
	ResetFilterSelections(false);
}

void APCLController::BuildLegendUI()
{
	if (!UIWidget || !UIWidget->WidgetTree)
	{
		return;
	}

	if (LegendPanel)
	{
		LegendPanel->RemoveFromParent();
		LegendPanel = nullptr;
	}
	LegendTextures.Reset();

	if (CurrentTabLayout != EPCLTabLayout::Visualize)
	{
		return;
	}

	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(UIWidget->WidgetTree->RootWidget);
	if (!RootCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("UI_PCL legend binding failed: root widget is not a canvas panel."));
		return;
	}

	const bool bCompact = CurrentRendererChoice == EPCLRendererChoice::RGB;
	const FVector2D LegendSize = bCompact ? FVector2D(LegendCompactWidth, LegendCompactHeight) : FVector2D(LegendExpandedWidth, LegendExpandedHeight);

	LegendPanel = NewObject<UCanvasPanel>(UIWidget);
	UCanvasPanelSlot* LegendSlot = RootCanvas->AddChildToCanvas(LegendPanel);
	if (LegendSlot)
	{
		LegendSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
		LegendSlot->SetAlignment(FVector2D(1.0f, 1.0f));
		LegendSlot->SetPosition(FVector2D(-64.0f, -82.0f));
		LegendSlot->SetSize(LegendSize);
		LegendSlot->SetZOrder(30);
	}

	UBorder* Background = CreateColorBlock(UIWidget, FLinearColor(0.05f, 0.05f, 0.06f, 0.78f));
	UCanvasPanelSlot* BackgroundSlot = LegendPanel->AddChildToCanvas(Background);
	if (BackgroundSlot)
	{
		BackgroundSlot->SetPosition(FVector2D::ZeroVector);
		BackgroundSlot->SetSize(LegendSize);
	}

	UBorder* Accent = CreateColorBlock(UIWidget, FLinearColor(0.58f, 0.23f, 1.0f, 1.0f));
	UCanvasPanelSlot* AccentSlot = LegendPanel->AddChildToCanvas(Accent);
	if (AccentSlot)
	{
		AccentSlot->SetPosition(FVector2D::ZeroVector);
		AccentSlot->SetSize(FVector2D(8.0f, LegendSize.Y));
	}

	UVerticalBox* Content = NewObject<UVerticalBox>(UIWidget);
	UCanvasPanelSlot* ContentSlot = LegendPanel->AddChildToCanvas(Content);
	if (ContentSlot)
	{
		ContentSlot->SetPosition(bCompact ? FVector2D(34.0f, 20.0f) : FVector2D(56.0f, 28.0f));
		ContentSlot->SetSize(bCompact ? FVector2D(290.0f, 44.0f) : FVector2D(270.0f, 260.0f));
	}

	if (CurrentRendererChoice == EPCLRendererChoice::RGB)
	{
		Content->AddChild(CreateText(UIWidget, TEXT("No legend"), 26, MakeSlateColor(0.78f, 0.78f, 0.82f)));
		return;
	}

	FString LegendTitle = PointCloudLayer ? PointCloudLayer->GetName() : FString();
	if (LegendTitle.TrimStartAndEnd().IsEmpty())
	{
		LegendTitle = TEXT("Point cloud layer");
	}

	UTextBlock* Title = CreateText(UIWidget, LegendTitle, 25, MakeSlateColor(0.78f, 0.78f, 0.82f));
	Content->AddChild(Title);
	SetVerticalSlotPadding(Title, FMargin(0.0f, 0.0f, 0.0f, 34.0f));

	if (CurrentRendererChoice == EPCLRendererChoice::Class)
	{
		UTextBlock* Heading = CreateText(UIWidget, TEXT("Class Code"), 22, MakeSlateColor(1.0f, 1.0f, 1.0f));
		Content->AddChild(Heading);
		SetVerticalSlotPadding(Heading, FMargin(0.0f, 0.0f, 0.0f, 10.0f));

		UScrollBox* ClassList = NewObject<UScrollBox>(UIWidget);
		ClassList->SetOrientation(EOrientation::Orient_Vertical);
		ClassList->SetScrollBarVisibility(ESlateVisibility::Visible);
		ClassList->SetAlwaysShowScrollbar(true);
		ClassList->SetScrollbarThickness(FVector2D(8.0f, 8.0f));
		Content->AddChild(ClassList);

		const int32 VisibleClassCodes[] = {1, 2, 3, 5, 6, 7, 9};
		for (int32 ClassCode : VisibleClassCodes)
		{
			FString Label;
			uint8 Red = 0;
			uint8 Green = 0;
			uint8 Blue = 0;
			GetStandardClassInfo(ClassCode, Label, Red, Green, Blue);
			AddLegendRow(UIWidget, ClassList, Label, FLinearColor(Red / 255.0f, Green / 255.0f, Blue / 255.0f, 1.0f));
		}

		return;
	}

	const bool bElevationLegend = CurrentRendererChoice == EPCLRendererChoice::Elevation;
	UTextBlock* Heading = CreateText(UIWidget, bElevationLegend ? TEXT("Elevation") : TEXT("Intensity"), 22, MakeSlateColor(1.0f, 1.0f, 1.0f));
	Content->AddChild(Heading);
	SetVerticalSlotPadding(Heading, FMargin(0.0f, 0.0f, 0.0f, 26.0f));

	UHorizontalBox* GradientRow = NewObject<UHorizontalBox>(UIWidget);
	Content->AddChild(GradientRow);

	USizeBox* GradientSizeBox = NewObject<USizeBox>(UIWidget);
	GradientSizeBox->SetWidthOverride(36.0f);
	GradientSizeBox->SetHeightOverride(122.0f);
	GradientRow->AddChild(GradientSizeBox);
	SetHorizontalSlotPadding(GradientSizeBox, FMargin(6.0f, 0.0f, 18.0f, 0.0f));

	TArray<FLinearColor> GradientColors;
	if (bElevationLegend)
	{
		GradientColors = {
			FLinearColor(0.95f, 0.12f, 0.08f),
			FLinearColor(1.0f, 0.9f, 0.2f),
			FLinearColor(0.35f, 0.95f, 0.48f),
			FLinearColor(0.25f, 0.82f, 1.0f),
			FLinearColor(0.22f, 0.12f, 1.0f)
		};
	}
	else
	{
		GradientColors = {
			FLinearColor::White,
			FLinearColor(0.65f, 0.65f, 0.65f),
			FLinearColor(0.16f, 0.16f, 0.16f),
			FLinearColor::Black
		};
	}

	if (UTexture2D* GradientTexture = CreateLegendGradientTexture(UIWidget, GradientColors))
	{
		LegendTextures.Add(GradientTexture);

		UImage* GradientImage = NewObject<UImage>(UIWidget);
		GradientImage->SetBrushFromTexture(GradientTexture, true);
		GradientSizeBox->AddChild(GradientImage);
	}

	UVerticalBox* LabelColumn = NewObject<UVerticalBox>(UIWidget);
	GradientRow->AddChild(LabelColumn);
	SetHorizontalSlotPadding(LabelColumn, FMargin(0.0f));

	LabelColumn->AddChild(CreateText(UIWidget, bElevationLegend ? TEXT("> 3.5") : TEXT("> 65,680"), 20, MakeSlateColor(1.0f, 1.0f, 1.0f)));

	USpacer* TopSpacer = NewObject<USpacer>(UIWidget);
	TopSpacer->SetSize(FVector2D(1.0f, 31.0f));
	LabelColumn->AddChild(TopSpacer);

	LabelColumn->AddChild(CreateText(UIWidget, bElevationLegend ? TEXT("1.5") : TEXT("38,032"), 20, MakeSlateColor(1.0f, 1.0f, 1.0f)));

	USpacer* BottomSpacer = NewObject<USpacer>(UIWidget);
	BottomSpacer->SetSize(FVector2D(1.0f, 31.0f));
	LabelColumn->AddChild(BottomSpacer);

	LabelColumn->AddChild(CreateText(UIWidget, bElevationLegend ? TEXT("< -1.5") : TEXT("< 10,385"), 20, MakeSlateColor(1.0f, 1.0f, 1.0f)));
}

void APCLController::ResetFilterSelections(bool bApplyFilters)
{
	TGuardValue<bool> UpdatingGuard(bUpdatingFilterCheckBoxes, true);

	if (ClassAllCheckBox)
	{
		ClassAllCheckBox->SetIsChecked(true);
	}

	for (TObjectPtr<UCheckBox> CheckBox : ClassFilterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->SetIsChecked(true);
		}
	}

	if (ReturnsAllCheckBox)
	{
		ReturnsAllCheckBox->SetIsChecked(true);
	}

	for (TObjectPtr<UCheckBox> CheckBox : ReturnsFilterCheckBoxes)
	{
		if (CheckBox)
		{
			CheckBox->SetIsChecked(true);
		}
	}

	if (bApplyFilters)
	{
		ApplyPointCloudFilters();
	}
}

bool APCLController::AreAllClassOptionsSelected() const
{
	if (ClassFilterCheckBoxes.IsEmpty())
	{
		return false;
	}

	for (const TObjectPtr<UCheckBox>& CheckBox : ClassFilterCheckBoxes)
	{
		if (!CheckBox || !CheckBox->IsChecked())
		{
			return false;
		}
	}

	return true;
}

bool APCLController::AreAnyClassOptionsSelected() const
{
	for (const TObjectPtr<UCheckBox>& CheckBox : ClassFilterCheckBoxes)
	{
		if (CheckBox && CheckBox->IsChecked())
		{
			return true;
		}
	}

	return false;
}

bool APCLController::AreAllReturnsOptionsSelected() const
{
	if (ReturnsFilterCheckBoxes.IsEmpty())
	{
		return false;
	}

	for (const TObjectPtr<UCheckBox>& CheckBox : ReturnsFilterCheckBoxes)
	{
		if (!CheckBox || !CheckBox->IsChecked())
		{
			return false;
		}
	}

	return true;
}

bool APCLController::AreAnyReturnsOptionsSelected() const
{
	for (const TObjectPtr<UCheckBox>& CheckBox : ReturnsFilterCheckBoxes)
	{
		if (CheckBox && CheckBox->IsChecked())
		{
			return true;
		}
	}

	return false;
}

void APCLController::ClearActiveFilters()
{
	if (PointCloudLayer && PointCloudLayer->APIObject)
	{
		if (auto LayerAPI = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGISPointCloudLayer>(PointCloudLayer->APIObject))
		{
			ActiveFilterCollection = MakeUnique<Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFilter>>();
			LayerAPI->SetFilters(*ActiveFilterCollection);
		}
	}

	ActiveClassCodeValues.Reset();
	ActiveReturnsValues.Reset();
	ActiveClassCodeFilter.Reset();
	ActiveReturnsFilter.Reset();
}

void APCLController::SetTabLayout(EPCLTabLayout Layout)
{
	CurrentTabLayout = Layout;

	float HeightOffset = 0.0f;
	if (Layout == EPCLTabLayout::Visualize)
	{
		HeightOffset = VisualizeTabHeightOffset;
	}
	else if (Layout == EPCLTabLayout::Filter)
	{
		HeightOffset = FilterTabHeightOffset;
	}

	for (const FName& WidgetName : ExpandableTabWidgetNames)
	{
		SetNamedWidgetHeightOffset(WidgetName, HeightOffset);
	}

	BuildLegendUI();
}

void APCLController::SetNamedWidgetHeightOffset(const FName& WidgetName, float HeightOffset)
{
	if (!UIWidget)
	{
		return;
	}

	UWidget* Widget = UIWidget->GetWidgetFromName(WidgetName);
	if (!Widget)
	{
		return;
	}

	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot);
	if (!CanvasSlot)
	{
		return;
	}

	if (!CachedTabWidgetSizes.Contains(WidgetName))
	{
		CachedTabWidgetSizes.Add(WidgetName, CanvasSlot->GetSize());
	}

	const FVector2D OriginalSize = CachedTabWidgetSizes[WidgetName];
	CanvasSlot->SetSize(FVector2D(OriginalSize.X, OriginalSize.Y + HeightOffset));
}
