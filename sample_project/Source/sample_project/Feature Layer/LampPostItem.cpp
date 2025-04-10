/* Copyright 2025 Esri
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

#include "LampPostItem.h"
#include "ArcGISMapsSDK/Components/ArcGISLocationComponent.h"
#include "Components/BoxComponent.h"

ALampPostItem::ALampPostItem()
{
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	Root->SetMobility(EComponentMobility::Movable);
	RootComponent = Root;

	pin = CreateDefaultSubobject<UStaticMeshComponent>("PinMesh");
	pin->SetupAttachment(Root);
	//pin->SetStaticMesh(lampPostMesh);
	pin->SetWorldScale3D(FVector(1, 1, 1));
	
	pointLight = CreateDefaultSubobject<UPointLightComponent>("Point Light");
	pointLight->SetupAttachment(Root);
	pointLight->SetRelativeLocation(FVector(-270.000000,0.000000,1020.000000));
	pointLight->IntensityUnits = ELightUnits::Lumens;
	pointLight->Intensity = 200;
	pointLight->AttenuationRadius = 10000;
	pointLight->SourceRadius = 500;
	pointLight->SoftSourceRadius = 1000;
	pointLight->SourceLength = 384;
	
#if ENGINE_MINOR_VERSION > 5
	pointLight->bAllowMegaLights = true;
#endif

	pointLight->SetHiddenInGame(true);
	
	locationComponent = CreateDefaultSubobject<UArcGISLocationComponent>("Location Component");
	locationComponent->SetupAttachment(Root);
}
