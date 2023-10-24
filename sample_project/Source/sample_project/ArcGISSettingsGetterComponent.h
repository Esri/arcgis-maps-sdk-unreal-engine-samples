// COPYRIGHT 1995-2023 ESRI
// TRADE SECRETS: ESRI PROPRIETARY AND CONFIDENTIAL
// Unpublished material - all rights reserved under the
// Copyright Laws of the United States and applicable international
// laws, treaties, and conventions.
//
// For additional information, contact:
// Attn: Contracts and Legal Department
// Environmental Systems Research Institute, Inc.
// 380 New York Street
// Redlands, California 92373
// USA
//
// email: legal@esri.com
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISMapsSDKProjectSettings.h"
#include "ArcGISSettingsGetterComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SAMPLE_PROJECT_API UArcGISSettingsGetterComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UArcGISSettingsGetterComponent();

	UFUNCTION(BlueprintCallable, Category = "ArcGISMapsSDK|ArcGISDefaultSettings")
	FString GetDefaultAPIKey();

	UFUNCTION(BlueprintCallable, Category = "ArcGISMapsSDK|ArcGISDefaultSettings")
	ECollisionEnabledInEditorWorld GetDefaultCollisionEnabled();
	
	UFUNCTION(BlueprintCallable, Category = "ArcGISMapsSDK|ArcGISDefaultSettings")
	bool GetDefaultUseStaticMesh();
};
