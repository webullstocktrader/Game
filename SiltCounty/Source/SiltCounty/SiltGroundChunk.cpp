#include "SiltGroundChunk.h"

#include "ProceduralMeshComponent.h"
#include "SiltTerrain.h"

namespace
{
	FLinearColor GroundColor(ESiltSurface Surface, float Noise)
	{
		FLinearColor Color;
		float Roughness = 0.55f;
		switch (Surface)
		{
		case ESiltSurface::Road:
			Color = FLinearColor(0.16f, 0.15f, 0.13f);
			Roughness = 0.48f;
			break;
		case ESiltSurface::Dirt:
			Color = FLinearColor(0.20f, 0.15f, 0.09f);
			Roughness = 0.62f;
			break;
		case ESiltSurface::Mud:
			Color = FLinearColor(0.09f, 0.055f, 0.028f);
			Roughness = 0.24f;
			break;
		case ESiltSurface::DeepMud:
			Color = FLinearColor(0.045f, 0.028f, 0.016f);
			Roughness = 0.10f;
			break;
		case ESiltSurface::Water:
		default:
			Color = FLinearColor(0.03f, 0.025f, 0.018f);
			Roughness = 0.08f;
			break;
		}

		Color.R = FMath::Clamp(Color.R + Noise * 0.03f, 0.f, 1.f);
		Color.G = FMath::Clamp(Color.G + Noise * 0.025f, 0.f, 1.f);
		Color.B = FMath::Clamp(Color.B + Noise * 0.015f, 0.f, 1.f);
		Color.A = Roughness;
		return Color;
	}
}

ASiltGroundChunk::ASiltGroundChunk()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);
	Tags.Add(TEXT("SiltCounty"));

	Ground = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Ground"));
	SetRootComponent(Ground);
	Ground->SetMobility(EComponentMobility::Movable);
	Ground->bUseComplexAsSimpleCollision = true;
	Ground->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Ground->SetCollisionProfileName(TEXT("BlockAll"));
	Ground->SetCanEverAffectNavigation(false);
	Ground->bCastDynamicShadow = true;

	Water = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Water"));
	Water->SetupAttachment(Ground);
	Water->SetMobility(EComponentMobility::Movable);
	Water->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Water->SetCanEverAffectNavigation(false);
	Water->bCastDynamicShadow = false;
}

void ASiltGroundChunk::Build(const FVector2D& MinXY, const FVector2D& MaxXY, float Step, UMaterialInterface* GroundMat, UMaterialInterface* WaterMat)
{
	const float SpanX = FMath::Max(Step, MaxXY.X - MinXY.X);
	const float SpanY = FMath::Max(Step, MaxXY.Y - MinXY.Y);
	const int32 NumX = FMath::Clamp(FMath::RoundToInt(SpanX / Step) + 1, 2, 220);
	const int32 NumY = FMath::Clamp(FMath::RoundToInt(SpanY / Step) + 1, 2, 220);

	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> Colors;
	TArray<FProcMeshTangent> Tangents;

	Vertices.SetNum(NumX * NumY);
	UVs.SetNum(NumX * NumY);
	Colors.SetNum(NumX * NumY);

	for (int32 Y = 0; Y < NumY; ++Y)
	{
		for (int32 X = 0; X < NumX; ++X)
		{
			const float AlphaX = static_cast<float>(X) / static_cast<float>(NumX - 1);
			const float AlphaY = static_cast<float>(Y) / static_cast<float>(NumY - 1);
			const float WorldX = FMath::Lerp(MinXY.X, MaxXY.X, AlphaX);
			const float WorldY = FMath::Lerp(MinXY.Y, MaxXY.Y, AlphaY);
			const int32 Index = Y * NumX + X;
			const float Height = SiltTerrain::SampleHeight(WorldX, WorldY);
			Vertices[Index] = FVector(WorldX, WorldY, Height);
			UVs[Index] = FVector2D(WorldX * 0.0004f, WorldY * 0.0004f);
			const float Noise = FMath::Frac(FMath::Sin(WorldX * 0.013f + WorldY * 0.017f) * 43758.5453f) * 2.f - 1.f;
			Colors[Index] = GroundColor(SiltTerrain::SampleSurface(WorldX, WorldY), Noise);
		}
	}

	Triangles.Reserve((NumX - 1) * (NumY - 1) * 6);
	for (int32 Y = 0; Y < NumY - 1; ++Y)
	{
		for (int32 X = 0; X < NumX - 1; ++X)
		{
			const int32 I = Y * NumX + X;
			Triangles.Add(I);
			Triangles.Add(I + 1);
			Triangles.Add(I + NumX);

			Triangles.Add(I + 1);
			Triangles.Add(I + NumX + 1);
			Triangles.Add(I + NumX);
		}
	}

	Normals.Init(FVector::ZeroVector, Vertices.Num());
	for (int32 Tri = 0; Tri + 2 < Triangles.Num(); Tri += 3)
	{
		const int32 I0 = Triangles[Tri];
		const int32 I1 = Triangles[Tri + 1];
		const int32 I2 = Triangles[Tri + 2];
		const FVector Normal = FVector::CrossProduct(Vertices[I1] - Vertices[I0], Vertices[I2] - Vertices[I0]).GetSafeNormal();
		Normals[I0] += Normal;
		Normals[I1] += Normal;
		Normals[I2] += Normal;
	}
	Tangents.Reserve(Vertices.Num());
	for (int32 Index = 0; Index < Vertices.Num(); ++Index)
	{
		FVector Normal = Normals[Index].GetSafeNormal();
		if (Normal.IsNearlyZero() || Normal.Z < 0.f)
		{
			Normal = FVector::UpVector;
		}
		Normals[Index] = Normal;
		Tangents.Add(FProcMeshTangent(FVector(1.f, 0.f, 0.f), false));
	}

	Ground->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, Colors, Tangents, true);
	if (GroundMat)
	{
		Ground->SetMaterial(0, GroundMat);
	}

	TArray<FVector> WaterVerts;
	TArray<int32> WaterTris;
	TArray<FVector> WaterNormals;
	TArray<FVector2D> WaterUVs;
	TArray<FLinearColor> WaterColors;
	TArray<FProcMeshTangent> WaterTangents;
	const float WaterZ = SiltTerrain::GetWaterLevel() + 12.f;

	for (int32 Y = 0; Y < NumY - 1; ++Y)
	{
		for (int32 X = 0; X < NumX - 1; ++X)
		{
			const int32 I00 = Y * NumX + X;
			const int32 I10 = I00 + 1;
			const int32 I01 = I00 + NumX;
			const int32 I11 = I01 + 1;
			const float Lowest = FMath::Min3(Vertices[I00].Z, Vertices[I10].Z, FMath::Min(Vertices[I01].Z, Vertices[I11].Z));
			if (Lowest > SiltTerrain::GetWaterLevel() + 30.f)
			{
				continue;
			}

			const int32 Base = WaterVerts.Num();
			const FVector Corners[4] = { Vertices[I00], Vertices[I10], Vertices[I11], Vertices[I01] };
			for (const FVector& Corner : Corners)
			{
				const float Depth = FMath::Clamp((SiltTerrain::GetWaterLevel() - Corner.Z) / 400.f, 0.f, 1.f);
				WaterVerts.Add(FVector(Corner.X, Corner.Y, WaterZ));
				WaterUVs.Add(FVector2D(Corner.X * 0.0002f, Corner.Y * 0.0002f));
				FLinearColor Wet = FMath::Lerp(FLinearColor(0.10f, 0.16f, 0.13f), FLinearColor(0.02f, 0.045f, 0.05f), Depth);
				Wet.A = FMath::Lerp(0.12f, 0.03f, Depth);
				WaterColors.Add(Wet);
			}
			WaterTris.Add(Base);
			WaterTris.Add(Base + 1);
			WaterTris.Add(Base + 3);
			WaterTris.Add(Base + 1);
			WaterTris.Add(Base + 2);
			WaterTris.Add(Base + 3);
		}
	}

	if (WaterVerts.Num() == 0)
	{
		Water->SetVisibility(false);
		return;
	}

	WaterNormals.Init(FVector::UpVector, WaterVerts.Num());
	WaterTangents.Init(FProcMeshTangent(FVector(1.f, 0.f, 0.f), false), WaterVerts.Num());
	Water->CreateMeshSection_LinearColor(0, WaterVerts, WaterTris, WaterNormals, WaterUVs, WaterColors, WaterTangents, false);
	Water->SetVisibility(true);
	if (WaterMat)
	{
		Water->SetMaterial(0, WaterMat);
	}
}
