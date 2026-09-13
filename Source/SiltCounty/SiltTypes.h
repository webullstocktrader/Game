#pragma once

#include "CoreMinimal.h"
#include "SiltTypes.generated.h"

UENUM(BlueprintType)
enum class ESiltDriver : uint8
{
	Chief,
	Gooch
};

UENUM(BlueprintType)
enum class ESiltSurface : uint8
{
	Mud,
	Pavement,
	Gravel,
	Grass,
	Water,
	Wood
};

UENUM(BlueprintType)
enum class ESiltGear : uint8
{
	Reverse,
	Neutral,
	Drive,
	Low
};

UENUM(BlueprintType)
enum class ESiltMatchPhase : uint8
{
	Booting,
	Intro,
	Playing,
	Complete
};

struct FSiltGroundHit
{
	bool bHit = false;
	FVector Position = FVector::ZeroVector;
	FVector Normal = FVector::UpVector;
	float SinkCm = 0.f;
	ESiltSurface Surface = ESiltSurface::Mud;
};

struct FSiltTruckInput
{
	float Throttle = 0.f;
	float Steer = 0.f;
	float Brake = 0.f;
	bool bHandbrake = false;
	bool bWinchToggle = false;
	bool bWinchIn = false;
	bool bWinchOut = false;
	bool bAirDown = false;
	bool bAirUp = false;
	bool bInteract = false;
	bool bReset = false;
	bool bSkipIntro = false;
};

namespace Silt
{
	UStaticMesh* CubeMesh();
	UStaticMesh* SphereMesh();
	UStaticMesh* CylinderMesh();
	UStaticMesh* PlaneMesh();
	UMaterialInterface* Material(const TCHAR* ShortName);
	UMaterialInterface* FallbackMaterial();

	FLinearColor SurfaceColor(ESiltSurface Surface);
	const TCHAR* DriverName(ESiltDriver Driver);
	const TCHAR* SurfaceName(ESiltSurface Surface);

	inline FName TagChief() { return FName(TEXT("Silt.Chief")); }
	inline FName TagGooch() { return FName(TEXT("Silt.Gooch")); }
	inline FName TagAnchor() { return FName(TEXT("Silt.Anchor")); }
	inline FName TagCrate() { return FName(TEXT("Silt.Crate")); }
}
