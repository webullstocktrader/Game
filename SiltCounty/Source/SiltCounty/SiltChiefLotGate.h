#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltChiefLotGate.generated.h"

class USceneComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

// Drive gate for Chief's fiction lot. Opens from replicated cash / bChiefGarageUnlocked.
UCLASS()
class SILTCOUNTY_API ASiltChiefLotGate : public AActor
{
	GENERATED_BODY()

public:
	ASiltChiefLotGate();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Hinge;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BarLow;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> BarHigh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> Sign;

	float OpenAlpha = 0.f;
	bool bOpening = false;
	bool bOpen = false;
};
