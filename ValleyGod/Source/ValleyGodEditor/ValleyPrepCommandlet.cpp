#include "ValleyPrepCommandlet.h"
#include "ValleyContentFactory.h"

UValleyPrepCommandlet::UValleyPrepCommandlet()
{
	IsClient = false;
	IsEditor = true;
	IsServer = false;
	LogToConsole = true;
	ShowErrorCount = true;
	UseCommandletResultAsExitCode = true;
}

int32 UValleyPrepCommandlet::Main(const FString& Params)
{
	UE_LOG(LogTemp, Display, TEXT("VALLEY GOD: baking M_* materials into Content/Materials..."));
	const bool bOk = FValleyContentFactory::EnsureContent();
	UE_LOG(LogTemp, Display, TEXT("VALLEY GOD prep %s"), bOk ? TEXT("ok") : TEXT("failed"));
	return bOk ? 0 : 1;
}
