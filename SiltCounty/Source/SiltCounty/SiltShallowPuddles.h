#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltShallowPuddles.generated.h"

// Visual cards only. No collision, buoyancy, or reflection captures.
UCLASS()
class SILTCOUNTY_API ASiltShallowPuddles : public AActor
{
	GENERATED_BODY()

public:
	ASiltShallowPuddles();

	virtual void BeginPlay() override;
	void BuildField();

private:
	bool bFieldBuilt = false;
};
