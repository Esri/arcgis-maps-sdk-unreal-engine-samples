// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Json.h"
#include "Http.h"
#include "QueryLocation.h"
#include "ArcGISMapsSDK/Components/ArcGISMapComponent.h"
#include "ArcGISMapsSDK/BlueprintNodes/GameEngine/Geometry/ArcGISSpatialReference.h"
#include "Geocoder.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AGeocoder : public AActor
{
	GENERATED_BODY()
	
public:	
	virtual void Tick(float DeltaTime) override;
	AGeocoder();
	
	UFUNCTION(BlueprintCallable)
	void SendAddressQuery(FString Address);

	//UFUNCTION(BlueprintCallable)
	//void SendLocationQuery(UArcGISPoint Point);

	UFUNCTION(Category = "Debug", CallInEditor, BlueprintCallable)
	void SendRequest();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	FString InAddress;

protected:
	virtual void BeginPlay() override;

private: 
	void ProcessQueryResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSucessfully);

	AQueryLocation* QueryLocation;
	UInputComponent* InputComponent;
};
