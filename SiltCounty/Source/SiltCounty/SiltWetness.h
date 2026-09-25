#pragma once

#include "CoreMinimal.h"

// Ambient FX wetness contract.
//
// Wetness is one scalar, 0–1. Tire spray and ground mist gate their intensity
// on this value. SiltWetness::Wetness(X, Y) forwards to SiltTerrain::SampleWetness.
// Mud Water owns that curve, including Gravel and Asphalt on the road value.
// Ambient FX does not invent a wetness curve.
namespace SiltWetness
{
	float Wetness(float X, float Y);
}
