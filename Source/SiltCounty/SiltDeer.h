#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltDeer.generated.h"

UCLASS()
class SILTCOUNTY_API ASiltDeer : public AActor
{
	GENERATED_BODY()

public:
	ASiltDeer();
	virtual void Tick(float DeltaSeconds) override;
	void Arm(const FVector& Start, const FVector& WanderA, const FVector& WanderB);

private:
	void BuildBody();
	FVector NearestThreat() const;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	FVector Home = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	FVector Velocity = FVector::ZeroVector;
	float Calm = 1.f;
	float Hop = 0.f;
};
