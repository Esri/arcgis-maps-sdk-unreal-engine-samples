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

#include "RouteManager.h"

#include "ArcGISMapsSDK/Utils/ArcGISMapsSDKProjectSettings.h"
#include "sample_project/InputManager.h"

// Sets default values
ARouteManager::ARouteManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned.
void ARouteManager::BeginPlay()
{
	Super::BeginPlay();

	InputManager->OnInputTrigger.AddDynamic(this, &ARouteManager::AddStop);

	// Make sure mouse cursor remains visible.
	auto playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (playerController)
	{
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
	}

	// Create the UI and add it to the viewport.
	if (UIWidgetClass)
	{
		UIWidget = CreateWidget<UUserWidget>(GetWorld(), UIWidgetClass);
		HideInstructions = UIWidget->FindFunction(FName("HideDirections"));
		if (UIWidget)
		{
			UIWidget->AddToViewport();
		}
	}

	auto mapActor = UGameplayStatics::GetActorOfClass(GetWorld(), AArcGISMapActor::StaticClass());
	if (mapActor)
	{
		MapComponent = mapActor->GetComponentByClass<UArcGISMapComponent>();
	}
}

void ARouteManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	InputManager->OnInputTrigger.RemoveDynamic(this, &ARouteManager::AddStop);
}

void ARouteManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If new routing information has been received, visualize the route.
	if (bShouldUpdateBreadcrumbs)
	{
		UpdateBreadcrumbs();
		bShouldUpdateBreadcrumbs = false;
	}
}

void ARouteManager::UpdateBreadcrumbs()
{
	FHitResult traceHit;
	FVector3d worldLocation;
	bool bTraceSuccess = false;
	uint16 counter = 0;
	FVector3d tangent;
	USplineMeshComponent* splineMesh;
	FVector3d* bcLocations = new FVector3d[Breadcrumbs.Num()];

	for (auto breadcrumb : Breadcrumbs)
	{
		RemoveTickPrerequisiteComponent(breadcrumb->ArcGISLocation);

		// Do a line trace to determine the height of breadcrumbs.
		worldLocation = FVector3d(breadcrumb->GetActorLocation().X, breadcrumb->GetActorLocation().Y, TraceLength / 2.f);
		bTraceSuccess = GetWorld()->LineTraceSingleByChannel(traceHit, worldLocation, worldLocation + TraceLength * FVector3d::DownVector,
															 ECC_Visibility, FCollisionQueryParams());
		worldLocation.Z = bTraceSuccess ? traceHit.ImpactPoint.Z + HeightOffset : 0.;

		breadcrumb->SetActorLocation(worldLocation);
		bcLocations[counter] = worldLocation;
		counter++;
	}

	// Remove the old route cue.
	for (auto spc : SplineMeshComponents)
	{
		spc->UnregisterComponent();
		spc->DestroyComponent();
	}
	SplineMeshComponents.Empty();

	// Add a spline mesh for each segment of the route.
	for (int i = 0; i < counter; i++)
	{
		if (i > 0)
		{
			splineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			splineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
			splineMesh->RegisterComponent();
			splineMesh->SetMobility(EComponentMobility::Movable);

			tangent = bcLocations[i] - bcLocations[i - 1];
			tangent.Normalize();
			tangent = tangent * 100.;

			splineMesh->SetStartAndEnd(bcLocations[i - 1], tangent, bcLocations[i], tangent);
			splineMesh->SetStartScale(RouteCueScale);
			splineMesh->SetEndScale(RouteCueScale);

			splineMesh->SetStaticMesh(RouteMesh);

			SplineMeshComponents.AddHead(splineMesh);
		}
	}
	delete[] bcLocations;
}

// Identify the location clicked on and make a line trace from there to find the point for placing the stop.
void ARouteManager::AddStop()
{
	FVector worldLocation;
	FVector worldDirection;
	APlayerController* playerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	playerController->DeprojectMousePositionToWorld(worldLocation, worldDirection);

	FHitResult traceHit;
	bool bTraceSuccess = GetWorld()->LineTraceSingleByChannel(traceHit, worldLocation, worldLocation + TraceLength * worldDirection, ECC_Visibility,
															  FCollisionQueryParams());
	if (!bIsRouting && bTraceSuccess && traceHit.GetActor()->GetClass() == AArcGISMapActor::StaticClass() && traceHit.bBlockingHit)
	{
		if (!MapComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("Could not find map component."));
			return;
		}

		Stops.AddHead(MapComponent->TransformEnginePositionToPoint(traceHit.ImpactPoint));

		// Update the list of stops.
		if (Stops.Num() > StopCount)
		{
			Stops.RemoveNode(Stops.GetTail());
		}
		if (Stops.Num() >= 1)
		{
			StartMarker->SetActorLocation(MapComponent->TransformPointToEnginePosition(Stops.GetTail()->GetValue()));
			StartMarker->SetActorHiddenInGame(false);
		}

		// Make a routing query if enough stops added.
		if (Stops.Num() == StopCount)
		{
			bIsRouting = true;
			PostRoutingRequest();

			EndMarker->SetActorLocation(MapComponent->TransformPointToEnginePosition(Stops.GetHead()->GetValue()));
			EndMarker->SetActorHiddenInGame(false);
		}
	}
}

void ARouteManager::ClearMap()
{
	// Remove the old route cue.
	for (auto spc : SplineMeshComponents)
	{
		spc->UnregisterComponent();
		spc->DestroyComponent();
	}
	SplineMeshComponents.Empty();

	// Remove the old breadcrumbs.
	for (auto Breadcrumb : Breadcrumbs)
	{
		Breadcrumb->Destroy();
	}
	Breadcrumbs.Empty();
	Stops.Empty();

	StartMarker->SetActorHiddenInGame(true);
	EndMarker->SetActorHiddenInGame(true);

	// Set the UI text to zero.
	auto widgetFunction = UIWidget->FindFunction(FName("SetTravelInfo"));
	UIWidget->ProcessEvent(widgetFunction, new FString("0"));
}

// Make a query for routing between the selected stops.
void ARouteManager::PostRoutingRequest()
{
	FString requestBody;
	FString stopCoordinates;
	UArcGISPoint* point;

	// Set up the query.
	FHttpRequestRef request = FHttpModule::Get().CreateRequest();
	request->OnProcessRequestComplete().BindUObject(this, &ARouteManager::ProcessQueryResponse);
	request->SetURL(RoutingServiceURL);
	request->SetVerb("POST");
	request->SetHeader("Content-Type", "application/x-www-form-urlencoded");

	// Make a string of the coordinates of the stops.
	for (auto stop : Stops)
	{
		point = stop;

		// If the geographic coordinates of the stop are not in terms of lat & lon, project them.
		if (point->GetSpatialReference()->GetWKID() != 4326)
		{
			auto ProjectedGeometry = UArcGISGeometryEngine::Project(point, UArcGISSpatialReference::CreateArcGISSpatialReference(4326));
			if (ProjectedGeometry != nullptr)
			{
				point = static_cast<UArcGISPoint*>(ProjectedGeometry);
			}
		}
		stopCoordinates.Append(FString::Printf(TEXT("%f,%f;"), point->GetX(), point->GetY()));
	}
	stopCoordinates.RemoveFromEnd(";");

	if (!MapComponent)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find map component."));
		return;
	}

	// Read the API key from the map component.
	FString apiToken = MapComponent ? MapComponent->GetAPIKey() : "";

	if (apiToken.IsEmpty())
	{
		if (const UArcGISMapsSDKProjectSettings* Settings = GetDefault<UArcGISMapsSDKProjectSettings>())
		{
			apiToken = Settings->APIKey;
		}
	}

	// Set the request body and send it.
	requestBody = TEXT("f=json&returnRoutes=true&token=") + apiToken + TEXT("&stops=") + stopCoordinates;
	request->SetContentAsString(requestBody);
	request->ProcessRequest();
}

void ARouteManager::ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	TSharedPtr<FJsonObject> jsonObj;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	// Process the response if the query was successful.
	if (FJsonSerializer::Deserialize(reader, jsonObj) && Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300)
	{
		ABreadcrumb* bc;
		double minutes;
		FString infoMessage;
		TSharedPtr<FJsonValue> routesField;
		TSharedPtr<FJsonValue> errorField;
		TSharedPtr<FJsonValue> geometryField;
		TSharedPtr<FJsonValue> attributesField;
		const TArray<TSharedPtr<FJsonValue>>* featuresField;
		const TArray<TSharedPtr<FJsonValue>>* pathsField;
		const TArray<TSharedPtr<FJsonValue>>* pointsField;
		const TArray<TSharedPtr<FJsonValue>>* stopCoordinates;

		// Remove the old breadcrumbs.
		for (auto breadcrumb : Breadcrumbs)
		{
			breadcrumb->Destroy();
		}
		Breadcrumbs.Empty();

		// Find the function that sets the info text of the UI.
		UFunction* widgetFunction = UIWidget->FindFunction(FName("SetTravelInfo"));

		// Parse the query response.
		if ((routesField = jsonObj->TryGetField(TEXT("routes"))))
		{
			jsonObj = routesField->AsObject();
			if (jsonObj->TryGetArrayField(TEXT("features"), featuresField))
			{
				for (auto feature : *featuresField)
				{
					jsonObj = feature->AsObject();
					attributesField = jsonObj->TryGetField(TEXT("attributes")); // checked later.
					if ((geometryField = jsonObj->TryGetField(TEXT("geometry"))))
					{
						jsonObj = geometryField->AsObject();
						if (jsonObj->TryGetArrayField(TEXT("paths"), pathsField))
						{
							for (auto path : *pathsField)
							{
								pointsField = &path->AsArray();
								for (auto point : *pointsField)
								{
									stopCoordinates = &point->AsArray();

									// Create a breadcrumb for each path point and set its location from the query response.
									FActorSpawnParameters SpawnParam = FActorSpawnParameters();
									SpawnParam.Owner = this;
									bc = GetWorld()->SpawnActor<ABreadcrumb>(ABreadcrumb::StaticClass(), FVector3d(0.), FRotator3d(0.), SpawnParam);

									bc->ArcGISLocation->SetPosition(UArcGISPoint::CreateArcGISPointWithXYSpatialReference(
										(*stopCoordinates)[0]->AsNumber(), (*stopCoordinates)[1]->AsNumber(),
										UArcGISSpatialReference::CreateArcGISSpatialReference(4326)));

									// Make sure this actor doesn't tick before all Breadcrumb actors have ticked.
									// Ensures that the transform of all Breadcrum actors have been updated before adding the route cue.
									AddTickPrerequisiteComponent(bc->ArcGISLocation);
									Breadcrumbs.AddHead(bc);
								}
							}
						}
					}
					if (attributesField)
					{
						jsonObj = attributesField->AsObject();
						if (jsonObj->TryGetNumberField(TEXT("Total_TravelTime"), minutes))
						{
							// Update the travel time info in the UI.
							if (widgetFunction)
							{
								infoMessage = FString::Printf(TEXT("%.2f"), minutes);
								UIWidget->ProcessEvent(widgetFunction, &infoMessage);
							}
						}
					}
				}
				// Visualize the route in the next tick.
				bShouldUpdateBreadcrumbs = true;
			}
		}
		else if ((errorField = jsonObj->TryGetField(TEXT("error"))))
		{
			jsonObj = errorField->AsObject();

			// Show the error message in the UI.
			if (jsonObj->TryGetStringField(TEXT("message"), infoMessage))
			{
				if (widgetFunction)
				{
					UIWidget->ProcessEvent(widgetFunction, &infoMessage);
				}
			}
		}
	}
	bIsRouting = false;
}