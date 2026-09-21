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
	ASiltCountyGameState();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	ESiltIntroPhase GetIntroPhase() const { return IntroPhase; }
	bool IsGameplay() const { return IntroPhase == ESiltIntroPhase::Gameplay; }
	FText GetSubtitle() const;
	void AuthoritySkipIntro();

private:
	UPROPERTY(Replicated)
	ESiltIntroPhase IntroPhase = ESiltIntroPhase::FloodOverlook;

	UPROPERTY(Replicated)
	float PhaseStartServerTime = -1.f;

	float PhaseDuration() const;
	void AdvancePhase();
};
