/* Copyright 2022 Esri
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

#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "Blueprint/UserWidget.h"
#include "Breadcrumb.h"
#include "Components/SplineMeshComponent.h"
#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "Json.h"
#include "RouteMarker.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "RouteManager.generated.h"
UCLASS()
class SAMPLE_PROJECT_API ARouteManager : public AActor
{
	GENERATED_BODY()

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// Sets default values for this actor's properties
	ARouteManager();

	UFUNCTION(BlueprintCallable)
	void ClearMap();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void PostRoutingRequest();
	void ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	UFUNCTION()
	void AddStop();
	UFUNCTION()
	void UpdateBreadcrumbs();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	AStaticMeshActor* StartMarker;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	AStaticMeshActor* EndMarker;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	class AInputManager* InputManager;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (AllowPrivateAccess))
	TSubclassOf<class UUserWidget> UIWidgetClass;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, meta = (AllowPrivateAccess))
	UUserWidget* UIWidget;

	TDoubleLinkedList<ABreadcrumb*> Breadcrumbs;
	TDoubleLinkedList<UArcGISPoint*> Stops;
	TDoubleLinkedList<USplineMeshComponent*> SplineMeshComponents;
	UArcGISMapComponent* MapComponent;
	UFunction* HideInstructions;
	UStaticMesh* RouteMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/SampleViewer/SharedResources/Geometries/Cube.Cube"));

	bool bIsRouting = false;
	bool bShouldUpdateBreadcrumbs = false;
	float HeightOffset = 200.0f;
	float TraceLength = 10000000.0f;
	FString RoutingServiceURL = "https://route-api.arcgis.com/arcgis/rest/services/World/Route/NAServer/Route_World/solve";
	FVector2D RouteCueScale = FVector2D(5.);
	int StopCount = 2;
};
