#include "SiltPrepCommandlet.h"
#include "SiltContentFactory.h"

int32 USiltPrepCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("SILT COUNTY: writing wet materials and the slice map..."));
	const bool bOk = FSiltContentFactory::EnsureContent();
	UE_LOG(LogTemp, Display, TEXT("SILT COUNTY prep %s"), bOk ? TEXT("ok") : TEXT("failed"));
	return bOk ? 0 : 1;
}
