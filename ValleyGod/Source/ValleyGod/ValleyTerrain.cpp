#include "ValleyTerrain.h"
#include "ValleyTypes.h"
#include "ProceduralMeshComponent.h"
#include "Materials/MaterialInterface.h"

AValleyTerrain::AValleyTerrain()
{
	PrimaryActorTick.bCanEverTick = false;
	GroundMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Ground"));
	SetRootComponent(GroundMesh);
	GroundMesh->bUseAsyncCooking = true;
	GroundMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	GroundMesh->SetCollisionResponseToAllChannels(ECR_Block);

	WaterMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Water"));
	WaterMesh->SetupAttachment(GroundMesh);
	WaterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterMesh->SetCastShadow(false);
}

float AValleyTerrain::RiverDistance(float X, float Y) const
{
	const float Bend = 520.f * FMath::Sin(Y * 0.00055f) + 180.f * FMath::PerlinNoise2D(FVector2D(X, Y) * 0.0004f);
	return FMath::Abs(X - Bend);
}

float AValleyTerrain::ComputeBaseHeight(float X, float Y) const
{
	const float Ridge = 0.000018f * (Y * Y) + 0.000004f * (X * X);
	float Height = Ridge;
	Height += 90.f * FMath::PerlinNoise2D(FVector2D(X, Y) * 0.00012f);
	Height += 28.f * FMath::PerlinNoise2D(FVector2D(X + 400.f, Y - 200.f) * 0.00035f);

	const float River = RiverDistance(X, Y);
	if (River < 780.f)
	{
		const float Bank = FMath::SmoothStep(0.f, 780.f, River);
		Height -= 95.f * (1.f - Bank);
	}

	const float Camp = FVector2D::Distance(FVector2D(X, Y), FVector2D(0.f, 700.f));
	if (Camp < 900.f)
	{
		const float T = 1.f - FMath::Clamp(Camp / 900.f, 0.f, 1.f);
		Height = FMath::Lerp(Height, 42.f, T * T);
	}

	return Height;
}

bool AValleyTerrain::IsGrass(float X, float Y, float Height) const
{
	if (RiverDistance(X, Y) < 420.f)
	{
		return false;
	}
	return Height > 70.f && FMath::PerlinNoise2D(FVector2D(X, Y) * 0.00022f) > -0.05f;
}

bool AValleyTerrain::IsStone(float X, float Y, float Height) const
{
	if (IsGrass(X, Y, Height) || RiverDistance(X, Y) < 480.f)
	{
		return false;
	}
	return FMath::PerlinNoise2D(FVector2D(X + 900.f, Y) * 0.0004f) > 0.38f;
}

void AValleyTerrain::AddQuad(TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs, TArray<FColor>& Colors,
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

void AValleyTerrain::BuildValley()
{
	const int32 Count = GridN * GridN;
	BaseHeight.SetNum(Count);
	for (int32 IY = 0; IY < GridN; ++IY)
	{
		for (int32 IX = 0; IX < GridN; ++IX)
		{
			const float X = -HalfExtentCm + IX * CellCm;
			const float Y = -HalfExtentCm + IY * CellCm;
			BaseHeight[IndexOf(IX, IY)] = ComputeBaseHeight(X, Y);
		}
	}
	Rebuild();
}

float AValleyTerrain::HeightAt(float X, float Y) const
{
	if (BaseHeight.Num() == 0)
	{
		return 0.f;
	}
	const float FX = (X + HalfExtentCm) / CellCm;
	const float FY = (Y + HalfExtentCm) / CellCm;
	const int32 X0 = FMath::Clamp(FMath::FloorToInt(FX), 0, GridN - 1);
	const int32 Y0 = FMath::Clamp(FMath::FloorToInt(FY), 0, GridN - 1);
	const int32 X1 = FMath::Clamp(X0 + 1, 0, GridN - 1);
	const int32 Y1 = FMath::Clamp(Y0 + 1, 0, GridN - 1);
	const float TX = FMath::Frac(FX);
	const float TY = FMath::Frac(FY);
	const float H00 = BaseHeight[IndexOf(X0, Y0)];
	const float H10 = BaseHeight[IndexOf(X1, Y0)];
	const float H01 = BaseHeight[IndexOf(X0, Y1)];
	const float H11 = BaseHeight[IndexOf(X1, Y1)];
	return FMath::Lerp(FMath::Lerp(H00, H10, TX), FMath::Lerp(H01, H11, TX), TY);
}

FVector AValleyTerrain::GroundAt(const FVector& WorldPos) const
{
	return FVector(WorldPos.X, WorldPos.Y, HeightAt(WorldPos.X, WorldPos.Y));
}

void AValleyTerrain::SetWet(bool bInWet)
{
	if (bWet == bInWet)
	{
		return;
	}
	bWet = bInWet;
	UMaterialInterface* Mat = bWet
		? (WetGroundMat ? WetGroundMat.Get() : Valley::Material(TEXT("M_DirtWet")))
		: (DryGroundMat ? DryGroundMat.Get() : Valley::Material(TEXT("M_Dirt")));
	if (Mat)
	{
		GroundMesh->SetMaterial(0, Mat);
	}
}

void AValleyTerrain::SetFlood(float ExtraZ)
{
	FloodExtraZ = ExtraZ;
	WaterMesh->SetRelativeLocation(FVector(0.f, 0.f, ExtraZ));
}

void AValleyTerrain::Rebuild()
{
	TArray<FVector> DirtV, GrassV, StoneV, WaterV, DirtN, GrassN, StoneN, WaterN;
	TArray<int32> DirtT, GrassT, StoneT, WaterT;
	TArray<FVector2D> DirtUV, GrassUV, StoneUV, WaterUV;
	TArray<FColor> DirtC, GrassC, StoneC, WaterC;

	for (int32 IY = 0; IY < GridN - 1; ++IY)
	{
		for (int32 IX = 0; IX < GridN - 1; ++IX)
		{
			const float X0 = -HalfExtentCm + IX * CellCm;
			const float Y0 = -HalfExtentCm + IY * CellCm;
			const float X1 = X0 + CellCm;
			const float Y1 = Y0 + CellCm;
			const FVector A(X0, Y0, BaseHeight[IndexOf(IX, IY)]);
			const FVector B(X1, Y0, BaseHeight[IndexOf(IX + 1, IY)]);
			const FVector C(X1, Y1, BaseHeight[IndexOf(IX + 1, IY + 1)]);
			const FVector D(X0, Y1, BaseHeight[IndexOf(IX, IY + 1)]);

			const bool bGrass = IsGrass(X0, Y0, A.Z);
			const bool bStone = !bGrass && IsStone(X0, Y0, A.Z);
			const FColor DirtCol(92, 58, 32);
			const FColor GrassCol(46, 78, 32);
			const FColor StoneCol(120, 114, 104);
			if (bGrass)
			{
				AddQuad(GrassV, GrassT, GrassN, GrassUV, GrassC, A, B, C, D, GrassCol);
			}
			else if (bStone)
			{
				AddQuad(StoneV, StoneT, StoneN, StoneUV, StoneC, A, B, C, D, StoneCol);
			}
			else
			{
				AddQuad(DirtV, DirtT, DirtN, DirtUV, DirtC, A, B, C, D, DirtCol);
			}

			if (RiverDistance(X0, Y0) < 360.f)
			{
				const float WaterZ = FMath::Max3(A.Z, B.Z, FMath::Max(C.Z, D.Z)) + 14.f;
				AddQuad(WaterV, WaterT, WaterN, WaterUV, WaterC,
					FVector(X0, Y0, WaterZ), FVector(X1, Y0, WaterZ), FVector(X1, Y1, WaterZ), FVector(X0, Y1, WaterZ),
					FColor(24, 48, 44, 150));
			}
		}
	}

	GroundMesh->ClearAllMeshSections();
	GroundMesh->CreateMeshSection(0, DirtV, DirtT, DirtN, DirtUV, DirtC, TArray<FProcMeshTangent>(), true);
	if (UMaterialInterface* Dirt = Valley::Material(TEXT("M_Dirt")))
	{
		GroundMesh->SetMaterial(0, Dirt);
	}
	if (GrassV.Num() > 0)
	{
		GroundMesh->CreateMeshSection(1, GrassV, GrassT, GrassN, GrassUV, GrassC, TArray<FProcMeshTangent>(), true);
		if (UMaterialInterface* Grass = Valley::Material(TEXT("M_Grass")))
		{
			GroundMesh->SetMaterial(1, Grass);
		}
	}
	if (StoneV.Num() > 0)
	{
		GroundMesh->CreateMeshSection(2, StoneV, StoneT, StoneN, StoneUV, StoneC, TArray<FProcMeshTangent>(), true);
		if (UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone")))
		{
			GroundMesh->SetMaterial(2, Stone);
		}
	}

	WaterMesh->ClearAllMeshSections();
	if (WaterV.Num() > 0)
	{
		WaterMesh->CreateMeshSection(0, WaterV, WaterT, WaterN, WaterUV, WaterC, TArray<FProcMeshTangent>(), false);
		if (UMaterialInterface* Water = Valley::Material(TEXT("M_Water")))
		{
			WaterMesh->SetMaterial(0, Water);
		}
	}
}

void AValleyTerrain::ApplyGroundMaterials(UMaterialInterface* Dirt, UMaterialInterface* Grass, UMaterialInterface* Wet)
{
	if (Dirt)
	{
		DryGroundMat = Dirt;
		GroundMesh->SetMaterial(0, Dirt);
	}
	else
	{
		DryGroundMat = Valley::Material(TEXT("M_Dirt"));
	}

	if (Grass)
	{
		GroundMesh->SetMaterial(1, Grass);
	}

	if (Wet)
	{
		WetGroundMat = Wet;
	}
	else
	{
		WetGroundMat = Valley::Material(TEXT("M_DirtWet"));
	}

	if (bWet && WetGroundMat)
	{
		GroundMesh->SetMaterial(0, WetGroundMat);
	}
}
