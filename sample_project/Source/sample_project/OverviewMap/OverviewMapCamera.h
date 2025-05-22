// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#pragma once

#include "CoreMinimal.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Actor.h"
#include "OverviewMapCamera.generated.h"

class AArcGISPawn;
class UArcGISLocationComponent;
class UArcGISCameraComponent;
class USceneCaptureComponent2D;

UCLASS()
class SAMPLE_PROJECT_API AOverviewMapCamera : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AOverviewMapCamera();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	void UpdateLocation();

private:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	UArcGISCameraComponent* ArcGISCamera;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	AArcGISPawn* ArcGISPawn;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	UArcGISLocationComponent* pawnLocationComponent;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	UArcGISLocationComponent* LocationComponent;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, meta=(AllowPrivateAccess))
	USceneCaptureComponent2D* SceneCaptureComponent;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	UTextureRenderTarget2D* Texture = LoadObject<UTextureRenderTarget2D>(nullptr,
		TEXT("/Script/Engine.TextureRenderTarget2D'/Game/SampleViewer/Samples/OverviewMap/UI/OverviewMapRenderTarget.OverviewMapRenderTarget'"));
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	UUserWidget* UIWidget;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> UIWidgetClass;
};
