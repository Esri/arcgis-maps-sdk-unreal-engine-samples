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

	FMatrix ViewMatrix;              // World -> View
	FMatrix ProjectionMatrix;        // CPU-side projection (unadjusted for RHI)
	FMatrix CombinedCPUViewProj;     // Returned by helper (unadjusted GPU differences)

	UGameplayStatics::GetViewProjectionMatrix(viewInfo, ViewMatrix, ProjectionMatrix, CombinedCPUViewProj);

	// Apply RHI/platform adjustments (Y flip, reversed Z, etc.) to match actual GPU usage.
	FMatrix AdjustedProjectionMatrix = AdjustProjectionMatrixForRHI(ProjectionMatrix);

	// We keep the material custom expression style: clipPos = mul(VP, float4(WorldPos,1)).
	// That implies VP should be constructed as Projection * View (column-vector convention).
	// (Our HLSL: mul(VP, v) treats v as a column vector on the right.)
	FMatrix GPUViewProjectionMatrix = AdjustedProjectionMatrix * ViewMatrix;

	auto MakeRow = [&](int r)
	{
		return FLinearColor(
			GPUViewProjectionMatrix.M[r][0],
			GPUViewProjectionMatrix.M[r][1],
			GPUViewProjectionMatrix.M[r][2],
			GPUViewProjectionMatrix.M[r][3]
		);
	};

	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow1"), MakeRow(0));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow2"), MakeRow(1));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow3"), MakeRow(2));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow4"), MakeRow(3));

	MPCInstance->SetScalarParameterValue(TEXT("ArcGISViewshedFarPlane"), ViewshedCamera->MaxViewDistanceOverride);

	auto RowToString = [](const FMatrix& M, int r)
	{
		return FString::Printf(TEXT("[% .6f, % .6f, % .6f, % .6f]"), M.M[r][0], M.M[r][1], M.M[r][2], M.M[r][3]);
	};

	UE_LOG(LogTemp, Warning, TEXT("Raw CPU ViewProj (pre-RHI):\nR0 %s\nR1 %s\nR2 %s\nR3 %s"),
		*RowToString(CombinedCPUViewProj,0), *RowToString(CombinedCPUViewProj,1), *RowToString(CombinedCPUViewProj,2), *RowToString(CombinedCPUViewProj,3));

	UE_LOG(LogTemp, Warning, TEXT("Adjusted GPU ViewProj (Projection*View):\nR0 %s\nR1 %s\nR2 %s\nR3 %s"),
		*RowToString(GPUViewProjectionMatrix,0), *RowToString(GPUViewProjectionMatrix,1), *RowToString(GPUViewProjectionMatrix,2), *RowToString(GPUViewProjectionMatrix,3));

	FLinearColor OutValue1; MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow1"), OutValue1);
	FLinearColor OutValue2; MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow2"), OutValue2);
	FLinearColor OutValue3; MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow3"), OutValue3);
	FLinearColor OutValue4; MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow4"), OutValue4);

	UE_LOG(LogTemp, Warning, TEXT("Stored GPU VP Rows:\nR1 %s\nR2 %s\nR3 %s\nR4 %s"), *OutValue1.ToString(), *OutValue2.ToString(), *OutValue3.ToString(), *OutValue4.ToString());

	ViewshedCamera->CaptureScene();
}
