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
	spatialReference = UArcGISSpatialReference::CreateArcGISSpatialReference(3857);

	if (!inputManager)
	{
		inputManager = Cast<AInputManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AInputManager::StaticClass()));
	}

	inputManager->OnInputTrigger.AddDynamic(this, &AGeometryCreator::StartGeometry);

	// Make sure mouse cursor remains visible
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

}

void AGeometryCreator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
			UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(lineMarkerGeo.X, lineMarkerGeo.Y, lineMarkerGeo.Z, spatialReference);

		if (bIsEnvelopeMode)
		{
			//StartPoint = hitPoint;
			//bIsDragging = true;
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
					UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(lastStopGeo.X, lastStopGeo.Y, lastStopGeo.Z, spatialReference);
				
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

				FeaturePoints.Add(lastStop);
				Interpolate(lastStop, lineMarker, FeaturePoints);
				FeaturePoints.Add(lineMarker);
			}
				//Stops array stores user-added points
				Stops.Add(lineMarker);

			if (bIsPolygonMode)
			{
				if (Stops.Num() >= 3)
				{
					//compute the last segment
					Interpolate(lineMarker, Stops[0], lastToStartInterpolationPoints); 
					CreateandCalculatePolygon();
				}
			}
			if (Stops.Num() >= 2)
			{
				RenderLine(FeaturePoints);
			}
		}
	}
}

void AGeometryCreator::RenderLine(TArray<AActor*>& Points)
{
	TArray<AActor*> allPoints;

	// Build clean list of valid points
	for (AActor* Point : Points)
	{
		/*if (!IsValid(Point))
			continue;

		FVector Location = Point->GetActorLocation();
		if (Location.IsNearlyZero())
		{
			Point->Destroy();
			continue;
		}*/
		allPoints.Add(Point);
	}

	// For polygons, also add the last segement to allPoints 
	if (bIsPolygonMode && Stops.Num() >= 3)
	{
		for (AActor* Point : lastToStartInterpolationPoints)
		{
			/* if (!IsValid(Point))
				continue;

			FVector Location = Point->GetActorLocation();
			if (Location.IsNearlyZero())
			{
				Point->Destroy();
				continue;
			}*/
			allPoints.Add(Point);

		}

		// Close polygon only after interpolated points are added
		AActor* First = allPoints[0];
		if (IsValid(First) && First->GetActorTransform().IsValid())
		{
			allPoints.Add(First);
		}
	}

	// draw lines between allPoints
	for (int i = 1; i < allPoints.Num(); i++)
	{
		TObjectPtr<USplineMeshComponent> SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		SplineMesh->RegisterComponent();
		SplineMesh->SetMobility(EComponentMobility::Movable);

		FVector start = allPoints[i - 1]->GetActorLocation();
		FVector end = allPoints[i]->GetActorLocation();

		if (Cast<ARouteMarker>(allPoints[i - 1]))
			start.Z += MarkerHeight;
		if (Cast<ARouteMarker>(allPoints[i]))
			end.Z += MarkerHeight;

		FVector tangent = (end - start).GetSafeNormal() * 100;

		SplineMesh->SetStartAndEnd(start, tangent, end, tangent);
		SplineMesh->SetStartScale(RouteCueScale);
		SplineMesh->SetEndScale(RouteCueScale);
		SplineMesh->SetStaticMesh(RouteMesh);

		SplineMeshComponents.AddHead(SplineMesh);
	}
}


/* void AGeometryCreator::RenderLine(TArray<AActor*>& Points)
{
	TArray<AActor*> allPoints;

	for (AActor* Point : FeaturePoints)
	{
		if (!IsValid(Point)) 
			continue;
		FVector Location = Point->GetActorLocation();
		if (Location.IsNearlyZero()) 
		{
			Point->Destroy();
			continue;
		}
        allPoints.Add(Point);
	}

	if (bIsPolygonMode)
	{
		// Close the polygon by repeating the first point
		//allPoints.Add(allPoints[0]);
		AActor* First = allPoints[0];
		if (IsValid(First) && First->GetActorTransform().IsValid())
		{
			allPoints.Add(First);
		}
		for (AActor* Point : lastToStartInterpolationPoints)
		{
			if (!IsValid(Point))
				continue;
			FVector Location = Point->GetActorLocation();
			if (Location.IsNearlyZero())
			{
				Point->Destroy();
				continue;
			}
			allPoints.Add(Point);
		}
	}

	TObjectPtr<USplineMeshComponent> SplineMesh;
	FVector tangent;

	for (int i = 1; i < allPoints.Num(); i++)
	{
		SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		SplineMesh->RegisterComponent();
		SplineMesh->SetMobility(EComponentMobility::Movable);

		FVector end = allPoints[i]->GetActorLocation();

		// Since interpolations are already at correct elevation, only alter the route markers

		if (Cast<ARouteMarker>(allPoints[i]) != nullptr)
		{
			end.Z = end.Z + MarkerHeight;
		}

		FVector start = allPoints[i - 1]->GetActorLocation();

		// Since interpolations are already at correct elevation, only alter the route markers

		if (Cast<ARouteMarker>(allPoints[i - 1]) != nullptr)
		{
			start.Z += MarkerHeight;
		}

		tangent = end - start;
		tangent.Normalize();
		tangent = tangent * 100;

		SplineMesh->SetStartAndEnd(start, tangent, end, tangent);
		SplineMesh->SetStartScale(RouteCueScale);
		SplineMesh->SetEndScale(RouteCueScale);
		SplineMesh->SetStaticMesh(RouteMesh);
		SplineMeshComponents.AddHead(SplineMesh);
	}
}*/

void AGeometryCreator::ClearLine()
{
	if (!SplineMeshComponents.IsEmpty())
	{
		for (auto point : SplineMeshComponents)
		{
			point->UnregisterComponent();
			point->DestroyComponent();
		}

		for (auto stop : FeaturePoints)
		{
			stop->Destroy();
		}

		for (auto stop : lastToStartInterpolationPoints)
		{
			stop->Destroy();
		}

		for (auto stop : Stops)
		{
			stop->Destroy();
		}

		SplineMeshComponents.Empty();
		FeaturePoints.Empty();
		Stops.Empty();
		Calculation = 0;
	}
}

void AGeometryCreator::CreateandCalculatePolygon()
{
	UArcGISPolygonBuilder* polygonBuilder = UArcGISPolygonBuilder::CreateArcGISPolygonBuilderFromSpatialReference(spatialReference);
	// Add points from Stops
	for (AActor* Stop : Stops)
	{
		UArcGISLocationComponent* LocationComp = Stop->FindComponentByClass<UArcGISLocationComponent>();
		auto stopGeo = ArcGISMap->EngineToGeographic(Stop->GetActorLocation());
		UArcGISPoint* location = UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(stopGeo.X, stopGeo.Y, stopGeo.Z, spatialReference);
		polygonBuilder->AddPoint(location);
	}

	auto polygon = Cast<UArcGISGeometry>(polygonBuilder->ToGeometry());

	Calculation = UArcGISGeometryEngine::AreaGeodetic(polygon, CurrentAreaUnit, EArcGISGeodeticCurveType::Geodesic);

	UpdateDisplay(Calculation);
}

float AGeometryCreator::GetArea()
{
	return GeodeticArea;
}

void AGeometryCreator::SetArea(float area)
{
	GeodeticArea = area;
}

UArcGISAreaUnit* AGeometryCreator::GetUnit()
{
	return Unit;
}

void AGeometryCreator::SetUnit(UArcGISAreaUnit* unit)
{
	Unit = unit;
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

void AGeometryCreator::EndGeometry()
{
	/* if (bIsEnvelopeMode)
	{
		FVector End = GetRaycastHitLocation();
		if (!End.IsZero() && Stops.Num() > 0)
		{
			CreateEnvelope(Stops.Last()->GetActorLocation(), End);
			bIsDragging = false;
		}
	}*/
}
/*
void AGeometryCreator::CreateEnvelope(const FVector& Start, const FVector& End)
{
	FBox2D Envelope(FVector2D(FMath::Min(Start.X, End.X), FMath::Min(Start.Y, End.Y)),
					FVector2D(FMath::Max(Start.X, End.X), FMath::Max(Start.Y, End.Y)));
	VisualizeEnvelope(Envelope);
}

void AGeometryCreator::VisualizeEnvelope(const FBox2D& Envelope)
{
	ClearGeometry();
	FVector Corners[4] = {FVector(Envelope.Min.X, Envelope.Min.Y, MarkerHeight), FVector(Envelope.Max.X, Envelope.Min.Y, MarkerHeight),
						  FVector(Envelope.Max.X, Envelope.Max.Y, MarkerHeight), FVector(Envelope.Min.X, Envelope.Max.Y, MarkerHeight)};

	for (int32 i = 0; i < 4; ++i)
	{
		AActor* Marker = GetWorld()->SpawnActor<AActor>(LineMarkerPrefab, Corners[i], FRotator::ZeroRotator);
		SetElevation(Marker);
		Stops.Add(Marker);
		FeaturePoints.Add(Marker);
		InterpolatePoints(Marker, GetWorld()->SpawnActor<AActor>(LineMarkerPrefab, Corners[(i + 1) % 4], FRotator::ZeroRotator), FeaturePoints);
	}

	FeaturePoints.Add(FeaturePoints[0]);
	RenderLine(FeaturePoints);
}
*/



