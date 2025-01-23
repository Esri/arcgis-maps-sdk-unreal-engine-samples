#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ViewshedCamera.generated.h"

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

private:
	UPROPERTY(EditAnywhere)
	UMaterial* ViewshedMaterial;

	UPROPERTY(EditAnywhere)
	int32 DepthWidth = 1024;

	UPROPERTY(EditAnywhere)
	int32 DepthHeight = 1024;

	TObjectPtr<UTextureRenderTarget2D> DepthTexture;
	USceneCaptureComponent2D* ViewshedCamera;
	FVector LastViewshedCameraPosition;
	FRotator LastViewshedCameraRotation;

	void CreateDepthTexture();
};
