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


#include "DeadReckoning.h"

ADeadReckoning::ADeadReckoning()
{
	PrimaryActorTick.bCanEverTick = true;
}

double ADeadReckoning::ToRadians(double degrees)
{
	return degrees * PI / 180.0;
}

double ADeadReckoning::ToDegrees(double radians)
{
	return radians * 180.0 / PI;
}

TArray<double> ADeadReckoning::DeadReckoningPoint(double speed, double timespan, TArray<double> currentPoint, double headingDegrees)
{
	auto predictiveDistance = speed * timespan;
	return MoveByDistanceAndHeading(currentPoint, predictiveDistance, headingDegrees);
}

TArray<double> ADeadReckoning::MoveByDistanceAndHeading(TArray<double> currentPoint, double distanceMeters, double headingDegrees)
{
	auto distRatio = distanceMeters / 6356752.3142;
	auto distRatioSine = FMath::Sin(distRatio);
	auto distRatioCosine = FMath::Cos(distRatio);

	auto startLonRad = ToRadians(currentPoint[0]);
	auto startLatRad = ToRadians(currentPoint[1]);

	auto startLatCos = FMath::Cos(startLatRad);
	auto startLatSin = FMath::Sin(startLatRad);

	auto endLatRads = FMath::Asin((startLatSin * distRatioCosine) + (startLatCos * distRatioSine * FMath::Cos(ToRadians(headingDegrees))));
	auto endLonRads = startLonRad + FMath::Atan2(FMath::Sin(ToRadians(headingDegrees)) * distRatioSine * startLatCos,
	                                             distRatioCosine - startLatSin * FMath::Sin(endLatRads));

	auto newLong = ToDegrees(endLonRads);
	auto newLat = ToDegrees(endLatRads);
	auto newPoint = {newLong, newLat};
	return newPoint;
}

double ADeadReckoning::Wrap360(int degrees)
{
	if (0 <= degrees && degrees < 360)
	{
		return degrees; // avoid rounding due to arithmetic ops if within range
	}
	else
	{
		return (degrees % 360 + 360) % 360; // sawtooth wave p:360, a:360
	} // sawtooth wave p:360, a:360
}