#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "SiltTypes.h"
#include "SiltTireSprayComponent.generated.h"

class UInstancedStaticMeshComponent;

// One wheel sample for local spray. Drive code is not changed.
USTRUCT()
struct FSiltWheelSpray
{
	GENERATED_BODY()

	UPROPERTY()
	bool bGrounded = false;

	UPROPERTY()
	ESiltSurface Surface = ESiltSurface::Dirt;

	UPROPERTY()
	FVector Contact = FVector::ZeroVector;

	UPROPERTY()
	FVector PointVelocity = FVector::ZeroVector;

	UPROPERTY()
	FVector Outward = FVector::RightVector;
};

USTRUCT()
struct FSiltSprayPuff
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Pos = FVector::ZeroVector;

	UPROPERTY()
	FVector Vel = FVector::ZeroVector;

	UPROPERTY()
	float Age = 1.f;

	UPROPERTY()
	float Life = 0.25f;

	UPROPERTY()
	float Size = 0.2f;

	UPROPERTY()
	bool bLive = false;
};

// Local tire spray and mud kick. Not replicated. Niagara is enabled on the project,
// but this environment cannot bake Niagara systems, so Pass A uses instanced sprites.
UCLASS(ClassGroup = (Silt), meta = (BlueprintSpawnableComponent))
class SILTCOUNTY_API USiltTireSprayComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	USiltTireSprayComponent();

	virtual void BeginPlay() override;

	// SinkAlpha is fed into SiltWetness::Wetness with each wheel's ESiltSurface.
	void UpdateWheels(float DeltaSeconds, float SinkAlpha, const FSiltWheelSpray* Wheels, int32 Count);

private:
	void ConfigureField(UInstancedStaticMeshComponent* Field) const;
	UMaterialInterface* LoadFx(const TCHAR* Name, const TCHAR* Fallback) const;
	void Seed(UInstancedStaticMeshComponent* Field, TArray<FSiltSprayPuff>& Puffs, int32 Count);
	void Emit(TArray<FSiltSprayPuff>& Puffs, int32& Cursor, const FVector& Pos, const FVector& Vel, float Life, float Size);
	void Simulate(UInstancedStaticMeshComponent* Field, TArray<FSiltSprayPuff>& Puffs, float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Spray;

	UPROPERTY()
	TObjectPtr<UInstancedStaticMeshComponent> Kick;

	UPROPERTY()
	TArray<FSiltSprayPuff> SprayPuffs;

	UPROPERTY()
	TArray<FSiltSprayPuff> KickPuffs;

	int32 SprayCursor = 0;
	int32 KickCursor = 0;
	float SprayCharge[4] = {};
	float KickCharge[4] = {};
	bool bReady = false;
};
