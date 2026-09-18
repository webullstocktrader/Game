#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "ValleyPrepCommandlet.generated.h"

UCLASS()
class UValleyPrepCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UValleyPrepCommandlet();
	virtual int32 Main(const FString& Params) override;
};
