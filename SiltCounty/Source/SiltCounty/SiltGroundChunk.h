#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltGroundChunk.generated.h"

class UProceduralMeshComponent;
class UMaterialInterface;

UCLASS()
class SILTCOUNTY_API ASiltGroundChunk : public AActor
{
	GENERATED_BODY()

public:
	ASiltGroundChunk();

	void Build(const FVector2D& MinXY, const FVector2D& MaxXY, float Step, UMaterialInterface* GroundMat, UMaterialInterface* WaterMat);

private:
	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> Ground;

	UPROPERTY()
	TObjectPtr<UProceduralMeshComponent> Water;
};
