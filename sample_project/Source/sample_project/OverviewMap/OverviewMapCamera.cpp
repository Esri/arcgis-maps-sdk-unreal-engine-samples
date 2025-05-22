// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */


#include "OverviewMapCamera.h"

#include "ArcGISPawn.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "ArcGISMapsSDK/Components/ArcGISCameraComponent.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Kismet/GameplayStatics.h"


AOverviewMapCamera::AOverviewMapCamera()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	ArcGISCamera = CreateDefaultSubobject<UArcGISCameraComponent>("ArcGISCamera");
	LocationComponent = CreateDefaultSubobject<UArcGISLocationComponent>("ArcGISLocation");
	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>("SceneCaptureComponent");
	
	ArcGISCamera->SetupAttachment(root);
	LocationComponent->SetupAttachment(root);
	SceneCaptureComponent->SetupAttachment(root);

	SceneCaptureComponent->ProjectionType = ECameraProjectionMode::Orthographic;
	SceneCaptureComponent->bUseFauxOrthoViewPos = true;
	SceneCaptureComponent->TextureTarget = Texture;

	ArcGISCamera->SetWorldRotation(FRotator(-90,0,0));
	SceneCaptureComponent->SetWorldRotation(FRotator(-90,0,0));
}

void AOverviewMapCamera::BeginPlay()
{
	Super::BeginPlay();
	
	// Create the UI and add it to the viewport
	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);

		if (UIWidget)
		{
			UIWidget->AddToViewport();
		}
	}
	
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (!PlayerController)
	{
		return;
	}
	
	PlayerController->SetInputMode(FInputModeGameAndUI());
	PlayerController->SetShowMouseCursor(true);
	ArcGISPawn = Cast<AArcGISPawn>(UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISPawn::StaticClass()));
}

void AOverviewMapCamera::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UWidgetLayoutLibrary::RemoveAllWidgets(UIWidget);
}

void AOverviewMapCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateLocation();
}

void AOverviewMapCamera::UpdateLocation()
{
	if (!ArcGISPawn)
	{
		return;
	}

	pawnLocationComponent = ArcGISPawn->FindComponentByClass<UArcGISLocationComponent>();

	if (!pawnLocationComponent)
	{
		GEngine->AddOnScreenDebugMessage(1, 1, FColor::Red, TEXT("Not Found"));
		return;
	}
	
	auto newPosition= UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(pawnLocationComponent->GetPosition()->GetX(), pawnLocationComponent->GetPosition()->GetY(), LocationComponent->GetPosition()->GetZ(), UArcGISSpatialReference::WGS84());
	LocationComponent->SetPosition(newPosition);
}
