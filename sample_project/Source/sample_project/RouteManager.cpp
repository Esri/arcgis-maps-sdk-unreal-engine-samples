// Fill out your copyright notice in the Description page of Project Settings.

#include "RouteManager.h"



// Sets default values
ARouteManager::ARouteManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ARouteManager::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();

	MapComponent = UArcGISMapComponent::GetMapComponent(this);

	
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		PC->bEnableClickEvents = true;
	}
	
}

// Called every frame
void ARouteManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

//AActor* ARouteManager::CreateMarker(FVector3d InEnginePosition)
//{
//	AActor* Marker = GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), InEnginePosition, FRotator(0.f));
//	//
//	
//	Marker->AddComponentByClass(USceneComponent::StaticClass(),false, FTransform3d(),false);
//	
//	UActorComponent* MeshComponent = Marker->AddComponentByClass(UStaticMeshComponent::StaticClass(),false, FTransform3d(),false);
//	
//	
//	
//	/*USceneComponent* Root = NewObject<USceneComponent>(Marker, USceneComponent::StaticClass(), TEXT("RootComponent"));
//	Marker->SetRootComponent(Root);
//
//	UStaticMeshComponent* MeshComponent = NewObject<UStaticMeshComponent>(Marker, UStaticMeshComponent::StaticClass(), TEXT("MarkerMesh"));
//	Marker->addcompo
//	MeshComponent->SetupAttachment(Root);
//
//	*/
//	if (MarkerMesh != nullptr) {
//
//		Cast<UStaticMeshComponent>(MeshComponent)->SetStaticMesh(MarkerMesh);
//	}
//	else {
//		UE_LOG(LogTemp, Warning, TEXT("No mesh"));
//
//	}
//
//	//UArcGISLocationComponent* ArcGISLocation = Marker->CreateDefaultSubobject<UArcGISLocationComponent>(TEXT("ArcGISLocation"));
//	//ArcGISLocation->SetupAttachment(Root);
//
//	return Marker;
//}

void ARouteManager::SetupInput()
{
	InputComponent = NewObject<UInputComponent>(this);
	InputComponent->RegisterComponent();

	if (InputComponent)
	{
		InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &ARouteManager::AddStop);
		EnableInput(GetWorld()->GetFirstPlayerController());
	}

}

void ARouteManager::AddStop()
{


	////using namespace Esri::ArcGISMapsSDK::Utils::GeoCoord;
	//using namespace Esri::GameEngine::Geometry;

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	FVector WorldLocation;
	FVector WorldDirection;
	float DistanceAboveGround = 5000.0f;
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FVector PlaneOrigin(0.0f, 0.0f, DistanceAboveGround);
	FVector ActorWorldLocation = FMath::LinePlaneIntersection(
		WorldLocation,
		WorldLocation + WorldDirection,
		PlaneOrigin,
		FVector::UpVector);

	FHitResult TraceHit;
	if (!bIsRouting && GetWorld()->LineTraceSingleByChannel(TraceHit,
		WorldLocation, WorldLocation + 100000.f * WorldDirection, ECC_Visibility, FCollisionQueryParams()))
	{
		if (TraceHit.bBlockingHit)
		{
			//if (GEngine) {

			//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("You are hitting: %s"), *TraceHit.GetActor()->GetName()));
			//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Impact Point: %s"), *TraceHit.ImpactPoint.ToString()));
			//	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Red, FString::Printf(TEXT("Normal Point: %s"), *TraceHit.ImpactNormal.ToString()));
			//}
			//UE_LOG(LogTemp, Warning, TEXT("You are hitting: %s"), *TraceHit.GetActor()->GetName());
			//UE_LOG(LogTemp, Warning, TEXT("Impact Point: %s"), *TraceHit.ImpactPoint.ToString());

			Stops.AddHead(GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), TraceHit.ImpactPoint, FRotator(0.f)));
			//RouteStops.Enqueue(GetWorld()->SpawnActor<ARouteMarker>(ARouteMarker::StaticClass(), TraceHit.ImpactPoint, FRotator(0.f)));
			//StopCount++;

			if (Stops.Num() > StopCount) {
				auto OldStop = Stops.GetTail();
				//ARouteMarker* OldMarker;
				//RouteStops.Dequeue(OldMarker);
				//StopCount--;
				//OldMarker->Destroy();

				OldStop->GetValue()->Destroy();
				Stops.RemoveNode(OldStop);
			}

			if (Stops.Num() == StopCount) {
				bIsRouting = true;
				PostRoutingRequest();
				
			}
		

		}
		//DrawDebugLine(GetWorld(), WorldLocation, WorldLocation + 100000.f * WorldDirection, FColor::Green, false, 10, 0, 1);

	}

	////UE_LOG(LogTemp, Warning, TEXT("Mouse Location: %f, %f, %f"), ActorWorldLocation.X, ActorWorldLocation.Y, ActorWorldLocation.Z);
	//CreateMarker(TraceHit.ImpactPoint);

	//AStaticMeshActor* MyNewActor = GetWorld()->SpawnActor<AStaticMeshActor>();
	//MyNewActor->SetMobility(EComponentMobility::Movable);
	//MyNewActor->SetActorLocation(TraceHit.ImpactPoint);
		//if (MarkerMesh != nullptr) {

	//	MyNewActor->FindComponentByClass<UStaticMeshComponent>()->SetStaticMesh(MyMesh);
	//}
	//else {
	//	UE_LOG(LogTemp, Warning, TEXT("No mesh"));

	//}

	//UArcGISPoint* pos = Marker->ArcGISLocation->GetPosition();

	//ArcGISPoint* pt = new ArcGISPoint(
	//	pos->GetX(), pos->GetY(), pos->GetZ(),
	//	ArcGISSpatialReference(pos->GetSpatialReference()));


	//ArcGISGeometry newgeom = ArcGISGeometryEngine::Project(*pt, ArcGISSpatialReference::WGS84());

	//ArcGISPoint* projpt = new ArcGISPoint(newgeom.GetHandle());

	////FGeoPosition ProjPos = GeoUtils::WebMercatorToWGS84LonLatAlt(FVector3d(pos->GetX(), pos->GetY(), pos->GetZ()));
	//////FGeoPosition ProjPos(0, 0, 0, 4326);
	////UE_LOG(LogTemp, Warning, TEXT("Position: (%f,%f,%f):%d -> (%f,%f,%f):4326"), 
	////	pos->GetX(), pos->GetY(), pos->GetZ(), pos->GetSpatialReference()->GetWKID(),
	////	ProjPos.X, ProjPos.Y, ProjPos.Z );

	//UE_LOG(LogTemp, Warning, TEXT("**Position: (%f,%f,%f):%d -> (%f,%f,%f):4326"),
	//	pos->GetX(), pos->GetY(), pos->GetZ(), *pos->GetSpatialReference()->GetWKText(),
	//	projpt->GetX(), projpt->GetY(), projpt->GetZ());


	//newgeom = ArcGISGeometryEngine::Project(*projpt,
	//	ArcGISSpatialReference(pos->GetSpatialReference()));


	//pt = new ArcGISPoint(newgeom.GetHandle());
	//UE_LOG(LogTemp, Warning, TEXT("converted back to %d: (%f,%f,%f)"), pt->GetSpatialReference().GetWKID(),
	//	pt->GetX(), pt->GetY(), pt->GetZ());


	////Marker->FindComponentByClass<UStaticMeshComponent>()->SetStaticMesh(MyMesh);
	////UStaticMeshComponent* MeshComponent;
	////MeshComponent = MyNewActor->GetStaticMeshComponent();
	////if (MyMesh)
	////	MeshComponent->SetStaticMesh(MyMesh);


}


void ARouteManager::PostRoutingRequest()
{
					/////////////////////////////////////////// TODO replace token

	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();


	//TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	//RequestObj->SetStringField(TEXT("f"), TEXT("json"));
	//RequestObj->SetStringField("token", "AAPKd92e3919a64e4758ae48061d88d35ee49rXW5paRxGMNDnw3m2hBpZXAElRU7MszfXmiCooHvKGW-vwB4patDMQdu72nTYiL");
	//RequestObj->SetStringField("stops", "-122.68782,45.51238;-122.690176,45.522054");
	//RequestObj->SetStringField("returnRoutes", "true"); // different in Unity

	FString RequestBody;
	FString StopCoordinates;
	//TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	//FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &ARouteManager::ProcessQueryResponse);
	//Request->SetURL("https://jsonplaceholder.typicode.com/posts");
	Request->SetURL("https://route-api.arcgis.com/arcgis/rest/services/World/Route/NAServer/Route_World/solve");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/x-www-form-urlencoded"); //application/json

	
	UArcGISPoint* Point;
	for (auto Stop: Stops){

		//ARouteMarker* Stop; 
		//Stops.Dequeue(Stop);
		
		//Stop->ArcGISLocation->GetPosition();
		Point = Stop->ArcGISLocation->GetPosition();
		//UArcGISSpatialReference* SR =  Point->GetSpatialReference();

		//Point = UArcGISPoint::CreateArcGISPointWithXYSpatialReference(5065036.83, 7284618.10, UArcGISSpatialReference::UArcGISSpatialReference::CreateArcGISSpatialReference(3857));
		
		UE_LOG(LogTemp, Display, TEXT("%f"), Point->GetX());

		if (Point->GetSpatialReference()->GetWKID() != 4326) {
			auto geometry = UArcGISGeometryEngine::Project(Point, UArcGISSpatialReference::CreateArcGISSpatialReference(4326));
			if (geometry != nullptr)
			{
				Point = static_cast<UArcGISPoint*>(geometry);
			}
		}

		StopCoordinates.Append(FString::Printf(TEXT("%f,%f;"), Point->GetX(), Point->GetY()));
	}

	StopCoordinates.RemoveFromEnd(";");

	//RequestBody = TEXT("f=json&token=AAPKd92e3919a64e4758ae48061d88d35ee49rXW5paRxGMNDnw3m2hBpZXAElRU7MszfXmiCooHvKGW-vwB4patDMQdu72nTYiL&stops=-74.000426,40.730123;-73.998477,40.729644");
	
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";


	RequestBody = TEXT("f=json&token=") + APIToken + "&stops=" + StopCoordinates;
	
	
	
	
	Request->SetContentAsString(RequestBody);
	//for (auto z: Request->GetContent())
	//	UE_LOG(LogTemp, Display, TEXT("--------%d"), z);

	//UE_LOG(LogTemp, Display, TEXT("--------%s"), *RequestBody);

	Request->ProcessRequest();

}

void ARouteManager::ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	if (FJsonSerializer::Deserialize(Reader, JsonObj)) {


		UE_LOG(LogTemp, Warning, TEXT("Response Code %d"), Response->GetResponseCode());
		UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());

		//FJsonSerializableArray arr;
		//if (ResponseObj->TryGetStringArrayField("routes", arr)) {
		//	while (!arr.IsEmpty())
		//	{
		//		UE_LOG(LogTemp, Display, TEXT("-------%d"), arr.Pop().Len());
		//	}
		//}
		//else {
		//	UE_LOG(LogTemp, Display, TEXT("Not Found"));

		//}


		//TSharedPtr<FJsonObject> ResponseObj;
		float Minutes;
		TSharedPtr<FJsonValue> routes;
		TSharedPtr<FJsonValue> geometry;
		TSharedPtr<FJsonValue> attributes;
		const TArray<TSharedPtr<FJsonValue>>* features;
		const TArray<TSharedPtr<FJsonValue>>* paths;
		const TArray<TSharedPtr<FJsonValue>>* points;
		const TArray<TSharedPtr<FJsonValue>>* coordinates;



		if (routes = JsonObj->TryGetField(TEXT("routes"))) {
			JsonObj = routes->AsObject();
			if (JsonObj->TryGetArrayField(TEXT("features"), features)) {

				for (auto feature : *features) {
					JsonObj = feature->AsObject();
					attributes = JsonObj->TryGetField(TEXT("attributes")); // checked later
					if (geometry = JsonObj->TryGetField(TEXT("geometry"))) {
						JsonObj = geometry->AsObject();
						if (JsonObj->TryGetArrayField(TEXT("paths"), paths)) {
							for (auto path : *paths) {
								points = &path->AsArray();
								for (auto point : *points) {
									coordinates = &point->AsArray();
									UE_LOG(LogTemp, Display, TEXT("+++++++++++++++[%f , %f]"), (*coordinates)[0]->AsNumber(), (*coordinates)[1]->AsNumber());
								}
							}
						}
						else {
							UE_LOG(LogTemp, Warning, TEXT("paths-------------"));
						}
					}
					else {
						UE_LOG(LogTemp, Warning, TEXT("geometry-------------"));
					}
					if (attributes) {
						JsonObj = attributes->AsObject();
						if (JsonObj->TryGetNumberField(TEXT("Total_TravelTime"), Minutes)) {
							UE_LOG(LogTemp, Display, TEXT("+++++++++++++++ time: %f "), Minutes);
						}
					}
					else {
						UE_LOG(LogTemp, Warning, TEXT("attributes-------------"));

					}
				}

			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("features-------------"));
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("routes-------------"));
		}
	}
	bIsRouting = false;
}