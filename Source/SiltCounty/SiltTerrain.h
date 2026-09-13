#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltTypes.h"
#include "SiltTerrain.generated.h"

class UProceduralMeshComponent;

UCLASS()
class SILTCOUNTY_API ASiltTerrain : public AActor
{
	GENERATED_BODY()

public:
	ASiltTerrain();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void BuildCounty();

	FSiltGroundHit Query(const FVector& WorldPos) const;
	void ApplyTire(const FVector& WorldPos, float LoadN, float Slip01, float PSI, float DeltaSeconds);

	FVector GetGarageCenter() const { return GarageCenter; }
	FVector GetHighwayLayBy() const { return HighwayLayBy; }
	FVector GetFordCenter() const { return FordCenter; }
	FVector GetWashout() const { return WashoutCenter; }
	FRotator GetGarageFacing() const { return FRotator(0.f, 0.f, 0.f); }

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> GroundMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProceduralMeshComponent> WaterMesh;

	static constexpr int32 GridN = 181;
	static constexpr float CellCm = 200.f;
	static constexpr float HalfExtentCm = 18000.f;

private:
	int32 IndexOf(int32 IX, int32 IY) const { return IY * GridN + IX; }
	void WorldToIndex(float X, float Y, int32& IX, int32& IY) const;
	void WorldToIndexBilinear(float X, float Y, int32& X0, int32& Y0, int32& X1, int32& Y1, float& TX, float& TY) const;
	float SampleBase(int32 IX, int32 IY) const;
	float SampleSink(int32 IX, int32 IY) const;
	float HeightAt(float X, float Y) const;
	ESiltSurface SurfaceAt(float X, float Y) const;
	FVector NormalAt(float X, float Y) const;

	float ComputeBaseHeight(float X, float Y) const;
	ESiltSurface ComputeSurface(float X, float Y) const;
	float RiverDistance(float X, float Y) const;
	float HighwayDistance(float X, float Y) const;
	bool InGaragePad(float X, float Y) const;
	bool InWashout(float X, float Y) const;

	void RebuildDirty();
	void RebuildFull();
	void AddQuad(TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs, TArray<FColor>& Colors,
		const FVector& A, const FVector& B, const FVector& C, const FVector& D, const FColor& Color);

	TArray<float> BaseHeight;
	TArray<float> SinkCm;
	TArray<uint8> SurfaceId;
	TArray<uint8> Dirty;

	FVector GarageCenter;
	FVector HighwayLayBy;
	FVector FordCenter;
	FVector WashoutCenter;

	float RebuildTimer = 0.f;
	bool bAnyDirty = false;
};
