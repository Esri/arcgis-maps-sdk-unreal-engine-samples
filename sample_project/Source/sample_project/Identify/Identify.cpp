// /* Copyright 2025 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#include "Identify.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISAttributeValue.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/ArcGISGeoElement.h"
#include "ArcGISMapsSDK/API/GameEngine/MapView/ArcGISIdentifyLayerResult.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/API/Standard/ArcGISElement.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISException.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISFuture.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/CAPI/Invoke.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISViewCoordinateTransformer.h"
#include "sample_project/InputManager.h"
#include "Components/ListView.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"   
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

	InputManager->OnInputTrigger.AddDynamic(this, &AIdentify::OnInputTriggered);


	auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);

	if (playerController)
	{
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
	}

	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		if (!UIWidget)
		{
			return;
		}

		UIWidget->AddToViewport();

		PropertyListView = Cast<UListView>(UIWidget->GetWidgetFromName(TEXT("ListView")));

		BuildingInfoPanel = Cast<UWidget>(UIWidget->GetWidgetFromName(TEXT("BuildingInfoPanel")));

		if (BuildingInfoPanel)
		{
			BuildingInfoPanel->SetVisibility(ESlateVisibility::Collapsed); 
		}

		if (!PropertyListView)
		{
			UE_LOG(LogTemp, Warning, TEXT("ListView not found on UI widget"));
		}
		
		if (UIWidget->FindFunction("ShowInstruction"))
		{
			UIWidget->ProcessEvent(UIWidget->FindFunction("ShowInstruction"), nullptr);
		}
	}
}

// Called every frame
void AIdentify::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FString GetStringFromAttributeValue(const Esri::GameEngine::Attributes::ArcGISAttributeValue& attributeValue)
{
	auto attributeType = attributeValue.GetAttributeValueType();

	if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::DateTime)
	{
		return attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::DateTime>().ToString();
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Float32)
	{
		return FString::SanitizeFloat(attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Float32>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Float64)
	{
		return FString::SanitizeFloat(attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Float64>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::GUID)
	{
		return attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::GUID>().ToString();
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Int16)
	{
		return FString::Printf(TEXT("%d"), attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Int16>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Int32)
	{
		return FString::Printf(TEXT("%d"), attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Int32>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Int64)
	{
		return FString::Printf(TEXT("%lld"), attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Int64>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Float64)
	{
		return FString::SanitizeFloat(attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Float64>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::String)
	{
		return attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::String>();
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Uint16)
	{
		return FString::Printf(TEXT("%d"), attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Uint16>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Uint32)
	{
		return FString::Printf(TEXT("%d"), attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Uint32>());
	}
	else if (attributeType == Esri::GameEngine::Attributes::ArcGISAttributeValueType::Uint64)
	{
		return FString::Printf(TEXT("%lld"), attributeValue.GetValue<Esri::GameEngine::Attributes::ArcGISAttributeValueType::Uint64>());
	}

	return "<unknown-type>";
}


FString AIdentify::IdentifyAtMouseClick()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PlayerController)
	{
		return "Null player controller";
	}

	FVector Location, Direction;
	const TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;
	PlayerController->DeprojectMousePositionToWorld(Location, Direction);

	if (!UKismetSystemLibrary::LineTraceSingle(GetWorld(), Location, Location + Direction * 10000000.0, TraceTypeQuery1, false, ActorsToIgnore,
											   EDrawDebugTrace::None, HitResult, true))
	{
		return "Line trace did not hit any actor";
	}

	auto geoPosition = MapComponent->TransformEnginePositionToPoint(HitResult.TraceStart)->APIObject;
	auto geoPosition2 = MapComponent->TransformEnginePositionToPoint(HitResult.TraceEnd)->APIObject;

	auto point1 = Esri::GameEngine::Geometry::ArcGISPoint(std::move(geoPosition->GetHandle()));
	auto point2 = Esri::GameEngine::Geometry::ArcGISPoint(std::move(geoPosition2->GetHandle()));

	auto view = MapComponent->GetView()->APIObject;

	try
	{
		auto results = view->IdentifyLayersAsync(point1, point2, -1);
		point1.SetHandle(nullptr);
		point2.SetHandle(nullptr);

		auto identifyLayerResults = results.Get();

		if (!identifyLayerResults)
		{
			return "Null identify layer results collection";
		}

		if (identifyLayerResults.GetSize() == 0)
		{
			return "Empty identify layer results collection";
		}

		auto identifyLayerResultsSize = identifyLayerResults.GetSize();

		FString outputString = "[";

		LastAttributes.Empty();

		for (int i = 0; i < identifyLayerResultsSize; i++)
		{
			outputString += "[";

			auto identifyLayerResult = identifyLayerResults.At(i);

			auto geoElements = identifyLayerResult.GetGeoElements();

			auto geoElementsSize = geoElements.GetSize();

			for (int j = 0; j < geoElementsSize; j++)
			{
				outputString += "{";

				auto feature = geoElements.At(j);

				auto attributes = feature.GetAttributes();

				auto attributeKeys = attributes.GetKeys();

				for (int k = 0; k < attributeKeys.Num(); k++)
				{
					auto attributeKey = attributeKeys[k];
					auto attributeValue = attributes.At(attributeKey);

					const FString ValueString = GetStringFromAttributeValue(attributeValue);

					outputString += "\"" + attributeKey + "\": \"" + ValueString + "\"";
					if (k < attributeKeys.Num() - 1)
					{
						outputString += ", ";
					}

					FAttributeRow Row;
					Row.Key = attributeKey;
					Row.Value = ValueString;
					LastAttributes.Add(Row);
				}

				outputString += "}";

				if (j < geoElementsSize - 1)
				{
					outputString += ", ";
				}
			}

			outputString += "]";

			if (i < identifyLayerResultsSize - 1)
			{
				outputString += ", ";
			}
		}

		outputString += "]";

		return outputString;
	}
	catch (Esri::Unreal::ArcGISException exception)
	{
		point1.SetHandle(nullptr);
		point2.SetHandle(nullptr);
		return "Error: " + exception.GetMessage();
	}
}

void AIdentify::RefreshListViewFromAttributes()
{
	PropertyListView->ClearListItems();

	UClass* RowClass = PropertyRowClass.Get();

	FProperty* KeyBaseProperty = RowClass->FindPropertyByName(TEXT("Key"));
	FProperty* ValueBaseProperty = RowClass->FindPropertyByName(TEXT("Value"));

	FStrProperty* KeyProp = CastField<FStrProperty>(KeyBaseProperty);
	FStrProperty* ValueProp = CastField<FStrProperty>(ValueBaseProperty);

	if (!KeyProp || !ValueProp)
	{
		UE_LOG(LogTemp, Warning, TEXT("Key or Value property not found or not FString on PropertyRowClass"));
		return;
	}

	for (const FAttributeRow& RowData : LastAttributes)
	{
		UObject* NewItemObject = NewObject<UObject>(this, PropertyRowClass);
		if (!NewItemObject)
		{
			UE_LOG(LogTemp, Warning, TEXT("NewItemObject is null"));
			continue;
		}

		KeyProp->SetPropertyValue_InContainer(NewItemObject, RowData.Key);
		ValueProp->SetPropertyValue_InContainer(NewItemObject, RowData.Value);

		PropertyListView->AddItem(NewItemObject);
	}
}

void AIdentify::OnInputTriggered()
{
	LastIdentifyOutput = IdentifyAtMouseClick();

	UE_LOG(LogTemp, Log, TEXT("%s"), *LastIdentifyOutput);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, LastIdentifyOutput);
	}

	if (LastAttributes.Num() > 0)
	{
		RefreshListViewFromAttributes();

		if (BuildingInfoPanel)
		{
			BuildingInfoPanel->SetVisibility(ESlateVisibility::Visible);
		}
	}
	else
	{
		if (BuildingInfoPanel)
		{
			BuildingInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}
}


