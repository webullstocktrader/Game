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

	void Arm(int32 InId, const FLinearColor& Skin, const FLinearColor& Cloth, const FLinearColor& Hair);
	int32 GetVillagerId() const { return VillagerId; }
	void SyncFromSim(const struct vg::Villager& Sim, AValleyTerrain* Terrain, float WorldTime);

private:
	void BuildBody(const FLinearColor& Skin, const FLinearColor& Cloth, const FLinearColor& Hair);
	UStaticMeshComponent* AddPart(const FName& Name, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat);

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	UPROPERTY()
	TObjectPtr<UTextRenderComponent> Speech;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ArmL;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ArmR;

	int32 VillagerId = 0;
	float WalkPhase = 0.f;
};
