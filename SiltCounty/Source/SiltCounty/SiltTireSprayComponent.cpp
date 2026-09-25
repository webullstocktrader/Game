#include "SiltTireSprayComponent.h"

#include "SiltCounty.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"

namespace
{
	constexpr int32 SprayPool = 40;
	constexpr int32 KickPool = 16;
	// Puffs per second at full wetness and speed. Charge is what makes the rate follow speed × wetness.
	constexpr float SprayPerSecond = 16.f;
	constexpr float KickPerSecond = 6.f;

	float SurfaceWetness(ESiltSurface Surface)
	{
		switch (Surface)
		{
		case ESiltSurface::Road: return 0.05f;
		case ESiltSurface::Dirt: return 0.22f;
		case ESiltSurface::Mud: return 0.82f;
		case ESiltSurface::DeepMud: return 1.f;
		case ESiltSurface::Water: return 0.95f;
		default: return 0.f;
		}
	}

	bool IsKickSurface(ESiltSurface Surface)
	{
		return Surface == ESiltSurface::Mud || Surface == ESiltSurface::DeepMud || Surface == ESiltSurface::Water;
	}
}

USiltTireSprayComponent::USiltTireSprayComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicated(false);

	Spray = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Spray"));
	Kick = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Kick"));
	Spray->SetupAttachment(this);
	Kick->SetupAttachment(this);
	ConfigureField(Spray);
	ConfigureField(Kick);
}

void USiltTireSprayComponent::ConfigureField(UInstancedStaticMeshComponent* Field) const
{
	if (!Field)
	{
		return;
	}
	Field->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Field->SetCastShadow(false);
	Field->SetCanEverAffectNavigation(false);
	Field->SetGenerateOverlapEvents(false);
	Field->SetMobility(EComponentMobility::Movable);
	Field->SetReceivesDecals(false);
	Field->SetIsReplicated(false);
}

UMaterialInterface* USiltTireSprayComponent::LoadFx(const TCHAR* Name, const TCHAR* Fallback) const
{
	const FString Path = FString::Printf(TEXT("/Game/SiltCounty/Materials/%s.%s"), Name, Name);
	if (UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, *Path))
	{
		return Mat;
	}
	const FString Backup = FString::Printf(TEXT("/Game/SiltCounty/Materials/%s.%s"), Fallback, Fallback);
	return LoadObject<UMaterialInterface>(nullptr, *Backup);
}

void USiltTireSprayComponent::Seed(UInstancedStaticMeshComponent* Field, TArray<FSiltSprayPuff>& Puffs, int32 Count)
{
	if (!Field)
	{
		return;
	}
	Puffs.SetNum(Count);
	Field->ClearInstances();
	for (int32 Index = 0; Index < Count; ++Index)
	{
		Field->AddInstance(FTransform(FRotator::ZeroRotator, FVector::ZeroVector, FVector::ZeroVector));
	}
}

void USiltTireSprayComponent::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (!Sphere)
	{
		return;
	}

	Spray->SetStaticMesh(Sphere);
	Kick->SetStaticMesh(Sphere);
	if (UMaterialInterface* SprayMat = LoadFx(TEXT("M_TireSpray"), TEXT("M_Rain")))
	{
		Spray->SetMaterial(0, SprayMat);
	}
	if (UMaterialInterface* KickMat = LoadFx(TEXT("M_MudKick"), TEXT("M_TireSpray")))
	{
		Kick->SetMaterial(0, KickMat);
	}

	Seed(Spray, SprayPuffs, SprayPool);
	Seed(Kick, KickPuffs, KickPool);
	bReady = true;
	UE_LOG(LogSiltCounty, Display, TEXT("Silt County tire spray ready (local, not replicated)."));
}

void USiltTireSprayComponent::Emit(TArray<FSiltSprayPuff>& Puffs, int32& Cursor, const FVector& Pos, const FVector& Vel, float Life, float Size)
{
	if (Puffs.Num() == 0)
	{
		return;
	}
	FSiltSprayPuff& Puff = Puffs[Cursor % Puffs.Num()];
	++Cursor;
	Puff.bLive = true;
	Puff.Age = 0.f;
	Puff.Life = Life;
	Puff.Size = Size;
	Puff.Pos = Pos;
	Puff.Vel = Vel;
}

void USiltTireSprayComponent::Simulate(UInstancedStaticMeshComponent* Field, TArray<FSiltSprayPuff>& Puffs, float DeltaSeconds)
{
	if (!Field)
	{
		return;
	}
	for (int32 Index = 0; Index < Puffs.Num(); ++Index)
	{
		FSiltSprayPuff& Puff = Puffs[Index];
		FVector Scale = FVector::ZeroVector;
		if (Puff.bLive)
		{
			Puff.Age += DeltaSeconds;
			if (Puff.Age >= Puff.Life)
			{
				Puff.bLive = false;
			}
			else
			{
				Puff.Vel.Z -= 420.f * DeltaSeconds;
				Puff.Pos += Puff.Vel * DeltaSeconds;
				const float T = Puff.Age / FMath::Max(Puff.Life, 0.01f);
				const float Span = Puff.Size * (1.f - T);
				Scale = FVector(Span);
			}
		}
		Field->UpdateInstanceTransform(Index, FTransform(FRotator::ZeroRotator, Puff.Pos, Scale), true, Index == Puffs.Num() - 1, true);
	}
}

void USiltTireSprayComponent::UpdateWheels(float DeltaSeconds, float SinkAlpha, const FSiltWheelSpray* Wheels, int32 Count)
{
	if (!bReady || !Wheels || Count <= 0 || DeltaSeconds <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	int32 SprayBurst = 0;
	int32 KickBurst = 0;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FSiltWheelSpray& Wheel = Wheels[Index];
		if (!Wheel.bGrounded)
		{
			continue;
		}

		const float Speed = Wheel.PointVelocity.Size2D();
		if (Speed < 140.f)
		{
			continue;
		}

		float Wet = SurfaceWetness(Wheel.Surface);
		if (Wheel.Surface == ESiltSurface::Mud || Wheel.Surface == ESiltSurface::DeepMud)
		{
			Wet *= 0.75f + 0.25f * FMath::Clamp(SinkAlpha, 0.f, 1.f);
		}
		const float Amount = FMath::Clamp(Wet * (Speed / 1200.f), 0.f, 1.f);
		if (Amount < 0.045f)
		{
			continue;
		}

		FVector Back(-Wheel.PointVelocity.X, -Wheel.PointVelocity.Y, 0.f);
		Back = Back.GetSafeNormal();
		if (Back.IsNearlyZero())
		{
			continue;
		}

		if (Index >= 4)
		{
			continue;
		}

		const FVector SprayDir = (Back * 0.72f + Wheel.Outward * 0.32f + FVector::UpVector * 0.42f).GetSafeNormal();
		SprayCharge[Index] = FMath::Min(SprayCharge[Index] + Amount * DeltaSeconds * SprayPerSecond, 2.f);
		while (SprayCharge[Index] >= 1.f && SprayBurst < 6)
		{
			Emit(SprayPuffs, SprayCursor, Wheel.Contact, SprayDir * (180.f + 720.f * Amount), 0.18f + 0.1f * Amount, 0.12f + 0.28f * Amount);
			SprayCharge[Index] -= 1.f;
			++SprayBurst;
			if (Amount > 0.55f && SprayBurst < 6)
			{
				Emit(SprayPuffs, SprayCursor, Wheel.Contact + Wheel.Outward * 18.f, SprayDir * (140.f + 480.f * Amount), 0.16f, 0.1f + 0.16f * Amount);
				++SprayBurst;
			}
		}

		if (IsKickSurface(Wheel.Surface) && Amount > 0.28f)
		{
			const FVector KickDir = (Back * 0.55f + Wheel.Outward * 0.5f + FVector::UpVector * 0.62f).GetSafeNormal();
			KickCharge[Index] = FMath::Min(KickCharge[Index] + Amount * DeltaSeconds * KickPerSecond, 2.f);
			if (KickCharge[Index] >= 1.f && KickBurst < 3)
			{
				Emit(KickPuffs, KickCursor, Wheel.Contact, KickDir * (90.f + 280.f * Amount), 0.28f + 0.12f * Amount, 0.28f + 0.55f * Amount);
				KickCharge[Index] -= 1.f;
				++KickBurst;
			}
		}
	}

	Simulate(Spray, SprayPuffs, DeltaSeconds);
	Simulate(Kick, KickPuffs, DeltaSeconds);
}
