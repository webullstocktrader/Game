#include "SiltGroundMist.h"

#include "SiltCounty.h"
#include "SiltTerrain.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

namespace
{
	constexpr int32 MaxCards = 42;
	constexpr float MaxLiftCm = 120.f;

	bool IsWetGround(ESiltSurface Surface)
	{
		return Surface == ESiltSurface::Mud || Surface == ESiltSurface::DeepMud || Surface == ESiltSurface::Water;
	}
}

ASiltGroundMist::ASiltGroundMist()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	Cards = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Cards"));
	SetRootComponent(Cards);
	Cards->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Cards->SetCastShadow(false);
	Cards->SetCanEverAffectNavigation(false);
	Cards->SetGenerateOverlapEvents(false);
	Cards->SetMobility(EComponentMobility::Movable);
	Cards->SetReceivesDecals(false);
}

bool ASiltGroundMist::TryAddCard(float X, float Y, FRandomStream& Rng)
{
	if (Anchors.Num() >= MaxCards)
	{
		return false;
	}
	if (!IsWetGround(SiltTerrain::SampleSurface(X, Y)))
	{
		return false;
	}

	const float Ground = SiltTerrain::SampleHeight(X, Y);
	const float Water = SiltTerrain::GetWaterLevel();
	const float Base = FMath::Max(Ground, Water);
	Anchors.Add(FVector(X, Y, Base));
	Lifts.Add(Rng.FRandRange(28.f, MaxLiftCm));
	Spans.Add(Rng.FRandRange(3.4f, 6.2f));
	Phases.Add(Rng.FRandRange(0.f, 6.28f));
	return true;
}

void ASiltGroundMist::AddPatch(const FVector2D& Center, float Radius, int32 Count, FRandomStream& Rng)
{
	for (int32 Index = 0; Index < Count && Anchors.Num() < MaxCards; ++Index)
	{
		const float Angle = Rng.FRandRange(0.f, 6.28318f);
		const float Dist = Rng.FRandRange(Radius * 0.15f, Radius);
		TryAddCard(Center.X + FMath::Cos(Angle) * Dist, Center.Y + FMath::Sin(Angle) * Dist, Rng);
	}
}

void ASiltGroundMist::AddWaterline(FRandomStream& Rng)
{
	// Coarse samples along the drive from the flooded town down through the slough.
	for (float Y = 64000.f; Y >= -32000.f && Anchors.Num() < MaxCards; Y -= 8000.f)
	{
		for (float X = -36000.f; X <= 16000.f && Anchors.Num() < MaxCards; X += 9000.f)
		{
			TryAddCard(X, Y, Rng);
		}
	}
}

void ASiltGroundMist::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		SetActorTickEnabled(false);
		return;
	}

	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	if (!Plane)
	{
		SetActorTickEnabled(false);
		return;
	}

	Cards->SetStaticMesh(Plane);
	if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SiltCounty/Materials/M_GroundMist.M_GroundMist")))
	{
		Cards->SetMaterial(0, Mat);
	}

	FRandomStream Rng(17);
	AddPatch(FVector2D(0.f, 52000.f), 6500.f, 14, Rng);
	const FVector2D Slough = SiltTerrain::GetContractXY();
	AddPatch(Slough, 5200.f, 12, Rng);
	AddWaterline(Rng);

	Cards->ClearInstances();
	for (int32 Index = 0; Index < Anchors.Num(); ++Index)
	{
		const FVector Pos(Anchors[Index].X, Anchors[Index].Y, Anchors[Index].Z + Lifts[Index]);
		Cards->AddInstance(FTransform(FRotator::ZeroRotator, Pos, FVector(Spans[Index], Spans[Index] * 0.72f, 1.f)));
	}

	bReady = Anchors.Num() > 0;
	UE_LOG(LogSiltCounty, Display, TEXT("Silt County ground mist: %d low cards. Fog volume unchanged."), Anchors.Num());
}

void ASiltGroundMist::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bReady || !Cards)
	{
		return;
	}

	const float Time = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	for (int32 Index = 0; Index < Anchors.Num(); ++Index)
	{
		const float DriftX = FMath::Sin(Time * 0.17f + Phases[Index]) * 180.f;
		const float DriftY = FMath::Cos(Time * 0.13f + Phases[Index]) * 140.f;
		const float X = Anchors[Index].X + DriftX;
		const float Y = Anchors[Index].Y + DriftY;
		const float Ground = SiltTerrain::SampleHeight(X, Y);
		const float Base = FMath::Max(Ground, SiltTerrain::GetWaterLevel());
		const float Bob = FMath::Sin(Time * 0.6f + Phases[Index]) * 8.f;
		const float Z = Base + FMath::Min(Lifts[Index] + Bob, MaxLiftCm);
		const FRotator Yaw(0.f, Index * 23.f + Time * 2.f, 0.f);
		Cards->UpdateInstanceTransform(
			Index,
			FTransform(Yaw, FVector(X, Y, Z), FVector(Spans[Index], Spans[Index] * 0.72f, 1.f)),
			true,
			Index == Anchors.Num() - 1,
			true);
	}
}
