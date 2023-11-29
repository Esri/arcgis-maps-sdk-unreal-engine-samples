/* Copyright 2023 Esri
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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "IWebSocket.h"
#include "PlaneController.h"
#include "StreamLayerQuery.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AStreamLayerQuery : public AActor
{
	GENERATED_BODY()

public:
	AStreamLayerQuery();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bConnectionStatus;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UUserWidget* UIWidget;

private:
	void Connect();
	void TryParseAndUpdatePlane(FString data);
	void SpawnPlane(FPlaneFeature);
	TMap<FString, APlaneController*> planeData;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta=(AllowPrivateAccess))
	float PlaneCountThreshold = 100;
	FString Url = "wss://geoeventsample1.esri.com:6143/arcgis/ws/services/FAAStream/StreamServer/subscribe";
	TSharedPtr<IWebSocket> webSocket;

protected:
	virtual void BeginPlay() override;
	void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
