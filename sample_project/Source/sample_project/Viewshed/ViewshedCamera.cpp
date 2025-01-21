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
	UKismetRenderingLibrary::SetGlobalTextureParameter(this, TEXT("_ArcGISViewshedDepthTex"), DepthTexture);
}

void AViewshedCamera::CreateDepthTexture()
{
	DepthTexture = NewObject<UTextureRenderTarget2D>(this);
	DepthTexture->InitAutoFormat(DepthWidth, DepthHeight);
	DepthTexture->RenderTargetFormat = ETextureRenderTargetFormat::RTF_Depth;
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

	FMatrix ViewProjectionMatrix = ViewshedCamera->GetViewProjectionMatrix();
	FMatrix GPUProjectionMatrix = AdjustProjectionMatrixForRHI(ViewProjectionMatrix);
	UKismetRenderingLibrary::SetGlobalMatrixParameter(this, TEXT("_ArcGISViewshedViewProjectionMatrix"), GPUProjectionMatrix);
	UKismetRenderingLibrary::SetGlobalFloatParameter(this, TEXT("_ArcGISViewshedFarPlane"), ViewshedCamera->FarPlane);

	LastViewshedCameraPosition = GetActorLocation();
	LastViewshedCameraRotation = GetActorRotation();
}
