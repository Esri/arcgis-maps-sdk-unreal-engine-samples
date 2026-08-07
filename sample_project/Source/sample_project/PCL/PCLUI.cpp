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
#include "Brushes/SlateRoundedBoxBrush.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/WidgetTransform.h"
#include "UObject/UnrealType.h"

#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISPointCloudLayer.h"

namespace PCLUIPrivate
{
constexpr double MaxPointSize = 16.0;
constexpr double MinPointSize = 2.0;
constexpr double MinPointsPerInch = 1.0;
constexpr int32 FilterReturnOptionCount = 4;
constexpr float PCLTabUIScale = 2.0f / 3.0f;
constexpr float CustomizeTabHeightOffset = 88.0f;
constexpr float VisualizeTabHeightOffset = 194.0f;
constexpr float FilterTabHeightOffset = 430.0f;
const FString PointCloudLayerSource =
	TEXT("https://tiles.arcgis.com/tiles/V6ZHFr6zdgNZuVG0/arcgis/rest/services/BARNEGAT_BAY_LiDAR_UTM/SceneServer");
const FName ExpandableTabWidgetNames[] = {TEXT("Background"),
										  TEXT("Switcher_PCLTabs"),
										  TEXT("Panel_VisualizeContent")};
const FName PCLRootCanvasWidgetName(TEXT("CanvasPanel_37"));
const FName PCLMainPanelWidgetName(TEXT("Panel_PCLMain"));
const FName PCLCollapseButtonWidgetName(TEXT("Button_Collapse"));
const FName PCLGearIconWidgetName(TEXT("Button_Gear"));
const FName PCLGearRuntimeIconWidgetName(TEXT("PCL_GearIcon_Runtime"));
const FName PCLInfoWidgetName(TEXT("wbp_Info"));
const FName PCLInfoButtonWidgetName(TEXT("wbp_InfoButton"));
const FName PCLInfoButtonControlName(TEXT("Button_21"));
const FName PCLMenuHiddenPropertyName(TEXT("IsMenuHidden"));
const FVector2D PCLGearButtonSize(48.0f, 48.0f);
const FVector2D PCLGearIconSize(34.0f, 34.0f);
const FLinearColor PCLGearPurple(0.309f, 0.063f, 1.0f, 1.0f);

struct FRendererWidgetNames
{
	EPCLRendererChoice RendererChoice;
	FName Row;
};

const FRendererWidgetNames RendererWidgetNames[] = {
	{EPCLRendererChoice::RGB, TEXT("Row_Checkbox_Renderer_RGB")},
	{EPCLRendererChoice::Class, TEXT("Row_Checkbox_Renderer_Class")},
	{EPCLRendererChoice::Elevation, TEXT("Row_Checkbox_Renderer_Elevation")},
	{EPCLRendererChoice::Intensity, TEXT("Row_Checkbox_Renderer_Intensity")}};

struct FGradientLegendInfo
{
	const TCHAR* Heading;
	const TCHAR* Labels[3];
	TArray<FLinearColor> Colors;
};

const FGradientLegendInfo ElevationLegendInfo = {
	TEXT("Elevation"),
	{TEXT("> 3.5"), TEXT("1.5"), TEXT("< -1.5")},
	{FLinearColor(0.95f, 0.12f, 0.08f),
	 FLinearColor(1.0f, 0.9f, 0.2f),
	 FLinearColor(0.35f, 0.95f, 0.48f),
	 FLinearColor(0.25f, 0.82f, 1.0f),
	 FLinearColor(0.22f, 0.12f, 1.0f)}};
const FGradientLegendInfo IntensityLegendInfo = {
	TEXT("Intensity"),
	{TEXT("> 65,680"), TEXT("38,032"), TEXT("< 10,385")},
	{FLinearColor::White, FLinearColor(0.65f, 0.65f, 0.65f), FLinearColor(0.16f, 0.16f, 0.16f), FLinearColor::Black}};
const int32 VisibleLegendClassCodes[] = {1, 2, 3, 5, 6, 7, 9};

const FRendererWidgetNames* FindRendererWidgetNames(EPCLRendererChoice rendererChoice)
{

	for (const FRendererWidgetNames& names : RendererWidgetNames)
	{

		if (names.RendererChoice == rendererChoice)
		{
			return &names;
		}
	}

	return nullptr;
}

FText FormatSliderValue(float value)
{
	return FText::FromString(FString::Printf(TEXT("%.0f"), value));
}

bool IsValidURL(const FString& source)
{
	return !source.IsEmpty() &&
		   (source.StartsWith(TEXT("https://"), ESearchCase::IgnoreCase) || source.StartsWith(TEXT("http://"), ESearchCase::IgnoreCase));
}

struct FStandardClassColor
{
	uint8 Red;
	uint8 Green;
	uint8 Blue;
};

const FStandardClassColor StandardClassColors[] = {
	{128, 128, 128},
	{190, 137, 12},
	{219, 255, 104},
	{246, 44, 28},
	{244, 102, 32},
	{199, 24, 255},
	{255, 255, 112},
	{152, 152, 152},
	{255, 186, 87},
	{246, 244, 22},
	{209, 98, 224},
	{218, 218, 218},
	{84, 167, 255},
	{255, 121, 198},
	{255, 160, 67},
	{255, 92, 92},
	{136, 255, 218},
	{141, 108, 255},
	{80, 80, 80}};

FStandardClassColor GetStandardClassColor(int32 classValue)
{

	if (classValue >= 0 && classValue < UE_ARRAY_COUNT(StandardClassColors))
	{
		return StandardClassColors[classValue];
	}

	return {128, 128, 128};
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

bool AreAllCheckBoxesChecked(const TArray<TObjectPtr<UCheckBox>>& checkBoxes)
{

	for (const TObjectPtr<UCheckBox>& checkBox : checkBoxes)
	{

		if (!checkBox || !checkBox->IsChecked())
		{
			return false;
		}
	}

	return true;
}

void SetCheckBoxesChecked(const TArray<TObjectPtr<UCheckBox>>& checkBoxes, bool bChecked)
{

	for (const TObjectPtr<UCheckBox>& checkBox : checkBoxes)
	{

		if (checkBox)
		{
			checkBox->SetIsChecked(bChecked);
		}
	}
}

template <typename WidgetType>
WidgetType* FindNamedWidget(UUserWidget* widget, const TCHAR* widgetName)
{
	return widget ? Cast<WidgetType>(widget->GetWidgetFromName(widgetName)) : nullptr;
}

template <typename EventType>
void BindDynamicEvent(EventType& event, UObject* object, const FName& functionName)
{
	FScriptDelegate delegate;
	delegate.BindUFunction(object, functionName);
	event.AddUnique(delegate);
}

void BindButtonClick(UButton* button, UObject* object, const FName& functionName)
{
	BindDynamicEvent(button->OnClicked, object, functionName);
}

void BindCheckStateChanged(UCheckBox* checkBox, UObject* object, const FName& functionName)
{
	BindDynamicEvent(checkBox->OnCheckStateChanged, object, functionName);
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
	FindPCLNamedWidget(uiWidget, PCLCollapseButtonWidgetName)
		->SetVisibility(bCollapsed ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	FindPCLNamedWidget(uiWidget, PCLGearIconWidgetName)
		->SetVisibility(bCollapsed ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

}


void APCLController::InitializePCLUI()
{
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
		PointSizeSlider = PCLUIPrivate::FindNamedWidget<USlider>(UIWidget, TEXT("Slider_PointsSize"));
		PointsPerInchSlider = PCLUIPrivate::FindNamedWidget<USlider>(UIWidget, TEXT("Slider_PointsPerInch"));
		PointSizeValueText = PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_PointSizeValue"));
		PointsPerInchValueText = PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_PointsPerInchValue"));
		UCheckBox* colorModulationCheckBox =
			PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_ColorModulation"));
		RGBRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_RGB"));
		ClassRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Class"));
		ElevationRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Elevation"));
		IntensityRendererCheckBox = PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_Renderer_Intensity"));
		UButton* customizeTabButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_CustomizeTab"));
		UButton* filterTabButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_FilterTab"));
		UButton* visualizeTabButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_VisualizeTab"));
		UButton* resetFiltersButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_ResetFilters"));
		SourceUrlTextBox = PCLUIPrivate::FindNamedWidget<UEditableTextBox>(UIWidget, TEXT("EditableTextBox_0"));
		LoadLayerButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_Load"));
		LoadLayerButtonText = PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_LoadLayer"));
		LegendPanel = PCLUIPrivate::FindNamedWidget<UCanvasPanel>(UIWidget, TEXT("Panel_PCLLegend"));
		UButton* collapseButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_Collapse"));
		UButton* gearButton = PCLUIPrivate::FindNamedWidget<UButton>(UIWidget, TEXT("Button_Gear"));
		UUserWidget* infoWidget = Cast<UUserWidget>(PCLUIPrivate::FindPCLNamedWidget(UIWidget, PCLUIPrivate::PCLInfoWidgetName));
		UUserWidget* infoButtonWidget =
			infoWidget ? Cast<UUserWidget>(infoWidget->GetWidgetFromName(PCLUIPrivate::PCLInfoButtonWidgetName)) : nullptr;
		UButton* infoButton =
			infoButtonWidget ? Cast<UButton>(infoButtonWidget->GetWidgetFromName(PCLUIPrivate::PCLInfoButtonControlName)) : nullptr;
		PCLUIPrivate::ConfigurePCLCollapseToggleAppearance(UIWidget, collapseButton, gearButton);
		LayerLoadStatusText = PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_LayerLoadStatus"));
		UIInteractionPanel = PCLUIPrivate::FindNamedWidget<UWidget>(UIWidget, TEXT("Background"));
		BuildDataLoaderUI();

		SourceUrlTextBox->SetText(FText::FromString(PCLUIPrivate::PointCloudLayerSource));
		PointSizeSlider->SetMinValue(PCLUIPrivate::MinPointSize);
		PointSizeSlider->SetMaxValue(PCLUIPrivate::MaxPointSize);
		PointSizeSlider->SetStepSize(1.0f / static_cast<float>(PCLUIPrivate::MaxPointSize - PCLUIPrivate::MinPointSize));
		PointSizeSlider->SetValue(FMath::Clamp(
			PointSizeSlider->GetValue(),
			static_cast<float>(PCLUIPrivate::MinPointSize),
			static_cast<float>(PCLUIPrivate::MaxPointSize)));
		PointSizeSlider->OnValueChanged.AddDynamic(this, &APCLController::OnPointSizeChanged);
		PointsPerInchSlider->SetMinValue(PCLUIPrivate::MinPointsPerInch);
		PointsPerInchSlider->SetValue(
			FMath::Max(PointsPerInchSlider->GetValue(), static_cast<float>(PCLUIPrivate::MinPointsPerInch)));
		PointsPerInchSlider->OnValueChanged.AddDynamic(this, &APCLController::OnPointsPerInchChanged);
		colorModulationCheckBox->SetIsChecked(bColorModulationEnabled);

		const TPair<UCheckBox*, FName> checkBoxBindings[] = {
			{colorModulationCheckBox, GET_FUNCTION_NAME_CHECKED(APCLController, OnColorModulationCheckStateChanged)},
			{RGBRendererCheckBox, GET_FUNCTION_NAME_CHECKED(APCLController, OnRGBRendererCheckStateChanged)},
			{ClassRendererCheckBox, GET_FUNCTION_NAME_CHECKED(APCLController, OnClassRendererCheckStateChanged)},
			{ElevationRendererCheckBox, GET_FUNCTION_NAME_CHECKED(APCLController, OnElevationRendererCheckStateChanged)},
			{IntensityRendererCheckBox, GET_FUNCTION_NAME_CHECKED(APCLController, OnIntensityRendererCheckStateChanged)}};

		for (const TPair<UCheckBox*, FName>& binding : checkBoxBindings)
		{
			PCLUIPrivate::BindCheckStateChanged(binding.Key, this, binding.Value);
		}

		const TPair<UButton*, FName> buttonBindings[] = {
			{customizeTabButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnCustomizeTabClicked)},
			{filterTabButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnFilterTabClicked)},
			{visualizeTabButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnVisualizeTabClicked)},
			{resetFiltersButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnResetFiltersClicked)},
			{LoadLayerButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnLoadPointCloudLayerClicked)},
			{collapseButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnCollapseButtonClicked)},
			{gearButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnCollapseButtonClicked)},
			{infoButton, GET_FUNCTION_NAME_CHECKED(APCLController, OnInfoButtonClicked)}};

		for (const TPair<UButton*, FName>& binding : buttonBindings)
		{
			PCLUIPrivate::BindButtonClick(binding.Key, this, binding.Value);
		}

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
}

void APCLController::UpdatePCLUI()
{
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

	const bool bShouldShowLegend =
		CurrentTabLayout == EPCLTabLayout::Visualize && !bPCLUICollapsed && mainPanel->IsVisible();
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
		ClassAllCheckBox->SetIsChecked(PCLUIPrivate::AreAllCheckBoxesChecked(ClassFilterCheckBoxes));
	}

	if (ReturnsAllCheckBox)
	{
		ReturnsAllCheckBox->SetIsChecked(PCLUIPrivate::AreAllCheckBoxesChecked(ReturnsFilterCheckBoxes));
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
	PCLUIPrivate::SetCheckBoxesChecked(filterCheckBoxes, bIsChecked);

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
	CachedPCLRootChildVisibilities.Reset();
	PCLUIPrivate::ApplyPCLCollapseToggleVisibility(UIWidget, false);
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
	}

	for (UWidget* child : collapsibleWidgets)
	{

		if (!child || PCLUIPrivate::IsPCLCollapsePersistentWidget(child))
		{
			continue;
		}

		if (bCollapsed)
		{
			CachedPCLRootChildVisibilities.Add(child->GetFName(), child->GetVisibility());
			child->SetVisibility(ESlateVisibility::Collapsed);
		}
		else if (const ESlateVisibility* cachedVisibility = CachedPCLRootChildVisibilities.Find(child->GetFName()))
		{
			child->SetVisibility(*cachedVisibility);
		}
	}

	if (!bCollapsed)
	{
		CachedPCLRootChildVisibilities.Reset();
	}

	bPCLUICollapsed = bCollapsed;
	PCLUIPrivate::ApplyPCLCollapseToggleVisibility(UIWidget, bPCLUICollapsed);
}

void APCLController::TogglePCLUICollapse()
{
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
	LayerLoadStatusText->SetText(FText::GetEmpty());
	LayerLoadStatusText->SetColorAndOpacity(FSlateColor(FLinearColor(0.42f, 0.78f, 0.04f)));
	LayerLoadStatusText->SetVisibility(ESlateVisibility::Hidden);
	LayerLoadStatusText->SetIsEnabled(true);
	LoadLayerButtonText->SetText(FText::FromString(TEXT("Load")));
}

void APCLController::SetLayerLoadStatus(bool bSucceeded) const
{
	LayerLoadStatusText->SetText(FText::FromString(bSucceeded ? TEXT("Layer loaded successfully...") : TEXT("Failed to load point cloud layer!")));
	LayerLoadStatusText->SetColorAndOpacity(FSlateColor(bSucceeded ? FLinearColor(0.42f, 0.78f, 0.04f) : FLinearColor(0.93f, 0.31f, 0.43f)));
	LayerLoadStatusText->SetVisibility(ESlateVisibility::Visible);

	if (!bSucceeded)
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

		for (const PCLUIPrivate::FRendererWidgetNames& names : PCLUIPrivate::RendererWidgetNames)
		{
			SetRendererOptionVisibility(names.RendererChoice, IsRendererAvailableFromCachedAttributes(names.RendererChoice));
		}
	}

	const TPair<UCheckBox*, EPCLRendererChoice> rendererCheckBoxes[] = {
		{RGBRendererCheckBox, EPCLRendererChoice::RGB},
		{ClassRendererCheckBox, EPCLRendererChoice::Class},
		{ElevationRendererCheckBox, EPCLRendererChoice::Elevation},
		{IntensityRendererCheckBox, EPCLRendererChoice::Intensity}};

	for (const TPair<UCheckBox*, EPCLRendererChoice>& rendererCheckBox : rendererCheckBoxes)
	{
		rendererCheckBox.Key->SetIsChecked(CurrentRendererChoice == rendererCheckBox.Value);
	}
}

void APCLController::UpdateColorModulationVisibility()
{
	const ESlateVisibility visibility = IntensityAttributeName.IsEmpty() ? ESlateVisibility::Hidden : ESlateVisibility::Visible;
	const FName widgetNames[] = {TEXT("Row_Checkbox_ColorModulation"), TEXT("Border_VisualizeDivider")};

	for (const FName& widgetName : widgetNames)
	{
		UIWidget->GetWidgetFromName(widgetName)->SetVisibility(visibility);
	}
}

void APCLController::SetRendererOptionVisibility(EPCLRendererChoice rendererChoice, bool bVisible)
{
	const ESlateVisibility visibility = bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed;
	const PCLUIPrivate::FRendererWidgetNames* names = PCLUIPrivate::FindRendererWidgetNames(rendererChoice);
	UIWidget->GetWidgetFromName(names->Row)->SetVisibility(visibility);
}

void APCLController::UpdateSliderValueTexts() const
{
	PointSizeValueText->SetText(PCLUIPrivate::FormatSliderValue(PointSizeSlider->GetValue()));
	PointsPerInchValueText->SetText(PCLUIPrivate::FormatSliderValue(PointsPerInchSlider->GetValue()));
}

void APCLController::BuildFilterTabUI()
{
	ClassFilterCheckBoxes.Reset();
	ClassFilterValues.Reset();
	ReturnsFilterCheckBoxes.Reset();
	ClassAllCheckBox = nullptr;
	ReturnsAllCheckBox = nullptr;

	RefreshAvailablePointCloudAttributes();
	const bool bHasClassCodeFilter = !ClassAttributeName.IsEmpty();
	const bool bHasReturnsFilter = !ReturnsAttributeName.IsEmpty();
	UIWidget->GetWidgetFromName(TEXT("Panel_FilterClassSection"))
		->SetVisibility(bHasClassCodeFilter ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	UIWidget->GetWidgetFromName(TEXT("SizeBox_FilterSectionDivider"))
		->SetVisibility(
			bHasClassCodeFilter && bHasReturnsFilter ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	UIWidget->GetWidgetFromName(TEXT("Panel_FilterReturnsSection"))
		->SetVisibility(bHasReturnsFilter ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	if (!bHasClassCodeFilter && !bHasReturnsFilter)
	{
		ClearActiveFilters();
		return;
	}

	if (bHasClassCodeFilter)
	{
		ClassAllCheckBox =
			PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_FilterClassAll"));
		PCLUIPrivate::BindCheckStateChanged(
			ClassAllCheckBox, this, GET_FUNCTION_NAME_CHECKED(APCLController, OnClassAllFilterCheckStateChanged));

		for (int32 classCode = 0; classCode <= 18; ++classCode)
		{
			UCheckBox* checkBox = Cast<UCheckBox>(
				UIWidget->GetWidgetFromName(FName(*FString::Printf(TEXT("Checkbox_FilterClass_%d"), classCode))));
			PCLUIPrivate::BindCheckStateChanged(
				checkBox, this, GET_FUNCTION_NAME_CHECKED(APCLController, OnFilterCheckStateChanged));
			ClassFilterCheckBoxes.Add(checkBox);
			ClassFilterValues.Add(classCode);
		}
	}

	if (bHasReturnsFilter)
	{
		ReturnsAllCheckBox =
			PCLUIPrivate::FindNamedWidget<UCheckBox>(UIWidget, TEXT("Checkbox_FilterReturnsAll"));
		PCLUIPrivate::BindCheckStateChanged(
			ReturnsAllCheckBox, this, GET_FUNCTION_NAME_CHECKED(APCLController, OnReturnsAllFilterCheckStateChanged));

		for (int32 index = 0; index < PCLUIPrivate::FilterReturnOptionCount; ++index)
		{
			UCheckBox* checkBox = Cast<UCheckBox>(
				UIWidget->GetWidgetFromName(FName(*FString::Printf(TEXT("Checkbox_FilterReturn_%d"), index))));
			PCLUIPrivate::BindCheckStateChanged(
				checkBox, this, GET_FUNCTION_NAME_CHECKED(APCLController, OnFilterCheckStateChanged));
			ReturnsFilterCheckBoxes.Add(checkBox);
		}
	}

	ResetFilterSelections(false);
}

void APCLController::BuildLegendUI()
{
	if (CurrentTabLayout != EPCLTabLayout::Visualize)
	{
		LegendPanel->SetVisibility(ESlateVisibility::Hidden);
		return;
	}

	const bool bCompact = CurrentRendererChoice == EPCLRendererChoice::RGB;
	UIWidget->GetWidgetFromName(TEXT("Panel_LegendCompact"))
		->SetVisibility(bCompact ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	UIWidget->GetWidgetFromName(TEXT("Panel_LegendExpanded"))
		->SetVisibility(bCompact ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (LegendTextures.Num() != UE_ARRAY_COUNT(PCLUIPrivate::VisibleLegendClassCodes) + 2)
	{
		LegendTextures.Reset();

		for (int32 classCode : PCLUIPrivate::VisibleLegendClassCodes)
		{
			const PCLUIPrivate::FStandardClassColor classColorComponents =
				PCLUIPrivate::GetStandardClassColor(classCode);
			const FLinearColor classColor(
				classColorComponents.Red / 255.0f,
				classColorComponents.Green / 255.0f,
				classColorComponents.Blue / 255.0f,
				1.0f);
			UTexture2D* circleTexture = PCLUIPrivate::CreateLegendCircleTexture(UIWidget, classColor);
			LegendTextures.Add(circleTexture);
			UImage* swatch = Cast<UImage>(
				UIWidget->GetWidgetFromName(FName(*FString::Printf(TEXT("Image_LegendClassSwatch_%d"), classCode))));
			swatch->SetBrushFromTexture(circleTexture, true);
		}

		LegendTextures.Add(
			PCLUIPrivate::CreateLegendGradientTexture(UIWidget, PCLUIPrivate::ElevationLegendInfo.Colors));
		LegendTextures.Add(
			PCLUIPrivate::CreateLegendGradientTexture(UIWidget, PCLUIPrivate::IntensityLegendInfo.Colors));
	}

	if (bCompact)
	{
		SyncLegendVisibilityWithMainPanel();
		return;
	}

	FString legendTitle = PointCloudLayer ? PointCloudLayer->GetName() : FString();

	if (legendTitle.TrimStartAndEnd().IsEmpty())
	{
		legendTitle = TEXT("Point cloud layer");
	}

	PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_LegendTitle"))
		->SetText(FText::FromString(legendTitle));
	const bool bClassLegend = CurrentRendererChoice == EPCLRendererChoice::Class;
	UIWidget->GetWidgetFromName(TEXT("Panel_LegendClass"))
		->SetVisibility(bClassLegend ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	UIWidget->GetWidgetFromName(TEXT("Panel_LegendGradient"))
		->SetVisibility(bClassLegend ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (!bClassLegend)
	{
		const bool bElevationLegend = CurrentRendererChoice == EPCLRendererChoice::Elevation;
		const PCLUIPrivate::FGradientLegendInfo& legendInfo =
			bElevationLegend ? PCLUIPrivate::ElevationLegendInfo : PCLUIPrivate::IntensityLegendInfo;
		PCLUIPrivate::FindNamedWidget<UTextBlock>(UIWidget, TEXT("Text_LegendGradientHeading"))
			->SetText(FText::FromString(legendInfo.Heading));

		for (int32 index = 0; index < UE_ARRAY_COUNT(legendInfo.Labels); ++index)
		{
			UTextBlock* label = Cast<UTextBlock>(
				UIWidget->GetWidgetFromName(FName(*FString::Printf(TEXT("Text_LegendGradientLabel_%d"), index))));
			label->SetText(FText::FromString(legendInfo.Labels[index]));
		}

		const int32 gradientTextureIndex =
			UE_ARRAY_COUNT(PCLUIPrivate::VisibleLegendClassCodes) + (bElevationLegend ? 0 : 1);
		PCLUIPrivate::FindNamedWidget<UImage>(UIWidget, TEXT("Image_LegendGradient"))
			->SetBrushFromTexture(LegendTextures[gradientTextureIndex], true);
	}

	SyncLegendVisibilityWithMainPanel();
}

void APCLController::ResetFilterSelections(bool bApplyFilters)
{
	TGuardValue<bool> updatingGuard(bUpdatingFilterCheckBoxes, true);

	if (ClassAllCheckBox)
	{
		ClassAllCheckBox->SetIsChecked(true);
	}

	PCLUIPrivate::SetCheckBoxesChecked(ClassFilterCheckBoxes, true);

	if (ReturnsAllCheckBox)
	{
		ReturnsAllCheckBox->SetIsChecked(true);
	}

	PCLUIPrivate::SetCheckBoxesChecked(ReturnsFilterCheckBoxes, true);

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
	UWidget* mainPanel = UIWidget->GetWidgetFromName(PCLUIPrivate::PCLMainPanelWidgetName);
	mainPanel->SetRenderTransformPivot(FVector2D(1.0f, 0.0f));
	mainPanel->SetRenderScale(FVector2D(PCLUIPrivate::PCLTabUIScale, PCLUIPrivate::PCLTabUIScale));
}

void APCLController::SetNamedWidgetHeightOffset(const FName& widgetName, float heightOffset)
{
	UWidget* widget = UIWidget->GetWidgetFromName(widgetName);
	UCanvasPanelSlot* canvasSlot = Cast<UCanvasPanelSlot>(widget->Slot);

	if (!CachedTabWidgetSizes.Contains(widgetName))
	{
		CachedTabWidgetSizes.Add(widgetName, canvasSlot->GetSize());
	}

	const FVector2D originalSize = CachedTabWidgetSizes[widgetName];
	canvasSlot->SetSize(FVector2D(originalSize.X, originalSize.Y + heightOffset));
}
