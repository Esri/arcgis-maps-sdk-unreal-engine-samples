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
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "ShaderParameterUtils.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Materials/MaterialParameterCollectionInstance.h"

AViewshedCamera::AViewshedCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	ViewshedCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ViewshedCamera"));
	ViewshedCamera->SetupAttachment(RootComponent);

	locationComponent = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("LocationComponent"));
	locationComponent->SetupAttachment(RootComponent);
	
	ViewshedCameraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ViewshedCameraMesh"));
	ViewshedCameraMesh->SetupAttachment(ViewshedCamera);
	ViewshedCameraMesh->SetStaticMesh(mesh);
}

void AViewshedCamera::BeginPlay()
{
	Super::BeginPlay();

	SetDepthTextureOnMaterial();

	SetViewProjectionMatrixOnMaterial();
}

void AViewshedCamera::SetDepthTextureOnMaterial()
{
	MPCInstance = GetWorld()->GetParameterCollectionInstance(GlobalTextureMPC);
	MID = UMaterialInstanceDynamic::Create(ViewshedMaterial, this);
	
	if (!MID)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Material Instance"));
		return;
	}
	
	MID->SetTextureParameterValue(FName("_ArcGISViewshedDepthTex"), DepthTexture);

	UE_LOG(LogTemp, Warning, TEXT("Texture parameter '%s' set correctly."), *DepthTexture->GetName());
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
	
	SetViewProjectionMatrixOnMaterial();

	LastViewshedCameraPosition = GetActorLocation();
	LastViewshedCameraRotation = GetActorRotation();
}

void AViewshedCamera::SetViewProjectionMatrixOnMaterial()
{
	FMinimalViewInfo viewInfo;
	viewInfo.Location = ViewshedCamera->GetComponentLocation();
	viewInfo.Rotation = ViewshedCamera->GetComponentRotation();
	viewInfo.FOV = ViewshedCamera->FOVAngle;

	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;
	FMatrix ViewProjectionMatrix;

	UGameplayStatics::GetViewProjectionMatrix(viewInfo, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);

	FMatrix AdjustedProjectionMatrix = AdjustProjectionMatrixForRHI(ProjectionMatrix);
	FMatrix GPUViewProjectionMatrix = ViewMatrix * AdjustedProjectionMatrix;

	auto MakeRow = [&](int r)
	{
		return FLinearColor(
			GPUViewProjectionMatrix.M[r][0],
			GPUViewProjectionMatrix.M[r][1],
			GPUViewProjectionMatrix.M[r][2],
			GPUViewProjectionMatrix.M[r][3]
		);
	};

	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow1"), MakeRow(1));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow2"), MakeRow(2));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow3"), MakeRow(3));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow4"), MakeRow(0));

	MPCInstance->SetScalarParameterValue(TEXT("ArcGISViewshedFarPlane"), ViewshedCamera->MaxViewDistanceOverride);

	FLinearColor OutValue1;
	MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow1"), OutValue1);
	FLinearColor OutValue2;
	MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow2"), OutValue2);
	FLinearColor OutValue3;
	MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow3"), OutValue3);
	FLinearColor OutValue4;
	MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow4"), OutValue4);
	
	UE_LOG(LogTemp, Warning, TEXT("Row 1: '%s', Row 2: '%s', Row 3: '%s', Row 4: '%s'"), *OutValue1.ToString(), *OutValue2.ToString(), *OutValue3.ToString(), *OutValue4.ToString());

	ViewshedCamera->CaptureScene();
}
