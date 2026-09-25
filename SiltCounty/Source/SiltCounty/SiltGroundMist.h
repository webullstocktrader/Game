#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltGroundMist.generated.h"

class UInstancedStaticMeshComponent;

// Low cards over flooded town, the south slough, and the waterline.
// Card presence and size follow SiltWetness::Wetness (SampleWetness at X, Y). Separate from the fog volume:
// a short band, opacity capped, trucks stay readable.
UCLASS()
class SILTCOUNTY_API ASiltGroundMist : public AActor
{
	GENERATED_BODY()

public:
	ASiltGroundMist();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	void AddPatch(const FVector2D& Center, float Radius, int32 Count, FRandomStream& Rng);
	void AddWaterline(FRandomStream& Rng);
	bool TryAddCard(float X, float Y, FRandomStream& Rng);

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Cards;

	TArray<FVector> Anchors;
	TArray<float> Lifts;
	TArray<float> Spans;
	TArray<float> Phases;
	bool bReady = false;
};
