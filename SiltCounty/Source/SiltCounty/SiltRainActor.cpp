#include "SiltRainActor.h"

#include "SiltCounty.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

namespace
{
	// Thin vertical streaks. A 100 cm engine cube at this scale is about 0.8 cm by 180 cm.
	const FVector RainStreakScale(0.008f, 0.008f, 1.8f);
	constexpr float RainWrapHalfCm = 2000.f;
}

ASiltRainActor::ASiltRainActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetReplicates(false);
	Tags.Add(TEXT("SiltCounty"));

	Drops = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Drops"));
	SetRootComponent(Drops);
	Drops->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Drops->SetCastShadow(false);
	Drops->SetMobility(EComponentMobility::Movable);
}

void ASiltRainActor::BeginPlay()
{
	Super::BeginPlay();

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* RainMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SiltCounty/Materials/M_Rain.M_Rain"));
	if (!Cube || !RainMat)
	{
		SetActorHiddenInGame(true);
		SetActorTickEnabled(false);
		return;
	}

	Drops->SetStaticMesh(Cube);
	Drops->SetMaterial(0, RainMat);

	Velocities.SetNum(DropCount);
	for (int32 Index = 0; Index < DropCount; ++Index)
	{
		const FVector Local(
			FMath::FRandRange(-RainWrapHalfCm, RainWrapHalfCm),
			FMath::FRandRange(-RainWrapHalfCm, RainWrapHalfCm),
			FMath::FRandRange(0.f, 1800.f));
		// Light sideways drift. Fall speed stays in this range for the whole session.
		Velocities[Index] = FVector(
			FMath::FRandRange(40.f, 120.f),
			FMath::FRandRange(-40.f, 40.f),
			FMath::FRandRange(-1300.f, -900.f));
		Drops->AddInstance(FTransform(FRotator::ZeroRotator, Local, RainStreakScale));
	}

	UE_LOG(LogSiltCounty, Display, TEXT("Silt County rain Pass A: %d ISM streaks, continuous light-moderate."), DropCount);
}

void ASiltRainActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Drops || Velocities.Num() == 0)
	{
		return;
	}

	APlayerCameraManager* Camera = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!Camera)
	{
		return;
	}

	const FVector Origin = Camera->GetCameraLocation();
	for (int32 Index = 0; Index < Velocities.Num(); ++Index)
	{
		FTransform Xform;
		if (!Drops->GetInstanceTransform(Index, Xform, true))
		{
			continue;
		}

		FVector Location = Xform.GetLocation() + Velocities[Index] * DeltaSeconds;
		FVector Relative = Location - Origin;
		auto Wrap = [](float Value, float HalfExtent)
		{
			const float Size = HalfExtent * 2.f;
			float Wrapped = FMath::Fmod(Value + HalfExtent, Size);
			if (Wrapped < 0.f)
			{
				Wrapped += Size;
			}
			return Wrapped - HalfExtent;
		};
		Relative.X = Wrap(Relative.X, RainWrapHalfCm);
		Relative.Y = Wrap(Relative.Y, RainWrapHalfCm);
		Relative.Z = Wrap(Relative.Z - 800.f, 1000.f) + 800.f;
		Location = Origin + Relative;
		Drops->UpdateInstanceTransform(Index, FTransform(FRotator(0.f, 0.f, -8.f), Location, RainStreakScale), true, Index == Velocities.Num() - 1, true);
	}
}
