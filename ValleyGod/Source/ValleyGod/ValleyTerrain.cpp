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
	if (IsGrass(X, Y, Height) || IsMud(X, Y, Height) || RiverDistance(X, Y) < 480.f)
	{
		return false;
	}
	return FMath::PerlinNoise2D(FVector2D(X + 900.f, Y) * 0.0004f) > 0.38f;
}

bool AValleyTerrain::IsMud(float X, float Y, float Height) const
{
	if (IsGrass(X, Y, Height))
	{
		return false;
	}
	const float River = RiverDistance(X, Y);
	return River >= 300.f && River < 560.f && Height < 95.f;
}

void AValleyTerrain::AddQuad(TArray<FVector>& Verts, TArray<int32>& Tris, TArray<FVector>& Norms, TArray<FVector2D>& UVs, TArray<FColor>& Colors,
	const FVector& A, const FVector& B, const FVector& C, const FVector& D, const FColor& Color)
{
	const int32 Base = Verts.Num();
	const FVector N = FVector::CrossProduct(B - A, D - A).GetSafeNormal();
	Verts.Append({ A, B, C, D });
	Norms.Append({ N, N, N, N });
	const float UVS = 0.0022f;
	UVs.Append({ FVector2D(A.X, A.Y) * UVS, FVector2D(B.X, B.Y) * UVS, FVector2D(C.X, C.Y) * UVS, FVector2D(D.X, D.Y) * UVS });
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
	if (UMaterialInterface* Mat = Valley::Material(bWet ? TEXT("M_DirtWet") : TEXT("M_Dirt")))
	{
		GroundMesh->SetMaterial(0, Mat);
	}
	if (UMaterialInterface* Grass = Valley::Material(bWet ? TEXT("M_GrassWet") : TEXT("M_Grass")))
	{
		GroundMesh->SetMaterial(1, Grass);
	}
}

void AValleyTerrain::SetFlood(float ExtraZ)
{
	FloodExtraZ = ExtraZ;
	WaterMesh->SetRelativeLocation(FVector(0.f, 0.f, ExtraZ));
}

void AValleyTerrain::Rebuild()
{
	TArray<FVector> DirtV, GrassV, StoneV, MudV, WaterV, DirtN, GrassN, StoneN, MudN, WaterN;
	TArray<int32> DirtT, GrassT, StoneT, MudT, WaterT;
	TArray<FVector2D> DirtUV, GrassUV, StoneUV, MudUV, WaterUV;
	TArray<FColor> DirtC, GrassC, StoneC, MudC, WaterC;

	auto Shade = [](const FColor& Base, float Amount) -> FColor
	{
		return FColor(
			static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Base.R * Amount), 0, 255)),
			static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Base.G * Amount), 0, 255)),
			static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(Base.B * Amount), 0, 255)),
			Base.A);
	};

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

			const float CX = (X0 + X1) * 0.5f;
			const float CY = (Y0 + Y1) * 0.5f;
			const float N = FMath::PerlinNoise2D(FVector2D(CX, CY) * 0.0005f);
			const float N2 = FMath::PerlinNoise2D(FVector2D(CX + 700.f, CY - 180.f) * 0.00115f);
			const float Amount = FMath::Clamp(0.84f + 0.16f * (0.5f + 0.5f * N) + 0.08f * N2, 0.78f, 1.06f);
			const uint8 ShadeByte = static_cast<uint8>(FMath::Clamp(FMath::RoundToInt(255.f * Amount), 0, 255));
			const FColor Vert(ShadeByte, ShadeByte, ShadeByte);

			const bool bGrass = IsGrass(X0, Y0, A.Z);
			const bool bMud = !bGrass && IsMud(X0, Y0, A.Z);
			const bool bStone = !bGrass && !bMud && IsStone(X0, Y0, A.Z);
			if (bGrass)
			{
				AddQuad(GrassV, GrassT, GrassN, GrassUV, GrassC, A, B, C, D, Vert);
			}
			else if (bMud)
			{
				AddQuad(MudV, MudT, MudN, MudUV, MudC, A, B, C, D, Vert);
			}
			else if (bStone)
			{
				AddQuad(StoneV, StoneT, StoneN, StoneUV, StoneC, A, B, C, D, Vert);
			}
			else
			{
				AddQuad(DirtV, DirtT, DirtN, DirtUV, DirtC, A, B, C, D, Vert);
			}

			const float River = RiverDistance(X0, Y0);
			if (River < 380.f)
			{
				const float WaterZ = FMath::Max3(A.Z, B.Z, FMath::Max(C.Z, D.Z)) + 12.f;
				const float Deep = 1.f - FMath::Clamp(River / 380.f, 0.f, 1.f);
				const FColor WaterCol = Shade(FColor(8, 22, 26, 170), 0.7f + 0.3f * (1.f - Deep));
				AddQuad(WaterV, WaterT, WaterN, WaterUV, WaterC,
					FVector(X0, Y0, WaterZ), FVector(X1, Y0, WaterZ), FVector(X1, Y1, WaterZ), FVector(X0, Y1, WaterZ),
					WaterCol);
			}
		}
	}

	GroundMesh->ClearAllMeshSections();
	GroundMesh->CreateMeshSection(0, DirtV, DirtT, DirtN, DirtUV, DirtC, TArray<FProcMeshTangent>(), true);
	if (UMaterialInterface* Dirt = Valley::Material(bWet ? TEXT("M_DirtWet") : TEXT("M_Dirt")))
	{
		GroundMesh->SetMaterial(0, Dirt);
	}
	if (GrassV.Num() > 0)
	{
		GroundMesh->CreateMeshSection(1, GrassV, GrassT, GrassN, GrassUV, GrassC, TArray<FProcMeshTangent>(), true);
		if (UMaterialInterface* Grass = Valley::Material(bWet ? TEXT("M_GrassWet") : TEXT("M_Grass")))
		{
			GroundMesh->SetMaterial(1, Grass);
		}
	}
	if (StoneV.Num() > 0)
	{
		GroundMesh->CreateMeshSection(2, StoneV, StoneT, StoneN, StoneUV, StoneC, TArray<FProcMeshTangent>(), true);
		if (UMaterialInterface* Stone = Valley::Material(TEXT("M_StoneGround")))
		{
			GroundMesh->SetMaterial(2, Stone);
		}
	}
	if (MudV.Num() > 0)
	{
		GroundMesh->CreateMeshSection(3, MudV, MudT, MudN, MudUV, MudC, TArray<FProcMeshTangent>(), true);
		if (UMaterialInterface* Mud = Valley::Material(TEXT("M_Mud")))
		{
			GroundMesh->SetMaterial(3, Mud);
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
