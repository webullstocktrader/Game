#include "SiltShallowPuddles.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "SiltCounty.h"
#include "SiltTerrain.h"
#include "SiltTypes.h"

ASiltShallowPuddles::ASiltShallowPuddles()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(false);
	Tags.Add(TEXT("SiltCounty"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	Root->SetMobility(EComponentMobility::Static);
	SetRootComponent(Root);
}

void ASiltShallowPuddles::BeginPlay()
{
	Super::BeginPlay();
	BuildField();
}

void ASiltShallowPuddles::BuildField()
{
	if (bFieldBuilt)
	{
		return;
	}
	bFieldBuilt = true;

	UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	UMaterialInterface* PuddleMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SiltCounty/Materials/M_Puddle.M_Puddle"));
	UMaterialInterface* Base = PuddleMat;
	if (!Base)
	{
		Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SiltCounty/Materials/M_FloodWater.M_FloodWater"));
		UE_LOG(LogSiltCounty, Warning, TEXT("Silt shallow puddles using M_FloodWater until M_Puddle is generated"));
	}
	if (!Plane || !Base)
	{
		UE_LOG(LogSiltCounty, Warning, TEXT("Silt shallow puddles skipped — mesh or material missing"));
		return;
	}

	const bool bHasWetness = PuddleMat != nullptr;
	TArray<FVector2D> Occupied;
	int32 Count = 0;

	auto AddCard = [&](float X, float Y, float Yaw, float ScaleX, float ScaleY, float Wetness)
	{
		const float Ground = SiltTerrain::SampleHeight(X, Y);
		const FName Name(*FString::Printf(TEXT("Puddle_%d"), Count));
		UStaticMeshComponent* Card = NewObject<UStaticMeshComponent>(this, Name);
		if (!Card)
		{
			return;
		}

		Card->SetupAttachment(GetRootComponent());
		Card->SetMobility(EComponentMobility::Static);
		Card->SetStaticMesh(Plane);
		Card->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Card->SetGenerateOverlapEvents(false);
		Card->SetCastShadow(false);
		Card->SetCanEverAffectNavigation(false);
		Card->SetRelativeLocation(FVector(X, Y, Ground + 6.f));
		Card->SetRelativeRotation(FRotator(0.f, Yaw, 0.f));
		Card->SetRelativeScale3D(FVector(ScaleX, ScaleY, 1.f));

		if (UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, this))
		{
			if (bHasWetness)
			{
				Mid->SetScalarParameterValue(TEXT("Wetness"), FMath::Clamp(Wetness, 0.f, 1.f));
			}
			Card->SetMaterial(0, Mid);
		}
		else
		{
			Card->SetMaterial(0, Base);
		}

		Card->RegisterComponent();
		Occupied.Add(FVector2D(X, Y));
		++Count;
	};

	struct FYardCard
	{
		float X;
		float Y;
		float Yaw;
		float ScaleX;
		float ScaleY;
		float Wetness;
	};

	// Apron south of the garage. Explicit so the pad reads wet even where the surface is dirt.
	const FYardCard Yard[] = {
		{-900.f, 77200.f, 18.f, 6.4f, 4.2f, 0.92f},
		{420.f, 76800.f, -24.f, 5.1f, 7.2f, 0.78f},
		{1680.f, 79200.f, 40.f, 4.6f, 5.4f, 0.85f},
		{-1900.f, 80400.f, 12.f, 7.5f, 4.8f, 1.f},
		{180.f, 81200.f, 66.f, 5.8f, 4.4f, 0.80f},
		{-420.f, 74200.f, 100.f, 8.0f, 5.2f, 0.95f},
		{2300.f, 73400.f, -50.f, 4.4f, 6.6f, 0.88f},
		{-2500.f, 77600.f, 150.f, 6.8f, 3.8f, 0.72f},
	};
	for (const FYardCard& Card : Yard)
	{
		AddCard(Card.X, Card.Y, Card.Yaw, Card.ScaleX, Card.ScaleY, Card.Wetness);
	}

	constexpr int32 MudCap = 36;
	int32 MudCount = 0;
	const float Water = SiltTerrain::GetWaterLevel();
	const FVector Chief = SiltTerrain::GetChiefSpawn();
	const FVector Gooch = SiltTerrain::GetGoochSpawn();

	auto TryMud = [&](float X, float Y, bool bShoulderOnly)
	{
		if (MudCount >= MudCap)
		{
			return;
		}
		if (FMath::Abs(X) < 1300.f && Y > 84800.f && Y < 91000.f)
		{
			return;
		}
		if (FVector2D::Distance(FVector2D(X, Y), FVector2D(Chief.X, Chief.Y)) < 280.f
			|| FVector2D::Distance(FVector2D(X, Y), FVector2D(Gooch.X, Gooch.Y)) < 280.f)
		{
			return;
		}
		for (const FVector2D& Other : Occupied)
		{
			if (FVector2D::Distance(FVector2D(X, Y), Other) < 2200.f)
			{
				return;
			}
		}

		const float Road = SiltTerrain::DistanceToRoad(X, Y);
		if (bShoulderOnly && (Road < 1400.f || Road > 3400.f))
		{
			return;
		}

		const float Height = SiltTerrain::SampleHeight(X, Y);
		if (Height < Water + 20.f)
		{
			return;
		}
		const ESiltSurface Surface = SiltTerrain::SampleSurface(X, Y);
		if (Surface != ESiltSurface::Mud && Surface != ESiltSurface::DeepMud)
		{
			return;
		}

		const float Wetness = Surface == ESiltSurface::DeepMud ? 1.f : 0.65f;
		const float ScaleX = 4.2f + static_cast<float>(MudCount % 5) * 0.75f;
		const float ScaleY = 3.6f + static_cast<float>((MudCount * 3) % 5) * 0.85f;
		const float Yaw = static_cast<float>((MudCount * 47) % 180) - 20.f;
		AddCard(X, Y, Yaw, ScaleX, ScaleY, Wetness);
		++MudCount;
	};

	for (int32 YIndex = 0; YIndex <= 25 && MudCount < MudCap; ++YIndex)
	{
		const float Y = -28000.f + static_cast<float>(YIndex) * 4500.f;
		for (int32 XIndex = 0; XIndex <= 10 && MudCount < MudCap; ++XIndex)
		{
			const float X = -32000.f + static_cast<float>(XIndex) * 4500.f;
			if (X <= 12000.f && Y <= 86000.f)
			{
				TryMud(X, Y, true);
			}
		}
	}

	const FVector2D Contract = SiltTerrain::GetContractXY();
	const float Radii[] = {1600.f, 2800.f, 4200.f, 5600.f};
	const int32 Samples[] = {5, 7, 8, 6};
	for (int32 Ring = 0; Ring < 4 && MudCount < MudCap; ++Ring)
	{
		for (int32 Step = 0; Step < Samples[Ring] && MudCount < MudCap; ++Step)
		{
			const float Angle = (2.f * PI * static_cast<float>(Step) / static_cast<float>(Samples[Ring])) + static_cast<float>(Ring) * 0.35f;
			TryMud(
				Contract.X + FMath::Cos(Angle) * Radii[Ring],
				Contract.Y + FMath::Sin(Angle) * Radii[Ring],
				false);
		}
	}

	UE_LOG(LogSiltCounty, Display, TEXT("Silt shallow puddles: %d (garage yard + driveable mud)"), Count);
}
