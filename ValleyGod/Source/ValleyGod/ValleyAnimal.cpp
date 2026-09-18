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
	const FLinearColor Snout = Fur * 0.75f + FLinearColor(0.08f, 0.05f, 0.03f);
	UMaterialInterface* FurUse = Valley::Tint(this, Valley::Material(TEXT("M_Fur")), Fur, TEXT("FurDyn"));
	UMaterialInterface* DarkUse = Valley::Tint(this, Valley::Material(TEXT("M_Hide")), Snout, TEXT("SnoutDyn"));

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

	Add(TEXT("Chest"), Sphere, FVector(Len * 28.f, 0.f, 52.f + Leg * 8.f), FRotator::ZeroRotator, FVector(Rad * 1.15f, Rad * 1.05f, Rad * 1.1f), FurUse);
	Add(TEXT("Body"), Cyl, FVector(0.f, 0.f, 50.f + Leg * 8.f), FRotator(90.f, 0.f, 0.f), FVector(Rad, Rad, Len), FurUse);
	Add(TEXT("Rump"), Sphere, FVector(-Len * 32.f, 0.f, 50.f + Leg * 8.f), FRotator::ZeroRotator, FVector(Rad * 1.2f, Rad * 1.1f, Rad * 1.15f), FurUse);
	Add(TEXT("Neck"), Cyl, FVector(Len * 42.f, 0.f, 68.f + Leg * 8.f), FRotator(38.f, 0.f, 0.f), FVector(Rad * 0.55f, Rad * 0.55f, Neck), FurUse);
	Add(TEXT("Head"), Sphere, FVector(Len * 58.f, 0.f, 86.f + Leg * 6.f), FRotator::ZeroRotator, FVector(0.22f, 0.16f, 0.16f), FurUse);
	Add(TEXT("Snout"), Sphere, FVector(Len * 70.f, 0.f, 82.f + Leg * 6.f), FRotator::ZeroRotator, FVector(0.16f, 0.10f, 0.10f), DarkUse);
	Add(TEXT("EarL"), Cone ? Cone : Cyl, FVector(Len * 54.f, -8.f, 100.f + Leg * 6.f), FRotator(0.f, 0.f, -18.f), FVector(0.05f, 0.04f, 0.14f), FurUse);
	Add(TEXT("EarR"), Cone ? Cone : Cyl, FVector(Len * 54.f, 8.f, 100.f + Leg * 6.f), FRotator(0.f, 0.f, 18.f), FVector(0.05f, 0.04f, 0.14f), FurUse);
	Add(TEXT("EyeL"), Sphere, FVector(Len * 62.f, -6.f, 90.f + Leg * 6.f), FRotator::ZeroRotator, FVector(0.04f, 0.03f, 0.03f), Valley::Material(TEXT("M_Eye")));
	Add(TEXT("EyeR"), Sphere, FVector(Len * 62.f, 6.f, 90.f + Leg * 6.f), FRotator::ZeroRotator, FVector(0.04f, 0.03f, 0.03f), Valley::Material(TEXT("M_Eye")));

	const float HipZ = Leg * 22.f;
	Add(TEXT("LegFL"), Cyl, FVector(Len * 24.f, -Rad * 38.f, HipZ), FRotator::ZeroRotator, FVector(0.07f, 0.07f, Leg), DarkUse);
	Add(TEXT("LegFR"), Cyl, FVector(Len * 24.f, Rad * 38.f, HipZ), FRotator::ZeroRotator, FVector(0.07f, 0.07f, Leg), DarkUse);
	Add(TEXT("LegRL"), Cyl, FVector(-Len * 22.f, -Rad * 40.f, HipZ), FRotator::ZeroRotator, FVector(0.08f, 0.08f, Leg), DarkUse);
	Add(TEXT("LegRR"), Cyl, FVector(-Len * 22.f, Rad * 40.f, HipZ), FRotator::ZeroRotator, FVector(0.08f, 0.08f, Leg), DarkUse);
	Add(TEXT("HoofFL"), Sphere, FVector(Len * 24.f, -Rad * 38.f, 6.f), FRotator::ZeroRotator, FVector(0.08f, 0.06f, 0.05f), DarkUse);
	Add(TEXT("HoofFR"), Sphere, FVector(Len * 24.f, Rad * 38.f, 6.f), FRotator::ZeroRotator, FVector(0.08f, 0.06f, 0.05f), DarkUse);
	Add(TEXT("HoofRL"), Sphere, FVector(-Len * 22.f, -Rad * 40.f, 6.f), FRotator::ZeroRotator, FVector(0.08f, 0.06f, 0.05f), DarkUse);
	Add(TEXT("HoofRR"), Sphere, FVector(-Len * 22.f, Rad * 40.f, 6.f), FRotator::ZeroRotator, FVector(0.08f, 0.06f, 0.05f), DarkUse);
	Add(TEXT("Tail"), Cyl, FVector(-Len * 48.f, 0.f, 58.f + Leg * 6.f), FRotator(55.f, 0.f, 0.f), FVector(0.06f, 0.05f, 0.28f), FurUse);
	if (Cube)
	{
		Add(TEXT("Withers"), Cube, FVector(Len * 10.f, 0.f, 62.f + Leg * 8.f), FRotator::ZeroRotator, FVector(Rad * 0.5f, Rad * 0.4f, 0.08f), FurUse);
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
