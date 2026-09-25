#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltGroundChunk.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

// One material per surface so physical materials and wetness parameters can differ.
// Fallback is used when an instance has not been generated yet.
struct FSiltGroundMaterials
{
	UMaterialInterface* Fallback = nullptr;
	UMaterialInterface* Road = nullptr;
	UMaterialInterface* Dirt = nullptr;
	UMaterialInterface* Mud = nullptr;
	UMaterialInterface* DeepMud = nullptr;
	UMaterialInterface* SiltBed = nullptr;
	UMaterialInterface* Water = nullptr;
};

UCLASS()
class SILTCOUNTY_API ASiltGroundChunk : public AActor
{
	GENERATED_BODY()

public:
	ASiltGroundChunk();

	void Build(const FVector2D& MinXY, const FVector2D& MaxXY, float Step, const FSiltGroundMaterials& Materials);

private:
	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> Ground;

	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> Water;
};
