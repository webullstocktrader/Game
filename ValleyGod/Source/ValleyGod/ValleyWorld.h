#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/ValleySim.h"
#include "ValleyWorld.generated.h"

class AValleyTerrain;
class AValleyVillager;
class AValleyAnimal;
class ADirectionalLight;
class ASkyLight;
class AExponentialHeightFog;
class APostProcessVolume;
class UPointLightComponent;

UCLASS()
class VALLEYGOD_API AValleyWorld : public AActor
{
	GENERATED_BODY()

public:
	AValleyWorld();
	virtual void Tick(float DeltaSeconds) override;

	void BuildValley();
	vg::World& Sim() { return Brain; }
	const vg::World& Sim() const { return Brain; }
	AValleyTerrain* GetTerrain() const { return Terrain; }
	AValleyVillager* FindVillager(int32 Id) const;
	void CommandWeather(vg::Weather Wx);
	void AdjustDayLength(float DeltaSeconds);
	void TogglePause();
	void PinVillager(int32 Id);
	int32 CyclePin();
	int32 GetPinnedId() const { return PinnedId; }

	static AValleyWorld* Get(const UWorld* World);

private:
	void StripTemplateActors();
	void SpawnAtmosphere();
	void SpawnTreesAndRocks();
	void SpawnSheltersAndFire();
	void SpawnPeople();
	void SpawnRain();
	void SpawnTornado();
	void UpdateSky();
	void UpdateWeatherVisuals(float DeltaSeconds);
	UStaticMeshComponent* Place(UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat, const FName& Name);

	vg::World Brain;

	UPROPERTY()
	TObjectPtr<AValleyTerrain> Terrain;

	UPROPERTY()
	TArray<TObjectPtr<AValleyVillager>> Villagers;

	UPROPERTY()
	TArray<TObjectPtr<AValleyAnimal>> Animals;

	UPROPERTY()
	TObjectPtr<ADirectionalLight> Sun;

	UPROPERTY()
	TObjectPtr<ASkyLight> Sky;

	UPROPERTY()
	TObjectPtr<AExponentialHeightFog> Fog;

	UPROPERTY()
	TObjectPtr<APostProcessVolume> Post;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> RainStreaks;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> TornadoParts;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Trees;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> FireLight;

	int32 PinnedId = -1;
	float RainClock = 0.f;
};
