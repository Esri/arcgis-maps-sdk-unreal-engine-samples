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
#include "InputCoreTypes.h"
#include "Slate/WidgetTransform.h"
#include "UObject/UnrealType.h"

#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISPointCloudLayer.h"

#include "sample_project/InputManager.h"

namespace PCLUIPrivate
{
constexpr double MaxPointSize = 16.0;
constexpr double MinPointSize = 2.0;
constexpr double MinPointsPerInch = 1.0;
constexpr float LegendCompactWidth = 345.0f;
constexpr float LegendCompactHeight = 76.0f;
constexpr float LegendExpandedWidth = 480.0f;
constexpr float LegendExpandedHeight = 382.0f;
constexpr float LegendExpandedContentWidth = LegendExpandedWidth - 82.0f;
constexpr float PCLTabUIScale = 2.0f / 3.0f;
constexpr float CustomizeTabHeightOffset = 88.0f;
constexpr float VisualizeTabHeightOffset = 194.0f;
constexpr float FilterTabHeightOffset = 430.0f;
const FString PointCloudLayerSource =
	TEXT("https://tiles.arcgis.com/tiles/V6ZHFr6zdgNZuVG0/arcgis/rest/services/BARNEGAT_BAY_LiDAR_UTM/SceneServer");
const FName ExpandableTabWidgetNames[] = {TEXT("CanvasPanel_37"),  TEXT("Background"),		TEXT("Switcher_PCLTabs"),
										  TEXT("Panel_Customize"), TEXT("Panel_Visualize"), TEXT("Panel_VisualizeContent"),
										  TEXT("Panel_Filter")};
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
	FSlateFontInfo font = textBlock->GetFont();
	font.FontObject =
		LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-Regular_Font.ChakraPetch-Regular_Font"));
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
	UObject* fontObject =
		LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-SemiBold_Font.ChakraPetch-SemiBold_Font"));
	FSlateFontInfo font = title->GetFont();
	font.FontObject = fontObject;
	font.Size = 24;
	title->SetFont(font);
}

void ApplyChakraPetchSemiBoldFont(UTextBlock* textBlock, int32 fontSize)
{
	UObject* fontObject =
		LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-SemiBold_Font.ChakraPetch-SemiBold_Font"));
	FSlateFontInfo font = textBlock->GetFont();
	font.FontObject = fontObject;
	font.Size = fontSize;
	textBlock->SetFont(font);
}

void ApplyChakraPetchRegularFont(UTextBlock* textBlock, int32 fontSize)
{
	UObject* fontObject =
		LoadObject<UObject>(nullptr, TEXT("/Game/SampleViewer/User-Interface/Fonts/ChakraPetch-Regular_Font.ChakraPetch-Regular_Font"));
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
WidgetType* FindNamedWidget(UUserWidget* widget, const TCHAR* widgetName)
{
	return widget ? Cast<WidgetType>(widget->GetWidgetFromName(widgetName)) : nullptr;
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

	UTexture2D* gearTexture = LoadObject<UTexture2D>(nullptr, TEXT("/Game/SampleViewer/User-Interface/gear_icon.gear_icon"));
	gearIcon->SetBrushFromTexture(gearTexture, true);
	gearIcon->SetDesiredSizeOverride(PCLGearIconSize);
	gearIcon->SetColorAndOpacity(FLinearColor::White);
	gearIcon->SetVisibility(ESlateVisibility::HitTestInvisible);
	button->SetContent(gearIcon);
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


void APCLController::InitializePCLUI()
{
	if (!InputManager)
	{
		InputManager = Cast<AInputManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AInputManager::StaticClass()));
	}

	if (InputManager)
	{
		InputManager->OnInputTrigger.AddDynamic(this, &APCLController::OnInputTriggered);
		InputManager->OnInputEnd.AddDynamic(this, &APCLController::OnInputEnded);
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
		UnitDropdown = PCLUIPrivate::FindNamedWidget<UComboBoxString>(UIWidget, TEXT("UnitDropDown"));
		PointSizeSlider = PCLUIPrivate::FindNamedWidget<USlider>(UIWidget, TEXT("Slider_PointsSize"));
		PointsPerInchSlider = PCLUIPrivate::FindNamedWidget<USlider>(UIWidget, TEXT("Slider_PointsPerInch"));
		PointSizeValueText = PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_PointSizeValue"));
		PointsPerInchValueText = PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_PointsPerInchValue"));
		ColorModulationCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_ColorModulation"));
		RGBRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_RGB"));
		ClassRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Class"));
		ElevationRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Elevation"));
		IntensityRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Intensity"));
		CustomizeTabButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_CustomizeTab"));
		FilterTabButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_FilterTab"));
		VisualizeTabButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_VisualizeTab"));
		FilterPanel = PCLUIPrivate::FindNamedWidget<UPanelWidget>(UIWidget, TEXT("Panel_FilterDynamicContent"));
		ResetFiltersButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_ResetFilters"));
		SourceUrlTextBox = PCLUIPrivate::FindNamedWidget<UEditableTextBox>(UIWidget, TEXT("EditableTextBox_0"));
		LoadLayerButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_Load"));
		CollapseButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_Collapse"));
		GearButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_Gear"));
		UUserWidget* infoWidget = Cast<UUserWidget>(PCLUIPrivate::FindPCLNamedWidget(UIWidget, PCLUIPrivate::PCLInfoWidgetName));
		UUserWidget* infoButtonWidget =
			infoWidget ? Cast<UUserWidget>(infoWidget->GetWidgetFromName(PCLUIPrivate::PCLInfoButtonWidgetName)) : nullptr;
		InfoButton =
			infoButtonWidget ? Cast<UButton>(infoButtonWidget->GetWidgetFromName(PCLUIPrivate::PCLInfoButtonControlName)) : nullptr;
		PCLUIPrivate::ConfigurePCLCollapseToggleAppearance(UIWidget, CollapseButton, GearButton);
		LayerLoadStatusText = PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_LayerLoadStatus"));
		UIInteractionPanel = PCLUIPrivate::FindNamedWidget<UWidget>(UIWidget, TEXT("Background"));
		BuildDataLoaderUI();

		if (SourceUrlTextBox)
		{
			SourceUrlTextBox->SetText(FText::FromString(PCLUIPrivate::PointCloudLayerSource));
		}

		if (LayerLoadStatusText)
		{
			LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
		}

		if (PointSizeSlider)
		{
			PointSizeSlider->SetMinValue(PCLUIPrivate::MinPointSize);
			PointSizeSlider->SetMaxValue(PCLUIPrivate::MaxPointSize);
			PointSizeSlider->SetStepSize(1.0f / static_cast<float>(PCLUIPrivate::MaxPointSize - PCLUIPrivate::MinPointSize));
			PointSizeSlider->SetValue(FMath::Clamp(PointSizeSlider->GetValue(), static_cast<float>(PCLUIPrivate::MinPointSize), static_cast<float>(PCLUIPrivate::MaxPointSize)));
			PointSizeSlider->OnValueChanged.AddDynamic(this, &APCLController::OnPointSizeChanged);
		}

		if (PointsPerInchSlider)
		{
			PointsPerInchSlider->SetMinValue(PCLUIPrivate::MinPointsPerInch);
			PointsPerInchSlider->SetValue(FMath::Max(PointsPerInchSlider->GetValue(), static_cast<float>(PCLUIPrivate::MinPointsPerInch)));
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

		InfoButton->OnClicked.AddDynamic(this, &APCLController::OnInfoButtonClicked);

		UpdateSliderValueTexts();
		UpdateRendererCheckBoxes();
		BuildFilterTabUI();
		SetTabLayout(EPCLTabLayout::Default);
		ConfigurePCLCollapseInitialState();

		if (UIWidget->FindFunction("ShowInstruction"))
		{
			UIWidget->ProcessEvent(UIWidget->FindFunction("ShowInstruction"), nullptr);
		}
	}
}

void APCLController::ShutdownPCLUI()
{
	SetMapInputBlockedByUI(false);

	if (InputManager)
	{
		InputManager->OnInputTrigger.RemoveDynamic(this, &APCLController::OnInputTriggered);
		InputManager->OnInputEnd.RemoveDynamic(this, &APCLController::OnInputEnded);
	}
}

void APCLController::UpdatePCLUI()
{
	HandlePCLCollapseInput();
	SyncLegendVisibilityWithMainPanel();
	UpdateMapInputForUIHover();
}

void APCLController::SyncLegendVisibilityWithMainPanel() const
{

	if (!UIWidget || !LegendPanel)
	{
		return;
	}

	const UWidget* mainPanel = PCLUIPrivate::FindPCLNamedWidget(UIWidget, PCLUIPrivate::PCLMainPanelWidgetName);

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

	if (!PCLUIPrivate::IsValidURL(source))
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
	PCLUIPrivate::FindPCLNamedWidget(UIWidget, PCLUIPrivate::PCLMainPanelWidgetName)->SetVisibility(ESlateVisibility::Hidden);

	if (LegendPanel)
	{
		LegendPanel->SetVisibility(ESlateVisibility::Hidden);
	}

	FBoolProperty* isMenuHiddenProperty =
		FindFProperty<FBoolProperty>(UIWidget->GetClass(), PCLUIPrivate::PCLMenuHiddenPropertyName);
	isMenuHiddenProperty->SetPropertyValue_InContainer(UIWidget, true);
}

void APCLController::ConfigurePCLCollapseInitialState()
{
	bPCLUICollapsed = false;
	bPointerDownOverPCLCollapseToggle = false;
	LastPCLCollapseToggleTimeSeconds = -1.0;
	CachedPCLRootChildVisibilities.Reset();
	PCLUIPrivate::ApplyPCLCollapseToggleVisibility(UIWidget, false);
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

	UCanvasPanel* rootCanvas = Cast<UCanvasPanel>(PCLUIPrivate::FindPCLNamedWidget(UIWidget, PCLUIPrivate::PCLRootCanvasWidgetName));

	if (!rootCanvas)
	{
		return;
	}

	TArray<UWidget*> collapsibleWidgets;

	for (int32 childIndex = 0; childIndex < rootCanvas->GetChildrenCount(); ++childIndex)
	{
		UWidget* child = rootCanvas->GetChildAt(childIndex);

		if (child && child->GetFName() == PCLUIPrivate::PCLMainPanelWidgetName)
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

			if (!child || PCLUIPrivate::IsPCLCollapsePersistentWidget(child))
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

			if (!child || PCLUIPrivate::IsPCLCollapsePersistentWidget(child))
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
	PCLUIPrivate::ApplyPCLCollapseToggleVisibility(UIWidget, bPCLUICollapsed);
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
	return PCLUIPrivate::IsWidgetUnderCursor(PCLUIPrivate::FindPCLNamedWidget(UIWidget, PCLUIPrivate::PCLCollapseButtonWidgetName)) ||
		   PCLUIPrivate::IsWidgetUnderCursor(PCLUIPrivate::FindPCLNamedWidget(UIWidget, PCLUIPrivate::PCLGearIconWidgetName));
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
		LayerLoadStatusText->SetColorAndOpacity(PCLUIPrivate::MakeSlateColor(0.42f, 0.78f, 0.04f));
		LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
		LayerLoadStatusText->SetIsEnabled(true);
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
		LoadLayerButtonText->SetColorAndOpacity(PCLUIPrivate::MakeSlateColor(1.0f, 1.0f, 1.0f));
		PCLUIPrivate::ApplyChakraPetchSemiBoldFont(LoadLayerButtonText, 20);

		if (auto* buttonSlot = Cast<UButtonSlot>(LoadLayerButtonText->Slot))
		{
			buttonSlot->SetHorizontalAlignment(HAlign_Center);
			buttonSlot->SetVerticalAlignment(VAlign_Center);
		}
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
		PointSizeValueText->SetText(PCLUIPrivate::FormatSliderValue(PointSizeSlider->GetValue()));
	}

	if (PointsPerInchValueText && PointsPerInchSlider)
	{
		PointsPerInchValueText->SetText(PCLUIPrivate::FormatSliderValue(PointsPerInchSlider->GetValue()));
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
		UCheckBox* checkBox = PCLUIPrivate::AddCheckBoxRow(UIWidget, scrollBox, label, true);
		checkBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnFilterCheckStateChanged);
		filterCheckBoxes.Add(checkBox);
	};

	if (bHasClassCodeFilter)
	{
		const PCLUIPrivate::FFilterSectionWidgets classSection = PCLUIPrivate::AddFilterSection(UIWidget, content, TEXT("Class Code"), 242.0f);
		ClassAllCheckBox = classSection.AllCheckBox;
		ClassAllCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnClassAllFilterCheckStateChanged);

		for (int32 classCode = 0; classCode <= 18; ++classCode)
		{
			addFilterOption(classSection.ScrollBox, PCLUIPrivate::GetClassCodeLabel(classCode), ClassFilterCheckBoxes);
			ClassFilterValues.Add(classCode);
		}
	}

	if (bHasClassCodeFilter && bHasReturnsFilter)
	{
		PCLUIPrivate::AddFilterSectionDivider(UIWidget, content);
	}

	if (bHasReturnsFilter)
	{
		const PCLUIPrivate::FFilterSectionWidgets returnsSection = PCLUIPrivate::AddFilterSection(UIWidget, content, TEXT("Returns"), 167.0f);
		ReturnsAllCheckBox = returnsSection.AllCheckBox;
		ReturnsAllCheckBox->OnCheckStateChanged.AddDynamic(this, &APCLController::OnReturnsAllFilterCheckStateChanged);

		for (int32 index = 0; index < UE_ARRAY_COUNT(PCLUIPrivate::FilterReturnLabels); ++index)
		{
			addFilterOption(returnsSection.ScrollBox, PCLUIPrivate::FilterReturnLabels[index], ReturnsFilterCheckBoxes);
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
		return;
	}

	const bool bCompact = CurrentRendererChoice == EPCLRendererChoice::RGB;
	const FVector2D legendSize = bCompact ? FVector2D(PCLUIPrivate::LegendCompactWidth, PCLUIPrivate::LegendCompactHeight) : FVector2D(PCLUIPrivate::LegendExpandedWidth, PCLUIPrivate::LegendExpandedHeight);

	LegendPanel = NewObject<UCanvasPanel>(UIWidget);
	LegendPanel->SetRenderTransformPivot(FVector2D(1.0f, 1.0f));
	LegendPanel->SetRenderScale(FVector2D(PCLUIPrivate::PCLTabUIScale, PCLUIPrivate::PCLTabUIScale));
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

	UBorder* background = PCLUIPrivate::CreateColorBlock(UIWidget, FLinearColor(0.03f, 0.03f, 0.035f, 0.88f));
	UCanvasPanelSlot* backgroundSlot = LegendPanel->AddChildToCanvas(background);

	if (backgroundSlot)
	{
		backgroundSlot->SetPosition(FVector2D::ZeroVector);
		backgroundSlot->SetSize(legendSize);
	}

	UBorder* accent = PCLUIPrivate::CreateColorBlock(UIWidget, FLinearColor(0.58f, 0.23f, 1.0f, 1.0f));
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
		contentSlot->SetSize(bCompact ? FVector2D(290.0f, 44.0f) : FVector2D(PCLUIPrivate::LegendExpandedContentWidth, 321.0f));
	}

	if (CurrentRendererChoice == EPCLRendererChoice::RGB)
	{
		content->AddChild(PCLUIPrivate::CreateText(UIWidget, TEXT("No legend"), 28, PCLUIPrivate::MakeSlateColor(0.78f, 0.78f, 0.82f)));
		return;
	}

	FString legendTitle = PointCloudLayer ? PointCloudLayer->GetName() : FString();

	if (legendTitle.TrimStartAndEnd().IsEmpty())
	{
		legendTitle = TEXT("Point cloud layer");
	}

	UTextBlock* title = PCLUIPrivate::CreateText(UIWidget, legendTitle, 24, PCLUIPrivate::MakeSlateColor(0.62f, 0.62f, 0.66f));
	PCLUIPrivate::ApplyLegendTitleFont(title);
	title->SetAutoWrapText(true);
	title->SetWrapTextAt(PCLUIPrivate::LegendExpandedContentWidth);
	title->SetRenderTranslation(FVector2D(-22.0f, 0.0f));
	content->AddChild(title);
	PCLUIPrivate::SetVerticalSlotPadding(title, FMargin(0.0f, 0.0f, 0.0f, 36.0f));

	if (CurrentRendererChoice == EPCLRendererChoice::Class)
	{
		UTextBlock* heading = PCLUIPrivate::CreateText(UIWidget, TEXT("Class Code"), 18, PCLUIPrivate::MakeSlateColor(1.0f, 1.0f, 1.0f));
		content->AddChild(heading);
		PCLUIPrivate::SetVerticalSlotPadding(heading, FMargin(0.0f, 0.0f, 0.0f, 8.0f));

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
			const PCLUIPrivate::FStandardClassInfo classInfo = PCLUIPrivate::GetStandardClassInfo(classCode);
			const FLinearColor classColor(
				classInfo.Red / 255.0f, classInfo.Green / 255.0f, classInfo.Blue / 255.0f, 1.0f);
			UTexture2D* circleTexture = PCLUIPrivate::CreateLegendCircleTexture(UIWidget, classColor);

			if (circleTexture)
			{
				LegendTextures.Add(circleTexture);
			}
			PCLUIPrivate::AddLegendRow(UIWidget, classList, classInfo.Label, classColor, circleTexture);
		}

		return;
	}

	const bool bElevationLegend = CurrentRendererChoice == EPCLRendererChoice::Elevation;
	UTextBlock* heading = PCLUIPrivate::CreateText(UIWidget, bElevationLegend ? TEXT("Elevation") : TEXT("Intensity"), 18, PCLUIPrivate::MakeSlateColor(1.0f, 1.0f, 1.0f));
	content->AddChild(heading);
	PCLUIPrivate::SetVerticalSlotPadding(heading, FMargin(0.0f, 0.0f, 0.0f, 26.0f));

	UHorizontalBox* gradientRow = NewObject<UHorizontalBox>(UIWidget);
	content->AddChild(gradientRow);

	USizeBox* gradientSizeBox = NewObject<USizeBox>(UIWidget);
	gradientSizeBox->SetWidthOverride(36.0f);
	gradientSizeBox->SetHeightOverride(122.0f);
	gradientRow->AddChild(gradientSizeBox);
	PCLUIPrivate::SetHorizontalSlotPadding(gradientSizeBox, FMargin(6.0f, 0.0f, 18.0f, 0.0f));

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

	if (UTexture2D* gradientTexture = PCLUIPrivate::CreateLegendGradientTexture(UIWidget, gradientColors))
	{
		LegendTextures.Add(gradientTexture);

		UImage* gradientImage = NewObject<UImage>(UIWidget);
		gradientImage->SetBrushFromTexture(gradientTexture, true);
		gradientSizeBox->AddChild(gradientImage);
	}

	UVerticalBox* labelColumn = NewObject<UVerticalBox>(UIWidget);
	gradientRow->AddChild(labelColumn);
	PCLUIPrivate::SetHorizontalSlotPadding(labelColumn, FMargin(0.0f));

	labelColumn->AddChild(PCLUIPrivate::CreateText(UIWidget, bElevationLegend ? TEXT("> 3.5") : TEXT("> 65,680"), 18, PCLUIPrivate::MakeSlateColor(1.0f, 1.0f, 1.0f)));

	USpacer* topSpacer = NewObject<USpacer>(UIWidget);
	topSpacer->SetSize(FVector2D(1.0f, 31.0f));
	labelColumn->AddChild(topSpacer);

	labelColumn->AddChild(PCLUIPrivate::CreateText(UIWidget, bElevationLegend ? TEXT("1.5") : TEXT("38,032"), 18, PCLUIPrivate::MakeSlateColor(1.0f, 1.0f, 1.0f)));

	USpacer* bottomSpacer = NewObject<USpacer>(UIWidget);
	bottomSpacer->SetSize(FVector2D(1.0f, 31.0f));
	labelColumn->AddChild(bottomSpacer);

	labelColumn->AddChild(PCLUIPrivate::CreateText(UIWidget, bElevationLegend ? TEXT("< -1.5") : TEXT("< 10,385"), 18, PCLUIPrivate::MakeSlateColor(1.0f, 1.0f, 1.0f)));
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

void APCLController::SetTabLayout(EPCLTabLayout layout)
{
	CurrentTabLayout = layout;

	float heightOffset = PCLUIPrivate::CustomizeTabHeightOffset;

	if (layout == EPCLTabLayout::Visualize)
	{
		heightOffset = PCLUIPrivate::VisualizeTabHeightOffset;
	}
	else if (layout == EPCLTabLayout::Filter)
	{
		heightOffset = PCLUIPrivate::FilterTabHeightOffset;
	}

	for (const FName& widgetName : PCLUIPrivate::ExpandableTabWidgetNames)
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

	UWidget* mainPanel = UIWidget->GetWidgetFromName(PCLUIPrivate::PCLMainPanelWidgetName);

	if (!mainPanel)
	{
		return;
	}

	mainPanel->SetRenderTransformPivot(FVector2D(1.0f, 0.0f));
	mainPanel->SetRenderScale(FVector2D(PCLUIPrivate::PCLTabUIScale, PCLUIPrivate::PCLTabUIScale));
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
