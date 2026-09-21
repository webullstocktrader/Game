#include "SiltCountyGameState.h"

#include "Misc/CommandLine.h"
#include "Net/UnrealNetwork.h"

ASiltCountyGameState::ASiltCountyGameState()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ASiltCountyGameState::BeginPlay()
{
	Super::BeginPlay();
	if (HasAuthority() && PhaseStartServerTime < 0.f)
	{
		PhaseStartServerTime = GetServerWorldTimeSeconds();
		if (FParse::Param(FCommandLine::Get(), TEXT("silt.skipintro")))
		{
			AuthoritySkipIntro();
		}
	}
}

void ASiltCountyGameState::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority() || IntroPhase == ESiltIntroPhase::Gameplay)
	{
		return;
	}
	if (GetServerWorldTimeSeconds() - PhaseStartServerTime >= PhaseDuration())
	{
		AdvancePhase();
	}
}

void ASiltCountyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASiltCountyGameState, IntroPhase);
	DOREPLIFETIME(ASiltCountyGameState, PhaseStartServerTime);
}

float ASiltCountyGameState::PhaseDuration() const
{
	switch (IntroPhase)
	{
	case ESiltIntroPhase::FloodOverlook: return 4.5f;
	case ESiltIntroPhase::Garage: return 4.0f;
	case ESiltIntroPhase::Dialogue: return 6.0f;
	default: return 0.f;
	}
}

void ASiltCountyGameState::AdvancePhase()
{
	switch (IntroPhase)
	{
	case ESiltIntroPhase::FloodOverlook: IntroPhase = ESiltIntroPhase::Garage; break;
	case ESiltIntroPhase::Garage: IntroPhase = ESiltIntroPhase::Dialogue; break;
	default: IntroPhase = ESiltIntroPhase::Gameplay; break;
	}
	PhaseStartServerTime = GetServerWorldTimeSeconds();
	ForceNetUpdate();
}

void ASiltCountyGameState::AuthoritySkipIntro()
{
	if (!HasAuthority())
	{
		return;
	}
	IntroPhase = ESiltIntroPhase::Gameplay;
	PhaseStartServerTime = GetServerWorldTimeSeconds();
	ForceNetUpdate();
}

FText ASiltCountyGameState::GetSubtitle() const
{
	const float Elapsed = GetServerWorldTimeSeconds() - PhaseStartServerTime;
	switch (IntroPhase)
	{
	case ESiltIntroPhase::FloodOverlook:
		return INVTEXT("South Silt County is flooding. Roads are gone. People are stuck.");
	case ESiltIntroPhase::Garage:
		return INVTEXT("County garage. Two trucks in the bay. Keys are in them.");
	case ESiltIntroPhase::Dialogue:
		if (Elapsed < 3.f)
		{
			return INVTEXT("Gooch: Chief, you ready for this adventure?");
		}
		return INVTEXT("Chief: Hell yeah, brother, let's get it.");
	default:
		return FText::GetEmpty();
	}
}
