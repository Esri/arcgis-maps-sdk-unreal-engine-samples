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


#include "LampPostQueryLayer.h"
#include "ArcGISPawn.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

ALampPostQueryLayer::ALampPostQueryLayer()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ALampPostQueryLayer::BeginPlay()
{
	Super::BeginPlay();

	CreateLink();
	ProcessWebRequest();
	ArcGISPawn = Cast<AArcGISPawn>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
}

void ALampPostQueryLayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}
