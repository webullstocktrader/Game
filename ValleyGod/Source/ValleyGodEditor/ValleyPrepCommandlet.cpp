#include "ValleyPrepCommandlet.h"
#include "ValleyContentFactory.h"

int32 UValleyPrepCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("VALLEY GOD: writing wet dirt and the slice map..."));
	const bool bOk = FValleyContentFactory::EnsureContent();
	UE_LOG(LogTemp, Display, TEXT("VALLEY GOD prep %s"), bOk ? TEXT("ok") : TEXT("failed"));
	return bOk ? 0 : 1;
}
