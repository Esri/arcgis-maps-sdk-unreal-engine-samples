/// COPYRIGHT 1995-2023 ESRI
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
#include "ArcGISSettingsGetterComponent.h"

// Sets default values for this component's properties
UArcGISSettingsGetterComponent::UArcGISSettingsGetterComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

FString UArcGISSettingsGetterComponent::GetDefaultAPIKey()
{
	if (auto settings = GetDefault<UArcGISMapsSDKProjectSettings>())
	{
		return settings->APIKey;
	}
	return FString();
}

ECollisionEnabledInEditorWorld UArcGISSettingsGetterComponent::GetDefaultCollisionEnabled()
{
	if (auto settings = GetDefault<UArcGISMapsSDKProjectSettings>())
	{
		return settings->CollisionEnabledInEditorWorld;
	}
	return ECollisionEnabledInEditorWorld::HonorMapSettings;
}
