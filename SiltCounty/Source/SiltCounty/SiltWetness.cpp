#include "SiltWetness.h"

#include "SiltCounty.h"

float SiltWetness::Wetness(ESiltSurface Surface, float SinkAlpha)
{
	static bool bLogged = false;
	if (!bLogged)
	{
		bLogged = true;
		UE_LOG(LogSiltCounty, Display, TEXT("Silt County Wetness: Pass A stand-in from surface + SinkAlpha. Swap SiltWetness::Wetness when Mud Water exposes its getter."));
	}

	// Driveable mud and shallow puddles. Sink deepens mud; road and dirt stay a film.
	const float Sink = FMath::Clamp(SinkAlpha, 0.f, 1.f);
	switch (Surface)
	{
	case ESiltSurface::Road:
		return 0.05f;
	case ESiltSurface::Dirt:
		return 0.22f;
	case ESiltSurface::Mud:
		return 0.82f * (0.75f + 0.25f * Sink);
	case ESiltSurface::DeepMud:
		return 1.f * (0.75f + 0.25f * Sink);
	case ESiltSurface::Water:
		return 0.95f;
	default:
		return 0.f;
	}
}
