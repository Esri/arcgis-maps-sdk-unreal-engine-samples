// Fill out your copyright notice in the Description page of Project Settings.


#include "Identify.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "sample_project/InputManager.h"

#include "ArcGISMapsSDK/API/ArcGISRuntime/Data/ArcGISFeature.h"
#include "ArcGISMapsSDK/API/GameEngine/Elevation/Base/ArcGISElevationSource.h"
#include "ArcGISMapsSDK/API/GameEngine/Geometry/ArcGISPoint.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/Base/ArcGISLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/ArcGISGeoElement.h"
#include "ArcGISMapsSDK/API/GameEngine/MapView/ArcGISIdentifyLayerResult.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISGeoElementImmutableCollection.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISIdentifyLayerResultImmutableCollection.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISElevationSourceViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISLayerViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewState.h"
#include "ArcGISMapsSDK/API/GameEngine/View/State/ArcGISViewStateMessage.h"
#include "ArcGISMapsSDK/API/Standard/ArcGISElement.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISFuture.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Elevation/ArcGISImageElevationSource.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Extent/ArcGISExtentCircle.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGISImageLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISBasemap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapElevation.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMapType.h"
#include "ArcGISMapsSDK/CAPI/Invoke.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"

#include <ArcGISMapsSDK/Utils/ArcGISViewCoordinateTransformer.h>
#include "ArcGISPawn.h"

// Sets default values
AIdentify::AIdentify()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AIdentify::BeginPlay()
{
	Super::BeginPlay();
	
	MapActor = Cast<AArcGISMapActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass()));

	if (MapActor)
	{
		MapComponent = MapActor->GetMapComponent();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapActor not found in the level!"));
	}

	//SpatialReference = MapComponent->GetOriginPosition()->GetSpatialReference();

	if (!InputManager)
	{
		InputManager = Cast<AInputManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AInputManager::StaticClass()));
	}

	InputManager->OnInputTrigger.AddDynamic(this, &AIdentify::IdentifyAtMouseClick);
	//InputManager->OnInputEnd.AddDynamic(this, &AIdentify::EndGeometry);

	auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (playerController)
	{
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
	}
}

// Called every frame
void AIdentify::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FString AIdentify::IdentifyAtMouseClick()
{
	FString outputString = "No output";
	APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController)
	{
		return outputString;
	}

	FVector Location, Direction;
	const TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	PlayerController->DeprojectMousePositionToWorld(Location, Direction);

	if (!UKismetSystemLibrary::LineTraceSingle(GetWorld(), Location, Location + Direction * 10000000.0, TraceTypeQuery1, false, ActorsToIgnore,
											   EDrawDebugTrace::None, HitResult, true))
	{
		return outputString;
	}

	auto geoPosition = MapComponent->TransformEnginePositionToPoint(HitResult.TraceStart)->APIObject;
	auto geoPosition2 = MapComponent->TransformEnginePositionToPoint(HitResult.TraceEnd)->APIObject;

	auto point1 = Esri::GameEngine::Geometry::ArcGISPoint(std::move(geoPosition->GetHandle()));
	auto point2 = Esri::GameEngine::Geometry::ArcGISPoint(std::move(geoPosition2->GetHandle()));

	TSharedPtr<Esri::GameEngine::View::ArcGISView> view = MapComponent->GetView()->APIObject;
	try
	{
		auto results = view->IdentifyLayersAsync(point1, point2, -1);
		point1.SetHandle(nullptr);
		point2.SetHandle(nullptr);

		auto futureResults = results.Get();
		if (!futureResults)
		{
			return outputString;
		}
		if (futureResults.GetSize() == 0)
		{
			return outputString;
		}
		auto singleResult = futureResults.At(0);

		auto geoElements = singleResult.GetGeoElementsImmutableCollection();
		if (geoElements.GetSize() == 0)
		{
			return outputString;
		}
		auto feature = geoElements.At(0);

		auto tmap = feature->GetAttributes();
		auto nameAttributeValue = tmap.Find("NAME");
		auto nameType = nameAttributeValue->GetAttributeValueType();

		if (nameType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::String)
		{
			auto name = nameAttributeValue->GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::String>();
			outputString = "Name: " + name;
		}

		auto dateAttributeValue = tmap.Find("LSTMODDATE");
		auto dateType = dateAttributeValue->GetAttributeValueType();

		if (dateType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::DateTime)
		{
			auto date = dateAttributeValue->GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::DateTime>();
			auto dateString = date.ToString();
			outputString = "Date: " + dateString;
		}
	}
	catch (Esri::Unreal::ArcGISException exception)
	{
		point1.SetHandle(nullptr);
		point2.SetHandle(nullptr);
		return "Error: " + exception.GetMessage();
	}
	return outputString;
}
