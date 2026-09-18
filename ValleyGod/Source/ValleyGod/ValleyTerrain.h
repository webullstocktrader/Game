#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ValleyTerrain.generated.h"

class UProceduralMeshComponent;

UCLASS()
class VALLEYGOD_API AValleyTerrain : public AActor
{
	GENERATED_BODY()

public:
	AValleyTerrain();

	void BuildValley();
	float HeightAt(float X, float Y) const;
	FVector GroundAt(const FVector& WorldPos) const;
	void SetWet(bool bWet);
	void SetFlood(float ExtraZ);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> GroundMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> WaterMesh;

	static constexpr int32 GridN = 81;
	static constexpr float CellCm = 180.f;
	static constexpr float HalfExtentCm = 7200.f;

private:
	int32 IndexOf(int32 IX, int32 IY) const { return IY * GridN + IX; }
	float ComputeBaseHeight(float X, float Y) const;
	float RiverDistance(float X, float Y) const;
	bool IsGrass(float X, float Y, float Height) const;
	bool IsStone(float X, float Y, float Height) const;
	bool IsMud(float X, float Y, float Height) const;
	void Rebuild();
	void AddQuad(TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs, TArray<FColor>& Colors,
		const FVector& A, const FVector& B, const FVector& C, const FVector& D, const FColor& Color);

	TArray<float> BaseHeight;
	float FloodExtraZ = 0.f;
	bool bWet = false;
};
