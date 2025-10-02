#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialParameterCollection.h"
#include "ViewshedCamera.generated.h"

class UArcGISLocationComponent;

UCLASS()
class SAMPLE_PROJECT_API AViewshedCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	AViewshedCamera();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UArcGISLocationComponent* locationComponent;

private:
	UPROPERTY(EditAnywhere)
	bool bPrintDebubMatrices = false;

	UPROPERTY(EditAnywhere)
	UMaterial* ViewshedMaterial;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UTextureRenderTarget2D> DepthTexture;

	UPROPERTY(EditAnywhere)
	USceneCaptureComponent2D* ViewshedCamera;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ViewshedCameraMesh;

	UPROPERTY(VisibleAnywhere)
	UStaticMesh* mesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Script/Engine.StaticMesh'/Game/SampleViewer/Samples/Viewshed/Icons/camera_3D_icon.camera_3D_icon'"));

	UPROPERTY(VisibleAnywhere)
	UMaterialParameterCollection* GlobalTextureMPC = LoadObject<UMaterialParameterCollection>(nullptr, TEXT("/Script/Engine.MaterialParameterCollection'/Game/SampleViewer/Samples/Viewshed/ViewshedMaterialParameterCollection.ViewshedMaterialParameterCollection'"));

	UPROPERTY(VisibleAnywhere)
	UMaterialInstanceDynamic* MID;

	UPROPERTY(VisibleAnywhere)
	UMaterialParameterCollectionInstance* MPCInstance;

	void LogViewshedMatrix(const TCHAR* Label, const FMatrix& MatrixToLog);

	void SetDepthTextureOnMaterial();
	void SetViewProjectionMatrixOnMaterial();

	FVector LastViewshedCameraPosition;
	FRotator LastViewshedCameraRotation;
};
