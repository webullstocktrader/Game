#include "ValleyWorld.h"
#include "ValleyTerrain.h"
#include "ValleyVillager.h"
#include "ValleyAnimal.h"
#include "ValleyTypes.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
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
	Valley::EnsureMaterials();
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
		L->SetIntensity(14.f);
		L->SetLightColor(FLinearColor(1.f, 0.94f, 0.82f));
		L->SetAtmosphereSunLight(true);
		L->SetDynamicShadowDistanceMovableLight(40000.f);
		L->SetUseTemperature(true);
		L->SetTemperature(5200.f);
	}

	Sky = GetWorld()->SpawnActor<ASkyLight>();
	if (USkyLightComponent* S = Sky->GetLightComponent())
	{
		S->SetIntensity(0.85f);
		S->bRealTimeCapture = true;
		S->SetLightColor(FLinearColor(0.78f, 0.86f, 0.95f));
	}

	GetWorld()->SpawnActor<ASkyAtmosphere>();
	GetWorld()->SpawnActor<AVolumetricCloud>();

	Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator);
	if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
	{
		FogComp->SetFogDensity(0.012f);
		FogComp->FogHeightFalloff = 0.16f;
		FogComp->SetVolumetricFog(false);
		FogComp->SetFogInscatteringColor(FLinearColor(0.55f, 0.62f, 0.58f));
	}

	Post = GetWorld()->SpawnActor<APostProcessVolume>();
	Post->bUnbound = true;
	FPostProcessSettings& P = Post->Settings;
	P.bOverride_AutoExposureMethod = true;
	P.AutoExposureMethod = AEM_Histogram;
	P.bOverride_AutoExposureBias = true;
	P.AutoExposureBias = 0.35f;
	P.bOverride_AutoExposureMinBrightness = true;
	P.AutoExposureMinBrightness = -2.0f;
	P.bOverride_AutoExposureMaxBrightness = true;
	P.AutoExposureMaxBrightness = 2.0f;
	P.bOverride_ColorSaturation = true;
	P.ColorSaturation = FVector4(0.94f, 0.92f, 0.86f, 1.f);
	P.bOverride_ColorContrast = true;
	P.ColorContrast = FVector4(1.1f, 1.07f, 1.04f, 1.f);
	P.bOverride_VignetteIntensity = true;
	P.VignetteIntensity = 0.22f;
	P.bOverride_BloomIntensity = true;
	P.BloomIntensity = 0.38f;
	P.bOverride_AmbientCubemapIntensity = false;
}

void AValleyWorld::SpawnTreesAndRocks()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Sphere = Valley::SphereMesh();
	UStaticMesh* Cone = Valley::ConeMesh();
	UMaterialInterface* Bark = Valley::Material(TEXT("M_Bark"));
	UMaterialInterface* BarkDark = Valley::Material(TEXT("M_BarkDark"));
	UMaterialInterface* Leaf = Valley::Material(TEXT("M_Foliage"));
	UMaterialInterface* LeafDark = Valley::Material(TEXT("M_FoliageDark"));
	UMaterialInterface* LeafSun = Valley::Material(TEXT("M_FoliageSun"));
	UMaterialInterface* LeafUnder = Valley::Material(TEXT("M_FoliageUnderside"));
	UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone"));
	UMaterialInterface* Moss = Valley::Material(TEXT("M_Moss"));
	UMaterialInterface* Wood = Valley::Material(TEXT("M_Wood"));
	if (!Cyl || !Sphere || !Terrain)
	{
		return;
	}

	auto Attach = [&](UStaticMeshComponent* Parent, UStaticMesh* Mesh, const FName& Name, const FVector& Rel, const FRotator& Rot,
					  const FVector& WorldScale, UMaterialInterface* Mat)
	{
		if (!Parent || !Mesh)
		{
			return;
		}
		const FVector PS = Parent->GetRelativeScale3D();
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
		Comp->SetStaticMesh(Mesh);
		Comp->SetRelativeLocation(Rel);
		Comp->SetRelativeRotation(Rot);
		Comp->SetRelativeScale3D(FVector(WorldScale.X / FMath::Max(FMath::Abs(PS.X), 0.01f), WorldScale.Y / FMath::Max(FMath::Abs(PS.Y), 0.01f),
			WorldScale.Z / FMath::Max(FMath::Abs(PS.Z), 0.01f)));
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Mat)
		{
			Comp->SetMaterial(0, Mat);
		}
		Comp->SetupAttachment(Parent);
		Comp->RegisterComponent();
	};

	FRandomStream Rng(19);
	int32 TreeN = 0;
	for (int32 I = 0; I < 52; ++I)
	{
		const float X = Rng.FRandRange(-5400.f, 5400.f);
		const float Y = Rng.FRandRange(-5400.f, 5400.f);
		if (FVector2D::Distance(FVector2D(X, Y), FVector2D(0.f, 700.f)) < 720.f)
		{
			continue;
		}
		if (FMath::Abs(X) < 520.f && FMath::Abs(Y) < 420.f)
		{
			continue;
		}
		const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
		const bool bPine = Rng.FRand() > 0.74f;
		const float H = bPine ? Rng.FRandRange(3.6f, 6.2f) : Rng.FRandRange(2.9f, 5.4f);
		const float TrunkR = bPine ? Rng.FRandRange(0.38f, 0.58f) : Rng.FRandRange(0.58f, 0.98f);
		const float Yaw = Rng.FRandRange(0.f, 360.f);
		UMaterialInterface* TrunkMat = (I % 3 == 0) ? BarkDark : Bark;
		UStaticMeshComponent* Trunk = Place(Cyl, G + FVector(0.f, 0.f, H * 48.f), FRotator(0.f, Yaw, 0.f), FVector(TrunkR, TrunkR, H), TrunkMat,
			FName(*FString::Printf(TEXT("Trunk%d"), TreeN)));
		Trees.Add(Trunk);
		TreeYaw.Add(Yaw);

		Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("Root%d"), TreeN)), FVector(0.f, 0.f, -46.f), FRotator::ZeroRotator,
			FVector(TrunkR * 2.15f, TrunkR * 2.05f, 0.42f), TrunkMat);
		Attach(Trunk, Cyl, FName(*FString::Printf(TEXT("Upper%d"), TreeN)), FVector(0.f, 0.f, 28.f), FRotator::ZeroRotator,
			FVector(TrunkR * 0.72f, TrunkR * 0.72f, H * 0.55f), TrunkMat);

		if (bPine && Cone)
		{
			Attach(Trunk, Cone, FName(*FString::Printf(TEXT("PineA%d"), TreeN)), FVector(0.f, 0.f, 8.f), FRotator::ZeroRotator,
				FVector(1.7f, 1.7f, 1.1f), LeafUnder);
			Attach(Trunk, Cone, FName(*FString::Printf(TEXT("PineB%d"), TreeN)), FVector(0.f, 0.f, 24.f), FRotator::ZeroRotator,
				FVector(2.0f, 2.0f, 1.7f), LeafDark);
			Attach(Trunk, Cone, FName(*FString::Printf(TEXT("PineC%d"), TreeN)), FVector(0.f, 0.f, 46.f), FRotator::ZeroRotator,
				FVector(1.55f, 1.55f, 1.5f), Leaf);
			Attach(Trunk, Cone, FName(*FString::Printf(TEXT("PineD%d"), TreeN)), FVector(0.f, 0.f, 66.f), FRotator::ZeroRotator,
				FVector(1.05f, 1.05f, 1.25f), LeafSun);
		}
		else
		{
			const float Spread = Rng.FRandRange(0.92f, 1.12f);
			Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("CanopyA%d"), TreeN)), FVector(0.f, 0.f, 48.f), FRotator::ZeroRotator,
				FVector(2.35f, 2.2f, 1.55f) * Spread, Leaf);
			Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("CanopyB%d"), TreeN)),
				FVector(Rng.FRandRange(-22.f, 22.f), Rng.FRandRange(-22.f, 22.f), 70.f), FRotator::ZeroRotator, FVector(1.75f, 1.85f, 1.15f), LeafDark);
			Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("CanopyC%d"), TreeN)),
				FVector(Rng.FRandRange(-16.f, 16.f), Rng.FRandRange(-16.f, 16.f), 34.f), FRotator::ZeroRotator, FVector(1.55f, 1.6f, 1.05f), LeafUnder);
			Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("CanopyD%d"), TreeN)),
				FVector(Rng.FRandRange(-14.f, 14.f), Rng.FRandRange(-10.f, 10.f), 58.f), FRotator::ZeroRotator, FVector(1.35f, 1.4f, 0.95f), LeafSun);
			Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("CanopyE%d"), TreeN)),
				FVector(Rng.FRandRange(-10.f, 10.f), Rng.FRandRange(-18.f, 18.f), 42.f), FRotator::ZeroRotator, FVector(1.2f, 1.35f, 0.9f), Leaf);
			Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("CanopyF%d"), TreeN)), FVector(0.f, 0.f, 22.f), FRotator::ZeroRotator,
				FVector(1.15f, 1.2f, 0.7f), LeafDark);
		}

		Attach(Trunk, Cyl, FName(*FString::Printf(TEXT("BranchA%d"), TreeN)), FVector(Rng.FRandRange(-8.f, 8.f), 6.f, 18.f),
			FRotator(Rng.FRandRange(22.f, 58.f), Rng.FRandRange(20.f, 80.f), 0.f), FVector(0.14f, 0.14f, H * 0.42f), TrunkMat);
		Attach(Trunk, Cyl, FName(*FString::Printf(TEXT("BranchB%d"), TreeN)), FVector(Rng.FRandRange(-6.f, 6.f), -8.f, 12.f),
			FRotator(Rng.FRandRange(18.f, 50.f), Rng.FRandRange(160.f, 240.f), 0.f), FVector(0.11f, 0.11f, H * 0.32f), BarkDark);
		if (Moss)
		{
			Attach(Trunk, Sphere, FName(*FString::Printf(TEXT("Moss%d"), TreeN)), FVector(18.f, 0.f, -20.f), FRotator::ZeroRotator,
				FVector(0.22f, 0.16f, 0.12f), Moss);
		}
		++TreeN;
	}

	for (int32 I = 0; I < 22; ++I)
	{
		const float X = Rng.FRandRange(-4200.f, 4200.f);
		const float Y = Rng.FRandRange(-2200.f, 2400.f);
		const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
		const FVector RockScale(Rng.FRandRange(0.45f, 1.35f), Rng.FRandRange(0.32f, 0.95f), Rng.FRandRange(0.22f, 0.58f));
		Place(Sphere, G + FVector(0.f, 0.f, RockScale.Z * 42.f),
			FRotator(Rng.FRandRange(0.f, 50.f), Rng.FRandRange(0.f, 180.f), Rng.FRandRange(-12.f, 12.f)), RockScale, Stone,
			FName(*FString::Printf(TEXT("Rock%d"), I)));
		if (I % 4 == 0 && Moss)
		{
			Place(Sphere, G + FVector(Rng.FRandRange(-18.f, 18.f), Rng.FRandRange(-12.f, 12.f), 22.f), FRotator::ZeroRotator,
				FVector(0.22f, 0.18f, 0.08f), Moss, FName(*FString::Printf(TEXT("RockMoss%d"), I)));
		}
	}

	if (Cone)
	{
		for (int32 I = 0; I < 28; ++I)
		{
			const float X = Rng.FRandRange(-1800.f, 1800.f);
			const float Y = Rng.FRandRange(-400.f, 2200.f);
			if (FVector2D::Distance(FVector2D(X, Y), FVector2D(0.f, 700.f)) < 180.f)
			{
				continue;
			}
			const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
			Place(Cone, G + FVector(0.f, 0.f, 14.f), FRotator(Rng.FRandRange(-8.f, 8.f), Rng.FRandRange(0.f, 180.f), 0.f),
				FVector(Rng.FRandRange(0.18f, 0.32f), Rng.FRandRange(0.16f, 0.28f), Rng.FRandRange(0.22f, 0.4f)), (I % 3 == 0) ? LeafDark : Leaf,
				FName(*FString::Printf(TEXT("Tuft%d"), I)));
		}
	}

	if (Wood)
	{
		for (int32 I = 0; I < 5; ++I)
		{
			const float X = Rng.FRandRange(-2400.f, 2400.f);
			const float Y = Rng.FRandRange(-800.f, 2000.f);
			const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
			Place(Cyl, G + FVector(0.f, 0.f, 16.f), FRotator(6.f, Rng.FRandRange(0.f, 180.f), 82.f), FVector(0.18f, 0.16f, 1.4f), Wood,
				FName(*FString::Printf(TEXT("Fallen%d"), I)));
		}
	}
}

void AValleyWorld::SpawnSheltersAndFire()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Cube = Valley::CubeMesh();
	UStaticMesh* Sphere = Valley::SphereMesh();
	UStaticMesh* Cone = Valley::ConeMesh();
	UStaticMesh* Plane = Valley::PlaneMesh();
	UMaterialInterface* Wood = Valley::Material(TEXT("M_Wood"));
	UMaterialInterface* WoodDark = Valley::Material(TEXT("M_WoodDark"));
	UMaterialInterface* Hide = Valley::Material(TEXT("M_Hide"));
	UMaterialInterface* HideDark = Valley::Material(TEXT("M_HideDark"));
	UMaterialInterface* Fire = Valley::Material(TEXT("M_Fire"));
	UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone"));
	UMaterialInterface* Charcoal = Valley::Material(TEXT("M_Charcoal"));
	if (!Cyl || !Terrain)
	{
		return;
	}

	for (int32 I = 0; I < Brain.ShelterCount; ++I)
	{
		const FVector G = Terrain->GroundAt(FVector(Brain.ShelterX[I], Brain.ShelterY[I], 0.f));
		const float Yaw = I * 18.f - 30.f;
		const FRotator Facing(0.f, Yaw, 0.f);
		auto Local = [&](const FVector& Offset) { return G + Facing.RotateVector(Offset); };

		Place(Cyl, Local(FVector(-78.f, -48.f, 86.f)), FRotator(4.f, Yaw, 0.f), FVector(0.16f, 0.16f, 1.7f), Wood,
			FName(*FString::Printf(TEXT("PoleA%d"), I)));
		Place(Cyl, Local(FVector(78.f, -48.f, 86.f)), FRotator(-4.f, Yaw, 0.f), FVector(0.16f, 0.16f, 1.7f), Wood,
			FName(*FString::Printf(TEXT("PoleB%d"), I)));
		Place(Cyl, Local(FVector(-50.f, 62.f, 58.f)), FRotator(12.f, Yaw, 28.f), FVector(0.13f, 0.13f, 1.55f), WoodDark,
			FName(*FString::Printf(TEXT("PoleC%d"), I)));
		Place(Cyl, Local(FVector(50.f, 62.f, 58.f)), FRotator(12.f, Yaw, -28.f), FVector(0.13f, 0.13f, 1.55f), WoodDark,
			FName(*FString::Printf(TEXT("PoleD%d"), I)));
		Place(Cyl, Local(FVector(0.f, 8.f, 148.f)), FRotator(0.f, Yaw + 90.f, 10.f), FVector(0.09f, 0.09f, 1.65f), Wood,
			FName(*FString::Printf(TEXT("Ridge%d"), I)));
		Place(Cyl, Local(FVector(-40.f, -10.f, 132.f)), FRotator(8.f, Yaw + 90.f, 0.f), FVector(0.07f, 0.07f, 1.2f), WoodDark,
			FName(*FString::Printf(TEXT("Rafter%d"), I)));
		if (Cube)
		{
			Place(Cube, Local(FVector(0.f, 12.f, 138.f)), FRotator(-28.f, Yaw, 0.f), FVector(2.45f, 2.2f, 0.055f), Hide,
				FName(*FString::Printf(TEXT("Roof%d"), I)));
			Place(Cube, Local(FVector(0.f, 28.f, 118.f)), FRotator(-38.f, Yaw, 4.f), FVector(2.2f, 1.6f, 0.04f), HideDark,
				FName(*FString::Printf(TEXT("RoofUnder%d"), I)));
			Place(Cube, Local(FVector(0.f, -32.f, 72.f)), FRotator(6.f, Yaw, 0.f), FVector(1.85f, 0.06f, 1.15f), Hide,
				FName(*FString::Printf(TEXT("Wall%d"), I)));
			Place(Cube, Local(FVector(-88.f, 8.f, 64.f)), FRotator(0.f, Yaw + 82.f, 8.f), FVector(1.4f, 0.05f, 1.0f), HideDark,
				FName(*FString::Printf(TEXT("WallSide%d"), I)));
			Place(Cube, Local(FVector(0.f, 10.f, 8.f)), FRotator(0.f, Yaw, 0.f), FVector(1.1f, 1.3f, 0.05f), HideDark,
				FName(*FString::Printf(TEXT("Bedding%d"), I)));
		}
		if (Plane)
		{
			Place(Plane, Local(FVector(70.f, -20.f, 90.f)), FRotator(80.f, Yaw + 20.f, 12.f), FVector(0.9f, 1.1f, 1.f), Hide,
				FName(*FString::Printf(TEXT("Flap%d"), I)));
		}
	}

	const FVector FireG = Terrain->GroundAt(FVector(Brain.FireX, Brain.FireY, 0.f));
	Place(Cyl, FireG + FVector(0.f, 0.f, 8.f), FRotator::ZeroRotator, FVector(0.95f, 0.95f, 0.06f), Charcoal ? Charcoal : Stone, TEXT("Hearth"));
	if (Sphere)
	{
		Place(Sphere, FireG + FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(0.55f, 0.5f, 0.12f), Charcoal ? Charcoal : Stone, TEXT("Ash"));
	}
	for (int32 S = 0; S < 9; ++S)
	{
		const float Ang = S * 40.f + (S % 2) * 8.f;
		const float Rad = 40.f + (S % 3) * 6.f;
		const FVector Ring = FRotator(0.f, Ang, 0.f).RotateVector(FVector(Rad, 0.f, 7.f + (S % 2) * 3.f));
		Place(Sphere ? Sphere : Cyl, FireG + Ring, FRotator(18.f + S * 7.f, Ang, S * 11.f),
			FVector(0.20f + (S % 3) * 0.04f, 0.15f, 0.11f + (S % 2) * 0.03f), Stone, FName(*FString::Printf(TEXT("Ring%d"), S)));
	}
	Place(Cyl, FireG + FVector(-10.f, 6.f, 32.f), FRotator(18.f, 20.f, 0.f), FVector(0.07f, 0.07f, 0.55f), WoodDark, TEXT("StickA"));
	Place(Cyl, FireG + FVector(12.f, -4.f, 34.f), FRotator(-16.f, 70.f, 0.f), FVector(0.065f, 0.065f, 0.58f), Wood, TEXT("StickB"));
	Place(Cyl, FireG + FVector(2.f, 12.f, 30.f), FRotator(14.f, -40.f, 8.f), FVector(0.06f, 0.06f, 0.5f), WoodDark, TEXT("StickC"));
	if (Cube)
	{
		Place(Cube, FireG + FVector(0.f, 0.f, 28.f), FRotator::ZeroRotator, FVector(0.28f, 0.28f, 0.38f), Fire, TEXT("Flame"));
		Place(Cube, FireG + FVector(4.f, -3.f, 52.f), FRotator(10.f, 25.f, 0.f), FVector(0.12f, 0.12f, 0.34f), Fire, TEXT("FlameTip"));
		Place(Cyl, FireG + FVector(108.f, 48.f, 14.f), FRotator(0.f, 22.f, 88.f), FVector(0.13f, 0.13f, 0.78f), Wood, TEXT("LogA"));
		Place(Cyl, FireG + FVector(122.f, 62.f, 20.f), FRotator(0.f, -18.f, 80.f), FVector(0.12f, 0.12f, 0.68f), WoodDark, TEXT("LogB"));
		Place(Cyl, FireG + FVector(100.f, 70.f, 12.f), FRotator(0.f, 72.f, 90.f), FVector(0.11f, 0.11f, 0.6f), Wood, TEXT("LogC"));
		Place(Cyl, FireG + FVector(-120.f, 36.f, 62.f), FRotator::ZeroRotator, FVector(0.09f, 0.09f, 1.22f), Wood, TEXT("RackL"));
		Place(Cyl, FireG + FVector(-72.f, 36.f, 62.f), FRotator::ZeroRotator, FVector(0.09f, 0.09f, 1.22f), Wood, TEXT("RackR"));
		Place(Cyl, FireG + FVector(-96.f, 36.f, 118.f), FRotator(0.f, 0.f, 90.f), FVector(0.07f, 0.07f, 0.55f), WoodDark, TEXT("RackBar"));
		Place(Cube, FireG + FVector(-96.f, 36.f, 102.f), FRotator(8.f, 6.f, 0.f), FVector(0.72f, 0.07f, 0.52f), Hide, TEXT("RackHide"));
		Place(Cube, FireG + FVector(-96.f, 48.f, 88.f), FRotator(-6.f, -8.f, 0.f), FVector(0.55f, 0.06f, 0.4f), HideDark, TEXT("RackHideB"));
		Place(Cyl, FireG + FVector(40.f, -110.f, 22.f), FRotator(0.f, 12.f, 90.f), FVector(0.18f, 0.18f, 1.05f), Wood, TEXT("Bench"));
		Place(Cyl, FireG + FVector(8.f, -118.f, 10.f), FRotator::ZeroRotator, FVector(0.1f, 0.1f, 0.18f), WoodDark, TEXT("BenchLegA"));
		Place(Cyl, FireG + FVector(72.f, -102.f, 10.f), FRotator::ZeroRotator, FVector(0.1f, 0.1f, 0.18f), WoodDark, TEXT("BenchLegB"));
		Place(Sphere ? Sphere : Cyl, FireG + FVector(48.f, -86.f, 10.f), FRotator(12.f, 30.f, 0.f), FVector(0.32f, 0.24f, 0.1f), Stone, TEXT("GrindStone"));
	}

	Place(Cyl, FireG + FVector(-40.f, 90.f, 70.f), FRotator(12.f, 8.f, 0.f), FVector(0.045f, 0.045f, 1.35f), Wood, TEXT("SpearA"));
	if (Cone)
	{
		Place(Cone, FireG + FVector(-40.f, 90.f, 142.f), FRotator(12.f, 8.f, 0.f), FVector(0.07f, 0.07f, 0.16f), Stone, TEXT("SpearTipA"));
	}
	Place(Cyl, FireG + FVector(-28.f, 98.f, 62.f), FRotator(18.f, -6.f, 4.f), FVector(0.04f, 0.04f, 1.2f), WoodDark, TEXT("SpearB"));
	if (Cone)
	{
		Place(Cone, FireG + FVector(-28.f, 98.f, 128.f), FRotator(18.f, -6.f, 4.f), FVector(0.065f, 0.065f, 0.14f), Stone, TEXT("SpearTipB"));
	}
	Place(Cyl, FireG + FVector(70.f, 88.f, 38.f), FRotator(0.f, 30.f, 18.f), FVector(0.055f, 0.055f, 0.7f), Wood, TEXT("AxeHaft"));
	Place(Sphere ? Sphere : Cyl, FireG + FVector(70.f, 88.f, 72.f), FRotator(0.f, 30.f, 0.f), FVector(0.16f, 0.10f, 0.08f), Stone, TEXT("AxeHead"));

	FireLight = NewObject<UPointLightComponent>(this, TEXT("FireLight"));
	FireLight->SetWorldLocation(FireG + FVector(0.f, 0.f, 70.f));
	FireLight->SetIntensity(4200.f);
	FireLight->SetLightColor(FLinearColor(1.f, 0.55f, 0.18f));
	FireLight->SetAttenuationRadius(1100.f);
	FireLight->SetCastShadows(true);
	FireLight->SetupAttachment(GetRootComponent());
	FireLight->RegisterComponent();
}

void AValleyWorld::SpawnPeople()
{
	// These eight adults are the entire human population on Earth this slice.
	// Do not spawn extra tribes, camps, or background people.
	const int32 Humans = FMath::Min(Brain.VillagerCount, vg::kEarthHumans);
	for (int32 I = 0; I < Humans; ++I)
	{
		AValleyVillager* V = GetWorld()->SpawnActor<AValleyVillager>();
		V->Arm(Brain.Villagers[I]);
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
	UMaterialInterface* Dust = Valley::Material(TEXT("M_Charcoal"));
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
			L->SetIntensity(bNight ? 0.45f : FMath::Lerp(13.5f, 6.5f, Gold));
			L->SetLightColor(bNight ? FLinearColor(0.28f, 0.36f, 0.58f) : FMath::Lerp(FLinearColor(1.f, 0.96f, 0.86f), FLinearColor(1.f, 0.58f, 0.3f), Gold));
		}
	}
	if (Sky)
	{
		if (USkyLightComponent* S = Sky->GetLightComponent())
		{
			S->SetIntensity(vg::IsNight(Hours) ? 0.22f : 0.9f);
		}
	}
	if (Fog)
	{
		if (UExponentialHeightFogComponent* F = Fog->GetComponent())
		{
			const bool bStorm = Brain.Sky != vg::Weather::Clear;
			F->SetFogDensity(vg::IsNight(Hours) ? 0.022f : (bStorm ? 0.028f : 0.012f));
		}
	}
	if (Post)
	{
		const bool bNight = vg::IsNight(Hours);
		Post->Settings.AutoExposureBias = bNight ? 0.05f : 0.4f;
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
	const int32 TreeCount = FMath::Min(Trees.Num(), TreeYaw.Num());
	for (int32 I = 0; I < TreeCount; ++I)
	{
		if (UStaticMeshComponent* Trunk = Trees[I])
		{
			Trunk->SetWorldRotation(FRotator(Lean, TreeYaw[I], 0.f));
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
