// Copyright 2026 Esri
// Licensed under the Apache License, Version 2.0
// https://www.apache.org/licenses/LICENSE-2.0

#include "SampleGameViewportClient.h"

#include "Framework/Application/SlateApplication.h"
#include "InputKeyEventArgs.h"
#include "Layout/WidgetPath.h"
#include "Widgets/SViewport.h"

bool USampleGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Key.IsMouseButton() && (EventArgs.Event == IE_Pressed || EventArgs.Event == IE_DoubleClick) &&
		FSlateApplication::IsInitialized())
	{
		auto& Slate = FSlateApplication::Get();
		const auto ViewportWidget = GetGameViewportWidget();
		const auto HitPath = Slate.LocateWindowUnderMouse(Slate.GetCursorPos(), Slate.GetInteractiveTopLevelWindows(), true);

		// Passive UMG panels can be hit-testable without handling mouse presses.
		if (ViewportWidget && HitPath.ContainsWidget(ViewportWidget.Get()))
		{
			static const FName UserWidgetType(TEXT("SObjectWidget"));
			for (int32 Index = 0; Index < HitPath.Widgets.Num(); ++Index)
			{
				if (HitPath.Widgets[Index].Widget->GetType() == UserWidgetType)
				{
					return true;
				}
			}
		}
	}

	return Super::InputKey(EventArgs);
}
