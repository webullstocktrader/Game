#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SiltTypes.h"
#include "SiltCountyGameState.generated.h"

UCLASS()
class SILTCOUNTY_API ASiltCountyGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
	ESiltMatchPhase Phase = ESiltMatchPhase::Booting;

	UPROPERTY()
	bool bCrateLoaded = false;

	UPROPERTY()
	bool bContractDone = false;

	FString ContractLine() const
	{
		if (bContractDone)
		{
			return TEXT("COUNTY CALL DONE. Crate's on Highway 6. That's the slice.");
		}
		if (bCrateLoaded)
		{
			return TEXT("COUNTY CALL: Crate's on the bed. Haul it to the Highway 6 lay-by.");
		}
		return TEXT("COUNTY CALL: Get the crate out of the ford. Haul it to the Highway 6 lay-by. If you bog, winch your brother.");
	}
};
