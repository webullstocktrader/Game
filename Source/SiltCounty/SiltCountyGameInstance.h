#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SiltCountyGameInstance.generated.h"

UCLASS()
class SILTCOUNTY_API USiltCountyGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY()
	bool bLocalCoop = true;
};
