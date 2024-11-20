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

#include "QueryLocation.h"

 // Sets default values
AQueryLocation::AQueryLocation()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set the root component 
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;

	// Add a static mesh component 
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MeshComponent->SetupAttachment(Root);
	MeshComponent->SetWorldScale3D(MeshScale);

	// Add a text render component and set the properties
	TextComponent = CreateDefaultSubobject<UTextRenderComponent>(TEXT("TextComponent"));
	TextComponent->SetupAttachment(Root);
	TextComponent->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
	TextComponent->SetTextRenderColor(FColor::Black);
	auto textMaterialAsset = LoadObject<UMaterial>(nullptr, TEXT("Material'/Game/SampleViewer/SharedResources/Materials/TextMaterialWithBackground.TextMaterialWithBackground'"));
	if (textMaterialAsset != nullptr)
	{
		TextComponent->SetMaterial(0, textMaterialAsset);
	}

	// Add an ArcGISLocation component
	ArcGISLocation = CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("ArcGISLocation"));
	ArcGISLocation->SetupAttachment(Root);
	ArcGISLocation->SetSurfacePlacementMode(EArcGISSurfacePlacementMode::OnTheGround);
}

void AQueryLocation::BeginPlay()
{
	Super::BeginPlay();

	PawnActor = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	auto mapActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass());
	if (mapActor)
	{
		MapComponent = mapActor->GetComponentByClass<UArcGISMapComponent>();
	}
}

void AQueryLocation::FinalizeAddressQuery()
{
	auto point = ArcGISLocation->GetPosition();
	
	// Place the pawn above the current query location
	auto PawnLocation = PawnActor->FindComponentByClass<UArcGISLocationComponent>();
	PawnLocation->SetPosition(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
		point->GetX(), point->GetY(), point->GetZ() + CameraDistanceToGround, UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));
	
	// Set up the text render component with the appropriate transform
	TextComponent->SetWorldScale3D(FVector3d(125.));
	TextComponent->SetWorldRotation(PawnActor->GetActorRotation() + FRotator3d(180, 0, 180));
	TextComponent->SetRelativeLocation(FVector3d(2000, 0, 4000));

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	CameraManager->StartCameraFade(1, 0, .1, FColor::Black, false, true);
	
	// Remove the callback for draw status and tick prerequisites 
	MapComponent->GetView()->APIObject->SetDrawStatusChanged(nullptr);
	RemoveTickPrerequisiteComponent(ArcGISLocation);
	RemoveTickPrerequisiteActor(PawnActor);
}

// Set this instance up for a geocoding query  
void AQueryLocation::SetupAddressQuery(UArcGISPoint* InPoint, FString InAddress)
{
	// Place and rotate itself at the location returned for the query 
	ArcGISLocation->SetPosition(InPoint);
	ArcGISLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(90., 0., 0.));

	// Place and rotate the pawn at the location returned for the query at a high altitude 
	auto PawnLocation = PawnActor->FindComponentByClass<UArcGISLocationComponent>();
	PawnLocation->SetPosition(InPoint);
	PawnLocation->SetRotation(UArcGISRotation::CreateArcGISRotation(0., 0., 0.));

	// Make sure the location components have updated the transform before determining the pawn elevation
	AddTickPrerequisiteActor(PawnActor);
	AddTickPrerequisiteComponent(ArcGISLocation);

	// Fade the camera until the elevation at the location has been determined
	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	CameraManager->SetManualCameraFade(1, FColor::Black, false);
	
	// Set up the static mesh component and the address cue
	MeshComponent->SetStaticMesh(PinMesh);
	MeshComponent->SetWorldScale3D(FVector3d(MeshScale));
	TextComponent->SetText(FText::FromString(InAddress));

	// Finalize the placement of the pawn once map is done loading at the new location 
	MapComponent->GetView()->APIObject->SetDrawStatusChanged([this](Esri::GameEngine::MapView::ArcGISDrawStatus DrawStatus) {
		if (DrawStatus == Esri::GameEngine::MapView::ArcGISDrawStatus::Completed)
		{
			FinalizeAddressQuery();
		}
	});
}

// Set this instance up for a reverse geocoding query  
void AQueryLocation::SetupLocationQuery(FVector3d InLocation)
{
	// calculate the distance of the selected location from the pawn. Used for scaling the mesh and text 
	float DistanceToPawn = FVector3d::Distance(PawnActor->GetActorLocation(), InLocation);
	
	// Place itself in the selected location and align with with the pawn rotation
	SetActorLocation(InLocation);
	UArcGISRotation* PawnRotation = PawnActor->FindComponentByClass<UArcGISLocationComponent>()->GetRotation();
	ArcGISLocation->SetRotation(PawnRotation);

	// Set up the static mesh component with the appropriate shape, material and scale
	MeshComponent->SetStaticMesh(PointMesh);
	MeshComponent->SetMaterial(0, PointMaterial);
	MeshComponent->SetWorldScale3D(FVector3d(0.00001 * MeshScale * DistanceToPawn));

	// Set up the text render component with the appropriate transform 
	TextComponent->SetRelativeRotation(FRotator3d(0, 180, 0)); // Text faces the pawn
	TextComponent->SetRelativeLocation(FVector3d(-0.02 * DistanceToPawn, 0, 0.04 * DistanceToPawn));
	TextComponent->SetWorldScale3D(FVector3d(0.00125 * DistanceToPawn));
	TextComponent->SetVisibility(false); // Hide the address cue until it has been updated
}

// Update the address cue with the new address
void AQueryLocation::UpdateAddressCue(FString InAddress)
{
	TextComponent->SetText(FText::FromString(InAddress));
	TextComponent->SetVisibility(true);
}
