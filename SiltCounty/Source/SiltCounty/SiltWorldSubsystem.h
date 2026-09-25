#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "SiltWorldSubsystem.generated.h"

class UMaterialInterface;

UCLASS()
class SILTCOUNTY_API USiltWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	void EnsureBuilt();
	bool IsBuilt() const { return bBuilt; }

private:
	bool bBuilt = false;

	void ClearTemplateActors(UWorld& World) const;
	void BuildTerrain(UWorld& World) const;
	void BuildPuddles(UWorld& World) const;
	void BuildDressing(UWorld& World) const;
	void BuildWeather(UWorld& World) const;

	UMaterialInterface* LoadMat(const TCHAR* ProjectPath, const TCHAR* Fallback) const;
	bool ChunkNeedsDetail(float CenterX, float CenterY) const;
};
