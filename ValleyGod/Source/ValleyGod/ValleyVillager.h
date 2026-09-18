#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ValleyVillager.generated.h"

class UTextRenderComponent;
class UCapsuleComponent;
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

	void Arm(const struct vg::Villager& Sim, UClass* PresentationClass = nullptr);
	int32 GetVillagerId() const { return VillagerId; }
	bool IsUsingMetaHuman() const { return Presentation != nullptr; }
	void SyncFromSim(const struct vg::Villager& Sim, AValleyTerrain* Terrain, float WorldTime);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void BuildBody(const struct vg::Villager& Sim);
	void AttachLabels(const struct vg::Villager& Sim);
	void SpawnPresentation(UClass* PresentationClass);
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

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ThighL;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> ThighR;

	UPROPERTY()
	TObjectPtr<UCapsuleComponent> ClickProbe;

	UPROPERTY()
	TObjectPtr<AActor> Presentation;

	int32 VillagerId = 0;
	float WalkPhase = 0.f;
	bool bWoman = true;
	float HeightScale = 1.f;
};
