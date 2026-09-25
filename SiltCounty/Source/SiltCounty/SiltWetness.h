#pragma once

#include "CoreMinimal.h"
#include "SiltTypes.h"

// Ambient FX wetness contract (Pass A).
//
// Wetness is one scalar, 0–1. Tire spray and ground mist gate their intensity
// on this value and on ESiltSurface (Road, Dirt, Mud, DeepMud, Water).
// Do not add a second wetness field beside it.
//
// Mud Water has not exposed a shared getter in this tree. SiltWetness::Wetness
// is the temporary stand-in: surface plus SinkAlpha (pass 0 where there is no
// truck sink, such as a ground mist sample).
//
// Hook: when Mud Water lands its API, change only the body of Wetness() so it
// returns that getter. Leave the spray and mist call sites on Wetness().
namespace SiltWetness
{
	float Wetness(ESiltSurface Surface, float SinkAlpha);
}
