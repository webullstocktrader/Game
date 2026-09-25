#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltRainActor.generated.h"

class UInstancedStaticMeshComponent;

UCLASS()
class SILTCOUNTY_API ASiltRainActor : public AActor
{
	GENERATED_BODY()

public:
	ASiltRainActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Drops;

	TArray<FVector> Velocities;

	// Pass A: one continuous light-moderate field. No storm ramp.
	static constexpr int32 DropCount = 280;
};
