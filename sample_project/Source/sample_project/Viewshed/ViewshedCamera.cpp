/* Copyright 2025 Esri
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
#include "ViewshedCamera.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/World.h"
#include "ShaderParameterUtils.h"

AViewshedCamera::AViewshedCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	ViewshedCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ViewshedCamera"));
	ViewshedCamera->SetupAttachment(RootComponent);
	ViewshedCamera->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
}

void AViewshedCamera::BeginPlay()
{
	Super::BeginPlay();

	if (!DepthTexture)
	{
		CreateDepthTexture();
	}

	ViewshedCamera->TextureTarget = DepthTexture;
	//UKismetRenderingLibrary::SetGlobalTextureParameter(this, TEXT("_ArcGISViewshedDepthTex"), DepthTexture);
}

void AViewshedCamera::CreateDepthTexture()
{
	DepthTexture = NewObject<UTextureRenderTarget2D>(this);
	DepthTexture->InitAutoFormat(DepthWidth, DepthHeight);
	DepthTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_R32f;
	DepthTexture->bAutoGenerateMips = false;
	DepthTexture->Filter = TF_Bilinear;
	DepthTexture->bForceLinearGamma = true;
	DepthTexture->UpdateResourceImmediate(true);
}

void AViewshedCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ViewshedMaterial)
	{
		return;
	}

	if (LastViewshedCameraPosition == GetActorLocation() && LastViewshedCameraRotation == GetActorRotation())
	{
		return;
	}

	//FMatrix ViewProjectionMatrix = ViewshedCamera->GetViewProjectionMatrix();
	//FMatrix GPUProjectionMatrix = AdjustProjectionMatrixForRHI(ViewProjectionMatrix);
	//UKismetRenderingLibrary::SetGlobalMatrixParameter(this, TEXT("_ArcGISViewshedViewProjectionMatrix"), GPUProjectionMatrix);
	//UKismetRenderingLibrary::SetGlobalFloatParameter(this, TEXT("_ArcGISViewshedFarPlane"), ViewshedCamera->FarPlane);

	LastViewshedCameraPosition = GetActorLocation();
	LastViewshedCameraRotation = GetActorRotation();
}
