#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sim/ValleySim.h"
#include "ValleyAssets.h"
#include "ValleyWorld.generated.h"

class AValleyTerrain;
class AValleyVillager;
class AValleyAnimal;
class ADirectionalLight;
class ASkyLight;
class AExponentialHeightFog;
class APostProcessVolume;
class UPointLightComponent;
class UStaticMeshComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

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
	int32 CycleContinent();
	int32 GetPinnedId() const { return PinnedId; }
	int32 GetFocusedContinent() const { return FocusedContinent; }
	FString GraphicsStatusLine() const;

	static AValleyWorld* Get(const UWorld* World);

private:
	void StripTemplateActors();
	void SpawnAtmosphere();
	void SpawnTreesAndRocks(const Valley::FOptionalAssets& Assets);
	void SpawnMiniatureEarth();
	void SpawnTribeMarks();
	void SpawnSheltersAndFire();
	void PlaceShelter(int32 Index);
	void SpawnPeople();
	void EnsureSpawnedPopulation();
	void EnsureWorkVisuals();
	void SpawnRain();
	void SpawnTornado();
	void UpdateSky();
	void UpdateWeatherVisuals(float DeltaSeconds);
	UStaticMeshComponent* Place(UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat, const FName& Name);
	UStaticMeshComponent* PlaceSized(UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, float TargetHeightCm, const FName& Name);
	UHierarchicalInstancedStaticMeshComponent* FoliagePool(UStaticMesh* Mesh, const FName& Name);
	void AddSizedInstance(UHierarchicalInstancedStaticMeshComponent* Pool, const FVector& Loc, const FRotator& Rot, float TargetHeightCm);

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
	TArray<TObjectPtr<UStaticMeshComponent>> HarvestTrunks;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> HarvestCrowns;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SitePads;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SiteFrames;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SiteWalls;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> SiteRoofs;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> CampSpearShafts;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> CampSpearTips;

	UPROPERTY()
	TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> FoliagePools;

	TArray<float> TreeBaseYaw;

	UPROPERTY()
	TObjectPtr<UPointLightComponent> FireLight;

	UPROPERTY()
	TSubclassOf<AActor> MaraMetaHumanClass;

	int32 PinnedId = -1;
	int32 FocusedContinent = 0;
	int32 SpawnedShelters = 0;
	float RainClock = 0.f;
	bool bMaraMetaHuman = false;
	bool bQuixelGround = false;
	bool bQuixelFoliage = false;
};
