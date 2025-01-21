#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Materials/MaterialInterface.h"
#include "ViewshedMap.generated.h"

UCLASS()
class SAMPLE_PROJECT_API AViewshed : public AActor
{
	GENERATED_BODY()
	
public:	
	AViewshed();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	UMaterialInterface* ViewshedMaterial;

	UFUNCTION()
	void InitializeMap();

	FTimerHandle TimerHandle;
};
