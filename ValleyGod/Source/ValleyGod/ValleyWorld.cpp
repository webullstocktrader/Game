#include "ValleyWorld.h"
#include "ValleyTerrain.h"
#include "ValleyVillager.h"
#include "ValleyAnimal.h"
#include "ValleyTypes.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/SkyLight.h"
#include "Engine/VolumetricCloud.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"

AValleyWorld::AValleyWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

AValleyWorld* AValleyWorld::Get(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	for (TActorIterator<AValleyWorld> It(World); It; ++It)
	{
		return *It;
	}
	return nullptr;
}

void AValleyWorld::BuildValley()
{
	vg::InitWorld(Brain);
	StripTemplateActors();
	Terrain = GetWorld()->SpawnActor<AValleyTerrain>(FVector::ZeroVector, FRotator::ZeroRotator);
	Terrain->BuildValley();
	SpawnAtmosphere();
	SpawnTreesAndRocks();
	SpawnSheltersAndFire();
	SpawnPeople();
	SpawnRain();
	SpawnTornado();
}

void AValleyWorld::CommandWeather(vg::Weather Wx)
{
	vg::SetWeather(Brain, Wx);
}

void AValleyWorld::AdjustDayLength(float DeltaSeconds)
{
	vg::SetDayLength(Brain, Brain.DayLengthSeconds + DeltaSeconds);
}

void AValleyWorld::TogglePause()
{
	vg::SetPaused(Brain, !Brain.Paused);
}

void AValleyWorld::PinVillager(int32 Id)
{
	PinnedId = Id;
}

int32 AValleyWorld::CyclePin()
{
	if (Brain.VillagerCount <= 0)
	{
		return -1;
	}
	PinnedId = (PinnedId + 1) % Brain.VillagerCount;
	return PinnedId;
}

AValleyVillager* AValleyWorld::FindVillager(int32 Id) const
{
	for (AValleyVillager* V : Villagers)
	{
		if (V && V->GetVillagerId() == Id)
		{
			return V;
		}
	}
	return nullptr;
}

void AValleyWorld::StripTemplateActors()
{
	TArray<AActor*> Kill;
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		const FString Name = It->GetName();
		if (Name.Contains(TEXT("Floor")) || Name.Contains(TEXT("SkySphere")) || Name.Contains(TEXT("AtmosphericFog"))
			|| Name.Contains(TEXT("Template")) || Name.Contains(TEXT("SM_SkySphere")))
		{
			Kill.Add(*It);
		}
	}
	for (AActor* Actor : Kill)
	{
		if (Actor && Actor != this)
		{
			Actor->Destroy();
		}
	}
}

UStaticMeshComponent* AValleyWorld::Place(UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat, const FName& Name)
{
	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
	Comp->SetStaticMesh(Mesh);
	Comp->SetWorldLocation(Loc);
	Comp->SetWorldRotation(Rot);
	Comp->SetWorldScale3D(Scale);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Mat)
	{
		Comp->SetMaterial(0, Mat);
	}
	Comp->SetupAttachment(GetRootComponent());
	Comp->RegisterComponent();
	return Comp;
}

void AValleyWorld::SpawnAtmosphere()
{
	Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-42.f, 200.f, 0.f));
	if (UDirectionalLightComponent* L = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
	{
		L->SetIntensity(8.f);
		L->SetLightColor(FLinearColor(1.f, 0.93f, 0.78f));
		L->SetAtmosphereSunLight(true);
		L->SetDynamicShadowDistanceMovableLight(40000.f);
	}

	Sky = GetWorld()->SpawnActor<ASkyLight>();
	if (USkyLightComponent* S = Sky->GetLightComponent())
	{
		S->SetIntensity(1.2f);
		S->bRealTimeCapture = true;
		S->SetLightColor(FLinearColor(0.72f, 0.8f, 0.9f));
	}

	GetWorld()->SpawnActor<ASkyAtmosphere>();
	GetWorld()->SpawnActor<AVolumetricCloud>();

	Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator);
	if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
	{
		FogComp->SetFogDensity(0.018f);
		FogComp->FogHeightFalloff = 0.14f;
		FogComp->SetVolumetricFog(true);
		FogComp->VolumetricFogExtinctionScale = 0.7f;
		FogComp->SetFogInscatteringColor(FLinearColor(0.45f, 0.52f, 0.48f));
	}

	Post = GetWorld()->SpawnActor<APostProcessVolume>();
	Post->bUnbound = true;
	FPostProcessSettings& P = Post->Settings;
	P.bOverride_AutoExposureMethod = true;
	P.AutoExposureMethod = AEM_Manual;
	P.bOverride_AutoExposureBias = true;
	P.AutoExposureBias = 0.2f;
	P.bOverride_ColorSaturation = true;
	P.ColorSaturation = FVector4(0.92f, 0.9f, 0.82f, 1.f);
	P.bOverride_ColorContrast = true;
	P.ColorContrast = FVector4(1.08f, 1.06f, 1.04f, 1.f);
	P.bOverride_VignetteIntensity = true;
	P.VignetteIntensity = 0.28f;
	P.bOverride_BloomIntensity = true;
	P.BloomIntensity = 0.45f;
	P.bOverride_AmbientCubemapIntensity = false;
}

void AValleyWorld::SpawnTreesAndRocks()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Sphere = Valley::SphereMesh();
	UMaterialInterface* Bark = Valley::Material(TEXT("M_Bark"));
	UMaterialInterface* Leaf = Valley::Material(TEXT("M_Foliage"));
	UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone"));
	if (!Cyl || !Sphere || !Terrain)
	{
		return;
	}

	FRandomStream Rng(19);
	int32 TreeN = 0;
	for (int32 I = 0; I < 48; ++I)
	{
		const float X = Rng.FRandRange(-5200.f, 5200.f);
		const float Y = Rng.FRandRange(-5200.f, 5200.f);
		if (FVector2D::Distance(FVector2D(X, Y), FVector2D(0.f, 700.f)) < 700.f)
		{
			continue;
		}
		if (FMath::Abs(X) < 500.f && FMath::Abs(Y) < 400.f)
		{
			continue;
		}
		const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
		const float H = Rng.FRandRange(2.4f, 4.6f);
		UStaticMeshComponent* Trunk = Place(Cyl, G + FVector(0.f, 0.f, H * 50.f), FRotator::ZeroRotator, FVector(0.28f, 0.28f, H), Bark, FName(*FString::Printf(TEXT("Trunk%d"), TreeN)));
		Trees.Add(Trunk);
		Place(Sphere, G + FVector(0.f, 0.f, H * 92.f), FRotator::ZeroRotator, FVector(1.6f, 1.6f, 1.3f) * Rng.FRandRange(0.9f, 1.2f), Leaf, FName(*FString::Printf(TEXT("Canopy%d"), TreeN)));
		Place(Sphere, G + FVector(Rng.FRandRange(-40.f, 40.f), Rng.FRandRange(-40.f, 40.f), H * 108.f), FRotator::ZeroRotator, FVector(1.1f, 1.2f, 0.9f), Leaf, FName(*FString::Printf(TEXT("CanopyB%d"), TreeN)));
		++TreeN;
	}

	for (int32 I = 0; I < 14; ++I)
	{
		const float X = Rng.FRandRange(-4000.f, 4000.f);
		const float Y = Rng.FRandRange(-2000.f, 2200.f);
		const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
		Place(Sphere, G + FVector(0.f, 0.f, 18.f), FRotator(Rng.FRandRange(0.f, 40.f), Rng.FRandRange(0.f, 180.f), 0.f),
			FVector(Rng.FRandRange(0.4f, 1.1f), Rng.FRandRange(0.3f, 0.8f), Rng.FRandRange(0.25f, 0.5f)), Stone,
			FName(*FString::Printf(TEXT("Rock%d"), I)));
	}
}

void AValleyWorld::SpawnSheltersAndFire()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Cube = Valley::CubeMesh();
	UMaterialInterface* Wood = Valley::Material(TEXT("M_Wood"));
	UMaterialInterface* Hide = Valley::Material(TEXT("M_Hide"));
	UMaterialInterface* Fire = Valley::Material(TEXT("M_Fire"));
	UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone"));
	if (!Cyl || !Terrain)
	{
		return;
	}

	for (int32 I = 0; I < Brain.ShelterCount; ++I)
	{
		const FVector G = Terrain->GroundAt(FVector(Brain.ShelterX[I], Brain.ShelterY[I], 0.f));
		Place(Cyl, G + FVector(-70.f, -40.f, 70.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 1.4f), Wood, FName(*FString::Printf(TEXT("PoleA%d"), I)));
		Place(Cyl, G + FVector(70.f, -40.f, 70.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 1.4f), Wood, FName(*FString::Printf(TEXT("PoleB%d"), I)));
		Place(Cyl, G + FVector(0.f, 50.f, 40.f), FRotator(0.f, 0.f, 55.f), FVector(0.1f, 0.1f, 1.6f), Wood, FName(*FString::Printf(TEXT("PoleC%d"), I)));
		if (Cube)
		{
			Place(Cube, G + FVector(0.f, 10.f, 120.f), FRotator(-28.f, 0.f, 0.f), FVector(2.2f, 2.0f, 0.08f), Hide, FName(*FString::Printf(TEXT("Roof%d"), I)));
		}
	}

	const FVector FireG = Terrain->GroundAt(FVector(Brain.FireX, Brain.FireY, 0.f));
	Place(Cyl, FireG + FVector(0.f, 0.f, 12.f), FRotator::ZeroRotator, FVector(0.7f, 0.7f, 0.12f), Stone, TEXT("Hearth"));
	if (Cube)
	{
		Place(Cube, FireG + FVector(0.f, 0.f, 28.f), FRotator::ZeroRotator, FVector(0.35f, 0.35f, 0.45f), Fire, TEXT("Flame"));
		Place(Cube, FireG + FVector(8.f, -6.f, 48.f), FRotator(12.f, 20.f, 0.f), FVector(0.16f, 0.16f, 0.4f), Fire, TEXT("FlameTip"));
	}

	FireLight = NewObject<UPointLightComponent>(this, TEXT("FireLight"));
	FireLight->SetWorldLocation(FireG + FVector(0.f, 0.f, 70.f));
	FireLight->SetIntensity(3500.f);
	FireLight->SetLightColor(FLinearColor(1.f, 0.55f, 0.18f));
	FireLight->SetAttenuationRadius(900.f);
	FireLight->SetCastShadows(true);
	FireLight->SetupAttachment(GetRootComponent());
	FireLight->RegisterComponent();
}

void AValleyWorld::SpawnPeople()
{
	const FLinearColor Skins[] = {
		FLinearColor(0.45f, 0.32f, 0.22f), FLinearColor(0.62f, 0.44f, 0.30f), FLinearColor(0.28f, 0.18f, 0.12f),
		FLinearColor(0.52f, 0.36f, 0.24f), FLinearColor(0.38f, 0.24f, 0.16f), FLinearColor(0.58f, 0.40f, 0.28f),
		FLinearColor(0.34f, 0.22f, 0.14f)
	};
	const FLinearColor Cloths[] = {
		FLinearColor(0.28f, 0.16f, 0.08f), FLinearColor(0.18f, 0.14f, 0.10f), FLinearColor(0.32f, 0.22f, 0.10f),
		FLinearColor(0.22f, 0.12f, 0.08f), FLinearColor(0.14f, 0.12f, 0.10f)
	};
	const FLinearColor Hairs[] = {
		FLinearColor(0.04f, 0.03f, 0.02f), FLinearColor(0.12f, 0.07f, 0.03f), FLinearColor(0.08f, 0.06f, 0.04f)
	};

	for (int32 I = 0; I < Brain.VillagerCount; ++I)
	{
		AValleyVillager* V = GetWorld()->SpawnActor<AValleyVillager>();
		V->Arm(I, Skins[I % 7], Cloths[I % 5], Hairs[I % 3]);
		V->SyncFromSim(Brain.Villagers[I], Terrain, 0.f);
		Villagers.Add(V);
	}

	for (int32 I = 0; I < Brain.AnimalCount; ++I)
	{
		AValleyAnimal* A = GetWorld()->SpawnActor<AValleyAnimal>();
		A->Arm(I);
		A->SyncFromSim(Brain.Animals[I], Terrain, 0.f);
		Animals.Add(A);
	}
}

void AValleyWorld::SpawnRain()
{
	UStaticMesh* Cube = Valley::CubeMesh();
	UMaterialInterface* Water = Valley::Material(TEXT("M_Water"));
	if (!Cube)
	{
		return;
	}
	for (int32 I = 0; I < 70; ++I)
	{
		UStaticMeshComponent* Streak = NewObject<UStaticMeshComponent>(this, FName(*FString::Printf(TEXT("Rain%d"), I)));
		Streak->SetStaticMesh(Cube);
		Streak->SetWorldScale3D(FVector(0.015f, 0.015f, 0.7f));
		Streak->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Streak->SetCastShadow(false);
		Streak->SetHiddenInGame(true);
		if (Water)
		{
			Streak->SetMaterial(0, Water);
		}
		Streak->SetupAttachment(GetRootComponent());
		Streak->RegisterComponent();
		RainStreaks.Add(Streak);
	}
}

void AValleyWorld::SpawnTornado()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UMaterialInterface* Dust = Valley::Material(TEXT("M_Dirt"));
	if (!Cyl)
	{
		return;
	}
	for (int32 I = 0; I < 6; ++I)
	{
		const float S = 0.4f + I * 0.35f;
		UStaticMeshComponent* Ring = Place(Cyl, FVector(0.f, 1800.f, 80.f + I * 90.f), FRotator::ZeroRotator, FVector(S, S, 0.9f), Dust,
			FName(*FString::Printf(TEXT("Tornado%d"), I)));
		Ring->SetHiddenInGame(true);
		Ring->SetCastShadow(false);
		TornadoParts.Add(Ring);
	}
}

void AValleyWorld::UpdateSky()
{
	const float Hours = Brain.TimeOfDayHours;
	const float Altitude = FMath::Sin((Hours - 6.f) / 12.f * PI) * 88.f;
	const float Pitch = -Altitude;
	const float Yaw = 20.f + Hours * 12.f;
	if (Sun)
	{
		Sun->SetActorRotation(FRotator(Pitch, Yaw, 0.f));
		if (UDirectionalLightComponent* L = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			const bool bNight = vg::IsNight(Hours);
			const float Dusk = FMath::Clamp(1.f - FMath::Abs(Hours - 18.f) / 2.5f, 0.f, 1.f);
			const float Dawn = FMath::Clamp(1.f - FMath::Abs(Hours - 6.f) / 2.5f, 0.f, 1.f);
			const float Gold = FMath::Max(Dusk, Dawn);
			L->SetIntensity(bNight ? 0.35f : FMath::Lerp(7.5f, 4.2f, Gold));
			L->SetLightColor(bNight ? FLinearColor(0.25f, 0.32f, 0.55f) : FMath::Lerp(FLinearColor(1.f, 0.95f, 0.82f), FLinearColor(1.f, 0.55f, 0.28f), Gold));
		}
	}
	if (Sky)
	{
		if (USkyLightComponent* S = Sky->GetLightComponent())
		{
			S->SetIntensity(vg::IsNight(Hours) ? 0.28f : 1.15f);
		}
	}
	if (Fog)
	{
		if (UExponentialHeightFogComponent* F = Fog->GetComponent())
		{
			const bool bStorm = Brain.Sky != vg::Weather::Clear;
			F->SetFogDensity(vg::IsNight(Hours) ? 0.03f : (bStorm ? 0.04f : 0.016f));
		}
	}
	if (Post)
	{
		const bool bNight = vg::IsNight(Hours);
		Post->Settings.AutoExposureBias = bNight ? -0.35f : 0.22f;
	}
	if (FireLight)
	{
		FireLight->SetIntensity(vg::IsNight(Hours) ? 8000.f : 2800.f);
	}
}

void AValleyWorld::UpdateWeatherVisuals(float DeltaSeconds)
{
	const bool bRain = Brain.Sky == vg::Weather::Rain || Brain.Sky == vg::Weather::Hurricane;
	if (Terrain)
	{
		Terrain->SetWet(bRain || Brain.Sky == vg::Weather::Flood);
		Terrain->SetFlood(Brain.FloodHeight);
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	FVector Cam = FVector::ZeroVector;
	if (PC && PC->PlayerCameraManager)
	{
		Cam = PC->PlayerCameraManager->GetCameraLocation();
	}

	for (UStaticMeshComponent* Streak : RainStreaks)
	{
		if (!Streak)
		{
			continue;
		}
		Streak->SetHiddenInGame(!bRain);
		if (!bRain)
		{
			continue;
		}
		FVector Loc = Streak->GetComponentLocation();
		Loc.Z -= 3200.f * DeltaSeconds;
		Loc.X += Brain.WindX * 400.f * DeltaSeconds;
		if (Loc.Z < Cam.Z - 500.f || RainClock < 0.05f)
		{
			Loc = Cam + FVector(FMath::FRandRange(-1600.f, 1600.f), FMath::FRandRange(-1600.f, 1600.f), FMath::FRandRange(200.f, 1400.f));
		}
		Streak->SetWorldLocation(Loc);
	}
	RainClock += DeltaSeconds;

	const bool bTornado = Brain.Sky == vg::Weather::Tornado;
	for (int32 I = 0; I < TornadoParts.Num(); ++I)
	{
		UStaticMeshComponent* Ring = TornadoParts[I];
		if (!Ring)
		{
			continue;
		}
		Ring->SetHiddenInGame(!bTornado);
		if (!bTornado)
		{
			continue;
		}
		const float Z = 70.f + I * 95.f;
		FVector G = Terrain ? Terrain->GroundAt(FVector(Brain.TornadoX, Brain.TornadoY, 0.f)) : FVector(Brain.TornadoX, Brain.TornadoY, 0.f);
		Ring->SetWorldLocation(G + FVector(0.f, 0.f, Z));
		Ring->AddWorldRotation(FRotator(0.f, 220.f * DeltaSeconds * (1.f + I * 0.15f), 0.f));
	}

	const float Lean = FMath::Clamp(Brain.WindX * 12.f + (Brain.Sky == vg::Weather::Hurricane ? 14.f : 0.f), 0.f, 22.f);
	for (UStaticMeshComponent* Trunk : Trees)
	{
		if (Trunk)
		{
			Trunk->SetWorldRotation(FRotator(Lean, 18.f, 0.f));
		}
	}
}

void AValleyWorld::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	vg::TickWorld(Brain, DeltaSeconds);
	UpdateSky();
	UpdateWeatherVisuals(DeltaSeconds);

	const float T = GetWorld()->TimeSeconds;
	for (AValleyVillager* V : Villagers)
	{
		if (V && V->GetVillagerId() >= 0 && V->GetVillagerId() < Brain.VillagerCount)
		{
			V->SyncFromSim(Brain.Villagers[V->GetVillagerId()], Terrain, T);
		}
	}
	for (AValleyAnimal* A : Animals)
	{
		if (A && A->GetAnimalId() >= 0 && A->GetAnimalId() < Brain.AnimalCount)
		{
			A->SyncFromSim(Brain.Animals[A->GetAnimalId()], Terrain, T);
		}
	}
}
