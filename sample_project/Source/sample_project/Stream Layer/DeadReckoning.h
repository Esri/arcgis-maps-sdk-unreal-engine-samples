/* Copyright 2023 Esri
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

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DeadReckoning.generated.h"

UCLASS()
class SAMPLE_PROJECT_API ADeadReckoning : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADeadReckoning();
	static double ToRadians(double degrees);
	static double ToDegrees(double radians);
	static TArray<double> DeadReckoningPoint(double speed, double timespan, TArray<double> currentPoint, double headingDegrees);
	static TArray<double> MoveByDistanceAndHeading(TArray<double> currentPoint, double distanceMeters, double headingDegrees);
	static double Wrap360(int degrees);
};
