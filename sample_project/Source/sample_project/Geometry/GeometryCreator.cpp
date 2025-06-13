// /* Copyright 2023 Esri* * Licensed under the Apache License Version 2.0 (the "License"); * you may not use this file except in compliance with the License. * You may obtain a copy of the License at * *     http://www.apache.org/licenses/LICENSE-2.0 * * Unless required by applicable law or agreed to in writing, software * distributed under the License is distributed on an "AS IS" BASIS, * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. * See the License for the specific language governing permissions and * limitations under the License. */


#include "GeometryCreator.h"

#include "sample_project/InputManager.h"

constexpr double Interval = 10000;
constexpr int TraceLen = 1000000;
constexpr int MarkerHeight = 300;

AGeometryCreator::AGeometryCreator()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AGeometryCreator::BeginPlay()
{
	Super::BeginPlay();

	AArcGISMapActor* MapActor = Cast<AArcGISMapActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass()));
	
	if (MapActor)
	{
		ArcGISMap = MapActor->GetMapComponent();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ArcGISMapActor not found in the level!"));
	}

	SpatialReference = UArcGISSpatialReference::CreateArcGISSpatialReference(3857);

	if (!inputManager)
	{
		inputManager = Cast<AInputManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AInputManager::StaticClass()));
	}

	inputManager->OnInputTrigger.AddDynamic(this, &AGeometryCreator::StartGeometry);
	inputManager->OnInputEnd.AddDynamic(this, &AGeometryCreator::EndGeometry);

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
		UnitDropdown = (UComboBoxString*)UIWidget->GetWidgetFromName(TEXT("UnitDropDown"));
		if (UIWidget->FindFunction("ShowInstruction"))
		{
			UIWidget->ProcessEvent(UIWidget->FindFunction("ShowInstruction"), nullptr);
		}
	}

	CurrentLinearUnit = UArcGISLinearUnit::CreateArcGISLinearUnit(EArcGISLinearUnitId::Miles);
	CurrentAreaUnit = UArcGISAreaUnit::CreateArcGISAreaUnit(EArcGISAreaUnitId::SquareMiles);
}

void AGeometryCreator::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	inputManager->OnInputTrigger.RemoveDynamic(this, &AGeometryCreator::StartGeometry);
	inputManager->OnInputEnd.RemoveDynamic(this, &AGeometryCreator::EndGeometry);
}

void AGeometryCreator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Continuously update visual cue in update
	if (bIsEnvelopeMode && bIsDragging)
	{
		UpdateDraggingVisualization();
	}
}

void AGeometryCreator::StartGeometry()
{
	FVector direction;
	FHitResult hit;
	FVector position;

	auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(position, direction);

	bool bTraceSuccess =
		GetWorld()->LineTraceSingleByChannel(hit, position, position + TraceLen * direction, ECC_Visibility, FCollisionQueryParams());

	if (bTraceSuccess && hit.GetActor()->GetClass() == AArcGISMapActor::StaticClass() && hit.bBlockingHit)
	{
		SpawnParam.Owner = this;
		
		auto lineMarker = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), hit.ImpactPoint, FRotator(0), SpawnParam);
		auto lineMarkerGeo = ArcGISMap->EngineToGeographic(hit.ImpactPoint);

        UArcGISPoint* hitPoint =
			UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(lineMarkerGeo.X, lineMarkerGeo.Y, lineMarkerGeo.Z, SpatialReference);

		if (bIsEnvelopeMode)
		{
			StartPoint = hitPoint;
			bIsDragging = true;
			Stops.Add(lineMarker);
		}
		else
		{
			if (bIsPolygonMode)
			{
                // Clear the last segment when adding a new polygon vertice				
				int32 RemovedCount = lastToStartInterpolationPoints.Num();
				for (int32 i = 0; i <= RemovedCount; i++)
				{
					if (!SplineMeshComponents.IsEmpty())
					{
						auto Last = SplineMeshComponents.GetHead();
						if (Last && IsValid(Last->GetValue()))
						{
							Last->GetValue()->UnregisterComponent();
							Last->GetValue()->DestroyComponent();
						}
						SplineMeshComponents.RemoveNode(Last);
					}
				}

				for (AActor* Point : lastToStartInterpolationPoints)
				{
					Point->Destroy();
				}
				lastToStartInterpolationPoints.Empty();
			}

			if (Stops.Num() > 0)
			{
				auto lastStop = Stops.Last();
                auto lastStopGeo = ArcGISMap->EngineToGeographic(lastStop->GetActorLocation());
				UArcGISPoint* lastPoint =
					UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(lastStopGeo.X, lastStopGeo.Y, lastStopGeo.Z, SpatialReference);
				
				if (bIsPolylineMode)
				{
					double distance =
						UArcGISGeometryEngine::DistanceGeodetic(lastPoint, hitPoint, CurrentLinearUnit,
																UArcGISAngularUnit::CreateArcGISAngularUnit(EArcGISAngularUnitId::Degrees),
																EArcGISGeodeticCurveType::Geodesic)
							->GetDistance();
					Calculation += distance;
					UpdateDisplay(Calculation);
				}
				//FeaturePoints array stores user-added points and interpolated points
				FeaturePoints.Add(lastStop);
				Interpolate(lastStop, lineMarker, FeaturePoints);
				FeaturePoints.Add(lineMarker);
			
			}
				//Stops array stores user-added points
				Stops.Add(lineMarker);

			if (bIsPolygonMode)
			{
				//create polygon when vertice count is 3 or more
				if (Stops.Num() >= 3)
				{
					//compute the last segment
					Interpolate(lineMarker, Stops[0], lastToStartInterpolationPoints); 
					CreateAndCalculatePolygon();
				}
			}
			//render line when vertice count is 2 or more
			if (Stops.Num() >= 2)
			{
				RenderLine(FeaturePoints);
			}
		}
	}
}

void AGeometryCreator::EndGeometry()
{
	if (bIsEnvelopeMode)
	{
		FVector direction;
		FHitResult hit;
		FVector position;

		auto PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		PlayerController->DeprojectMousePositionToWorld(position, direction);

		bool bTraceSuccess =
			GetWorld()->LineTraceSingleByChannel(hit, position, position + TraceLen * direction, ECC_Visibility, FCollisionQueryParams());

		if (bTraceSuccess && hit.GetActor()->GetClass() == AArcGISMapActor::StaticClass() && hit.bBlockingHit)
		{
			FVector AdjustedPoint = hit.Location;
			UArcGISPoint* EndPoint = ArcGISMap->EngineToGeographic(AdjustedPoint);
			CreateAndCalculateEnvelope(StartPoint, EndPoint);
		}
		bIsDragging = false;		
	}
}

void AGeometryCreator::RenderLine(TArray<AActor*>& Points)
{
	//allPoints contains FeaturePoints plus the last segment for polygons
	TArray<AActor*> allPoints = Points;

	if (bIsPolygonMode && Stops.Num() >= 3)
	{
		for (AActor* Point : lastToStartInterpolationPoints)
		{
			allPoints.Add(Point);
		}

		// Close polygon only after interpolated points are added
		AActor* First = allPoints[0];
		if (IsValid(First) && First->GetActorTransform().IsValid())
		{
			allPoints.Add(First);
		}
	}

	for (int i = 1; i < allPoints.Num(); i++)
	{
		TObjectPtr<USplineMeshComponent> SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		SplineMesh->RegisterComponent();
		SplineMesh->SetMobility(EComponentMobility::Movable);

		FVector start = allPoints[i - 1]->GetActorLocation();
		FVector end = allPoints[i]->GetActorLocation();

		FVector tangent = (end - start).GetSafeNormal() * 100;

		SplineMesh->SetStartAndEnd(start, tangent, end, tangent);
		SplineMesh->SetStartScale(RouteCueScale);
		SplineMesh->SetEndScale(RouteCueScale);
		SplineMesh->SetStaticMesh(RouteMesh);

		SplineMeshComponents.AddHead(SplineMesh);
	}
}

void AGeometryCreator::ClearLine()
{
	for (auto point : FeaturePoints)
	{
		point->Destroy();
	}

	for (auto point : lastToStartInterpolationPoints)
	{
		point->Destroy();
	}

	for (auto stop : Stops)
	{
		stop->Destroy();
	}

	FeaturePoints.Empty();
	Stops.Empty();
	Calculation = 0;

	if (!SplineMeshComponents.IsEmpty())
	{
		for (auto point : SplineMeshComponents)
		{
			point->UnregisterComponent();
			point->DestroyComponent();
		}
		SplineMeshComponents.Empty();	
	}
}

void AGeometryCreator::CreateAndCalculatePolygon()
{
	UArcGISPolygonBuilder* polygonBuilder = UArcGISPolygonBuilder::CreateArcGISPolygonBuilderFromSpatialReference(SpatialReference);

	for (AActor* Stop : Stops)
	{
		auto stopGeo = ArcGISMap->EngineToGeographic(Stop->GetActorLocation());
		UArcGISPoint* location = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(stopGeo.X, stopGeo.Y, stopGeo.Z, SpatialReference);
		polygonBuilder->AddPoint(location);
	}

	auto polygon = Cast<UArcGISGeometry>(polygonBuilder->ToGeometry());

	Calculation = UArcGISGeometryEngine::AreaGeodetic(polygon, CurrentAreaUnit, EArcGISGeodeticCurveType::Geodesic);

	UpdateDisplay(Calculation);
}

void AGeometryCreator::CreateAndCalculateEnvelope(UArcGISPoint* Start, UArcGISPoint* End)
{
	double MinX = FMath::Min(Start->GetX(), End->GetX());
	double MinY = FMath::Min(Start->GetY(), End->GetY());
	double MaxX = FMath::Max(Start->GetX(), End->GetX());
	double MaxY = FMath::Max(Start->GetY(), End->GetY());

	UArcGISEnvelopeBuilder* EnvelopeBuilder = UArcGISEnvelopeBuilder::CreateArcGISEnvelopeBuilderFromSpatialReference(SpatialReference);
	EnvelopeBuilder->SetXY(MinX, MinY, MaxX, MaxY);

	UArcGISGeometry* Envelope = EnvelopeBuilder->ToGeometry();

	VisualizeEnvelope(MinX, MinY, MaxX, MaxY, SpatialReference);

	Calculation = UArcGISGeometryEngine::AreaGeodetic(Envelope, CurrentAreaUnit, EArcGISGeodeticCurveType::Geodesic);

	UpdateDisplay(Calculation);
}

void AGeometryCreator::VisualizeEnvelope(double MinX, double MinY, double MaxX, double MaxY, UArcGISSpatialReference* SpatialRef)
{
	ClearLine(); 

	TArray<UArcGISPoint*> Corners;
	Corners.Add(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(MinX, MinY, SpatialRef)); // Bottom Left
	Corners.Add(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(MaxX, MinY, SpatialRef)); // Bottom Right
	Corners.Add(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(MaxX, MaxY, SpatialRef)); // Top Right
	Corners.Add(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(MinX, MaxY, SpatialRef)); // Top Left

	TArray<AActor*> markers;

	for (UArcGISPoint* Corner : Corners)
	{
		FVector FlatWorldPos = ArcGISMap->GeographicToEngine(Corner);

		// Raycast to get terrain surface elevation
		FVector RayStart = FlatWorldPos + FVector(0, 0, 5000);
		FVector RayEnd = FlatWorldPos - FVector(0, 0, 5000);

		FHitResult Hit;
		if (GetWorld()->LineTraceSingleByChannel(Hit, RayStart, RayEnd, ECC_Visibility))
		{
			FVector AdjustedWorldPos = Hit.ImpactPoint + FVector(0, 0, MarkerHeight);

			SpawnParam.Owner = this;
			auto marker = GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), AdjustedWorldPos, FRotator(0), SpawnParam);

			if (marker)
			{
				markers.Add(marker);
				Stops.Add(marker);
			}
		}
	}

	// Connect corners and interpolate
	for (int32 i = 0; i < markers.Num(); ++i)
	{
		AActor* currentMarker = markers[i];
		AActor* nextMarker = markers[(i + 1) % markers.Num()];

		FeaturePoints.Add(currentMarker);
		Interpolate(currentMarker, nextMarker, FeaturePoints); 
	}

	FeaturePoints.Add(markers[0]);
	
	RenderLine(FeaturePoints); 
}

//update the visualization while dragging
void AGeometryCreator::UpdateDraggingVisualization()
{
	FVector WorldOrigin;
	FVector WorldDirection;
	FVector MousePosition;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PlayerController || !PlayerController->DeprojectMousePositionToWorld(WorldOrigin, WorldDirection))
	{
		return;
	}

	FHitResult Hit;
	FVector TraceEnd = WorldOrigin + (TraceLen * WorldDirection);

	if (GetWorld()->LineTraceSingleByChannel(Hit, WorldOrigin, TraceEnd, ECC_Visibility))
	{
		if (Hit.bBlockingHit && Hit.GetActor()->IsA<AArcGISMapActor>())
		{
			FVector AdjustedHitPoint = Hit.ImpactPoint;
			UArcGISPoint* CurrentPoint = ArcGISMap->EngineToGeographic(AdjustedHitPoint);

			CreateAndCalculateEnvelope(StartPoint, CurrentPoint);
		}
	}
}

float AGeometryCreator::GetCalculation()
{
	return Calculation;
}

void AGeometryCreator::SetCalculation(float calculation)
{
	Calculation = calculation;
}

UArcGISLinearUnit* AGeometryCreator::GetLinearUnit()
{
	return CurrentLinearUnit;
}

UArcGISAreaUnit* AGeometryCreator::GetAreaUnit()
{
	return CurrentAreaUnit;
}

void AGeometryCreator::SetLinearUnit(UArcGISLinearUnit* LU)
{
	CurrentLinearUnit = LU;
}

void AGeometryCreator::SetAreaUnit(UArcGISAreaUnit* AU)
{
	CurrentAreaUnit = AU;
}

void AGeometryCreator::UpdateDisplay(float Value)
{
	if (UIWidget)
	{
		UTextBlock* ResultText = Cast<UTextBlock>(UIWidget->GetWidgetFromName(TEXT("Result")));
		if (ResultText)
		{
			FString FormattedString = FString::Printf(TEXT("%.2f"), Value);
			ResultText->SetText(FText::FromString(FormattedString));
		}
	}
}

//Interpolate points between two given points, then returns an array of points excluding the two 
void AGeometryCreator::Interpolate(AActor* start, AActor* end, TArray<AActor*>& Points)
{
	FVector A = start->GetActorLocation();
	FVector B = end->GetActorLocation();
	float Dist = FVector::Dist(A, B);
	int32 Count = FMath::FloorToInt(Dist / InterpolationInterval);

	for (int32 i = 1; i < Count; ++i)
	{
		SpawnParam.Owner = this;
		FVector nextInterpolationPos = FMath::Lerp(A, B, i / static_cast<float>(Count));
		AActor* nextInterpolation =
			GetWorld()->SpawnActor<ABreadcrumb>(ABreadcrumb::StaticClass(), nextInterpolationPos, FRotator::ZeroRotator, SpawnParam);
		Points.Add(nextInterpolation);
	}
}






