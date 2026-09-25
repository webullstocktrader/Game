#include "SiltGroundChunk.h"

#include "ProceduralMeshComponent.h"
#include "SiltTerrain.h"

namespace
{
	FLinearColor GroundColor(ESiltSurface Surface, float Noise)
	{
		// Map-bible palette: olive mud, black wet asphalt, wet gravel, saturated soil.
		// Vertex color RGB = albedo tint; A = roughness for M_WetGround.
		FLinearColor Color;
		float Roughness = 0.55f;
		switch (Surface)
		{
		case ESiltSurface::Asphalt:
			Color = FLinearColor(0.045f, 0.045f, 0.048f);
			Roughness = 0.22f;
			break;
		case ESiltSurface::Gravel:
		case ESiltSurface::Road:
			Color = FLinearColor(0.22f, 0.20f, 0.17f);
			Roughness = 0.58f;
			break;
		case ESiltSurface::Dirt:
			// Wet olive soil (not dry brown farm dirt).
			Color = FLinearColor(0.14f, 0.13f, 0.07f);
			Roughness = 0.48f;
			break;
		case ESiltSurface::Mud:
			Color = FLinearColor(0.08f, 0.075f, 0.035f);
			Roughness = 0.18f;
			break;
		case ESiltSurface::DeepMud:
			Color = FLinearColor(0.035f, 0.040f, 0.022f);
			Roughness = 0.08f;
			break;
		case ESiltSurface::Water:
		default:
			Color = FLinearColor(0.025f, 0.035f, 0.032f);
			Roughness = 0.06f;
			break;
		}

		// Soft mottling; asphalt stays darker / tighter.
		const float Mottling = (Surface == ESiltSurface::Asphalt) ? 0.012f : 0.028f;
		Color.R = FMath::Clamp(Color.R + Noise * Mottling, 0.f, 1.f);
		Color.G = FMath::Clamp(Color.G + Noise * Mottling * 0.9f, 0.f, 1.f);
		Color.B = FMath::Clamp(Color.B + Noise * Mottling * 0.7f, 0.f, 1.f);
		Color.A = Roughness;
		return Color;
	}

	float Hash01(float X, float Y)
	{
		return FMath::Frac(FMath::Sin(X * 0.013f + Y * 0.017f) * 43758.5453f);
	}

	UMaterialInterface* MaterialFor(ESiltSurface Surface, const FSiltGroundMaterials& Materials)
	{
		UMaterialInterface* Chosen = nullptr;
		switch (Surface)
		{
		case ESiltSurface::Road:
		case ESiltSurface::Gravel:
		case ESiltSurface::Asphalt:
			Chosen = Materials.Road;
			break;
		case ESiltSurface::Dirt: Chosen = Materials.Dirt; break;
		case ESiltSurface::Mud: Chosen = Materials.Mud; break;
		case ESiltSurface::DeepMud: Chosen = Materials.DeepMud; break;
		default: Chosen = Materials.SiltBed; break;
		}
		return Chosen ? Chosen : Materials.Fallback;
	}

	struct FGroundSection
	{
		TArray<FVector> Vertices;
		TArray<int32> Triangles;
		TArray<FVector> Normals;
		TArray<FVector2D> UVs;
		TArray<FLinearColor> Colors;
		TArray<FProcMeshTangent> Tangents;
		TMap<int32, int32> Remap;

		int32 Use(
			int32 Source,
			const TArray<FVector>& SrcVerts,
			const TArray<FVector>& SrcNormals,
			const TArray<FVector2D>& SrcUVs,
			const TArray<FLinearColor>& SrcColors,
			const TArray<FProcMeshTangent>& SrcTangents)
		{
			if (const int32* Found = Remap.Find(Source))
			{
				return *Found;
			}
			const int32 NewIndex = Vertices.Num();
			Remap.Add(Source, NewIndex);
			Vertices.Add(SrcVerts[Source]);
			Normals.Add(SrcNormals[Source]);
			UVs.Add(SrcUVs[Source]);
			Colors.Add(SrcColors[Source]);
			Tangents.Add(SrcTangents[Source]);
			return NewIndex;
		}
	};
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
	Ground->bReceivesDecals = true;

	Water = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Water"));
	Water->SetupAttachment(Ground);
	Water->SetMobility(EComponentMobility::Movable);
	Water->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Water->SetCanEverAffectNavigation(false);
	Water->SetCastShadow(false);
	Water->bCastDynamicShadow = false;
	Water->bReceivesDecals = false;
}

void ASiltGroundChunk::Build(const FVector2D& MinXY, const FVector2D& MaxXY, float Step, const FSiltGroundMaterials& Materials)
{
	const float SpanX = FMath::Max(Step, MaxXY.X - MinXY.X);
	const float SpanY = FMath::Max(Step, MaxXY.Y - MinXY.Y);
	const int32 NumX = FMath::Clamp(FMath::RoundToInt(SpanX / Step) + 1, 2, 220);
	const int32 NumY = FMath::Clamp(FMath::RoundToInt(SpanY / Step) + 1, 2, 220);

	TArray<FVector> Vertices;
	TArray<FVector2D> UVs;
	TArray<FLinearColor> Colors;
	TArray<ESiltSurface> Surfaces;

	Vertices.SetNum(NumX * NumY);
	UVs.SetNum(NumX * NumY);
	Colors.SetNum(NumX * NumY);
	Surfaces.SetNum(NumX * NumY);

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
			Surfaces[Index] = SiltTerrain::SampleSurface(WorldX, WorldY);
			const float Noise = FMath::Frac(FMath::Sin(WorldX * 0.013f + WorldY * 0.017f) * 43758.5453f) * 2.f - 1.f;
			Colors[Index] = GroundColor(Surfaces[Index], Noise);
		}
	}

	TArray<FVector> Normals;
	Normals.Init(FVector::ZeroVector, Vertices.Num());
	TArray<int32> AllTriangles;
	AllTriangles.Reserve((NumX - 1) * (NumY - 1) * 6);
	for (int32 Y = 0; Y < NumY - 1; ++Y)
	{
		for (int32 X = 0; X < NumX - 1; ++X)
		{
			const int32 I = Y * NumX + X;
			AllTriangles.Add(I);
			AllTriangles.Add(I + 1);
			AllTriangles.Add(I + NumX);
			AllTriangles.Add(I + 1);
			AllTriangles.Add(I + NumX + 1);
			AllTriangles.Add(I + NumX);
		}
	}

	for (int32 Tri = 0; Tri + 2 < AllTriangles.Num(); Tri += 3)
	{
		const int32 I0 = AllTriangles[Tri];
		const int32 I1 = AllTriangles[Tri + 1];
		const int32 I2 = AllTriangles[Tri + 2];
		const FVector Normal = FVector::CrossProduct(Vertices[I1] - Vertices[I0], Vertices[I2] - Vertices[I0]).GetSafeNormal();
		Normals[I0] += Normal;
		Normals[I1] += Normal;
		Normals[I2] += Normal;
	}

	TArray<FProcMeshTangent> Tangents;
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

	FGroundSection Sections[7];
	for (int32 Y = 0; Y < NumY - 1; ++Y)
	{
		for (int32 X = 0; X < NumX - 1; ++X)
		{
			const int32 I00 = Y * NumX + X;
			const int32 I10 = I00 + 1;
			const int32 I01 = I00 + NumX;
			const int32 I11 = I01 + 1;
			const float CenterX = (Vertices[I00].X + Vertices[I11].X) * 0.5f;
			const float CenterY = (Vertices[I00].Y + Vertices[I11].Y) * 0.5f;
			const ESiltSurface Surface = SiltTerrain::SampleSurface(CenterX, CenterY);
			const int32 SectionIndex = FMath::Clamp(static_cast<int32>(Surface), 0, 6);
			FGroundSection& Section = Sections[SectionIndex];
			const int32 V0 = Section.Use(I00, Vertices, Normals, UVs, Colors, Tangents);
			const int32 V1 = Section.Use(I10, Vertices, Normals, UVs, Colors, Tangents);
			const int32 V2 = Section.Use(I11, Vertices, Normals, UVs, Colors, Tangents);
			const int32 V3 = Section.Use(I01, Vertices, Normals, UVs, Colors, Tangents);
			Section.Triangles.Add(V0);
			Section.Triangles.Add(V1);
			Section.Triangles.Add(V3);
			Section.Triangles.Add(V1);
			Section.Triangles.Add(V2);
			Section.Triangles.Add(V3);
		}
	}

	int32 MeshSection = 0;
	const ESiltSurface SectionSurfaces[7] = {
		ESiltSurface::Road,
		ESiltSurface::Dirt,
		ESiltSurface::Mud,
		ESiltSurface::DeepMud,
		ESiltSurface::Water,
		ESiltSurface::Gravel,
		ESiltSurface::Asphalt
	};
	for (int32 Index = 0; Index < 7; ++Index)
	{
		FGroundSection& Section = Sections[Index];
		if (Section.Vertices.Num() == 0)
		{
			continue;
		}
		Ground->CreateMeshSection_LinearColor(
			MeshSection,
			Section.Vertices,
			Section.Triangles,
			Section.Normals,
			Section.UVs,
			Section.Colors,
			Section.Tangents,
			true);
		if (UMaterialInterface* SurfaceMaterial = MaterialFor(SectionSurfaces[Index], Materials))
		{
			Ground->SetMaterial(MeshSection, SurfaceMaterial);
		}
		++MeshSection;
	}

	TArray<FVector> WaterVerts;
	TArray<int32> WaterTris;
	TArray<FVector> WaterNormals;
	TArray<FVector2D> WaterUVs;
	TArray<FLinearColor> WaterColors;
	TArray<FProcMeshTangent> WaterTangents;
	const float WaterLevel = SiltTerrain::GetWaterLevel();
	const float WaterZ = WaterLevel + 12.f;

	for (int32 Y = 0; Y < NumY - 1; ++Y)
	{
		for (int32 X = 0; X < NumX - 1; ++X)
		{
			const int32 I00 = Y * NumX + X;
			const int32 I10 = I00 + 1;
			const int32 I01 = I00 + NumX;
			const int32 I11 = I01 + 1;
			const float Lowest = FMath::Min3(Vertices[I00].Z, Vertices[I10].Z, FMath::Min(Vertices[I01].Z, Vertices[I11].Z));
			if (Lowest > WaterLevel + 40.f)
			{
				continue;
			}

			const int32 Base = WaterVerts.Num();
			const FVector Corners[4] = { Vertices[I00], Vertices[I10], Vertices[I11], Vertices[I01] };
			for (const FVector& Corner : Corners)
			{
				const float Depth = FMath::Clamp((WaterLevel - Corner.Z) / 480.f, 0.f, 1.f);
				const float Shore = 1.f - FMath::SmoothStep(0.f, 0.22f, Depth);
				const float Break = Hash01(Corner.X, Corner.Y);
				const float Foam = Shore * FMath::Lerp(0.35f, 1.f, Break);
				WaterVerts.Add(FVector(Corner.X, Corner.Y, WaterZ));
				WaterUVs.Add(FVector2D(Corner.X * 0.0002f, Corner.Y * 0.0002f));
				// R = depth, G = shoreline foam. The floodwater material reads both.
				WaterColors.Add(FLinearColor(Depth, Foam, 0.15f, 1.f));
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
	if (Materials.Water)
	{
		Water->SetMaterial(0, Materials.Water);
	}
	else if (Materials.Fallback)
	{
		Water->SetMaterial(0, Materials.Fallback);
	}
}
