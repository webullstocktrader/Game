#include "ValleyAnimal.h"
#include "ValleyTypes.h"
#include "ValleyTerrain.h"
#include "Sim/ValleySim.h"
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
	UMaterialInterface* Hide = Valley::Material(TEXT("M_Hide"));
	if (!Cyl)
	{
		return;
	}

	auto Add = [&](const FName& Name, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale)
	{
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
		Comp->SetStaticMesh(Mesh);
		Comp->SetRelativeLocation(Loc);
		Comp->SetRelativeRotation(Rot);
		Comp->SetRelativeScale3D(Scale);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Hide)
		{
			Comp->SetMaterial(0, Hide);
		}
		Comp->SetupAttachment(GetRootComponent());
		Comp->RegisterComponent();
		Parts.Add(Comp);
	};

	Add(TEXT("Body"), Cyl, FVector(0.f, 0.f, 52.f), FRotator(90.f, 0.f, 0.f), FVector(0.28f, 0.28f, 0.85f));
	Add(TEXT("Neck"), Cyl, FVector(42.f, 0.f, 70.f), FRotator(35.f, 0.f, 0.f), FVector(0.14f, 0.14f, 0.28f));
	if (Cube)
	{
		Add(TEXT("Head"), Cube, FVector(68.f, 0.f, 82.f), FRotator::ZeroRotator, FVector(0.28f, 0.16f, 0.16f));
		Add(TEXT("EarL"), Cube, FVector(62.f, -8.f, 96.f), FRotator::ZeroRotator, FVector(0.05f, 0.03f, 0.12f));
		Add(TEXT("EarR"), Cube, FVector(62.f, 8.f, 96.f), FRotator::ZeroRotator, FVector(0.05f, 0.03f, 0.12f));
	}
	Add(TEXT("LegFL"), Cyl, FVector(28.f, -12.f, 22.f), FRotator::ZeroRotator, FVector(0.07f, 0.07f, 0.44f));
	Add(TEXT("LegFR"), Cyl, FVector(28.f, 12.f, 22.f), FRotator::ZeroRotator, FVector(0.07f, 0.07f, 0.44f));
	Add(TEXT("LegRL"), Cyl, FVector(-28.f, -12.f, 22.f), FRotator::ZeroRotator, FVector(0.07f, 0.07f, 0.44f));
	Add(TEXT("LegRR"), Cyl, FVector(-28.f, 12.f, 22.f), FRotator::ZeroRotator, FVector(0.07f, 0.07f, 0.44f));
	if (Sphere)
	{
		Add(TEXT("Tail"), Sphere, FVector(-48.f, 0.f, 58.f), FRotator::ZeroRotator, FVector(0.1f, 0.08f, 0.08f));
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
