#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ValleyVillager.generated.h"

class UTextRenderComponent;
class AValleyTerrain;

namespace vg
{
	struct Villager;
}

UCLASS()
class VALLEYGOD_API AValleyVillager : public AActor
{
	GENERATED_BODY()

public:
	AValleyVillager();
	virtual void Tick(float DeltaSeconds) override;

	void Arm(const struct vg::Villager& Sim);
	int32 GetVillagerId() const { return VillagerId; }
	void SyncFromSim(const struct vg::Villager& Sim, AValleyTerrain* Terrain, float WorldTime);

private:
	void BuildBody(const struct vg::Villager& Sim);
	UStaticMeshComponent* AddPart(const FName& Name, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat);

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY()
	TObjectPtr<UTextRenderComponent> Speech;

	UPROPERTY()
	TObjectPtr<UTextRenderComponent> Nameplate;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ArmL;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ArmR;

	int32 VillagerId = 0;
	float WalkPhase = 0.f;
	bool bWoman = true;
	float HeightScale = 1.f;
};
