// Copyright 2026 Esri
// Licensed under the Apache License, Version 2.0
// https://www.apache.org/licenses/LICENSE-2.0

#pragma once

#include "Engine/GameViewportClient.h"
#include "SampleGameViewportClient.generated.h"

UCLASS()
class USampleGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	bool InputKey(const FInputKeyEventArgs& EventArgs) override;
};
