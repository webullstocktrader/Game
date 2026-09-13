#include "SiltTerrain.h"
#include "ProceduralMeshComponent.h"
#include "SiltTypes.h"
#include "Materials/MaterialInterface.h"

ASiltTerrain::ASiltTerrain()
{
	PrimaryActorTick.bCanEverTick = true;
	GroundMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Ground"));
	SetRootComponent(GroundMesh);
	GroundMesh->bUseAsyncCooking = true;
	GroundMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GroundMesh->SetCollisionResponseToAllChannels(ECR_Block);

	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Water"));
	WaterMesh->SetupAttachment(GroundMesh);
	WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterMesh->SetCastShadow(false);

	GarageCenter = FVector(-12000.f, 0.f, 40.f);
	HighwayLayBy = FVector(8000.f, -3500.f, 40.f);
	FordCenter = FVector(3800.f, 1400.f, -20.f);
	WashoutCenter = FVector(8000.f, 2400.f, 10.f);
}

void ASiltTerrain::BeginPlay()
{
	Super::BeginPlay();
}

void ASiltTerrain::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bAnyDirty)
	{
		return;
	}
	RebuildTimer += DeltaSeconds;
	if (RebuildTimer >= 0.12f)
	{
		RebuildTimer = 0.f;
		RebuildDirty();
	}
}

void ASiltTerrain::WorldToIndex(float X, float Y, int32& IX, int32& IY) const
{
	IX = FMath::Clamp(FMath::RoundToInt((X + HalfExtentCm) / CellCm), 0, GridN - 1);
	IY = FMath::Clamp(FMath::RoundToInt((Y + HalfExtentCm) / CellCm), 0, GridN - 1);
}

void ASiltTerrain::WorldToIndexBilinear(float X, float Y, int32& X0, int32& Y0, int32& X1, int32& Y1, float& TX, float& TY) const
{
	const float FX = (X + HalfExtentCm) / CellCm;
	const float FY = (Y + HalfExtentCm) / CellCm;
	X0 = FMath::Clamp(FMath::FloorToInt(FX), 0, GridN - 1);
	Y0 = FMath::Clamp(FMath::FloorToInt(FY), 0, GridN - 1);
	X1 = FMath::Clamp(X0 + 1, 0, GridN - 1);
	Y1 = FMath::Clamp(Y0 + 1, 0, GridN - 1);
	TX = FMath::Frac(FX);
	TY = FMath::Frac(FY);
}

float ASiltTerrain::SampleBase(int32 IX, int32 IY) const
{
	return BaseHeight[IndexOf(IX, IY)];
}

float ASiltTerrain::SampleSink(int32 IX, int32 IY) const
{
	return SinkCm[IndexOf(IX, IY)];
}

float ASiltTerrain::RiverDistance(float X, float Y) const
{
	// A crooked county creek running NW to SE through the ford.
	const FVector2D A(-4000.f, 11000.f);
	const FVector2D B(FordCenter.X, FordCenter.Y);
	const FVector2D C(15000.f, -8000.f);
	const FVector2D P(X, Y);
	const float D1 = FMath::PointDistToSegment(FVector(P.X, P.Y, 0.f), FVector(A.X, A.Y, 0.f), FVector(B.X, B.Y, 0.f));
	const float D2 = FMath::PointDistToSegment(FVector(P.X, P.Y, 0.f), FVector(B.X, B.Y, 0.f), FVector(C.X, C.Y, 0.f));
	const float Meander = 220.f * FMath::PerlinNoise2D(FVector2D(X, Y) * 0.00045f);
	return FMath::Min(D1, D2) - Meander;
}

float ASiltTerrain::HighwayDistance(float X, float Y) const
{
	// Highway 6 is a north-south lie of pavement.
	return FMath::Abs(X - 8000.f);
}

bool ASiltTerrain::InGaragePad(float X, float Y) const
{
	return FMath::Abs(X - GarageCenter.X) < 1800.f && FMath::Abs(Y - GarageCenter.Y) < 1600.f;
}

bool ASiltTerrain::InWashout(float X, float Y) const
{
	return FMath::Abs(X - WashoutCenter.X) < 700.f && FMath::Abs(Y - WashoutCenter.Y) < 500.f;
}

float ASiltTerrain::ComputeBaseHeight(float X, float Y) const
{
	float Height = 70.f * FMath::PerlinNoise2D(FVector2D(X, Y) * 0.00011f);
	Height += 32.f * FMath::PerlinNoise2D(FVector2D(X + 900.f, Y - 400.f) * 0.00032f);

	if (InGaragePad(X, Y))
	{
		return FMath::Lerp(Height, 28.f, 0.92f);
	}

	const float Hwy = HighwayDistance(X, Y);
	if (Hwy < 700.f && !InWashout(X, Y))
	{
		const float T = 1.f - FMath::Clamp(Hwy / 700.f, 0.f, 1.f);
		return FMath::Lerp(Height, 36.f, T * T);
	}

	const float River = RiverDistance(X, Y);
	const bool bFord = FVector2D::Distance(FVector2D(X, Y), FVector2D(FordCenter.X, FordCenter.Y)) < 1600.f;
	if (River < 900.f)
	{
		const float Cut = bFord ? 55.f : 120.f;
		const float Bank = FMath::SmoothStep(0.f, 900.f, River);
		Height -= Cut * (1.f - Bank);
	}

	if (InWashout(X, Y))
	{
		Height -= 28.f;
	}

	return Height;
}

ESiltSurface ASiltTerrain::ComputeSurface(float X, float Y) const
{
	if (InGaragePad(X, Y))
	{
		return ESiltSurface::Gravel;
	}

	const float Hwy = HighwayDistance(X, Y);
	if (Hwy < 420.f && !InWashout(X, Y))
	{
		return ESiltSurface::Pavement;
	}

	const float River = RiverDistance(X, Y);
	const bool bFord = FVector2D::Distance(FVector2D(X, Y), FVector2D(FordCenter.X, FordCenter.Y)) < 1600.f;
	if (River < (bFord ? 380.f : 280.f))
	{
		return ESiltSurface::Water;
	}

	if (InWashout(X, Y) || Hwy < 800.f)
	{
		return ESiltSurface::Mud;
	}

	const float Noise = FMath::PerlinNoise2D(FVector2D(X, Y) * 0.0002f);
	if (Noise > 0.35f)
	{
		return ESiltSurface::Grass;
	}
	return ESiltSurface::Mud;
}

void ASiltTerrain::BuildCounty()
{
	const int32 Count = GridN * GridN;
	BaseHeight.SetNum(Count);
	SinkCm.SetNumZeroed(Count);
	SurfaceId.SetNum(Count);
	Dirty.SetNumZeroed(Count);

	for (int32 IY = 0; IY < GridN; ++IY)
	{
		for (int32 IX = 0; IX < GridN; ++IX)
		{
			const float X = -HalfExtentCm + IX * CellCm;
			const float Y = -HalfExtentCm + IY * CellCm;
			const int32 I = IndexOf(IX, IY);
			BaseHeight[I] = ComputeBaseHeight(X, Y);
			SurfaceId[I] = static_cast<uint8>(ComputeSurface(X, Y));
		}
	}

	RebuildFull();
}

float ASiltTerrain::HeightAt(float X, float Y) const
{
	int32 X0, Y0, X1, Y1;
	float TX, TY;
	WorldToIndexBilinear(X, Y, X0, Y0, X1, Y1, TX, TY);
	const float H00 = SampleBase(X0, Y0) - SampleSink(X0, Y0);
	const float H10 = SampleBase(X1, Y0) - SampleSink(X1, Y0);
	const float H01 = SampleBase(X0, Y1) - SampleSink(X0, Y1);
	const float H11 = SampleBase(X1, Y1) - SampleSink(X1, Y1);
	return FMath::Lerp(FMath::Lerp(H00, H10, TX), FMath::Lerp(H01, H11, TX), TY);
}

ESiltSurface ASiltTerrain::SurfaceAt(float X, float Y) const
{
	int32 IX, IY;
	WorldToIndex(X, Y, IX, IY);
	return static_cast<ESiltSurface>(SurfaceId[IndexOf(IX, IY)]);
}

FVector ASiltTerrain::NormalAt(float X, float Y) const
{
	const float E = 80.f;
	const float Hx = HeightAt(X + E, Y) - HeightAt(X - E, Y);
	const float Hy = HeightAt(X, Y + E) - HeightAt(X, Y - E);
	FVector N(-Hx / (2.f * E), -Hy / (2.f * E), 1.f);
	return N.GetSafeNormal();
}

FSiltGroundHit ASiltTerrain::Query(const FVector& WorldPos) const
{
	FSiltGroundHit Hit;
	if (BaseHeight.Num() == 0)
	{
		return Hit;
	}
	int32 IX, IY;
	WorldToIndex(WorldPos.X, WorldPos.Y, IX, IY);
	Hit.bHit = true;
	Hit.Position = FVector(WorldPos.X, WorldPos.Y, HeightAt(WorldPos.X, WorldPos.Y));
	Hit.Normal = NormalAt(WorldPos.X, WorldPos.Y);
	Hit.SinkCm = SampleSink(IX, IY);
	Hit.Surface = SurfaceAt(WorldPos.X, WorldPos.Y);
	return Hit;
}

void ASiltTerrain::ApplyTire(const FVector& WorldPos, float LoadN, float Slip01, float PSI, float DeltaSeconds)
{
	if (BaseHeight.Num() == 0)
	{
		return;
	}

	int32 CX, CY;
	WorldToIndex(WorldPos.X, WorldPos.Y, CX, CY);
	if (static_cast<ESiltSurface>(SurfaceId[IndexOf(CX, CY)]) != ESiltSurface::Mud
		&& static_cast<ESiltSurface>(SurfaceId[IndexOf(CX, CY)]) != ESiltSurface::Water)
	{
		return;
	}

	// Floor it in the soup and the county keeps the hole.
	const float PsiDig = FMath::Clamp((PSI - 12.f) / 24.f, 0.f, 1.f);
	const float Dig = FMath::Clamp(Slip01, 0.f, 1.5f);
	const float LoadK = FMath::Clamp(LoadN / 9000.f, 0.2f, 2.4f);
	const float Amount = DeltaSeconds * Dig * LoadK * (0.35f + 1.4f * PsiDig) * 22.f;

	if (Amount <= KINDA_SMALL_NUMBER)
	{
		return;
	}

	for (int32 DY = -2; DY <= 2; ++DY)
	{
		for (int32 DX = -2; DX <= 2; ++DX)
		{
			const int32 IX = FMath::Clamp(CX + DX, 0, GridN - 1);
			const int32 IY = FMath::Clamp(CY + DY, 0, GridN - 1);
			const float W = 1.f / (1.f + 0.65f * (DX * DX + DY * DY));
			const int32 I = IndexOf(IX, IY);
			const ESiltSurface Surf = static_cast<ESiltSurface>(SurfaceId[I]);
			if (Surf != ESiltSurface::Mud && Surf != ESiltSurface::Water)
			{
				continue;
			}
			const float Cap = Surf == ESiltSurface::Water ? 70.f : 95.f;
			SinkCm[I] = FMath::Min(Cap, SinkCm[I] + Amount * W);
			Dirty[I] = 1;
			bAnyDirty = true;
		}
	}
}

void ASiltTerrain::AddQuad(TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs, TArray<FColor>& Colors,
	const FVector& A, const FVector& B, const FVector& C, const FVector& D, const FColor& Color)
{
	const int32 Base = Verts.Num();
	const FVector N = FVector::CrossProduct(B - A, D - A).GetSafeNormal();
	Verts.Append({ A, B, C, D });
	Norms.Append({ N, N, N, N });
	UVs.Append({ FVector2D(0, 0), FVector2D(1, 0), FVector2D(1, 1), FVector2D(0, 1) });
	Colors.Append({ Color, Color, Color, Color });
	Tris.Append({ Base, Base + 1, Base + 2, Base, Base + 2, Base + 3 });
}

void ASiltTerrain::RebuildFull()
{
	TArray<FVector> Verts, Norms, WaterVerts, WaterNorms;
	TArray<int32> Tris, WaterTris;
	TArray<FVector2D> UVs, WaterUVs;
	TArray<FColor> Colors, WaterColors;

	const int32 Quads = (GridN - 1) * (GridN - 1);
	Verts.Reserve(Quads * 4);
	Tris.Reserve(Quads * 6);

	for (int32 IY = 0; IY < GridN - 1; ++IY)
	{
		for (int32 IX = 0; IX < GridN - 1; ++IX)
		{
			const float X0 = -HalfExtentCm + IX * CellCm;
			const float Y0 = -HalfExtentCm + IY * CellCm;
			const float X1 = X0 + CellCm;
			const float Y1 = Y0 + CellCm;
			const FVector A(X0, Y0, SampleBase(IX, IY) - SampleSink(IX, IY));
			const FVector B(X1, Y0, SampleBase(IX + 1, IY) - SampleSink(IX + 1, IY));
			const FVector C(X1, Y1, SampleBase(IX + 1, IY + 1) - SampleSink(IX + 1, IY + 1));
			const FVector D(X0, Y1, SampleBase(IX, IY + 1) - SampleSink(IX, IY + 1));

			const ESiltSurface Surf = static_cast<ESiltSurface>(SurfaceId[IndexOf(IX, IY)]);
			FLinearColor Lc = Silt::SurfaceColor(Surf);
			const float Wet = 1.f - FMath::Clamp(SampleSink(IX, IY) / 80.f, 0.f, 0.55f);
			Lc *= Wet;
			if (SampleSink(IX, IY) > 8.f)
			{
				Lc *= 0.72f;
			}
			AddQuad(Verts, Tris, Norms, UVs, Colors, A, B, C, D, Lc.ToFColor(true));

			if (Surf == ESiltSurface::Water)
			{
				const float WaterZ = FMath::Max(A.Z, FMath::Max(B.Z, FMath::Max(C.Z, D.Z))) + 18.f;
				AddQuad(WaterVerts, WaterTris, WaterNorms, WaterUVs, WaterColors,
					FVector(X0, Y0, WaterZ), FVector(X1, Y0, WaterZ), FVector(X1, Y1, WaterZ), FVector(X0, Y1, WaterZ),
					FColor(20, 36, 32, 140));
			}
		}
	}

	GroundMesh->ClearAllMeshSections();
	GroundMesh->CreateMeshSection(0, Verts, Tris, Norms, UVs, Colors, TArray<FProcMeshTangent>(), true);
	if (UMaterialInterface* Mud = Silt::Material(TEXT("M_Mud")))
	{
		GroundMesh->SetMaterial(0, Mud);
	}

	WaterMesh->ClearAllMeshSections();
	if (WaterVerts.Num() > 0)
	{
		WaterMesh->CreateMeshSection(0, WaterVerts, WaterTris, WaterNorms, WaterUVs, WaterColors, TArray<FProcMeshTangent>(), false);
		if (UMaterialInterface* Water = Silt::Material(TEXT("M_Water")))
		{
			WaterMesh->SetMaterial(0, Water);
		}
	}

	bAnyDirty = false;
	Dirty.Init(0, GridN * GridN);
}

void ASiltTerrain::RebuildDirty()
{
	// Full rebuild is cheap enough at this resolution and keeps ruts honest.
	RebuildFull();
}
