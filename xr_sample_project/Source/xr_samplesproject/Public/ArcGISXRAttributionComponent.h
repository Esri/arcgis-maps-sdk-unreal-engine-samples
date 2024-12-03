// Copyright 2024 Esri.
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License.
// You may obtain a copy of the License at: http://www.apache.org/licenses/LICENSE-2.0
#pragma once

#include "ArcGISMapsSDK/Components/ArcGISActorComponent.h"
#include "Delegates/Delegate.h"

#include "ArcGISXRAttributionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttributionChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class XR_SAMPLESPROJECT_API UArcGISXRAttributionComponent : public UArcGISActorComponent
{
	GENERATED_BODY()

public:
	UArcGISXRAttributionComponent();

	UPROPERTY(BlueprintAssignable)
	FOnAttributionChanged OnAttributionChanged;

protected:
	void OnArcGISMapComponentChanged(UArcGISMapComponent* InMapComponent) override;

private:
	void BroadcastAttributionChange();
};
