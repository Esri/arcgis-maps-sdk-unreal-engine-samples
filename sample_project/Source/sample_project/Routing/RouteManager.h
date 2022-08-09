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

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blueprint/UserWidget.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SplineMeshComponent.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Classes/GameFramework/PlayerController.h"
#include "Json.h"
#include "Http.h"
#include "Breadcrumb.h"
#include "RouteMarker.h"
#include "ArcGISMapsSDK/Actors/ArcGISMapActor.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISGeometryEngine.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
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

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	
	void PostRoutingRequest();
	void ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);
	void SetupInput();
	void AddStop();

	TSubclassOf<class UUserWidget> UIWidgetClass;
	UUserWidget* UIWidget;
	UArcGISMapComponent* MapComponent;
	TDoubleLinkedList < USplineMeshComponent*> SplineMeshComponents;
	UStaticMesh* RouteMesh;
	TDoubleLinkedList<ARouteMarker*> Stops;
	TDoubleLinkedList<ABreadcrumb*> Breadcrumbs;
	bool bIsRouting = false;
	bool bShouldUpdateBreadcrums = false;
	FVector2D RouteCueScale = FVector2D(5.);
	int StopCount = 2;
};
