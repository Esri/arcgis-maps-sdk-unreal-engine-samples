/* Copyright 2024 Esri
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

#include "FeatureItem.h"
#include "Components/BoxComponent.h"

AFeatureItem::AFeatureItem()
{
	PrimaryActorTick.bCanEverTick = true;

	pin = CreateDefaultSubobject<UStaticMeshComponent>("PinMesh");
	pin->SetStaticMesh(pinMesh);
	pin->SetupAttachment(Root);
	pin->SetWorldScale3D(FVector(5, 5, 5));

	collider = CreateDefaultSubobject<UBoxComponent>("Collider");
	collider->SetupAttachment(Root);
	collider->SetRelativeLocation(FVector(0, 0, 1150));
	collider->SetWorldScale3D(FVector(5, 5, 10));
	collider->SetCollisionProfileName(FName("BlockAll"));
}
