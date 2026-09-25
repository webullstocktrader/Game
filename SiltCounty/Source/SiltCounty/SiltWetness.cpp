#include "SiltWetness.h"

#include "SiltCounty.h"
#include "SiltTerrain.h"

float SiltWetness::Wetness(float X, float Y)
{
	static bool bLogged = false;
	if (!bLogged)
	{
		bLogged = true;
		UE_LOG(LogSiltCounty, Display, TEXT("Silt County Wetness comes from SiltTerrain::SampleWetness."));
	}

	return SiltTerrain::SampleWetness(X, Y);
}
