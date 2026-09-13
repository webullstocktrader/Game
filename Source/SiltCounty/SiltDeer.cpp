#include "SiltDeer.h"
#include "SiltTypes.h"
#include "SiltTruck.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

ASiltDeer::ASiltDeer()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void ASiltDeer::Arm(const FVector& Start, const FVector& WanderA, const FVector& WanderB)
{
	Home = Start;
	Target = WanderB;
	SetActorLocation(Start);
	BuildBody();
}

void ASiltDeer::BuildBody()
{
	UStaticMesh* Cube = Silt::CubeMesh();
	UStaticMesh* Sphere = Silt::SphereMesh();
	UStaticMesh* Cyl = Silt::CylinderMesh();
	UMaterialInterface* Hide = Silt::Material(TEXT("M_Wood"));
	if (!Cube || !Cyl)
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
		return Comp;
	};

	Add(TEXT("Body"), Cube, FVector(0.f, 0.f, 70.f), FRotator::ZeroRotator, FVector(1.1f, 0.38f, 0.42f));
	Add(TEXT("Neck"), Cube, FVector(50.f, 0.f, 95.f), FRotator(20.f, 0.f, 0.f), FVector(0.35f, 0.18f, 0.18f));
	Add(TEXT("Head"), Cube, FVector(78.f, 0.f, 108.f), FRotator::ZeroRotator, FVector(0.35f, 0.2f, 0.18f));
	Add(TEXT("EarL"), Cube, FVector(70.f, -8.f, 124.f), FRotator::ZeroRotator, FVector(0.06f, 0.04f, 0.14f));
	Add(TEXT("EarR"), Cube, FVector(70.f, 8.f, 124.f), FRotator::ZeroRotator, FVector(0.06f, 0.04f, 0.14f));
	Add(TEXT("LegFL"), Cyl, FVector(35.f, -14.f, 30.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 0.6f));
	Add(TEXT("LegFR"), Cyl, FVector(35.f, 14.f, 30.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 0.6f));
	Add(TEXT("LegRL"), Cyl, FVector(-35.f, -14.f, 30.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 0.6f));
	Add(TEXT("LegRR"), Cyl, FVector(-35.f, 14.f, 30.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 0.6f));
	if (Sphere)
	{
		Add(TEXT("Tail"), Sphere, FVector(-58.f, 0.f, 78.f), FRotator::ZeroRotator, FVector(0.12f, 0.1f, 0.1f));
	}
}

FVector ASiltDeer::NearestThreat() const
{
	FVector Best = FVector::ZeroVector;
	float BestD = 2600.f;
	for (TActorIterator<ASiltTruck> It(GetWorld()); It; ++It)
	{
		const float D = FVector::Dist(GetActorLocation(), It->GetActorLocation());
		if (D < BestD)
		{
			BestD = D;
			Best = It->GetActorLocation();
		}
	}
	return BestD < 2600.f ? Best : FVector::ZeroVector;
}

void ASiltDeer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const FVector Threat = NearestThreat();
	FVector Desired = Target;
	if (!Threat.IsNearlyZero())
	{
		Calm = 0.f;
		const FVector Away = (GetActorLocation() - Threat).GetSafeNormal2D();
		Desired = GetActorLocation() + Away * 4000.f;
	}
	else
	{
		Calm = FMath::Min(1.f, Calm + DeltaSeconds * 0.15f);
		if (FVector::Dist2D(GetActorLocation(), Target) < 200.f)
		{
			Target = Home + FVector(FMath::FRandRange(-1800.f, 1800.f), FMath::FRandRange(-1800.f, 1800.f), 0.f);
		}
		Desired = Target;
	}

	const FVector To = (Desired - GetActorLocation()).GetSafeNormal2D();
	const float Speed = Calm < 0.4f ? 1400.f : 220.f;
	Velocity = FMath::VInterpTo(Velocity, To * Speed, DeltaSeconds, 3.f);
	FVector Loc = GetActorLocation() + Velocity * DeltaSeconds;
	Hop += DeltaSeconds * (Calm < 0.4f ? 14.f : 4.f);
	Loc.Z = Home.Z + FMath::Abs(FMath::Sin(Hop)) * (Calm < 0.4f ? 28.f : 8.f);
	SetActorLocation(Loc);
	if (!To.IsNearlyZero())
	{
		SetActorRotation(To.Rotation());
	}
}
