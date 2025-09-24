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
#include "Materials/MaterialParameterCollectionInstance.h"

AViewshedCamera::AViewshedCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	ViewshedCamera = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ViewshedCamera"));
	ViewshedCamera->SetupAttachment(RootComponent);
	ViewshedCamera->CaptureSource = ESceneCaptureSource::SCS_SceneDepth;
	ViewshedCamera->OrthoWidth = 5000;

	ViewshedCameraMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ViewshedCameraMesh"));
	ViewshedCameraMesh->SetWorldScale3D(FVector(10));
	ViewshedCameraMesh->SetupAttachment(ViewshedCamera);
	ViewshedCameraMesh->SetStaticMesh(mesh);
}

void AViewshedCamera::BeginPlay()
{
	Super::BeginPlay();

	if (!DepthTexture)
	{
		//CreateDepthTexture();
	}

	//ViewshedCamera->TextureTarget = DepthTexture;
	SetGlobalParameters();
	//UKismetRenderingLibrary::SetGlobalTextureParameter(this, TEXT("_ArcGISViewshedDepthTex"), DepthTexture);
}

void AViewshedCamera::CreateDepthTexture()
{
	DepthTexture = NewObject<UTextureRenderTarget2D>(this);
	//DepthTexture->InitCustomFormat(1024, 1024, PF_R32_FLOAT, true); // Example size and format
	DepthTexture->InitAutoFormat(DepthWidth, DepthHeight);
	DepthTexture->RenderTargetFormat = RTF_R32f;
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

	/*FMatrix ViewProjectionMatrix = ViewshedCamera->GetViewProjectionMatrix();
	FMatrix GPUProjectionMatrix = AdjustProjectionMatrixForRHI(ViewProjectionMatrix);
	UKismetRenderingLibrary::SetGlobalMatrixParameter(this, TEXT("_ArcGISViewshedViewProjectionMatrix"), GPUProjectionMatrix);
	UKismetRenderingLibrary::SetGlobalFloatParameter(this, TEXT("_ArcGISViewshedFarPlane"), ViewshedCamera->FarPlane);
	*/
	GetProjectionMatrix();
	
	LastViewshedCameraPosition = GetActorLocation();
	LastViewshedCameraRotation = GetActorRotation();
}

/*void AViewshedCamera::GetProjectionMatrix()
{
	FMinimalViewInfo viewInfo;
	viewInfo.Location = ViewshedCamera->GetComponentLocation();
	viewInfo.Rotation = ViewshedCamera->GetComponentRotation();
	viewInfo.FOV = ViewshedCamera->FOVAngle;
	
	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;
	FMatrix ViewProjectionMatrix;
	
	UGameplayStatics::GetViewProjectionMatrix(viewInfo, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);
	FMatrix GPUProjectionMatrix = AdjustProjectionMatrixForRHI(ViewProjectionMatrix);
	FMatrix Transposed = GPUProjectionMatrix.GetTransposed();
	
	// Break into rows
	FVector4 Row0(GPUProjectionMatrix.M[0][0], GPUProjectionMatrix.M[0][1], GPUProjectionMatrix.M[0][2], GPUProjectionMatrix.M[0][3]);
	FVector4 Row1(GPUProjectionMatrix.M[1][0], GPUProjectionMatrix.M[1][1], GPUProjectionMatrix.M[1][2], GPUProjectionMatrix.M[1][3]);
	FVector4 Row2(GPUProjectionMatrix.M[2][0], GPUProjectionMatrix.M[2][1], GPUProjectionMatrix.M[2][2], GPUProjectionMatrix.M[2][3]);
	FVector4 Row3(GPUProjectionMatrix.M[3][0], GPUProjectionMatrix.M[3][1], GPUProjectionMatrix.M[3][2], GPUProjectionMatrix.M[3][3]);

	UE_LOG(LogTemp, Warning, TEXT("Row 1: '%s', Row 2: '%s', Row 3: '%s', Row 4: '%s'"), *Row0.ToString(), *Row1.ToString(), *Row2.ToString(), *Row3.ToString());
	
	// Pass to material
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow1"), FLinearColor(Row0.X, Row0.Y, Row0.Z, Row0.W));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow2"), FLinearColor(Row1.X, Row1.Y, Row1.Z, Row1.W));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow3"), FLinearColor(Row2.X, Row2.Y, Row2.Z, Row2.W));
	MPCInstance->SetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow4"), FLinearColor(Row3.X, Row3.Y, Row3.Z, Row3.W));

	// Pass far plane
	MPCInstance->SetScalarParameterValue(TEXT("ArcGISViewshedFarPlane"), ViewshedCamera->MaxViewDistanceOverride);
}*/

void AViewshedCamera::GetProjectionMatrix()
{
	FMinimalViewInfo viewInfo;
	ViewshedCamera->GetCameraView(GetWorld()->GetTimeSeconds(), viewInfo);
	/*viewInfo.Location = ViewshedCamera->GetComponentLocation();
	viewInfo.Rotation = ViewshedCamera->GetComponentRotation();
	viewInfo.FOV = ViewshedCamera->FOVAngle;*/

	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;
	FMatrix ViewProjectionMatrix;

	UGameplayStatics::GetViewProjectionMatrix(viewInfo, ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);
	
	// Adjust for RHI (e.g., DirectX vs OpenGL clip space)
	FMatrix GPUViewProjectionMatrix = AdjustProjectionMatrixForRHI(ViewProjectionMatrix);
	
	// Transpose for HLSL column-major
	FMatrix Transposed = GPUViewProjectionMatrix.GetTransposed();
	
	// Send full rows (with W component) to the material
	auto SendRow = [&](int rowIndex, const TCHAR* paramName)
	{
		MPCInstance->SetVectorParameterValue(
			paramName,
			FLinearColor(
				Transposed.M[rowIndex][0],
				Transposed.M[rowIndex][1],
				Transposed.M[rowIndex][2],
				Transposed.M[rowIndex][3]
			)
		);
	};

	SendRow(0, TEXT("ArcGISViewshedViewProjectionMatrixRow0"));
	SendRow(1, TEXT("ArcGISViewshedViewProjectionMatrixRow1"));
	SendRow(2, TEXT("ArcGISViewshedViewProjectionMatrixRow2"));
	SendRow(3, TEXT("ArcGISViewshedViewProjectionMatrixRow3"));

	MPCInstance->SetScalarParameterValue(TEXT("ArcGISViewshedFarPlane"), ViewshedCamera->MaxViewDistanceOverride);

	FLinearColor OutValue1;
	MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow0"), OutValue1);
	FLinearColor OutValue2;
	MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow1"), OutValue2);
	FLinearColor OutValue3;
	MPCInstance->GetVectorParameterValue(TEXT("ArcGISViewshedViewProjectionMatrixRow2"), OutValue3);
	float OutValue4;
	MPCInstance->GetScalarParameterValue(TEXT("ArcGISViewshedFarPlane"), OutValue4);
	
	UE_LOG(LogTemp, Warning, TEXT("Row 1: '%s', Row 2: '%s', Row 3: '%s', Row 4: '%f'"),
		*OutValue1.ToString(), *OutValue2.ToString(), *OutValue3.ToString(), OutValue4);
	ViewshedCamera->CaptureScene();
}

void AViewshedCamera::SetGlobalParameters()
{
	MPCInstance = GetWorld()->GetParameterCollectionInstance(GlobalTextureMPC);
	MID = UMaterialInstanceDynamic::Create(ViewshedMaterial, this);
	
	if (!MID)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load Material Instance"));
		return;
	}
	
	MID->SetTextureParameterValue(FName("_ArcGISViewshedDepthTex"), DepthTexture);

	UTexture* RetrievedTexture = nullptr;
	MID->GetTextureParameterValue(FName("_ArcGISViewshedDepthTex"), RetrievedTexture);

	UE_LOG(LogTemp, Warning, TEXT("Texture parameter '%s' set correctly."), *DepthTexture->GetName());
}
