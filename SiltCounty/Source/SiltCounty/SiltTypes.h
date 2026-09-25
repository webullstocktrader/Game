#pragma once

#include "CoreMinimal.h"
#include "SiltTypes.generated.h"

UENUM(BlueprintType)
enum class ESiltIntroPhase : uint8
{
	FloodOverlook,
	Garage,
	Dialogue,
	Gameplay
};

UENUM(BlueprintType)
enum class ESiltSurface : uint8
{
	Road,
	Dirt,
	Mud,
	DeepMud,
	Water
};

UENUM(BlueprintType)
enum class ESiltContractState : uint8
{
	Locked,
	Available,
	Towing,
	Complete
};

UENUM(BlueprintType)
enum class ESiltJobKind : uint8
{
	RescueContract,
	RebuildBridge,
	RestorePower,
	ClearCulvert
};

// Locked personal-garage price. South Slough Pull pays the same amount once.
namespace SiltGarage
{
	inline constexpr float ChiefGarageUnlockPrice = 420.69f;
	inline constexpr float SouthSloughPullAward = 420.69f;
}
