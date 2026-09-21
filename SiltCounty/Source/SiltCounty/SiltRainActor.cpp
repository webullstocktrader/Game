#include "SiltRainActor.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"

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

	const FVector Scale(0.012f, 0.012f, 1.5f);
	Velocities.SetNum(DropCount);
	for (int32 Index = 0; Index < DropCount; ++Index)
	{
		const FVector Local(
			FMath::FRandRange(-2000.f, 2000.f),
			FMath::FRandRange(-2000.f, 2000.f),
			FMath::FRandRange(0.f, 1800.f));
		Velocities[Index] = FVector(FMath::FRandRange(-80.f, 180.f), FMath::FRandRange(-40.f, 40.f), FMath::FRandRange(-1600.f, -1100.f));
		Drops->AddInstance(FTransform(FRotator::ZeroRotator, Local, Scale));
	}
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
	const FVector Scale(0.012f, 0.012f, 1.5f);
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
		Relative.X = Wrap(Relative.X, 2000.f);
		Relative.Y = Wrap(Relative.Y, 2000.f);
		Relative.Z = Wrap(Relative.Z - 800.f, 1000.f) + 800.f;
		Location = Origin + Relative;
		Drops->UpdateInstanceTransform(Index, FTransform(FRotator(0.f, 0.f, -8.f), Location, Scale), true, Index == Velocities.Num() - 1, true);
	}
}
