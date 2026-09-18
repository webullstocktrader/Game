#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ValleyAnimal.generated.h"

class AValleyTerrain;

namespace vg
{
	struct Animal;
}

UCLASS()
class VALLEYGOD_API AValleyAnimal : public AActor
{
	GENERATED_BODY()

public:
	AValleyAnimal();
	void Arm(int32 InId);
	int32 GetAnimalId() const { return AnimalId; }
	void SyncFromSim(const struct vg::Animal& Sim, AValleyTerrain* Terrain, float WorldTime);

private:
	void BuildBody();

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	int32 AnimalId = 0;
	float Hop = 0.f;
};
