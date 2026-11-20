/* Copyright 2022 Esri
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

#include "sample_project.h"
#include "Modules/ModuleManager.h"

#if PLATFORM_WINDOWS
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"
#include "Engine/UserInterfaceSettings.h"
#include "Misc/CoreDelegates.h"
#include "UnrealClient.h"
#include "Widgets/SWindow.h"

class FSampleProjectModule : public FDefaultGameModuleImpl
{
public:
	void StartupModule() override
	{
		BeginFrameHandle = FCoreDelegates::OnBeginFrame.AddStatic(&UpdateGameUIScale);
	}

	void ShutdownModule() override
	{
		FCoreDelegates::OnBeginFrame.Remove(BeginFrameHandle);
	}

private:
	static void UpdateGameUIScale()
	{
		UGameViewportClient* ViewportClient = GEngine ? GEngine->GameViewport : nullptr;
		if (!ViewportClient || !ViewportClient->Viewport)
		{
			return;
		}

		const TSharedPtr<SWindow> Window = ViewportClient->GetWindow();
		const FIntPoint Size = ViewportClient->Viewport->GetSizeXY();
		if (!Window || Size.X <= 0 || Size.Y <= 0)
		{
			return;
		}

		// Leave room for the sample controls and footer in a small viewport.
		constexpr float MinimumLayoutShortSide = 1080.0f;
		const float FitScale = FMath::Min(Size.X, Size.Y) / MinimumLayoutShortSide;
		// The game layer cancels Slate's window DPI, so supply it to UMG explicitly.
		GetMutableDefault<UUserInterfaceSettings>()->ApplicationScale = FMath::Min(Window->GetDPIScaleFactor(), FitScale);
	}

	FDelegateHandle BeginFrameHandle;
};
#else
using FSampleProjectModule = FDefaultGameModuleImpl;
#endif

IMPLEMENT_PRIMARY_GAME_MODULE(FSampleProjectModule, sample_project, "sample_project");
