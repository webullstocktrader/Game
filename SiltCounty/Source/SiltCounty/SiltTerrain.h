#pragma once

#include "CoreMinimal.h"
#include "SiltTypes.h"

// Deterministic county query shared by terrain meshes, trucks, and mission markers.
// All distances are Unreal units (centimeters). The county is 8km x 8km.
namespace SiltTerrain
{
	float GetCountyMin();
	float GetCountyMax();
	float GetWaterLevel();

	float SampleHeight(float X, float Y);
	float DistanceToRoad(float X, float Y);
	ESiltSurface SampleSurface(float X, float Y);
	float SampleWetness(float X, float Y);
	const TCHAR* SurfaceLabel(ESiltSurface Surface);

	FVector GetChiefSpawn();
	FVector GetGoochSpawn();
	FRotator GetTruckYaw();
	FVector GetContractSpawn();
	FVector GetDropZone();
	FVector2D GetContractXY();
}
