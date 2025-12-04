// /* Copyright 2025 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */

#include "Identify.h"
#include "ArcGISMapsSDK/API/GameEngine/Attributes/ArcGISAttributeValue.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Layers/Base/ArcGISLayer.h"
#include "ArcGISMapsSDK/API/GameEngine/Map/ArcGISGeoElement.h"
#include "ArcGISMapsSDK/API/GameEngine/MapView/ArcGISIdentifyLayerResult.h"
#include "ArcGISMapsSDK/API/GameEngine/View/ArcGISView.h"
#include "ArcGISMapsSDK/API/Standard/ArcGISElement.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISArrayBuilder.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISException.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISFuture.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableArray.h"
#include "ArcGISMapsSDK/API/Unreal/ArcGISImmutableCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/ArcGIS3DObjectSceneLayer.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Layers/Base/ArcGISLayerCollection.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Map/ArcGISMap.h"
#include "ArcGISMapsSDK/CAPI/Invoke.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/Utils/ArcGISViewCoordinateTransformer.h"
#include "ArcGISPawn.h"
#include "Components/ListView.h"
#include "Components/TextBlock.h"
#include "Engine/World.h"
#include "Internationalization/Text.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/Material.h"
#include "Materials/MaterialParameterCollection.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/UnrealType.h"
#include "sample_project/InputManager.h"

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
		playerController->bEnableTouchEvents = true;
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
		CurrentPageText = Cast<UTextBlock>(UIWidget->GetWidgetFromName(TEXT("CurrentPage")));
		TotalPageText = Cast<UTextBlock>(UIWidget->GetWidgetFromName(TEXT("TotalPage")));
		BuildingList = Cast<UScrollBox>(UIWidget->GetWidgetFromName(TEXT("BuildingList")));

		if (!BuildingList)
		{
			UE_LOG(LogTemp, Error, TEXT("BuildingList ScrollBox not found!"));
		}


		if (BuildingInfoPanel)
		{
			BuildingInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	SetupHighlightAttributesOnMap();
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
		return TEXT("Null player controller");
	}

	FVector Location, Direction;
	const TArray<AActor*> ActorsToIgnore;
	FHitResult HitResult;

	PlayerController->DeprojectMousePositionToWorld(Location, Direction);

	const FVector Start = Location;
	const FVector End = Location + Direction * Length;

	auto geoPositionStart = MapComponent->TransformEnginePositionToPoint(Start)->APIObject;
	auto geoPositionEnd = MapComponent->TransformEnginePositionToPoint(End)->APIObject;

	auto point1 = Esri::GameEngine::Geometry::ArcGISPoint(std::move(geoPositionStart->GetHandle()));
	auto point2 = Esri::GameEngine::Geometry::ArcGISPoint(std::move(geoPositionEnd->GetHandle()));

	auto view = MapComponent->GetView()->APIObject;

	try
	{
		auto results = view->IdentifyLayersAsync(point1, point2, -1);
		point1.SetHandle(nullptr);
		point2.SetHandle(nullptr);

		auto identifyLayerResults = results.Get();

		if (!identifyLayerResults)
		{
			AllFeaturesAttributes.Empty();
			LastAttributes.Empty();
			return TEXT("Null identify layer results collection");
		}

		if (identifyLayerResults.GetSize() == 0)
		{
			AllFeaturesAttributes.Empty();
			LastAttributes.Empty();
			return TEXT("Empty identify layer results collection");
		}

		auto identifyLayerResultsSize = identifyLayerResults.GetSize();

		FString outputString = TEXT("[");

		AllFeaturesAttributes.Empty();
		LastAttributes.Empty();

		//parse geoElements and attributes
		for (int i = 0; i < identifyLayerResultsSize; i++)
		{
			outputString += TEXT("[");

			auto identifyLayerResult = identifyLayerResults.At(i);
			auto geoElements = identifyLayerResult.GetGeoElements();
			auto geoElementsSize = geoElements.GetSize();

			for (int j = 0; j < geoElementsSize; j++)
			{
				outputString += TEXT("{");

				auto feature = geoElements.At(j);
				auto attributes = feature.GetAttributes();
				auto attributeKeys = attributes.GetKeys();

				FFeatureAttributeSet FeatureSet;

				for (int k = 0; k < attributeKeys.Num(); k++)
				{
					auto attributeKey = attributeKeys[k];
					auto attributeValue = attributes.At(attributeKey);

					const FString ValueString = GetStringFromAttributeValue(attributeValue);
					FString ValueStringForUI = ValueString;

					if (attributeKey == "LSTMODDATE" && IsInvalidDateString(ValueString))
					{
						ValueStringForUI = TEXT("");
					}

					outputString += TEXT("\"") + attributeKey + TEXT("\": \"") + ValueString + TEXT("\"");
					if (k < attributeKeys.Num() - 1)
					{
						outputString += TEXT(", ");
					}

					FAttributeRow Row;
					Row.Key = attributeKey;
					Row.Value = ValueStringForUI;
					FeatureSet.Attributes.Add(Row);
				}

				AllFeaturesAttributes.Add(FeatureSet);

				outputString += TEXT("}");

				if (j < geoElementsSize - 1)
				{
					outputString += TEXT(", ");
				}
			}

			outputString += TEXT("]");

			if (i < identifyLayerResultsSize - 1)
			{
				outputString += TEXT(", ");
			}
		}

		outputString += TEXT("]");

		if (AllFeaturesAttributes.Num() > 0)
		{
			CurrentFeatureIndex = 0;
			LastAttributes = AllFeaturesAttributes[0].Attributes;
		}
		else
		{
			LastAttributes.Empty();
		}

		return outputString;
	}
	catch (Esri::Unreal::ArcGISException exception)
	{
		point1.SetHandle(nullptr);
		point2.SetHandle(nullptr);

		AllFeaturesAttributes.Empty();
		LastAttributes.Empty();

		return TEXT("Error: ") + exception.GetMessage();
	}
}

void AIdentify::SetupHighlightAttributesOnMap()
{
	if (!MapComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("MapComponent is null"));
		return;
	}

	UArcGISMap* Map = MapComponent->GetMap();

	if (!Map)
	{
		UE_LOG(LogTemp, Warning, TEXT("Map is null"));
		return;
	}

	UArcGISLayerCollection* MapLayers = Map->GetLayers();
	if (!MapLayers)
	{
		UE_LOG(LogTemp, Warning, TEXT("Layers is null"));
		return;
	}

	UArcGIS3DObjectSceneLayer* ObjectSceneLayer = nullptr;

	if (!HighlightLayerName.IsEmpty())
	{
		for (int32 i = 0; i < MapLayers->GetSize(); ++i)
		{
			if (auto* Layer = MapLayers->At(i))
			{
				if (auto* Candidate = Cast<UArcGIS3DObjectSceneLayer>(Layer))
				{
					if (Candidate->GetName() == HighlightLayerName)
					{
						ObjectSceneLayer = Candidate;
						UE_LOG(LogTemp, Warning, TEXT("found layer to highlight"));
						break;
					}
				}
			}
		}
	}

	auto LayerAttributes = Esri::Unreal::ArcGISImmutableArray<FString>::CreateBuilder();
	LayerAttributes.Add(HighlightIdFieldName);

	auto LayerAPI = StaticCastSharedPtr<Esri::GameEngine::Layers::ArcGIS3DObjectSceneLayer>(ObjectSceneLayer->APIObject);

	LayerAPI->SetAttributesToVisualize(LayerAttributes.MoveToArray());

	if (HighlightMaterial)
	{
		ObjectSceneLayer->SetMaterialReference(HighlightMaterial);

		UE_LOG(LogTemp, Log, TEXT("Highlight material set on layer %s (field %s)"), *ObjectSceneLayer->GetName(), *HighlightIdFieldName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("HighlightMaterial is null on Identify; layer = %s"), *ObjectSceneLayer->GetName());
	}
}

// Find the current feature's OBJECTID
int64 AIdentify::GetCurrentFeatureID() const
{
	if (!AllFeaturesAttributes.IsValidIndex(CurrentFeatureIndex))
	{
		return -1;
	}

	const FFeatureAttributeSet& FeatureSet = AllFeaturesAttributes[CurrentFeatureIndex];

	for (const FAttributeRow& Row : FeatureSet.Attributes)
	{
		if (Row.Key == HighlightIdFieldName)
		{
			return FCString::Atoi64(*Row.Value);
		}
	}

	return -1;
}

//updates the Material Parameter Collection so that every building’s material knows which feature ID should be highlighted.
void AIdentify::ApplySelectionToMaterial()
{
	if (!BuildingSelectionCollection)
	{
		return;
	}

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	const int64 FeatureID = GetCurrentFeatureID();

	UMaterialParameterCollectionInstance* Instance = World->GetParameterCollectionInstance(BuildingSelectionCollection);

	if (!Instance)
	{
		return;
	}

	const float SelectedIdFloat = (FeatureID >= 0) ? static_cast<float>(FeatureID) : -1.0f;

	//Write the selected feature ID into the Material Parameter Collection
	Instance->SetScalarParameterValue(TEXT("SelectedID"), SelectedIdFloat);
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
		UE_LOG(LogTemp, Warning, TEXT("Key or Value property not found"));
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

//show next feature/geoElements within the current selection
void AIdentify::ShowNextFeature()
{
	const int32 Total = AllFeaturesAttributes.Num();
	if (Total <= 1)
	{
		return;
	}

	CurrentFeatureIndex = (CurrentFeatureIndex + 1) % Total;
	ApplyCurrentFeature();
}

void AIdentify::ShowPreviousFeature()
{
	const int32 Total = AllFeaturesAttributes.Num();
	if (Total <= 1)
	{
		return;
	}

	CurrentFeatureIndex = (CurrentFeatureIndex - 1 + Total) % Total;
	ApplyCurrentFeature();
}


void AIdentify::UpdatePageTexts()
{
	if (!CurrentPageText || !TotalPageText)
	{
		return;
	}

	const int32 Total = AllFeaturesAttributes.Num();

	if (Total <= 0)
	{
		CurrentPageText->SetText(FText::GetEmpty());
		TotalPageText->SetText(FText::GetEmpty());
	}
	else
	{
		CurrentPageText->SetText(FText::AsNumber(CurrentFeatureIndex + 1));
		TotalPageText->SetText(FText::AsNumber(Total));
	}
}

void AIdentify::ApplyCurrentFeature()
{
	if (!AllFeaturesAttributes.IsValidIndex(CurrentFeatureIndex))
	{
		LastAttributes.Empty();
		RefreshListViewFromAttributes();
		UpdatePageTexts();
		ApplySelectionToMaterial();
		return;
	}

	LastAttributes = AllFeaturesAttributes[CurrentFeatureIndex].Attributes;

	RefreshListViewFromAttributes();
	UpdatePageTexts();
	ApplySelectionToMaterial();
}

int32 AIdentify::GetFeatureCount() const
{
	return AllFeaturesAttributes.Num();
}

int32 AIdentify::GetCurrentFeatureIndex() const
{
	return CurrentFeatureIndex;
}

void AIdentify::SelectFeatureByIndex(int32 Index)
{
	const int32 Total = AllFeaturesAttributes.Num();
	if (Total <= 0)
	{
		return;
	}


	CurrentFeatureIndex = FMath::Clamp(Index, 0, Total - 1);

	ApplyCurrentFeature(); 
}

void AIdentify::OnBuildingSelected(bool bIsChecked)
{
	if (!bIsChecked)
	{
		return;
	}

	int32 SelectedIndex = INDEX_NONE;

	for (int32 i = 0; i < BuildingCheckBoxes.Num(); ++i)
	{
		if (i == CurrentFeatureIndex)
		{
			continue;
		}

		if (BuildingCheckBoxes[i] && BuildingCheckBoxes[i]->IsChecked())
		{
			SelectedIndex = i;
			break;
		}
	}

	CurrentFeatureIndex = SelectedIndex;

	for (int32 i = 0; i < BuildingCheckBoxes.Num(); ++i)
	{
		if (BuildingCheckBoxes[i])
		{
			BuildingCheckBoxes[i]->SetIsChecked(i == CurrentFeatureIndex);
		}
	}

	ApplyCurrentFeature();
}


void AIdentify::UpdateBuildingListUI()
{
	if (!BuildingList)
	{
		return;
	}

	BuildingList->ClearChildren();
	BuildingCheckBoxes.Empty();

	const int32 Total = AllFeaturesAttributes.Num();

	for (int32 i = 0; i < Total; ++i)
	{
		UHorizontalBox* Row = NewObject<UHorizontalBox>(UIWidget);
		if (!Row)
		{
			continue;
		}

		UCheckBox* Check = NewObject<UCheckBox>(UIWidget);
		if (!Check)
		{
			continue;
		}

		Check->SetIsChecked(i == CurrentFeatureIndex);

		BuildingCheckBoxes.Add(Check);

		Check->OnCheckStateChanged.AddDynamic(this, &AIdentify::OnBuildingSelected);

		UTextBlock* Label = NewObject<UTextBlock>(UIWidget);
		if (Label)
		{
			Label->SetText(FText::FromString(FString::Printf(TEXT("Building %d"), i + 1)));
			Label->SetColorAndOpacity(FSlateColor(FLinearColor::White));
			FSlateFontInfo FontInfo = Label->Font;
			FontInfo.Size = 14; 
			Label->SetFont(FontInfo);
		}

		Row->AddChildToHorizontalBox(Check);
		if (Label)
		{
			Row->AddChildToHorizontalBox(Label);
		}

		BuildingList->AddChild(Row);
	}
}

//a simple method to filter out incorrect date format which will be resolved in future release.
bool AIdentify::IsInvalidDateString(const FString& DateString)
{
	if (DateString.Contains(TEXT("-00.00.00")))
	{
		return true;
	}

	return false;
}

void AIdentify::OnInputTriggered()
{
	LastIdentifyOutput = IdentifyAtMouseClick();

	if (LastAttributes.Num() > 0)
	{
		RefreshListViewFromAttributes();
		UpdatePageTexts();

		if (BuildingInfoPanel)
		{
			BuildingInfoPanel->SetVisibility(ESlateVisibility::Visible);
		}

		ApplySelectionToMaterial();
		UpdateBuildingListUI();

	}
	else
	{
		if (BuildingInfoPanel)
		{
			BuildingInfoPanel->SetVisibility(ESlateVisibility::Collapsed);
		}

		if (BuildingSelectionCollection)
		{
			if (auto* Instance = GetWorld()->GetParameterCollectionInstance(BuildingSelectionCollection))
			{
				Instance->SetScalarParameterValue(TEXT("SelectedID"), -1.0f);
			}
		}
	}
}