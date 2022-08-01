// Fill out your copyright notice in the Description page of Project Settings.


#include "Geocoder.h"

// Sets default values
AGeocoder::AGeocoder()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void AGeocoder::SendRequest()
{
	//if (InAddress.IsEmpty()) {

	//	SendAddressQuery(TEXT("1600 Pennsylvania Ave NW,DC"));
	//}
	//else {
	//	SendAddressQuery(InAddress);

	//}

	//SendLocationQuery(UArcGISPoint::CreateArcGISPointWithXY(-79.99,40.69));
	SendLocationQuery(UArcGISPoint::CreateArcGISPointWithXY(-76, 40));


}

// Called when the game starts or when spawned
void AGeocoder::BeginPlay()
{
	Super::BeginPlay();

	SetupInput();

	// Make sure mouse cursor remains visible
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (PC)
	{
		PC->bShowMouseCursor = true;
		//PC->bEnableClickEvents = true;

	}
	GetWorld()->GetGameViewport()->SetMouseCaptureMode(EMouseCaptureMode::CaptureDuringMouseDown);


}

// Called every frame
void AGeocoder::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bShouldSendLocationQuery) {
		SendLocationQuery(QueryLocation->ArcGISLocation->GetPosition());
		RemoveTickPrerequisiteComponent(QueryLocation->ArcGISLocation);
		bShouldSendLocationQuery = false;
	}
}

// Bind the handler for selecting a stop point
void AGeocoder::SetupInput()
{
	if (!InputComponent) {
		InputComponent = NewObject<UInputComponent>(this);
	}

	InputComponent->RegisterComponent();
	InputComponent->BindAction("PlaceRoutePoint", IE_Pressed, this, &AGeocoder::SelectLocation);
	EnableInput(GetWorld()->GetFirstPlayerController());
}

// Make a query for routing between the selected stops 
void AGeocoder::SendAddressQuery(FString Address)
{
	if (bWaitingForResponse) {
		return;
	}

	UArcGISMapComponent* MapComponent = UArcGISMapComponent::GetMapComponent(this);

	FString Url = "https://geocode-api.arcgis.com/arcgis/rest/services/World/GeocodeServer/findAddressCandidates";
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";
	FString Query;

	// Set up the query 
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGeocoder::ProcessAddressQueryResponse);
	Query = FString::Printf(TEXT("%s/?f=json&token=%s&address=%s"), *Url, *APIToken, *Address);
	Request->SetURL(Query.Replace(TEXT(" "), TEXT("%20")));
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "x-www-form-urlencoded"); //application/json  ---  application/x-www-form-urlencoded
	Request->ProcessRequest();
	bWaitingForResponse = true;
}

void AGeocoder::ProcessAddressQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully)
{
	FString ResponseAddress = "";
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	UE_LOG(LogTemp, Warning, TEXT("response code %d"), Response->GetResponseCode());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, JsonObj) &&
		Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {
		const TArray<TSharedPtr<FJsonValue>>* Candidates;
		TSharedPtr<FJsonValue> Location;
		double PointX, PointY;
		//AQueryLocation* LocationActor;

		if (JsonObj->TryGetArrayField(TEXT("candidates"), Candidates)) {
			if (Candidates->Num() > 0) {
				TSharedPtr<FJsonValue> candidate = (*Candidates)[0];

				JsonObj = candidate->AsObject();
				if (JsonObj->TryGetStringField(TEXT("Address"), ResponseAddress)) {
					UE_LOG(LogTemp, Warning, TEXT("Address: %s"), *ResponseAddress);
				}

				if (Location = JsonObj->TryGetField(TEXT("location"))) {
					JsonObj = Location->AsObject();
					JsonObj->TryGetNumberField("x", PointX);
					JsonObj->TryGetNumberField("y", PointY);

					// Spawn a QueryLocation actor is not already created
					if (QueryLocation == nullptr) {
						FActorSpawnParameters SpawnParam = FActorSpawnParameters();
						SpawnParam.Owner = this;
						QueryLocation = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(), FVector3d(0.), FRotator3d(0.), SpawnParam);
					}
					// Place the QueryLocation actor at the queried XY location and at high altitude
					QueryLocation->SetupAddressQuery(UArcGISPoint::CreateArcGISPointWithXYZSpatialReference(
						PointX, PointY, 10000,
						UArcGISSpatialReference::CreateArcGISSpatialReference(4326)), ResponseAddress);
				}
			}
		}
	}
	bWaitingForResponse = false;
}

void AGeocoder::SendLocationQuery(UArcGISPoint* InPoint)
{
	if (bWaitingForResponse) {
		return;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////// TODO convert spatial reference if needed
	UArcGISMapComponent* MapComponent = UArcGISMapComponent::GetMapComponent(this);

	FString Url = "https://geocode.arcgis.com/arcgis/rest/services/World/GeocodeServer/reverseGeocode";
	FString APIToken = MapComponent ? MapComponent->GetAPIkey() : "";
	FString Query;

	// Set up the query 
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	Request->OnProcessRequestComplete().BindUObject(this, &AGeocoder::ProcessLocationQueryResponse);
	Query = FString::Printf(TEXT("%s/?f=json&location=%f,%f"), *Url, InPoint->GetX(), InPoint->GetY());
	Request->SetURL(Query.Replace(TEXT(" "), TEXT("%20")));
	Request->SetVerb("GET");
	Request->SetHeader("Content-Type", "x-www-form-urlencoded");
	UE_LOG(LogTemp, Warning, TEXT("--------%s"), *Query);
	Request->ProcessRequest();
	bWaitingForResponse = true;
}

void AGeocoder::ProcessLocationQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully) {
	FString ResponseAddress = "";
	TSharedPtr<FJsonObject> JsonObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

	//UE_LOG(LogTemp, Warning, TEXT("response code %d"), Response->GetResponseCode());
	UE_LOG(LogTemp, Warning, TEXT("%s"), *Response->GetContentAsString());

	if (FJsonSerializer::Deserialize(Reader, JsonObj) && Response->GetResponseCode() > 199 && Response->GetResponseCode() < 300) {

		TSharedPtr<FJsonValue> AddressField;
		if (AddressField = JsonObj->TryGetField((TEXT("address")))) {


			JsonObj = AddressField->AsObject();
			if (JsonObj->TryGetStringField(TEXT("Match_addr"), ResponseAddress)) {
				UE_LOG(LogTemp, Warning, TEXT(">>>>>>>>>Address: %s"), *ResponseAddress);
			}
		}
	}

	if (QueryLocation != nullptr) {
		QueryLocation->UpdateAddressCue(ResponseAddress);
	}
	bWaitingForResponse = false;
}

//// Identify the location clicked on and make a line trace from there to find the point for placing the stop 
void AGeocoder::SelectLocation()
{
	if (bWaitingForResponse) {
		return;
	}

	float TraceLength = 1000000.f;
	FVector WorldLocation;
	FVector WorldDirection;
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection);

	FHitResult TraceHit;
	if (GetWorld()->LineTraceSingleByChannel(TraceHit,
		WorldLocation, WorldLocation + TraceLength * WorldDirection, ECC_Visibility, FCollisionQueryParams()))
	{
		if (TraceHit.GetActor()->GetClass() == AArcGISMapActor::StaticClass())
		{
			UE_LOG(LogTemp, Warning, TEXT("--------clicked %s"), *TraceHit.GetActor()->GetActorLabel());

			if (QueryLocation == nullptr)
			{
				FActorSpawnParameters SpawnParam = FActorSpawnParameters();
				SpawnParam.Owner = this;
				QueryLocation = GetWorld()->SpawnActor<AQueryLocation>(AQueryLocation::StaticClass(),
					FVector3d(0.), FRotator(0.), SpawnParam);

			}

			QueryLocation->SetupLocationQuery(TraceHit.ImpactPoint);

			AddTickPrerequisiteComponent(QueryLocation->ArcGISLocation);
			bShouldSendLocationQuery = true;

		}
	}

}
