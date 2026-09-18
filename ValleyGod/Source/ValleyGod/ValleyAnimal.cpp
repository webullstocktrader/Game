#include "ValleyAnimal.h"
#include "ValleyTypes.h"
#include "ValleyTerrain.h"
#include "Sim/ValleySim.h"
#include "Sim/ValleyPalette.h"
#include "Components/StaticMeshComponent.h"

AValleyAnimal::AValleyAnimal()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AValleyAnimal::Arm(int32 InId)
{
	AnimalId = InId;
	BuildBody();
}

void AValleyAnimal::BuildBody()
{
	UStaticMesh* Cube = Valley::CubeMesh();
	UStaticMesh* Sphere = Valley::SphereMesh();
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Cone = Valley::ConeMesh();
	if (!Cyl || !Sphere)
	{
		return;
	}

	const vg::AnimalLook& Look = vg::AnimalLookAt(AnimalId);
	const FLinearColor Fur(Look.FurR, Look.FurG, Look.FurB);
	const FLinearColor BellyCol(Look.BellyR, Look.BellyG, Look.BellyB);
	const FLinearColor Dark(Look.DarkR, Look.DarkG, Look.DarkB);
	UMaterialInterface* FurUse = Valley::Tint(this, Valley::Material(TEXT("M_Fur")), Fur, TEXT("FurDyn"));
	UMaterialInterface* BellyUse = Valley::Tint(this, Valley::Material(TEXT("M_FurBelly")), BellyCol, TEXT("BellyDyn"));
	UMaterialInterface* DarkUse = Valley::Tint(this, Valley::Material(TEXT("M_FurDark")), Dark, TEXT("DarkDyn"));
	UMaterialInterface* EyeUse = Valley::Material(TEXT("M_Eye"));
	UMaterialInterface* Ivory = Valley::Tint(this, Valley::Material(TEXT("M_Hide")), FLinearColor(0.82f, 0.76f, 0.62f), TEXT("IvoryDyn"));

	auto Add = [&](const FName& Name, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat)
	{
		if (!Mesh)
		{
			return;
		}
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
		Comp->SetStaticMesh(Mesh);
		Comp->SetRelativeLocation(Loc);
		Comp->SetRelativeRotation(Rot);
		Comp->SetRelativeScale3D(Scale);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Mat)
		{
			Comp->SetMaterial(0, Mat);
		}
		Comp->SetupAttachment(GetRootComponent());
		Comp->RegisterComponent();
		Parts.Add(Comp);
	};

	const float Len = Look.BodyLen;
	const float Rad = Look.BodyRad;
	const float Leg = Look.LegLen;
	const float Neck = Look.NeckLen;
	const float HeadS = Look.HeadScale > 0.f ? Look.HeadScale : 0.2f;
	const float BodyZ = 48.f + Leg * 10.f;
	const float HeadZ = BodyZ + (Look.Tusks ? 8.f : 36.f);
	const float NeckPitch = Look.Tusks ? 18.f : 42.f;

	Add(TEXT("Chest"), Sphere, FVector(Len * 30.f, 0.f, BodyZ + 4.f), FRotator::ZeroRotator, FVector(Rad * 1.2f, Rad * 1.08f, Rad * 1.12f), FurUse);
	Add(TEXT("Body"), Cyl, FVector(0.f, 0.f, BodyZ), FRotator(90.f, 0.f, 0.f), FVector(Rad, Rad * 0.92f, Len), FurUse);
	Add(TEXT("Belly"), Sphere, FVector(-Len * 4.f, 0.f, BodyZ - Rad * 28.f), FRotator::ZeroRotator, FVector(Rad * 1.05f, Rad * 0.95f, Rad * 0.7f), BellyUse);
	Add(TEXT("Rump"), Sphere, FVector(-Len * 34.f, 0.f, BodyZ + 2.f), FRotator::ZeroRotator, FVector(Rad * 1.28f, Rad * 1.18f, Rad * 1.2f), FurUse);
	Add(TEXT("HaunchL"), Sphere, FVector(-Len * 28.f, -Rad * 28.f, BodyZ - 6.f), FRotator::ZeroRotator, FVector(Rad * 0.7f, Rad * 0.55f, Rad * 0.75f), FurUse);
	Add(TEXT("HaunchR"), Sphere, FVector(-Len * 28.f, Rad * 28.f, BodyZ - 6.f), FRotator::ZeroRotator, FVector(Rad * 0.7f, Rad * 0.55f, Rad * 0.75f), FurUse);
	Add(TEXT("ShoulderL"), Sphere, FVector(Len * 24.f, -Rad * 26.f, BodyZ + 2.f), FRotator::ZeroRotator, FVector(Rad * 0.55f, Rad * 0.45f, Rad * 0.6f), FurUse);
	Add(TEXT("ShoulderR"), Sphere, FVector(Len * 24.f, Rad * 26.f, BodyZ + 2.f), FRotator::ZeroRotator, FVector(Rad * 0.55f, Rad * 0.45f, Rad * 0.6f), FurUse);
	Add(TEXT("Neck"), Cyl, FVector(Len * 44.f, 0.f, BodyZ + 18.f), FRotator(NeckPitch, 0.f, 0.f), FVector(Rad * 0.5f, Rad * 0.5f, Neck), FurUse);
	Add(TEXT("Head"), Sphere, FVector(Len * 60.f, 0.f, HeadZ), FRotator::ZeroRotator, FVector(HeadS, HeadS * 0.72f, HeadS * 0.78f), FurUse);
	Add(TEXT("Snout"), Sphere, FVector(Len * 72.f, 0.f, HeadZ - 6.f), FRotator::ZeroRotator, FVector(HeadS * 0.72f, HeadS * 0.42f, HeadS * 0.42f), DarkUse);
	Add(TEXT("EarL"), Cone ? Cone : Cyl, FVector(Len * 56.f, -8.f, HeadZ + 14.f), FRotator(0.f, 0.f, -18.f), FVector(0.05f, 0.04f, Look.Tusks ? 0.10f : 0.16f), FurUse);
	Add(TEXT("EarR"), Cone ? Cone : Cyl, FVector(Len * 56.f, 8.f, HeadZ + 14.f), FRotator(0.f, 0.f, 18.f), FVector(0.05f, 0.04f, Look.Tusks ? 0.10f : 0.16f), FurUse);
	Add(TEXT("EyeL"), Sphere, FVector(Len * 64.f, -6.f, HeadZ + 4.f), FRotator::ZeroRotator, FVector(0.04f, 0.03f, 0.03f), EyeUse);
	Add(TEXT("EyeR"), Sphere, FVector(Len * 64.f, 6.f, HeadZ + 4.f), FRotator::ZeroRotator, FVector(0.04f, 0.03f, 0.03f), EyeUse);

	const float UpperH = FMath::Max(BodyZ * 0.42f, 18.f);
	const float LowerH = FMath::Max(BodyZ * 0.36f, 16.f);
	const float HipZ = BodyZ - UpperH * 0.5f;
	const float ShinZ = 6.f + LowerH * 0.5f;
	auto AddLeg = [&](const TCHAR* Upper, const TCHAR* Lower, const TCHAR* Hoof, float LX, float LY)
	{
		Add(Upper, Cyl, FVector(LX, LY, HipZ), FRotator::ZeroRotator, FVector(0.11f, 0.11f, UpperH / 100.f), FurUse);
		Add(Lower, Cyl, FVector(LX, LY, ShinZ), FRotator::ZeroRotator, FVector(0.065f, 0.065f, LowerH / 100.f), DarkUse);
		Add(Hoof, Sphere, FVector(LX, LY, 6.f), FRotator::ZeroRotator, FVector(0.09f, 0.07f, 0.05f), DarkUse);
	};
	AddLeg(TEXT("UFL"), TEXT("LFL"), TEXT("HoofFL"), Len * 26.f, -Rad * 36.f);
	AddLeg(TEXT("UFR"), TEXT("LFR"), TEXT("HoofFR"), Len * 26.f, Rad * 36.f);
	AddLeg(TEXT("URL"), TEXT("LRL"), TEXT("HoofRL"), -Len * 24.f, -Rad * 40.f);
	AddLeg(TEXT("URR"), TEXT("LRR"), TEXT("HoofRR"), -Len * 24.f, Rad * 40.f);
	Add(TEXT("Tail"), Cyl, FVector(-Len * 50.f, 0.f, BodyZ + 8.f), FRotator(Look.Tusks ? 25.f : 55.f, 0.f, 0.f),
		FVector(0.06f, 0.05f, Look.Tusks ? 0.18f : 0.32f), FurUse);
	if (Cube)
	{
		Add(TEXT("Withers"), Cube, FVector(Len * 12.f, 0.f, BodyZ + 14.f), FRotator::ZeroRotator, FVector(Rad * 0.55f, Rad * 0.38f, 0.08f), FurUse);
	}
	if (Look.Antlers)
	{
		Add(TEXT("AntlerL"), Cyl, FVector(Len * 54.f, -10.f, HeadZ + 22.f), FRotator(-12.f, -18.f, -28.f), FVector(0.04f, 0.04f, 0.42f), DarkUse);
		Add(TEXT("AntlerLT"), Cyl, FVector(Len * 50.f, -18.f, HeadZ + 42.f), FRotator(18.f, -40.f, -12.f), FVector(0.03f, 0.03f, 0.22f), DarkUse);
		Add(TEXT("AntlerR"), Cyl, FVector(Len * 54.f, 10.f, HeadZ + 22.f), FRotator(-12.f, 18.f, 28.f), FVector(0.04f, 0.04f, 0.42f), DarkUse);
		Add(TEXT("AntlerRT"), Cyl, FVector(Len * 50.f, 18.f, HeadZ + 42.f), FRotator(18.f, 40.f, 12.f), FVector(0.03f, 0.03f, 0.22f), DarkUse);
	}
	if (Look.Tusks)
	{
		Add(TEXT("TuskL"), Cone ? Cone : Cyl, FVector(Len * 78.f, -5.f, HeadZ - 10.f), FRotator(78.f, 0.f, -16.f), FVector(0.04f, 0.03f, 0.12f), Ivory);
		Add(TEXT("TuskR"), Cone ? Cone : Cyl, FVector(Len * 78.f, 5.f, HeadZ - 10.f), FRotator(78.f, 0.f, 16.f), FVector(0.04f, 0.03f, 0.12f), Ivory);
	}
}

void AValleyAnimal::SyncFromSim(const vg::Animal& Sim, AValleyTerrain* Terrain, float WorldTime)
{
	SetActorHiddenInGame(!Sim.Alive);
	if (!Sim.Alive)
	{
		return;
	}
	FVector Loc(Sim.X, Sim.Y, 0.f);
	if (Terrain)
	{
		Loc.Z = Terrain->HeightAt(Sim.X, Sim.Y);
	}
	Hop = WorldTime * 7.f;
	Loc.Z += FMath::Abs(FMath::Sin(Hop)) * 6.f;
	SetActorLocation(Loc);
	SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(Sim.Heading), 0.f));
}
