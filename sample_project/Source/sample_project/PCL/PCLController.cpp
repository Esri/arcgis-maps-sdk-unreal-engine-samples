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
#include "Blueprint/WidgetTree.h"
#include "Brushes/SlateColorBrush.h"
#include "Brushes/SlateRoundedBoxBrush.h"
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
#include "Framework/Application/SlateApplication.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputCoreTypes.h"
#include "Slate/WidgetTransform.h"
#include "UObject/UnrealType.h"

#include "ArcGISMapsSDK/API/GameEngine/ArcGISLoadStatus.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGISPointCloudLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudAttribute.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorModulation.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorStop.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudColorUniqueValue.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudFixedSizeAlgorithm.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRGBRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudReturnType.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudStretchRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudUniqueValueRenderer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilter.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/PointCloud/ArcGISPointCloudValueFilterMode.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/Symbology/ArcGISSymbolSizeUnits.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISCollection.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISException.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Extent/ArcGISExtentRectangle.h"
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
constexpr float LegendCompactWidth = 345.0f;
constexpr float LegendCompactHeight = 76.0f;
constexpr float LegendExpandedWidth = 480.0f;
constexpr float LegendExpandedHeight = 382.0f;
constexpr float LegendExpandedContentWidth = LegendExpandedWidth - 82.0f;
constexpr float PCLTabUIScale = 2.0f / 3.0f;
constexpr float CustomizeTabHeightOffset = 88.0f;
constexpr float VisualizeTabHeightOffset = 194.0f;
constexpr float FilterTabHeightOffset = 430.0f;
constexpr float PointCloudLayerLoadRetryInterval = 0.25f;
constexpr int32 MaxPointCloudLayerLoadRetries = 40;
const FString PointCloudLayerSource =
	TEXT("https://tiles.arcgis.com/tiles/V6ZHFr6zdgNZuVG0/arcgis/rest/services/BARNEGAT_BAY_LiDAR_UTM/SceneServer");
const FName ExpandableTabWidgetNames[] = {TEXT("CanvasPanel_37"),  TEXT("Background"),		TEXT("Switcher_PCLTabs"),
										  TEXT("Panel_Customize"), TEXT("Panel_Visualize"), TEXT("Panel_VisualizeContent"),
										  TEXT("Panel_Filter")};
const Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType FilterReturnValues[] = {
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::FirstOfMany,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::Last,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::LastOfMany,
	Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudReturnType::Single};
const TCHAR* FilterReturnLabels[] = {TEXT("First of many"), TEXT("Last"), TEXT("Last of many"), TEXT("Single")};
const FName PCLRootCanvasWidgetName(TEXT("CanvasPanel_37"));
const FName PCLMainPanelWidgetName(TEXT("Panel_PCLMain"));
const FName PCLCollapseButtonWidgetName(TEXT("Button_Collapse"));
const FName PCLCollapseIconWidgetName(TEXT("Collapse"));
const FName PCLGearIconWidgetName(TEXT("Button_Gear"));
const FName PCLGearRuntimeIconWidgetName(TEXT("PCL_GearIcon_Runtime"));
const FName PCLInfoWidgetName(TEXT("wbp_Info"));
const FName PCLInfoButtonWidgetName(TEXT("wbp_InfoButton"));
const FName PCLInfoButtonControlName(TEXT("Button_21"));
const FName PCLMenuHiddenPropertyName(TEXT("IsMenuHidden"));
const FVector2D PCLGearButtonSize(48.0f, 48.0f);
const FVector2D PCLGearIconSize(34.0f, 34.0f);
const FLinearColor PCLGearPurple(0.309f, 0.063f, 1.0f, 1.0f);

FText FormatSliderValue(float value)
{
	return FText::FromString(FString::Printf(TEXT("%.0f"), value));
}

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

FString GetClassCodeLabel(int32 classCode)
{
	return GetStandardClassInfo(classCode).Label;
}

FSlateColor MakeSlateColor(float red, float green, float blue, float alpha = 1.0f)
{
	return FSlateColor(FLinearColor(red, green, blue, alpha));
}

void ConfigureTextBlock(UTextBlock* textBlock, int32 fontSize, const FSlateColor& color)
{

	if (!textBlock)
	{
		return;
	}

	FSlateFontInfo font = textBlock->GetFont();

	if (UObject* fontObject =
			LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-Regular_Font.ChakraPetch-Regular_Font")))
	{
		font.FontObject = fontObject;
	}
	font.Size = fontSize;
	textBlock->SetFont(font);
	textBlock->SetColorAndOpacity(color);
}

UTextBlock* CreateText(UObject* outer, const FString& text, int32 fontSize, const FSlateColor& color)
{
	UTextBlock* textBlock = NewObject<UTextBlock>(outer);
	textBlock->SetText(FText::FromString(text));
	ConfigureTextBlock(textBlock, fontSize, color);
	return textBlock;
}

UBorder* CreateColorBlock(UObject* outer, const FLinearColor& color)
{
	UBorder* colorBlock = NewObject<UBorder>(outer);
	colorBlock->SetBrushColor(color);
	return colorBlock;
}

UTexture2D* CreateLegendCircleTexture(UObject* outer, const FLinearColor& color)
{
	constexpr int32 textureSize = 32;
	constexpr float center = (textureSize - 1) * 0.5f;
	constexpr float radius = 10.5f;

	UTexture2D* texture = UTexture2D::CreateTransient(textureSize, textureSize, PF_B8G8R8A8);

	if (!texture)
	{
		return nullptr;
	}

	texture->SRGB = true;
	texture->CompressionSettings = TC_VectorDisplacementmap;
	texture->MipGenSettings = TMGS_NoMipmaps;

	FTexture2DMipMap& mip = texture->GetPlatformData()->Mips[0];
	void* data = mip.BulkData.Lock(LOCK_READ_WRITE);
	FColor* pixels = static_cast<FColor*>(data);
	const FColor fillColor = color.ToFColor(true);

	for (int32 y = 0; y < textureSize; ++y)
	{

		for (int32 x = 0; x < textureSize; ++x)
		{
			const float dx = x - center;
			const float dy = y - center;
			pixels[y * textureSize + x] = (dx * dx + dy * dy) <= radius * radius ? fillColor : FColor(0, 0, 0, 0);
		}
	}

	mip.BulkData.Unlock();
	texture->UpdateResource();
	return texture;
}

void ApplyLegendTitleFont(UTextBlock* title)
{

	if (!title)
	{
		return;
	}

	UObject* fontObject =
		LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-SemiBold_Font.ChakraPetch-SemiBold_Font"));

	if (!fontObject)
	{
		return;
	}

	FSlateFontInfo font = title->GetFont();
	font.FontObject = fontObject;
	font.Size = 24;
	title->SetFont(font);
}

void ApplyChakraPetchSemiBoldFont(UTextBlock* textBlock, int32 fontSize)
{

	if (!textBlock)
	{
		return;
	}

	UObject* fontObject =
		LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-SemiBold_Font.ChakraPetch-SemiBold_Font"));

	if (!fontObject)
	{
		return;
	}

	FSlateFontInfo font = textBlock->GetFont();
	font.FontObject = fontObject;
	font.Size = fontSize;
	textBlock->SetFont(font);
}

void ApplyChakraPetchRegularFont(UTextBlock* textBlock, int32 fontSize)
{

	if (!textBlock)
	{
		return;
	}

	UObject* fontObject =
		LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-Regular_Font.ChakraPetch-Regular_Font"));

	if (!fontObject)
	{
		return;
	}

	FSlateFontInfo font = textBlock->GetFont();
	font.FontObject = fontObject;
	font.Size = fontSize;
	textBlock->SetFont(font);
}

UHorizontalBox* AddLegendRow(UObject* outer,
							 UPanelWidget* parent,
							 const FString& label,
							 const FLinearColor& color,
							 UTexture2D* circleTexture = nullptr)
{
	UHorizontalBox* row = NewObject<UHorizontalBox>(outer);
	parent->AddChild(row);

	if (auto* verticalSlot = Cast<UVerticalBoxSlot>(row->Slot))
	{
		verticalSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f));
	}
	else if (auto* scrollSlot = Cast<UScrollBoxSlot>(row->Slot))
	{
		scrollSlot->SetPadding(FMargin(0.0f, 0.0f, 0.0f, 1.0f));
	}

	USizeBox* swatchBox = NewObject<USizeBox>(outer);
	swatchBox->SetWidthOverride(26.0f);
	swatchBox->SetHeightOverride(24.0f);
	row->AddChild(swatchBox);

	if (auto* swatchSlot = Cast<UHorizontalBoxSlot>(swatchBox->Slot))
	{
		swatchSlot->SetPadding(FMargin(0.0f, 0.0f, 8.0f, 0.0f));
		swatchSlot->SetVerticalAlignment(VAlign_Center);
		swatchSlot->SetHorizontalAlignment(HAlign_Center);
	}

	if (circleTexture)
	{
		UImage* swatch = NewObject<UImage>(outer);
		swatch->SetBrushFromTexture(circleTexture, true);
		swatchBox->AddChild(swatch);
	}
	else
	{
		UBorder* swatch = CreateColorBlock(outer, color);
		swatchBox->AddChild(swatch);
	}

	UTextBlock* labelText = CreateText(outer, label, 18, MakeSlateColor(1.0f, 1.0f, 1.0f));
	ApplyChakraPetchSemiBoldFont(labelText, 18);
	row->AddChild(labelText);

	if (auto* labelSlot = Cast<UHorizontalBoxSlot>(labelText->Slot))
	{
		labelSlot->SetPadding(FMargin(0.0f));
		labelSlot->SetVerticalAlignment(VAlign_Center);
	}

	return row;
}

void AddGradientStep(UObject* outer, UVerticalBox* gradientBox, const FLinearColor& color)
{
	UBorder* step = CreateColorBlock(outer, color);
	gradientBox->AddChild(step);

	if (auto* stepSlot = Cast<UVerticalBoxSlot>(step->Slot))
	{
		stepSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
	}
}

FLinearColor EvaluateGradientColor(const TArray<FLinearColor>& colors, float t)
{

	if (colors.IsEmpty())
	{
		return FLinearColor::White;
	}

	if (colors.Num() == 1)
	{
		return colors[0];
	}

	const float scaled = FMath::Clamp(t, 0.0f, 1.0f) * static_cast<float>(colors.Num() - 1);
	const int32 index = FMath::Min(FMath::FloorToInt(scaled), colors.Num() - 2);
	const float localT = scaled - static_cast<float>(index);
	return FMath::Lerp(colors[index], colors[index + 1], localT);
}

UTexture2D* CreateLegendGradientTexture(UObject* outer, const TArray<FLinearColor>& topToBottomColors)
{
	constexpr int32 textureWidth = 16;
	constexpr int32 textureHeight = 128;

	UTexture2D* texture = UTexture2D::CreateTransient(textureWidth, textureHeight, PF_B8G8R8A8);

	if (!texture)
	{
		return nullptr;
	}

	texture->SRGB = true;
	texture->CompressionSettings = TC_VectorDisplacementmap;
	texture->MipGenSettings = TMGS_NoMipmaps;

	FTexture2DMipMap& mip = texture->GetPlatformData()->Mips[0];
	void* data = mip.BulkData.Lock(LOCK_READ_WRITE);
	FColor* pixels = static_cast<FColor*>(data);

	for (int32 y = 0; y < textureHeight; ++y)
	{
		const float t = static_cast<float>(y) / static_cast<float>(textureHeight - 1);
		const FColor color = EvaluateGradientColor(topToBottomColors, t).ToFColor(true);

		for (int32 x = 0; x < textureWidth; ++x)
		{
			pixels[y * textureWidth + x] = color;
		}
	}

	mip.BulkData.Unlock();
	texture->UpdateResource();
	return texture;
}

void SetVerticalSlotPadding(UWidget* widget, const FMargin& padding)
{

	if (auto* verticalSlot = Cast<UVerticalBoxSlot>(widget ? widget->Slot : nullptr))
	{
		verticalSlot->SetPadding(padding);
	}
	else if (auto* scrollSlot = Cast<UScrollBoxSlot>(widget ? widget->Slot : nullptr))
	{
		scrollSlot->SetPadding(padding);
	}
}

void SetHorizontalSlotPadding(UWidget* widget, const FMargin& padding)
{

	if (auto* slot = Cast<UHorizontalBoxSlot>(widget ? widget->Slot : nullptr))
	{
		slot->SetPadding(padding);
		slot->SetVerticalAlignment(VAlign_Center);
	}
}

UCheckBox* AddCheckBoxRow(UObject* outer, UPanelWidget* parent, const FString& label, bool bChecked)
{
	UHorizontalBox* row = NewObject<UHorizontalBox>(outer);
	parent->AddChild(row);
	SetVerticalSlotPadding(row, FMargin(0.0f, 1.0f, 0.0f, 1.0f));

	UCheckBox* checkBox = NewObject<UCheckBox>(outer);
	FCheckBoxStyle checkBoxStyle = checkBox->GetWidgetStyle();
	FSlateColorBrush uncheckedBrush(FLinearColor(0.82f, 0.82f, 0.84f, 1.0f));
	FSlateColorBrush uncheckedHoveredBrush(FLinearColor(0.92f, 0.92f, 0.94f, 1.0f));
	FSlateRoundedBoxBrush checkedBrush(FLinearColor(0.61f, 0.24f, 1.0f, 1.0f), 0.0f, FLinearColor::White, 2.0f, FVector2D(22.0f, 22.0f));
	FSlateRoundedBoxBrush checkedHoveredBrush(FLinearColor(0.69f, 0.36f, 1.0f, 1.0f), 0.0f, FLinearColor::White, 2.0f, FVector2D(22.0f, 22.0f));
	uncheckedBrush.ImageSize = FVector2D(22.0f, 22.0f);
	uncheckedHoveredBrush.ImageSize = FVector2D(22.0f, 22.0f);
	checkedBrush.ImageSize = FVector2D(22.0f, 22.0f);
	checkedHoveredBrush.ImageSize = FVector2D(22.0f, 22.0f);
	checkBoxStyle.SetUncheckedImage(uncheckedBrush);
	checkBoxStyle.SetUncheckedHoveredImage(uncheckedHoveredBrush);
	checkBoxStyle.SetUncheckedPressedImage(uncheckedHoveredBrush);
	checkBoxStyle.SetCheckedImage(checkedBrush);
	checkBoxStyle.SetCheckedHoveredImage(checkedHoveredBrush);
	checkBoxStyle.SetCheckedPressedImage(checkedHoveredBrush);
	checkBoxStyle.SetUndeterminedImage(checkedBrush);
	checkBoxStyle.SetUndeterminedHoveredImage(checkedHoveredBrush);
	checkBoxStyle.SetUndeterminedPressedImage(checkedHoveredBrush);
	checkBox->SetWidgetStyle(checkBoxStyle);
	checkBox->SetIsChecked(bChecked);
	row->AddChild(checkBox);
	SetHorizontalSlotPadding(checkBox, FMargin(0.0f, 0.0f, 10.0f, 0.0f));

	UTextBlock* labelText = CreateText(outer, label, 22, MakeSlateColor(1.0f, 1.0f, 1.0f));
	ApplyChakraPetchSemiBoldFont(labelText, 22);
	row->AddChild(labelText);
	SetHorizontalSlotPadding(labelText, FMargin(0.0f));

	return checkBox;
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

struct FFilterSectionWidgets
{
	UScrollBox* ScrollBox;
	UCheckBox* AllCheckBox;
};

FFilterSectionWidgets AddFilterSection(UObject* outer, UVerticalBox* parent, const FString& title, float height)
{
	UTextBlock* heading = CreateText(outer, title, 24, MakeSlateColor(0.68f, 0.68f, 0.72f));
	ApplyChakraPetchSemiBoldFont(heading, 24);
	parent->AddChild(heading);
	SetVerticalSlotPadding(heading, FMargin(0.0f, 0.0f, 0.0f, 10.0f));

	USizeBox* sectionBox = NewObject<USizeBox>(outer);
	sectionBox->SetHeightOverride(height);
	parent->AddChild(sectionBox);
	SetVerticalSlotPadding(sectionBox, FMargin(0.0f, 0.0f, 0.0f, 12.0f));

	UScrollBox* scrollBox = NewObject<UScrollBox>(outer);
	scrollBox->SetOrientation(EOrientation::Orient_Vertical);
	scrollBox->SetScrollBarVisibility(ESlateVisibility::Visible);
	scrollBox->SetAlwaysShowScrollbar(true);
	scrollBox->SetAlwaysShowScrollbarTrack(true);
	scrollBox->SetScrollbarThickness(FVector2D(18.0f, 18.0f));
	sectionBox->AddChild(scrollBox);

	UCheckBox* allCheckBox = AddCheckBoxRow(outer, scrollBox, TEXT("<all>"), true);
	return {scrollBox, allCheckBox};
}

void AddFilterSectionDivider(UObject* outer, UVerticalBox* parent)
{
	USizeBox* dividerBox = NewObject<USizeBox>(outer);
	dividerBox->SetWidthOverride(253.0f);
	dividerBox->SetHeightOverride(3.0f);
	parent->AddChild(dividerBox);

	if (auto* dividerSlot = Cast<UVerticalBoxSlot>(dividerBox->Slot))
	{
		dividerSlot->SetHorizontalAlignment(HAlign_Center);
		dividerSlot->SetPadding(FMargin(0.0f, 3.0f, 0.0f, 9.0f));
	}

	UBorder* divider = CreateColorBlock(outer, FLinearColor(0.74f, 0.74f, 0.74f, 1.0f));
	dividerBox->AddChild(divider);
}

template <typename WidgetType>
WidgetType* FindNamedWidget(UUserWidget* widget, const TCHAR* widgetName, bool bWarnIfMissing = true)
{
	WidgetType* namedWidget = widget ? Cast<WidgetType>(widget->GetWidgetFromName(widgetName)) : nullptr;

	if (!namedWidget && bWarnIfMissing)
	{
		UE_LOG(LogTemp, Warning, TEXT("UI_PCL widget binding failed: %s"), widgetName);
	}

	return namedWidget;
}

UWidget* FindPCLNamedWidget(UUserWidget* widget, const FName& widgetName)
{
	return widget ? widget->GetWidgetFromName(widgetName) : nullptr;
}

bool IsPCLCollapsePersistentWidget(const UWidget* widget)
{

	if (!widget)
	{
		return false;
	}

	const FName widgetName = widget->GetFName();
	return widgetName == PCLCollapseButtonWidgetName || widgetName == PCLGearIconWidgetName || widgetName == PCLInfoWidgetName;
}

bool IsWidgetUnderCursor(const UWidget* widget)
{

	if (!widget || !widget->IsVisible() || !FSlateApplication::IsInitialized())
	{
		return false;
	}

	const FGeometry& geometry = widget->GetCachedGeometry();
	return !geometry.GetLocalSize().IsNearlyZero() && geometry.IsUnderLocation(FSlateApplication::Get().GetCursorPos());
}

FSlateRoundedBoxBrush MakePCLGearButtonBrush()
{
	FSlateRoundedBoxBrush brush(PCLGearPurple, 0.0f);
	brush.ImageSize = PCLGearButtonSize;
	return brush;
}

void ConfigurePCLCollapseButton(UButton* button)
{

	if (!button)
	{
		return;
	}

	FButtonStyle collapseStyle = button->GetStyle();
	collapseStyle.SetHovered(collapseStyle.Normal);
	collapseStyle.SetPressed(collapseStyle.Normal);
	collapseStyle.SetDisabled(collapseStyle.Normal);
	collapseStyle.SetNormalPadding(FMargin(0.0f));
	collapseStyle.SetPressedPadding(FMargin(0.0f));
	button->SetStyle(collapseStyle);
	button->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	button->SetRenderTransform(FWidgetTransform(FVector2D::ZeroVector, FVector2D(1.0f, 1.0f), FVector2D::ZeroVector, 180.0f));
}

void ConfigurePCLGearButton(UUserWidget* uiWidget, UButton* button)
{

	if (!uiWidget || !uiWidget->WidgetTree || !button)
	{
		return;
	}

	FSlateRoundedBoxBrush gearBrush = MakePCLGearButtonBrush();
	FButtonStyle gearStyle = button->GetStyle();
	gearStyle.SetNormal(gearBrush);
	gearStyle.SetHovered(gearBrush);
	gearStyle.SetPressed(gearBrush);
	gearStyle.SetDisabled(gearBrush);
	gearStyle.SetNormalPadding(FMargin(0.0f));
	gearStyle.SetPressedPadding(FMargin(0.0f));
	button->SetStyle(gearStyle);
	button->SetBackgroundColor(FLinearColor::White);
	button->SetColorAndOpacity(FLinearColor::White);

	UImage* gearIcon = Cast<UImage>(uiWidget->WidgetTree->FindWidget(PCLGearRuntimeIconWidgetName));

	if (!gearIcon)
	{
		gearIcon = uiWidget->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), PCLGearRuntimeIconWidgetName);
	}

	if (gearIcon)
	{

		if (UTexture2D* gearTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/SampleViewer/User-Interface/gear_icon.gear_icon")))
		{
			gearIcon->SetBrushFromTexture(gearTexture, true);
		}
		gearIcon->SetDesiredSizeOverride(PCLGearIconSize);
		gearIcon->SetColorAndOpacity(FLinearColor::White);
		gearIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
		button->SetContent(gearIcon);
	}
}

void ConfigurePCLCollapseToggleAppearance(UUserWidget* uiWidget, UButton* collapseButton, UButton* gearButton)
{
	ConfigurePCLCollapseButton(collapseButton);
	ConfigurePCLGearButton(uiWidget, gearButton);
}

void ApplyPCLCollapseToggleVisibility(UUserWidget* uiWidget, bool bCollapsed)
{

	if (UWidget* collapseButton = FindPCLNamedWidget(uiWidget, PCLCollapseButtonWidgetName))
	{
		collapseButton->SetVisibility(bCollapsed ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	if (UWidget* gearButton = FindPCLNamedWidget(uiWidget, PCLGearIconWidgetName))
	{
		gearButton->SetVisibility(bCollapsed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (UWidget* collapseIcon = FindPCLNamedWidget(uiWidget, PCLCollapseIconWidgetName))
	{
		collapseIcon->SetVisibility(bCollapsed ? ESlateVisibility::Collapsed : ESlateVisibility::HitTestInvisible);
	}
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
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapActor not found in the level!"));
		return;
	}

	MapComponent = MapActor->GetMapComponent();

	if (!MapComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapComponent not found on ArcGISMapActor!"));
		return;
	}

	if (UArcGISPoint* originPosition = MapComponent->GetOriginPosition())
	{
		SpatialReference = originPosition->GetSpatialReference();
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

		FInputModeGameAndUI inputMode;
		inputMode.SetHideCursorDuringCapture(false);
		inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		playerController->SetInputMode(inputMode);
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
		FilterPanel = FindNamedWidget<UPanelWidget>(UIWidget, TEXT("Panel_FilterDynamicContent"));
		ResetFiltersButton = FindNamedWidget<UButton>(UIWidget, TEXT("Button_ResetFilters"));
		SourceUrlTextBox = FindNamedWidget<UEditableTextBox>(UIWidget, TEXT("EditableTextBox_0"));
		LoadLayerButton = FindNamedWidget<UButton>(UIWidget, TEXT("Button_Load"));
		CollapseButton = FindNamedWidget<UButton>(UIWidget, TEXT("Button_Collapse"), false);
		GearButton = FindNamedWidget<UButton>(UIWidget, TEXT("Button_Gear"), false);
		UUserWidget* infoWidget = Cast<UUserWidget>(FindPCLNamedWidget(UIWidget, PCLInfoWidgetName));
		UUserWidget* infoButtonWidget =
			infoWidget ? Cast<UUserWidget>(infoWidget->GetWidgetFromName(PCLInfoButtonWidgetName)) : nullptr;
		InfoButton =
			infoButtonWidget ? Cast<UButton>(infoButtonWidget->GetWidgetFromName(PCLInfoButtonControlName)) : nullptr;
		UE_LOG(LogTemp, Display, TEXT("UI_PCL collapse widgets: Button_Collapse=%s Button_Gear=%s"),
			   CollapseButton ? *CollapseButton->GetClass()->GetName() : TEXT("missing"),
			   GearButton ? *GearButton->GetClass()->GetName() : TEXT("missing"));
		ConfigurePCLCollapseToggleAppearance(UIWidget, CollapseButton, GearButton);
		LayerLoadStatusText = FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_LayerLoadStatus"), false);
		UIInteractionPanel = FindNamedWidget<UWidget>(UIWidget, TEXT("Background"));
		BuildDataLoaderUI();

		if (SourceUrlTextBox)
		{
			SourceUrlTextBox->SetText(FText::FromString(PointCloudLayerSource));
		}

		if (LayerLoadStatusText)
		{
			LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
		}

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

		if (ResetFiltersButton)
		{
			ResetFiltersButton->OnClicked.AddDynamic(this, &APCLController::OnResetFiltersClicked);
		}

		if (LoadLayerButton)
		{
			LoadLayerButton->OnClicked.AddDynamic(this, &APCLController::OnLoadPointCloudLayerClicked);
		}

		if (CollapseButton)
		{
			CollapseButton->OnClicked.AddDynamic(this, &APCLController::OnCollapseButtonClicked);
		}

		if (GearButton)
		{
			GearButton->OnClicked.AddDynamic(this, &APCLController::OnCollapseButtonClicked);
		}

		if (InfoButton)
		{
			InfoButton->OnClicked.AddDynamic(this, &APCLController::OnInfoButtonClicked);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("UI_PCL info button binding failed."));
		}

		UpdateSliderValueTexts();
		UpdateRendererCheckBoxes();
		BuildFilterTabUI();
		SetTabLayout(EPCLTabLayout::Default);
		ConfigurePCLCollapseInitialState();
		DeferredPointCloudLayerSource = PointCloudLayerSource;
		bDeferredZoomWhenLoaded = true;
		DeferredPointCloudLayerRetrySeconds = PointCloudLayerLoadRetryInterval;
		PointCloudLayerLoadRetryCount = 0;
		ApplyPointCloudVisualization();
		ApplyPointCloudFilters();

		if (UIWidget->FindFunction("ShowInstruction"))
		{
			UIWidget->ProcessEvent(UIWidget->FindFunction("ShowInstruction"), nullptr);
		}
	}
}

void APCLController::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	SetMapInputBlockedByUI(false);

	if (InputManager)
	{
		InputManager->OnInputTrigger.RemoveDynamic(this, &APCLController::OnInputTriggered);
		InputManager->OnInputEnd.RemoveDynamic(this, &APCLController::OnInputEnded);
	}

	Super::EndPlay(endPlayReason);
}

void APCLController::Tick(float deltaTime)
{
	Super::Tick(deltaTime);
	HandlePCLCollapseInput();
	SyncLegendVisibilityWithMainPanel();
	UpdateMapInputForUIHover();

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

void APCLController::SyncLegendVisibilityWithMainPanel() const
{

	if (!UIWidget || !LegendPanel)
	{
		return;
	}

	const UWidget* mainPanel = FindPCLNamedWidget(UIWidget, PCLMainPanelWidgetName);

	if (!mainPanel)
	{
		return;
	}

	const bool bShouldShowLegend = !bPCLUICollapsed && mainPanel->IsVisible();
	const ESlateVisibility legendVisibility = bShouldShowLegend ? ESlateVisibility::Visible : ESlateVisibility::Hidden;

	if (LegendPanel->GetVisibility() != legendVisibility)
	{
		LegendPanel->SetVisibility(legendVisibility);
	}
}

void APCLController::UpdateMapInputForUIHover()
{
	bool bShouldBlockMapInput = false;

	if (UIInteractionPanel && UIInteractionPanel->IsVisible() && FSlateApplication::IsInitialized())
	{
		const FGeometry& panelGeometry = UIInteractionPanel->GetCachedGeometry();

		if (!panelGeometry.GetLocalSize().IsNearlyZero())
		{
			bShouldBlockMapInput = panelGeometry.IsUnderLocation(FSlateApplication::Get().GetCursorPos());
		}
	}

	if (!bShouldBlockMapInput && UIWidget && FSlateApplication::IsInitialized())
	{
		bShouldBlockMapInput = IsPCLCollapseToggleUnderCursor();
	}

	SetMapInputBlockedByUI(bShouldBlockMapInput);
}

void APCLController::SetMapInputBlockedByUI(bool bBlocked)
{

	if (bMapInputBlockedByUI == bBlocked)
	{
		return;
	}

	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (!playerController)
	{
		return;
	}

	if (bBlocked)
	{
		playerController->FlushPressedKeys();
		playerController->SetIgnoreLookInput(true);
		playerController->SetIgnoreMoveInput(true);

		FInputModeUIOnly inputMode;
		inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		playerController->SetInputMode(inputMode);
	}
	else
	{
		playerController->SetIgnoreLookInput(false);
		playerController->SetIgnoreMoveInput(false);

		FInputModeGameAndUI inputMode;
		inputMode.SetHideCursorDuringCapture(false);
		inputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		playerController->SetInputMode(inputMode);
	}

	playerController->bShowMouseCursor = true;
	bMapInputBlockedByUI = bBlocked;
}

void APCLController::OnInputTriggered()
{
	bPointerDownOverPCLCollapseToggle = IsPCLCollapseToggleUnderCursor();

	if (bPointerDownOverPCLCollapseToggle)
	{
		SetMapInputBlockedByUI(true);
	}
}

void APCLController::OnInputEnded()
{

	if (bPointerDownOverPCLCollapseToggle && IsPCLCollapseToggleUnderCursor())
	{
		TogglePCLUICollapse();
	}

	bPointerDownOverPCLCollapseToggle = false;
}

void APCLController::OnPointSizeChanged(float value)
{
	UpdateSliderValueTexts();

	auto renderer = GetLoadedRenderer(PointCloudLayer);

	if (renderer)
	{
		Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudFixedSizeAlgorithm sizeAlgorithm(
			FMath::Clamp(static_cast<double>(value), MinPointSize, MaxPointSize), Esri::GameEngine::Map::Symbology::ArcGISSymbolSizeUnits::DIPs);
		renderer.SetSizeAlgorithm(sizeAlgorithm);
	}
}

void APCLController::OnPointsPerInchChanged(float value)
{
	UpdateSliderValueTexts();

	auto renderer = GetLoadedRenderer(PointCloudLayer);

	if (renderer)
	{
		renderer.SetPointsPerInch(FMath::Max(static_cast<double>(value), MinPointsPerInch));
	}
}

void APCLController::SetColorModulationEnabled(bool bEnabled)
{

	if (bColorModulationEnabled == bEnabled)
	{
		return;
	}

	bColorModulationEnabled = bEnabled;

	auto renderer = GetLoadedRenderer(PointCloudLayer);

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

void APCLController::OnColorModulationCheckStateChanged(bool bIsChecked)
{
	SetColorModulationEnabled(bIsChecked);
}

void APCLController::OnRGBRendererCheckStateChanged(bool bIsChecked)
{
	HandleRendererCheckStateChanged(bIsChecked, EPCLRendererChoice::RGB);
}

void APCLController::OnClassRendererCheckStateChanged(bool bIsChecked)
{
	HandleRendererCheckStateChanged(bIsChecked, EPCLRendererChoice::Class);
}

void APCLController::OnElevationRendererCheckStateChanged(bool bIsChecked)
{
	HandleRendererCheckStateChanged(bIsChecked, EPCLRendererChoice::Elevation);
}

void APCLController::OnIntensityRendererCheckStateChanged(bool bIsChecked)
{
	HandleRendererCheckStateChanged(bIsChecked, EPCLRendererChoice::Intensity);
}

void APCLController::HandleRendererCheckStateChanged(bool bIsChecked, EPCLRendererChoice rendererChoice)
{

	if (bUpdatingRendererCheckBoxes)
	{
		return;
	}

	if (bIsChecked)
	{
		SetPointCloudRenderer(rendererChoice);
		return;
	}

	if (CurrentRendererChoice == rendererChoice)
	{
		UpdateRendererCheckBoxes();
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

	TGuardValue<bool> updatingGuard(bUpdatingFilterCheckBoxes, true);

	if (ClassAllCheckBox)
	{
		bool bAllClassesChecked = true;

		for (TObjectPtr<UCheckBox> checkBox : ClassFilterCheckBoxes)
		{
			bAllClassesChecked = bAllClassesChecked && checkBox && checkBox->IsChecked();
		}
		ClassAllCheckBox->SetIsChecked(bAllClassesChecked);
	}

	if (ReturnsAllCheckBox)
	{
		bool bAllReturnsChecked = true;

		for (TObjectPtr<UCheckBox> checkBox : ReturnsFilterCheckBoxes)
		{
			bAllReturnsChecked = bAllReturnsChecked && checkBox && checkBox->IsChecked();
		}
		ReturnsAllCheckBox->SetIsChecked(bAllReturnsChecked);
	}

	ApplyPointCloudFilters();
}

void APCLController::OnClassAllFilterCheckStateChanged(bool bIsChecked)
{
	SetAllFilterOptionsChecked(ClassFilterCheckBoxes, bIsChecked);
}

void APCLController::OnReturnsAllFilterCheckStateChanged(bool bIsChecked)
{
	SetAllFilterOptionsChecked(ReturnsFilterCheckBoxes, bIsChecked);
}

void APCLController::SetAllFilterOptionsChecked(const TArray<TObjectPtr<UCheckBox>>& filterCheckBoxes, bool bIsChecked)
{

	if (bUpdatingFilterCheckBoxes)
	{
		return;
	}

	TGuardValue<bool> updatingGuard(bUpdatingFilterCheckBoxes, true);

	for (const TObjectPtr<UCheckBox>& checkBox : filterCheckBoxes)
	{

		if (checkBox)
		{
			checkBox->SetIsChecked(bIsChecked);
		}
	}

	ApplyPointCloudFilters();
}

void APCLController::OnResetFiltersClicked()
{
	ResetFilterSelections(true);
}

void APCLController::OnLoadPointCloudLayerClicked()
{
	FString source = SourceUrlTextBox ? SourceUrlTextBox->GetText().ToString() : FString();
	source.TrimStartAndEndInline();

	if (SourceUrlTextBox)
	{
		SourceUrlTextBox->SetText(FText::FromString(source));
	}

	if (!IsValidURL(source))
	{
		SetLayerLoadStatus(false);
		return;
	}

	DeferredPointCloudLayerSource.Reset();
	PointCloudLayerLoadRetryCount = 0;
	CreatePointCloudLayer(source, true);
}

void APCLController::OnCollapseButtonClicked()
{
	TogglePCLUICollapse();
}

void APCLController::OnInfoButtonClicked()
{

	if (!UIWidget)
	{
		return;
	}

	if (UWidget* mainPanel = FindPCLNamedWidget(UIWidget, PCLMainPanelWidgetName))
	{
		mainPanel->SetVisibility(ESlateVisibility::Hidden);
	}

	if (LegendPanel)
	{
		LegendPanel->SetVisibility(ESlateVisibility::Hidden);
	}

	if (FBoolProperty* isMenuHiddenProperty =
			FindFProperty<FBoolProperty>(UIWidget->GetClass(), PCLMenuHiddenPropertyName))
	{
		isMenuHiddenProperty->SetPropertyValue_InContainer(UIWidget, true);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UI_PCL property binding failed: IsMenuHidden"));
	}
}

void APCLController::ConfigurePCLCollapseInitialState()
{
	bPCLUICollapsed = false;
	bPointerDownOverPCLCollapseToggle = false;
	LastPCLCollapseToggleTimeSeconds = -1.0;
	CachedPCLRootChildVisibilities.Reset();
	ApplyPCLCollapseToggleVisibility(UIWidget, false);
}

void APCLController::HandlePCLCollapseInput()
{

	if (!UIWidget || !FSlateApplication::IsInitialized())
	{
		bPointerDownOverPCLCollapseToggle = false;
		return;
	}

	const bool bPointerDown = FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton);
	const bool bPointerOverToggle = IsPCLCollapseToggleUnderCursor();

	if (bPointerDown)
	{
		bPointerDownOverPCLCollapseToggle = bPointerDownOverPCLCollapseToggle || bPointerOverToggle;
		return;
	}

	if (bPointerDownOverPCLCollapseToggle && bPointerOverToggle)
	{
		TogglePCLUICollapse();
	}

	bPointerDownOverPCLCollapseToggle = false;
}

void APCLController::SetPCLUICollapsed(bool bCollapsed)
{

	if (!UIWidget)
	{
		return;
	}

	UCanvasPanel* rootCanvas = Cast<UCanvasPanel>(FindPCLNamedWidget(UIWidget, PCLRootCanvasWidgetName));

	if (!rootCanvas)
	{
		return;
	}

	TArray<UWidget*> collapsibleWidgets;

	for (int32 childIndex = 0; childIndex < rootCanvas->GetChildrenCount(); ++childIndex)
	{
		UWidget* child = rootCanvas->GetChildAt(childIndex);

		if (child && child->GetFName() == PCLMainPanelWidgetName)
		{

			if (const UPanelWidget* mainPanel = Cast<UPanelWidget>(child))
			{

				for (int32 mainChildIndex = 0; mainChildIndex < mainPanel->GetChildrenCount(); ++mainChildIndex)
				{
					collapsibleWidgets.Add(mainPanel->GetChildAt(mainChildIndex));
				}
			}
			continue;
		}

		collapsibleWidgets.Add(child);
	}

	if (bCollapsed)
	{
		CachedPCLRootChildVisibilities.Reset();

		for (UWidget* child : collapsibleWidgets)
		{

			if (!child || IsPCLCollapsePersistentWidget(child))
			{
				continue;
			}

			CachedPCLRootChildVisibilities.Add(child->GetFName(), child->GetVisibility());
			child->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
	else
	{

		for (UWidget* child : collapsibleWidgets)
		{

			if (!child || IsPCLCollapsePersistentWidget(child))
			{
				continue;
			}

			if (const ESlateVisibility* cachedVisibility = CachedPCLRootChildVisibilities.Find(child->GetFName()))
			{
				child->SetVisibility(*cachedVisibility);
			}
		}

		CachedPCLRootChildVisibilities.Reset();
	}

	bPCLUICollapsed = bCollapsed;
	ApplyPCLCollapseToggleVisibility(UIWidget, bPCLUICollapsed);
	UE_LOG(LogTemp, Display, TEXT("UI_PCL collapse state changed: %s"), bPCLUICollapsed ? TEXT("collapsed") : TEXT("expanded"));
}

void APCLController::TogglePCLUICollapse()
{
	const double currentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

	if (LastPCLCollapseToggleTimeSeconds >= 0.0 && currentTimeSeconds - LastPCLCollapseToggleTimeSeconds < 0.05)
	{
		return;
	}

	LastPCLCollapseToggleTimeSeconds = currentTimeSeconds;
	SetPCLUICollapsed(!bPCLUICollapsed);

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
	}
}

bool APCLController::IsPCLCollapseToggleUnderCursor() const
{
	return IsWidgetUnderCursor(FindPCLNamedWidget(UIWidget, PCLCollapseButtonWidgetName)) ||
		   IsWidgetUnderCursor(FindPCLNamedWidget(UIWidget, PCLGearIconWidgetName));
}

void APCLController::BuildDataLoaderUI()
{

	if (!UIWidget)
	{
		return;
	}

	if (LayerLoadStatusText)
	{
		LayerLoadStatusText->SetText(FText::GetEmpty());
		LayerLoadStatusText->SetColorAndOpacity(MakeSlateColor(0.42f, 0.78f, 0.04f));
		LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
		LayerLoadStatusText->SetIsEnabled(true);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("UI_PCL is missing the Text_LayerLoadStatus TextBlock."));
	}

	if (LoadLayerButton)
	{
		LoadLayerButtonText = Cast<UTextBlock>(LoadLayerButton->GetContent());

		if (!LoadLayerButtonText)
		{
			LoadLayerButtonText = NewObject<UTextBlock>(UIWidget, TEXT("Text_LoadLayer"));
			LoadLayerButton->AddChild(LoadLayerButtonText);
		}

		LoadLayerButtonText->SetText(FText::FromString(TEXT("Load")));
		LoadLayerButtonText->SetColorAndOpacity(MakeSlateColor(1.0f, 1.0f, 1.0f));
		ApplyChakraPetchSemiBoldFont(LoadLayerButtonText, 20);

		if (auto* buttonSlot = Cast<UButtonSlot>(LoadLayerButtonText->Slot))
		{
			buttonSlot->SetHorizontalAlignment(HAlign_Center);
			buttonSlot->SetVerticalAlignment(VAlign_Center);
		}
	}
}

void APCLController::DeferPointCloudLayerLoad(const FString& source, bool bZoomWhenLoaded)
{
	++PointCloudLayerLoadRetryCount;

	if (PointCloudLayerLoadRetryCount > MaxPointCloudLayerLoadRetries)
	{
		DeferredPointCloudLayerSource.Reset();
		SetLayerLoadStatus(false);

		if (LoadLayerButton)
		{
			LoadLayerButton->SetIsEnabled(true);
		}
		UE_LOG(LogTemp, Warning, TEXT("ArcGIS Map was not ready after %.1f seconds; point cloud layer load was cancelled."),
			   MaxPointCloudLayerLoadRetries * PointCloudLayerLoadRetryInterval);
		return;
	}

	DeferredPointCloudLayerSource = source;
	bDeferredZoomWhenLoaded = bZoomWhenLoaded;
	DeferredPointCloudLayerRetrySeconds = PointCloudLayerLoadRetryInterval;

	if (LoadLayerButton)
	{
		LoadLayerButton->SetIsEnabled(false);
	}

	if (LayerLoadStatusText)
	{
		LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void APCLController::SetLayerLoadStatus(bool bSucceeded) const
{

	if (!LayerLoadStatusText)
	{
		return;
	}

	LayerLoadStatusText->SetText(FText::FromString(bSucceeded ? TEXT("Layer loaded successfully...") : TEXT("Failed to load point cloud layer!")));
	LayerLoadStatusText->SetColorAndOpacity(FSlateColor(bSucceeded ? FLinearColor(0.42f, 0.78f, 0.04f) : FLinearColor(0.93f, 0.31f, 0.43f)));
	LayerLoadStatusText->SetVisibility(ESlateVisibility::Visible);

	if (!bSucceeded && LoadLayerButton)
	{
		LoadLayerButton->SetIsEnabled(true);
	}
}

void APCLController::CreatePointCloudLayer(const FString& source, bool bZoomWhenLoaded)
{

	if (!IsValidURL(source))
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
	catch (const Esri::Unreal::ArcGISException& loadError)
	{
		UE_LOG(LogTemp, Verbose, TEXT("ArcGIS Map is not ready for point cloud layers: %s"), *loadError.GetMessage());
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
	catch (const Esri::Unreal::ArcGISException& loadError)
	{
		SetLayerLoadStatus(false);
		UE_LOG(LogTemp, Warning, TEXT("Invalid point cloud layer source '%s': %s"), *source, *loadError.GetMessage());
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
	candidateLayer->APIObject->SetDoneLoading([weakThis, weakCandidate, requestId, bZoomWhenLoaded,
											   source](Esri::Unreal::ArcGISException& loadError) {
		const bool bHadLoadError = static_cast<bool>(loadError);
		const FString loadErrorMessage = bHadLoadError ? loadError.GetMessage() : FString();

		AsyncTask(ENamedThreads::GameThread, [weakThis, weakCandidate, requestId, bZoomWhenLoaded, bHadLoadError, loadErrorMessage, source]() {
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
				UE_LOG(LogTemp, Warning, TEXT("Failed to load point cloud layer from '%s': %s"), *source,
					   loadErrorMessage.IsEmpty() ? TEXT("Unknown load error") : *loadErrorMessage);
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
			UE_LOG(LogTemp, Display, TEXT("Loaded point cloud layer from '%s'."), *source);
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
					else
					{
						UE_LOG(LogTemp, Warning, TEXT("Point cloud layer loaded, but its extent has no center for updating the origin."));
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Point cloud layer loaded, but it has no extent for updating the origin."));
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

				if (!viewActor || !controller->MapComponent->ZoomToExtent(viewActor, layerExtent))
				{
					UE_LOG(LogTemp, Warning, TEXT("Point cloud layer loaded, but zoom to layer failed."));
				}
			}
		});
	});

	mapLayers->Add(candidateLayer);
}

void APCLController::ApplyPointCloudVisualization()
{

	if (!PointCloudLayer)
	{
		return;
	}

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
		FMath::Clamp(PointSizeSlider ? static_cast<double>(PointSizeSlider->GetValue()) : DefaultPointSize, MinPointSize, MaxPointSize);
	const double pointsPerInch =
		FMath::Max(PointsPerInchSlider ? static_cast<double>(PointsPerInchSlider->GetValue()) : DefaultPointsPerInch, MinPointsPerInch);

	EnsureAvailableRendererSelected();

	auto applyRenderer = [&](auto& renderer) {
		ConfigurePointCloudRendererSettings(renderer, pointSize, bColorModulationEnabled, IntensityAttributeName);
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
					AddClassValue(uniqueValues, classValue);
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
				AddColorStop(stops, ElevationLow, FColor(42, 43, 238, 255), TEXT("< -1.5"));
				AddColorStop(stops, 0.0, FColor(40, 210, 246, 255), TEXT(""));
				AddColorStop(stops, ElevationMid, FColor(91, 248, 134, 255), TEXT("1.5"));
				AddColorStop(stops, 2.5, FColor(250, 244, 73, 255), TEXT(""));
				AddColorStop(stops, ElevationHigh, FColor(255, 59, 22, 255), TEXT("> 3.5"));

				Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudStretchRenderer renderer(ElevationAttributeName, stops);
				applyRenderer(renderer);
				return;
			}
			break;
		case EPCLRendererChoice::Intensity:

			if (!IntensityAttributeName.IsEmpty())
			{
				Esri::Unreal::ArcGISCollection<Esri::GameEngine::Layers::PointCloud::ArcGISPointCloudColorStop> stops;
				AddColorStop(stops, IntensityLow, FColor(0, 0, 0, 255), TEXT("< 10,385"));
				AddColorStop(stops, IntensityMid, FColor(128, 128, 128, 255), TEXT("38,032"));
				AddColorStop(stops, IntensityHigh, FColor(255, 255, 255, 255), TEXT("> 65,680"));

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

	const int32 selectedClassOptionCount = CountSelectedOptions(ClassFilterCheckBoxes);
	const bool bUseClassFilter = !ClassAttributeName.IsEmpty() && selectedClassOptionCount > 0 &&
								 selectedClassOptionCount < ClassFilterCheckBoxes.Num();

	if (bUseClassFilter)
	{
		AddSelectedPointCloudFilter(
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

	const int32 selectedReturnsOptionCount = CountSelectedOptions(ReturnsFilterCheckBoxes);
	const bool bUseReturnsFilter = !ReturnsAttributeName.IsEmpty() && selectedReturnsOptionCount > 0 &&
								   selectedReturnsOptionCount < ReturnsFilterCheckBoxes.Num();

	if (bUseReturnsFilter)
	{
		AddSelectedPointCloudFilter(
			ReturnsFilterCheckBoxes,
			static_cast<int32>(UE_ARRAY_COUNT(FilterReturnValues)),
			ActiveReturnsValues,
			ActiveReturnsFilter,
			*ActiveFilterCollection,
			[](int32 index) { return FilterReturnValues[index]; },
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
		const FString normalizedName = NormalizeAttributeName(name);

		if (RGBAttributeName.IsEmpty() && IsRGBAttribute(attribute, normalizedName))
		{
			RGBAttributeName = name;
		}

		if (ClassAttributeName.IsEmpty() &&
			(MatchesAttributeName(normalizedName, TEXT("CLASSCODE")) || MatchesAttributeName(normalizedName, TEXT("CLASSIFICATION")) ||
			 MatchesAttributeName(normalizedName, TEXT("CLASS"))))
		{
			ClassAttributeName = name;
		}

		if (ElevationAttributeName.IsEmpty() && (MatchesAttributeName(normalizedName, TEXT("ELEVATION")) ||
												 MatchesAttributeName(normalizedName, TEXT("HEIGHT")) || normalizedName == TEXT("Z")))
		{
			ElevationAttributeName = name;
		}

		if (IntensityAttributeName.IsEmpty() && MatchesAttributeName(normalizedName, TEXT("INTENSITY")))
		{
			IntensityAttributeName = name;
		}

		if (ReturnsAttributeName.IsEmpty() && MatchesAttributeName(normalizedName, TEXT("RETURNS")))
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

void APCLController::UpdateRendererCheckBoxes()
{
	TGuardValue<bool> updatingGuard(bUpdatingRendererCheckBoxes, true);
	const bool bHasLoadedRendererAttributes =
		!RGBAttributeName.IsEmpty() || !ClassAttributeName.IsEmpty() || !ElevationAttributeName.IsEmpty() || !IntensityAttributeName.IsEmpty();

	UpdateColorModulationVisibility();

	if (bHasLoadedRendererAttributes)
	{
		EnsureAvailableRendererSelected();
		SetRendererOptionVisibility(EPCLRendererChoice::RGB, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::RGB));
		SetRendererOptionVisibility(EPCLRendererChoice::Class, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Class));
		SetRendererOptionVisibility(EPCLRendererChoice::Elevation, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Elevation));
		SetRendererOptionVisibility(EPCLRendererChoice::Intensity, IsRendererAvailableFromCachedAttributes(EPCLRendererChoice::Intensity));
	}

	const auto updateRendererCheckBox = [this](UCheckBox* checkBox, EPCLRendererChoice rendererChoice)
	{

		if (!checkBox)
		{
			return;
		}

		const bool bIsSelected = CurrentRendererChoice == rendererChoice;
		checkBox->SetIsChecked(bIsSelected);
	};

	updateRendererCheckBox(RGBRendererCheckBox, EPCLRendererChoice::RGB);
	updateRendererCheckBox(ClassRendererCheckBox, EPCLRendererChoice::Class);
	updateRendererCheckBox(ElevationRendererCheckBox, EPCLRendererChoice::Elevation);
	updateRendererCheckBox(IntensityRendererCheckBox, EPCLRendererChoice::Intensity);
}

void APCLController::UpdateColorModulationVisibility()
{
	const ESlateVisibility visibility = IntensityAttributeName.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::Visible;

	if (UWidget* row = UIWidget ? UIWidget->GetWidgetFromName(TEXT("Row_Checkbox_ColorModulation")) : nullptr)
	{
		row->SetVisibility(visibility);
	}

	if (UWidget* divider = UIWidget ? UIWidget->GetWidgetFromName(TEXT("Border_VisualizeDivider")) : nullptr)
	{
		divider->SetVisibility(visibility);
	}
}

void APCLController::SetRendererOptionVisibility(EPCLRendererChoice rendererChoice, bool bVisible)
{
	const ESlateVisibility visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	const TCHAR* rowName = nullptr;
	const TCHAR* checkBoxName = nullptr;
	const TCHAR* textName = nullptr;

	switch (rendererChoice)
	{
		case EPCLRendererChoice::RGB:
			rowName = TEXT("Row_Checkbox_Renderer_RGB");
			checkBoxName = TEXT("Checkbox_Renderer_RGB");
			textName = TEXT("Text_Renderer_RGB");
			break;
		case EPCLRendererChoice::Class:
			rowName = TEXT("Row_Checkbox_Renderer_Class");
			checkBoxName = TEXT("Checkbox_Renderer_Class");
			textName = TEXT("Text_Renderer_Class");
			break;
		case EPCLRendererChoice::Elevation:
			rowName = TEXT("Row_Checkbox_Renderer_Elevation");
			checkBoxName = TEXT("Checkbox_Renderer_Elevation");
			textName = TEXT("Text_Renderer_Elevation");
			break;
		case EPCLRendererChoice::Intensity:
			rowName = TEXT("Row_Checkbox_Renderer_Intensity");
			checkBoxName = TEXT("Checkbox_Renderer_Intensity");
			textName = TEXT("Text_Renderer_Intensity");
			break;
		default:
			return;
	}

	if (UWidget* row = UIWidget ? UIWidget->GetWidgetFromName(rowName) : nullptr)
	{
		row->SetVisibility(visibility);
		return;
	}

	if (UWidget* checkBox = UIWidget ? UIWidget->GetWidgetFromName(checkBoxName) : nullptr)
	{
		checkBox->SetVisibility(visibility);
		checkBox->SetIsEnabled(bVisible);
	}

	if (UWidget* text = UIWidget ? UIWidget->GetWidgetFromName(textName) : nullptr)
	{
		text->SetVisibility(visibility);
		text->SetIsEnabled(bVisible);
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

	RefreshAvailablePointCloudAttributes();
	const bool bHasClassCodeFilter = !ClassAttributeName.IsEmpty();
	const bool bHasReturnsFilter = !ReturnsAttributeName.IsEmpty();

	if (!bHasClassCodeFilter && !bHasReturnsFilter)
	{
		ClearActiveFilters();
		return;
	}

	UVerticalBox* content = NewObject<UVerticalBox>(UIWidget);
	FilterPanel->AddChild(content);

	if (auto* canvasSlot = Cast<UCanvasPanelSlot>(content->Slot))
	{
		canvasSlot->SetAnchors(FAnchors(0.0f, 0.0f, 1.0f, 0.0f));
		canvasSlot->SetOffsets(FMargin(0.0f, 0.0f, 0.0f, 492.0f));
	}

	auto addFilterOption = [this](UScrollBox* scrollBox,
								  const FString& label,
								  TArray<TObjectPtr<UCheckBox>>& filterCheckBoxes)
	{
		UCheckBox* checkBox = AddCheckBoxRow(UIWidget, scrollBox, label, true);
		checkBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnFilterCheckStateChanged);
		filterCheckBoxes.Add(checkBox);
	};

	if (bHasClassCodeFilter)
	{
		const FFilterSectionWidgets classSection = AddFilterSection(UIWidget, content, TEXT("Class Code"), 242.0f);
		ClassAllCheckBox = classSection.AllCheckBox;
		ClassAllCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnClassAllFilterCheckStateChanged);

		for (int32 classCode = 0; classCode <= 18; ++classCode)
		{
			addFilterOption(classSection.ScrollBox, GetClassCodeLabel(classCode), ClassFilterCheckBoxes);
			ClassFilterValues.Add(classCode);
		}
	}

	if (bHasClassCodeFilter && bHasReturnsFilter)
	{
		AddFilterSectionDivider(UIWidget, content);
	}

	if (bHasReturnsFilter)
	{
		const FFilterSectionWidgets returnsSection = AddFilterSection(UIWidget, content, TEXT("Returns"), 167.0f);
		ReturnsAllCheckBox = returnsSection.AllCheckBox;
		ReturnsAllCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnReturnsAllFilterCheckStateChanged);

		for (int32 index = 0; index < UE_ARRAY_COUNT(FilterReturnLabels); ++index)
		{
			addFilterOption(returnsSection.ScrollBox, FilterReturnLabels[index], ReturnsFilterCheckBoxes);
		}
	}

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

	UCanvasPanel* rootCanvas = Cast<UCanvasPanel>(UIWidget->WidgetTree->RootWidget);

	if (!rootCanvas)
	{
		UE_LOG(LogTemp, Warning, TEXT("UI_PCL legend binding failed: root widget is not a canvas panel."));
		return;
	}

	const bool bCompact = CurrentRendererChoice == EPCLRendererChoice::RGB;
	const FVector2D legendSize = bCompact ? FVector2D(LegendCompactWidth, LegendCompactHeight) : FVector2D(LegendExpandedWidth, LegendExpandedHeight);

	LegendPanel = NewObject<UCanvasPanel>(UIWidget);
	LegendPanel->SetRenderTransformPivot(FVector2D(1.0f, 1.0f));
	LegendPanel->SetRenderScale(FVector2D(PCLTabUIScale, PCLTabUIScale));
	UCanvasPanelSlot* legendSlot = rootCanvas->AddChildToCanvas(LegendPanel);

	if (legendSlot)
	{
		legendSlot->SetAnchors(FAnchors(1.0f, 1.0f, 1.0f, 1.0f));
		legendSlot->SetAlignment(FVector2D(1.0f, 1.0f));
		legendSlot->SetPosition(FVector2D(-50.0f, -34.0f));
		legendSlot->SetSize(legendSize);
		legendSlot->SetZOrder(30);
	}

	SyncLegendVisibilityWithMainPanel();

	UBorder* background = CreateColorBlock(UIWidget, FLinearColor(0.03f, 0.03f, 0.035f, 0.88f));
	UCanvasPanelSlot* backgroundSlot = LegendPanel->AddChildToCanvas(background);

	if (backgroundSlot)
	{
		backgroundSlot->SetPosition(FVector2D::ZeroVector);
		backgroundSlot->SetSize(legendSize);
	}

	UBorder* accent = CreateColorBlock(UIWidget, FLinearColor(0.58f, 0.23f, 1.0f, 1.0f));
	UCanvasPanelSlot* accentSlot = LegendPanel->AddChildToCanvas(accent);

	if (accentSlot)
	{
		accentSlot->SetPosition(FVector2D::ZeroVector);
		accentSlot->SetSize(FVector2D(8.0f, legendSize.Y));
	}

	UVerticalBox* content = NewObject<UVerticalBox>(UIWidget);
	UCanvasPanelSlot* contentSlot = LegendPanel->AddChildToCanvas(content);

	if (contentSlot)
	{
		contentSlot->SetPosition(bCompact ? FVector2D(34.0f, 20.0f) : FVector2D(59.0f, 24.0f));
		contentSlot->SetSize(bCompact ? FVector2D(290.0f, 44.0f) : FVector2D(LegendExpandedContentWidth, 321.0f));
	}

	if (CurrentRendererChoice == EPCLRendererChoice::RGB)
	{
		content->AddChild(CreateText(UIWidget, TEXT("No legend"), 28, MakeSlateColor(0.78f, 0.78f, 0.82f)));
		return;
	}

	FString legendTitle = PointCloudLayer ? PointCloudLayer->GetName() : FString();

	if (legendTitle.TrimStartAndEnd().IsEmpty())
	{
		legendTitle = TEXT("Point cloud layer");
	}

	UTextBlock* title = CreateText(UIWidget, legendTitle, 24, MakeSlateColor(0.62f, 0.62f, 0.66f));
	ApplyLegendTitleFont(title);
	title->SetAutoWrapText(true);
	title->SetWrapTextAt(LegendExpandedContentWidth);
	title->SetRenderTranslation(FVector2D(-22.0f, 0.0f));
	content->AddChild(title);
	SetVerticalSlotPadding(title, FMargin(0.0f, 0.0f, 0.0f, 36.0f));

	if (CurrentRendererChoice == EPCLRendererChoice::Class)
	{
		UTextBlock* heading = CreateText(UIWidget, TEXT("Class Code"), 18, MakeSlateColor(1.0f, 1.0f, 1.0f));
		content->AddChild(heading);
		SetVerticalSlotPadding(heading, FMargin(0.0f, 0.0f, 0.0f, 8.0f));

		USizeBox* classListBox = NewObject<USizeBox>(UIWidget);
		classListBox->SetWidthOverride(299.0f);
		classListBox->SetHeightOverride(158.0f);
		content->AddChild(classListBox);

		UScrollBox* classList = NewObject<UScrollBox>(UIWidget);
		classList->SetOrientation(EOrientation::Orient_Vertical);
		classList->SetScrollBarVisibility(ESlateVisibility::Visible);
		classList->SetAlwaysShowScrollbar(true);
		classList->SetAlwaysShowScrollbarTrack(true);
		classList->SetScrollbarThickness(FVector2D(18.0f, 18.0f));
		classListBox->AddChild(classList);

		const int32 visibleClassCodes[] = {1, 2, 3, 5, 6, 7, 9};

		for (int32 classCode : visibleClassCodes)
		{
			const FStandardClassInfo classInfo = GetStandardClassInfo(classCode);
			const FLinearColor classColor(
				classInfo.Red / 255.0f, classInfo.Green / 255.0f, classInfo.Blue / 255.0f, 1.0f);
			UTexture2D* circleTexture = CreateLegendCircleTexture(UIWidget, classColor);

			if (circleTexture)
			{
				LegendTextures.Add(circleTexture);
			}
			AddLegendRow(UIWidget, classList, classInfo.Label, classColor, circleTexture);
		}

		return;
	}

	const bool bElevationLegend = CurrentRendererChoice == EPCLRendererChoice::Elevation;
	UTextBlock* heading = CreateText(UIWidget, bElevationLegend ? TEXT("Elevation") : TEXT("Intensity"), 18, MakeSlateColor(1.0f, 1.0f, 1.0f));
	content->AddChild(heading);
	SetVerticalSlotPadding(heading, FMargin(0.0f, 0.0f, 0.0f, 26.0f));

	UHorizontalBox* gradientRow = NewObject<UHorizontalBox>(UIWidget);
	content->AddChild(gradientRow);

	USizeBox* gradientSizeBox = NewObject<USizeBox>(UIWidget);
	gradientSizeBox->SetWidthOverride(36.0f);
	gradientSizeBox->SetHeightOverride(122.0f);
	gradientRow->AddChild(gradientSizeBox);
	SetHorizontalSlotPadding(gradientSizeBox, FMargin(6.0f, 0.0f, 18.0f, 0.0f));

	TArray<FLinearColor> gradientColors;

	if (bElevationLegend)
	{
		gradientColors = {FLinearColor(0.95f, 0.12f, 0.08f), FLinearColor(1.0f, 0.9f, 0.2f), FLinearColor(0.35f, 0.95f, 0.48f),
						  FLinearColor(0.25f, 0.82f, 1.0f), FLinearColor(0.22f, 0.12f, 1.0f)};
	}
	else
	{
		gradientColors = {FLinearColor::White, FLinearColor(0.65f, 0.65f, 0.65f), FLinearColor(0.16f, 0.16f, 0.16f), FLinearColor::Black};
	}

	if (UTexture2D* gradientTexture = CreateLegendGradientTexture(UIWidget, gradientColors))
	{
		LegendTextures.Add(gradientTexture);

		UImage* gradientImage = NewObject<UImage>(UIWidget);
		gradientImage->SetBrushFromTexture(gradientTexture, true);
		gradientSizeBox->AddChild(gradientImage);
	}

	UVerticalBox* labelColumn = NewObject<UVerticalBox>(UIWidget);
	gradientRow->AddChild(labelColumn);
	SetHorizontalSlotPadding(labelColumn, FMargin(0.0f));

	labelColumn->AddChild(CreateText(UIWidget, bElevationLegend ? TEXT("> 3.5") : TEXT("> 65,680"), 18, MakeSlateColor(1.0f, 1.0f, 1.0f)));

	USpacer* topSpacer = NewObject<USpacer>(UIWidget);
	topSpacer->SetSize(FVector2D(1.0f, 31.0f));
	labelColumn->AddChild(topSpacer);

	labelColumn->AddChild(CreateText(UIWidget, bElevationLegend ? TEXT("1.5") : TEXT("38,032"), 18, MakeSlateColor(1.0f, 1.0f, 1.0f)));

	USpacer* bottomSpacer = NewObject<USpacer>(UIWidget);
	bottomSpacer->SetSize(FVector2D(1.0f, 31.0f));
	labelColumn->AddChild(bottomSpacer);

	labelColumn->AddChild(CreateText(UIWidget, bElevationLegend ? TEXT("< -1.5") : TEXT("< 10,385"), 18, MakeSlateColor(1.0f, 1.0f, 1.0f)));
}

void APCLController::ResetFilterSelections(bool bApplyFilters)
{
	TGuardValue<bool> updatingGuard(bUpdatingFilterCheckBoxes, true);

	if (ClassAllCheckBox)
	{
		ClassAllCheckBox->SetIsChecked(true);
	}

	for (TObjectPtr<UCheckBox> checkBox : ClassFilterCheckBoxes)
	{

		if (checkBox)
		{
			checkBox->SetIsChecked(true);
		}
	}

	if (ReturnsAllCheckBox)
	{
		ReturnsAllCheckBox->SetIsChecked(true);
	}

	for (TObjectPtr<UCheckBox> checkBox : ReturnsFilterCheckBoxes)
	{

		if (checkBox)
		{
			checkBox->SetIsChecked(true);
		}
	}

	if (bApplyFilters)
	{
		ApplyPointCloudFilters();
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

void APCLController::SetTabLayout(EPCLTabLayout layout)
{
	CurrentTabLayout = layout;

	float heightOffset = CustomizeTabHeightOffset;

	if (layout == EPCLTabLayout::Visualize)
	{
		heightOffset = VisualizeTabHeightOffset;
	}
	else if (layout == EPCLTabLayout::Filter)
	{
		heightOffset = FilterTabHeightOffset;
	}

	for (const FName& widgetName : ExpandableTabWidgetNames)
	{
		SetNamedWidgetHeightOffset(widgetName, heightOffset);
	}

	ApplyTabUIScale();
	BuildLegendUI();
}

void APCLController::ApplyTabUIScale()
{

	if (!UIWidget)
	{
		return;
	}

	UWidget* mainPanel = UIWidget->GetWidgetFromName(PCLMainPanelWidgetName);

	if (!mainPanel)
	{
		return;
	}

	mainPanel->SetRenderTransformPivot(FVector2D(1.0f, 0.0f));
	mainPanel->SetRenderScale(FVector2D(PCLTabUIScale, PCLTabUIScale));
}

void APCLController::SetNamedWidgetHeightOffset(const FName& widgetName, float heightOffset)
{

	if (!UIWidget)
	{
		return;
	}

	UWidget* widget = UIWidget->GetWidgetFromName(widgetName);

	if (!widget)
	{
		return;
	}

	UCanvasPanelSlot* canvasSlot = Cast<UCanvasPanelSlot>(widget->Slot);

	if (!canvasSlot)
	{
		return;
	}

	if (!CachedTabWidgetSizes.Contains(widgetName))
	{
		CachedTabWidgetSizes.Add(widgetName, canvasSlot->GetSize());
	}

	const FVector2D originalSize = CachedTabWidgetSizes[widgetName];
	canvasSlot->SetSize(FVector2D(originalSize.X, originalSize.Y + heightOffset));
}
