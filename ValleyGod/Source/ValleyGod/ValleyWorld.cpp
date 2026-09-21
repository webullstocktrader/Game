#include "ValleyWorld.h"
#include "ValleyTerrain.h"
#include "ValleyVillager.h"
#include "ValleyAnimal.h"
#include "ValleyTypes.h"
#include "ValleyAssets.h"
#include "Sim/ValleyLookPaths.h"
#include "Engine/StaticMesh.h"
#include "Components/TextRenderComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
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
	const Valley::FOptionalAssets Assets = Valley::DiscoverOptionalAssets();
	MaraMetaHumanClass = Assets.MaraClass;
	bQuixelGround = Assets.Dirt != nullptr || Assets.Grass != nullptr;
	bQuixelFoliage = Assets.Trees.Num() > 0 || Assets.GrassMeshes.Num() > 0 || Assets.Rocks.Num() > 0;
	Terrain = GetWorld()->SpawnActor<AValleyTerrain>(FVector::ZeroVector, FRotator::ZeroRotator);
	Terrain->BuildValley();
	Terrain->ApplyGroundMaterials(Assets.Dirt, Assets.Grass, Assets.WetDirt);
	SpawnAtmosphere();
	SpawnMiniatureEarth();
	SpawnTribeMarks();
	SpawnTreesAndRocks(Assets);
	SpawnSheltersAndFire();
	SpawnPeople();
	EnsureWorkVisuals();
	bMaraMetaHuman = false;
	if (AValleyVillager* Mara = FindVillager(vg::kMetaHumanMilestoneSlot))
	{
		bMaraMetaHuman = Mara->IsUsingMetaHuman();
	}
	SpawnRain();
	SpawnTornado();
	UE_LOG(LogTemp, Display, TEXT("Valley God: %s"), *GraphicsStatusLine());
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

int32 AValleyWorld::CycleContinent()
{
	if (Brain.ContinentCount <= 0)
	{
		return 0;
	}
	FocusedContinent = (FocusedContinent + 1) % Brain.ContinentCount;
	return FocusedContinent;
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
	Comp->SetCastShadow(true);
	Comp->bAffectDistanceFieldLighting = true;
	if (Mat)
	{
		Comp->SetMaterial(0, Mat);
	}
	Comp->SetupAttachment(GetRootComponent());
	Comp->RegisterComponent();
	return Comp;
}

UStaticMeshComponent* AValleyWorld::PlaceSized(UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, float TargetHeightCm, const FName& Name)
{
	if (!Mesh)
	{
		return nullptr;
	}
	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const float Height = FMath::Max(Bounds.BoxExtent.Z * 2.f, 1.f);
	const float S = TargetHeightCm / Height;
	const float BottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
	return Place(Mesh, Loc - FVector(0.f, 0.f, BottomZ * S), Rot, FVector(S), nullptr, Name);
}

UHierarchicalInstancedStaticMeshComponent* AValleyWorld::FoliagePool(UStaticMesh* Mesh, const FName& Name)
{
	if (!Mesh)
	{
		return nullptr;
	}
	for (UHierarchicalInstancedStaticMeshComponent* Existing : FoliagePools)
	{
		if (Existing && Existing->GetStaticMesh() == Mesh)
		{
			return Existing;
		}
	}
	UHierarchicalInstancedStaticMeshComponent* Pool = NewObject<UHierarchicalInstancedStaticMeshComponent>(this, Name);
	Pool->SetStaticMesh(Mesh);
	Pool->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Pool->SetCastShadow(true);
	Pool->bAffectDistanceFieldLighting = true;
	Pool->InstanceStartCullDistance = 7000.f;
	Pool->InstanceEndCullDistance = 16000.f;
	Pool->SetupAttachment(GetRootComponent());
	Pool->RegisterComponent();
	FoliagePools.Add(Pool);
	return Pool;
}

void AValleyWorld::AddSizedInstance(UHierarchicalInstancedStaticMeshComponent* Pool, const FVector& Loc, const FRotator& Rot, float TargetHeightCm)
{
	if (!Pool || !Pool->GetStaticMesh())
	{
		return;
	}
	const FBoxSphereBounds Bounds = Pool->GetStaticMesh()->GetBounds();
	const float Height = FMath::Max(Bounds.BoxExtent.Z * 2.f, 1.f);
	const float S = TargetHeightCm / Height;
	const float BottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;
	const FTransform Xform(Rot, Loc - FVector(0.f, 0.f, BottomZ * S), FVector(S));
	Pool->AddInstance(Xform, true);
}

FString AValleyWorld::GraphicsStatusLine() const
{
	return FString::Printf(TEXT("Look  Mara %s  ·  ground %s  ·  foliage %s"),
		bMaraMetaHuman ? TEXT("MetaHuman") : TEXT("procedural"),
		bQuixelGround ? TEXT("Quixel") : TEXT("procedural"),
		bQuixelFoliage ? TEXT("Quixel") : TEXT("procedural"));
}

void AValleyWorld::SpawnAtmosphere()
{
	Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-42.f, 200.f, 0.f));
	if (UDirectionalLightComponent* L = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
	{
		L->SetIntensity(16.f);
		L->SetLightColor(FLinearColor(1.f, 0.94f, 0.82f));
		L->SetAtmosphereSunLight(true);
		L->SetDynamicShadowDistanceMovableLight(50000.f);
		L->SetUseTemperature(true);
		L->SetTemperature(5300.f);
		L->bEnableLightShaftBloom = true;
		L->LightShaftBloomScale = 0.35f;
		L->ContactShadowLength = 0.12f;
	}

	Sky = GetWorld()->SpawnActor<ASkyLight>();
	if (USkyLightComponent* S = Sky->GetLightComponent())
	{
		S->SetIntensity(1.05f);
		S->bRealTimeCapture = true;
		S->SetLightColor(FLinearColor(0.78f, 0.86f, 0.95f));
		S->OcclusionMaxDistance = 1200.f;
		S->OcclusionExponent = 1.4f;
	}

	GetWorld()->SpawnActor<ASkyAtmosphere>();
	GetWorld()->SpawnActor<AVolumetricCloud>();

	Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(FVector(0.f, 0.f, 180.f), FRotator::ZeroRotator);
	if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
	{
		FogComp->SetFogDensity(0.018f);
		FogComp->FogHeightFalloff = 0.14f;
		FogComp->SetVolumetricFog(true);
		FogComp->VolumetricFogScatteringDistribution = 0.72f;
		FogComp->VolumetricFogExtinctionScale = 0.85f;
		FogComp->VolumetricFogAlbedo = FColor(186, 194, 188);
		FogComp->SetFogInscatteringColor(FLinearColor(0.58f, 0.64f, 0.60f));
	}

	Post = GetWorld()->SpawnActor<APostProcessVolume>();
	Post->bUnbound = true;
	FPostProcessSettings& P = Post->Settings;
	P.bOverride_AutoExposureMethod = true;
	P.AutoExposureMethod = AEM_Histogram;
	P.bOverride_AutoExposureBias = true;
	P.AutoExposureBias = 0.28f;
	P.bOverride_AutoExposureMinBrightness = true;
	P.AutoExposureMinBrightness = -1.6f;
	P.bOverride_AutoExposureMaxBrightness = true;
	P.AutoExposureMaxBrightness = 1.6f;
	P.bOverride_ColorSaturation = true;
	P.ColorSaturation = FVector4(0.96f, 0.94f, 0.88f, 1.f);
	P.bOverride_ColorContrast = true;
	P.ColorContrast = FVector4(1.12f, 1.08f, 1.04f, 1.f);
	P.bOverride_ColorGamma = true;
	P.ColorGamma = FVector4(1.02f, 1.0f, 0.96f, 1.f);
	P.bOverride_VignetteIntensity = true;
	P.VignetteIntensity = 0.28f;
	P.bOverride_BloomIntensity = true;
	P.BloomIntensity = 0.52f;
	P.bOverride_AmbientOcclusionIntensity = true;
	P.AmbientOcclusionIntensity = 0.7f;
	P.bOverride_AmbientOcclusionRadius = true;
	P.AmbientOcclusionRadius = 52.f;
	P.bOverride_IndirectLightingColor = true;
	P.IndirectLightingColor = FLinearColor(1.02f, 0.98f, 0.92f);
	P.bOverride_MotionBlurAmount = true;
	P.MotionBlurAmount = 0.22f;
	P.bOverride_AmbientCubemapIntensity = false;
}

void AValleyWorld::SpawnTreesAndRocks(const Valley::FOptionalAssets& Assets)
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Sphere = Valley::SphereMesh();
	UStaticMesh* Cone = Valley::ConeMesh();
	UMaterialInterface* Bark = Valley::Material(TEXT("M_Bark"));
	UMaterialInterface* Leaf = Valley::Material(TEXT("M_Foliage"));
	UMaterialInterface* LeafDark = Valley::Material(TEXT("M_FoliageDark"));
	UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone"));
	if (!Terrain)
	{
		return;
	}

	FRandomStream Rng(19);
	auto InClearing = [](float X, float Y)
	{
		return FVector2D::Distance(FVector2D(X, Y), FVector2D(0.f, 700.f)) < 700.f
			|| (FMath::Abs(X) < 500.f && FMath::Abs(Y) < 400.f);
	};

	auto ScatterUntil = [&](int32 Want, int32 MaxAttempts, auto&& PlaceOne)
	{
		int32 Placed = 0;
		for (int32 Attempt = 0; Attempt < MaxAttempts && Placed < Want; ++Attempt)
		{
			if (PlaceOne(Attempt, Placed))
			{
				++Placed;
			}
		}
		return Placed;
	};

	int32 TreeN = 0;
	if (Assets.Trees.Num() > 0)
	{
		const int32 Want = vg::PreferredTreeScatterCount();
		ScatterUntil(Want, Want * 4, [&](int32 Attempt, int32 /*Placed*/)
		{
			const float X = Rng.FRandRange(-5200.f, 5200.f);
			const float Y = Rng.FRandRange(-5200.f, 5200.f);
			if (InClearing(X, Y))
			{
				return false;
			}
			UStaticMesh* Mesh = Assets.Trees[Attempt % Assets.Trees.Num()];
			const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
			const float Yaw = Rng.FRandRange(0.f, 360.f);
			if (UStaticMeshComponent* Placed = PlaceSized(Mesh, G, FRotator(0.f, Yaw, 0.f),
					Rng.FRandRange(380.f, 720.f), FName(*FString::Printf(TEXT("QTree%d"), TreeN))))
			{
				Trees.Add(Placed);
				TreeBaseYaw.Add(Yaw);
				++TreeN;
				return true;
			}
			return false;
		});
	}
	else if (Cyl && Sphere)
	{
		const int32 Want = 56;
		ScatterUntil(Want, Want * 4, [&](int32 /*Attempt*/, int32 /*Placed*/)
		{
			const float X = Rng.FRandRange(-5200.f, 5200.f);
			const float Y = Rng.FRandRange(-5200.f, 5200.f);
			if (InClearing(X, Y))
			{
				return false;
			}
			const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
			const float H = Rng.FRandRange(2.8f, 5.2f);
			const float TrunkR = Rng.FRandRange(0.42f, 0.7f);
			UStaticMeshComponent* Trunk = Place(Cyl, G + FVector(0.f, 0.f, H * 48.f), FRotator::ZeroRotator, FVector(TrunkR, TrunkR, H), Bark,
				FName(*FString::Printf(TEXT("Trunk%d"), TreeN)));
			Trees.Add(Trunk);
			TreeBaseYaw.Add(18.f);

			auto LeafOn = [&](const FName& Name, const FVector& Rel, const FVector& Scale, UMaterialInterface* Mat)
			{
				UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
				Comp->SetStaticMesh(Sphere);
				Comp->SetRelativeLocation(Rel);
				Comp->SetRelativeScale3D(Scale);
				Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				if (Mat)
				{
					Comp->SetMaterial(0, Mat);
				}
				Comp->SetupAttachment(Trunk);
				Comp->RegisterComponent();
			};

			LeafOn(FName(*FString::Printf(TEXT("CanopyA%d"), TreeN)), FVector(0.f, 0.f, 48.f), FVector(2.2f, 2.1f, 1.5f) / FVector(TrunkR, TrunkR, H) * Rng.FRandRange(0.9f, 1.15f), Leaf);
			LeafOn(FName(*FString::Printf(TEXT("CanopyB%d"), TreeN)), FVector(Rng.FRandRange(-18.f, 18.f), Rng.FRandRange(-18.f, 18.f), 68.f), FVector(1.6f, 1.7f, 1.1f) / FVector(TrunkR, TrunkR, H), LeafDark);
			LeafOn(FName(*FString::Printf(TEXT("CanopyC%d"), TreeN)), FVector(Rng.FRandRange(-12.f, 12.f), Rng.FRandRange(-12.f, 12.f), 36.f), FVector(1.4f, 1.5f, 1.0f) / FVector(TrunkR, TrunkR, H), Leaf);
			Place(Sphere, G + FVector(0.f, 0.f, 18.f), FRotator::ZeroRotator, FVector(TrunkR * 1.8f, TrunkR * 1.8f, 0.35f), Bark,
				FName(*FString::Printf(TEXT("Root%d"), TreeN)));
			if (Cone)
			{
				Place(Cyl, G + FVector(Rng.FRandRange(-20.f, 20.f), Rng.FRandRange(-20.f, 20.f), H * 70.f), FRotator(Rng.FRandRange(20.f, 55.f), Rng.FRandRange(0.f, 180.f), 0.f),
					FVector(0.12f, 0.12f, H * 0.35f), Bark, FName(*FString::Printf(TEXT("Branch%d"), TreeN)));
			}
			++TreeN;
			return true;
		});
	}

	if (Assets.Trees.Num() > 0)
	{
		int32 SaplingN = 0;
		ScatterUntil(80, 240, [&](int32 Attempt, int32 /*Placed*/)
		{
			const float X = Rng.FRandRange(-4800.f, 4800.f);
			const float Y = Rng.FRandRange(-4800.f, 4800.f);
			if (InClearing(X, Y))
			{
				return false;
			}
			UStaticMesh* Mesh = Assets.Trees[Attempt % Assets.Trees.Num()];
			if (UHierarchicalInstancedStaticMeshComponent* Pool = FoliagePool(Mesh, FName(*FString::Printf(TEXT("SaplingPool%d"), Attempt % Assets.Trees.Num()))))
			{
				const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
				AddSizedInstance(Pool, G, FRotator(0.f, Rng.FRandRange(0.f, 360.f), 0.f), Rng.FRandRange(90.f, 220.f));
				++SaplingN;
				return true;
			}
			return false;
		});
	}

	if (Assets.GrassMeshes.Num() > 0)
	{
		const int32 Want = vg::PreferredGrassScatterCount();
		ScatterUntil(Want, Want * 3, [&](int32 Attempt, int32 /*Placed*/)
		{
			const float X = Rng.FRandRange(-4200.f, 4200.f);
			const float Y = Rng.FRandRange(-2200.f, 4200.f);
			if (InClearing(X, Y))
			{
				return false;
			}
			UStaticMesh* Mesh = Assets.GrassMeshes[Attempt % Assets.GrassMeshes.Num()];
			if (UHierarchicalInstancedStaticMeshComponent* Pool = FoliagePool(Mesh, FName(*FString::Printf(TEXT("GrassPool%d"), Attempt % Assets.GrassMeshes.Num()))))
			{
				const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
				AddSizedInstance(Pool, G, FRotator(0.f, Rng.FRandRange(0.f, 360.f), 0.f), Rng.FRandRange(40.f, 120.f));
				return true;
			}
			return false;
		});
	}
	else if (Cone)
	{
		const int32 Want = vg::ProceduralGrassTuftCount();
		if (UHierarchicalInstancedStaticMeshComponent* Pool = FoliagePool(Cone, TEXT("TuftPool")))
		{
			if (Leaf)
			{
				Pool->SetMaterial(0, Leaf);
			}
			ScatterUntil(Want, Want * 3, [&](int32 /*Attempt*/, int32 /*Placed*/)
			{
				const float X = Rng.FRandRange(-4200.f, 4200.f);
				const float Y = Rng.FRandRange(-2200.f, 4200.f);
				if (InClearing(X, Y))
				{
					return false;
				}
				const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
				const float S = Rng.FRandRange(28.f, 70.f);
				AddSizedInstance(Pool, G, FRotator(0.f, Rng.FRandRange(0.f, 180.f), 0.f), S);
				return true;
			});
		}
	}

	if (Assets.Rocks.Num() > 0)
	{
		const int32 Want = vg::PreferredRockScatterCount();
		ScatterUntil(Want, Want * 3, [&](int32 Attempt, int32 /*Placed*/)
		{
			const float X = Rng.FRandRange(-4000.f, 4000.f);
			const float Y = Rng.FRandRange(-2000.f, 2200.f);
			UStaticMesh* Mesh = Assets.Rocks[Attempt % Assets.Rocks.Num()];
			if (UHierarchicalInstancedStaticMeshComponent* Pool = FoliagePool(Mesh, FName(*FString::Printf(TEXT("RockPool%d"), Attempt % Assets.Rocks.Num()))))
			{
				const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
				AddSizedInstance(Pool, G, FRotator(Rng.FRandRange(0.f, 25.f), Rng.FRandRange(0.f, 180.f), 0.f), Rng.FRandRange(35.f, 140.f));
				return true;
			}
			return false;
		});
	}
	else if (Sphere)
	{
		for (int32 I = 0; I < 36; ++I)
		{
			const float X = Rng.FRandRange(-4000.f, 4000.f);
			const float Y = Rng.FRandRange(-2000.f, 2200.f);
			const FVector G = Terrain->GroundAt(FVector(X, Y, 0.f));
			Place(Sphere, G + FVector(0.f, 0.f, 18.f), FRotator(Rng.FRandRange(0.f, 40.f), Rng.FRandRange(0.f, 180.f), 0.f),
				FVector(Rng.FRandRange(0.4f, 1.1f), Rng.FRandRange(0.3f, 0.8f), Rng.FRandRange(0.25f, 0.5f)), Stone,
				FName(*FString::Printf(TEXT("Rock%d"), I)));
		}
	}
}

void AValleyWorld::SpawnMiniatureEarth()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UMaterialInterface* Water = Valley::Material(TEXT("M_Water"));
	UMaterialInterface* Grass = Valley::Material(TEXT("M_Grass"));
	UMaterialInterface* Dirt = Valley::Material(TEXT("M_Dirt"));
	if (!Cyl)
	{
		return;
	}

	// Engine cylinder: 50cm radius, 100cm tall. Ocean sits under the sculpted valley.
	const float OceanRadius = 23000.f;
	Place(Cyl, FVector(0.f, 0.f, -140.f), FRotator::ZeroRotator,
		FVector(OceanRadius / 50.f, OceanRadius / 50.f, 0.4f), Water, TEXT("MiniatureOcean"));

	for (int32 I = 1; I < Brain.ContinentCount; ++I)
	{
		const vg::Continent& Land = Brain.Continents[I];
		const float Height = 64.f;
		const float CenterZ = AValleyTerrain::OffMeshStandZ - Height * 0.5f;
		UMaterialInterface* Mat = (I % 2 == 0) ? Dirt : Grass;
		Place(Cyl, FVector(Land.X, Land.Y, CenterZ), FRotator::ZeroRotator,
			FVector(Land.Radius / 50.f, Land.Radius / 50.f, Height / 100.f), Mat,
			FName(*FString::Printf(TEXT("Continent%d"), I)));
	}
}

void AValleyWorld::SpawnTribeMarks()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UMaterialInterface* Cloth = Valley::Material(TEXT("M_ClothOchre"));
	if (!Cyl)
	{
		return;
	}

	const FLinearColor Colors[8] = {
		FLinearColor(0.75f, 0.38f, 0.12f),
		FLinearColor(0.62f, 0.16f, 0.12f),
		FLinearColor(0.78f, 0.72f, 0.48f),
		FLinearColor(0.16f, 0.32f, 0.14f),
		FLinearColor(0.48f, 0.46f, 0.40f),
		FLinearColor(0.18f, 0.36f, 0.52f),
		FLinearColor(0.62f, 0.68f, 0.72f),
		FLinearColor(0.40f, 0.30f, 0.22f)
	};

	for (int32 T = 0; T < Brain.TribeCount && T < 8; ++T)
	{
		const vg::Tribe& Tribe = Brain.Tribes[T];
		const FVector G = Terrain ? Terrain->StandAt(Tribe.CampX, Tribe.CampY)
								  : FVector(Tribe.CampX, Tribe.CampY, AValleyTerrain::OffMeshStandZ);
		UMaterialInterface* Mat = Valley::Tint(this, Cloth, Colors[T], FName(*FString::Printf(TEXT("CampTint%d"), T)));
		// Flat disc so the camp reads from the overhead view. Radius 50 * 32 = 16m.
		Place(Cyl, G + FVector(0.f, 0.f, 30.f), FRotator::ZeroRotator, FVector(32.f, 32.f, 0.06f), Mat,
			FName(*FString::Printf(TEXT("CampDisc%d"), T)));

		UTextRenderComponent* Label = NewObject<UTextRenderComponent>(this, FName(*FString::Printf(TEXT("LandLabel%d"), T)));
		const FString Title = FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(Tribe.Name ? Tribe.Name : ""));
		Label->SetText(FText::FromString(Title));
		Label->SetWorldSize(1100.f);
		Label->SetTextRenderColor(FColor(255, 236, 200));
		Label->SetHorizontalAlignment(EHTA_Center);
		Label->SetVerticalAlignment(EVRTA_TextCenter);
		Label->SetWorldLocation(G + FVector(0.f, 0.f, 220.f));
		Label->SetWorldRotation(FRotator(-90.f, 0.f, 0.f));
		Label->SetCastShadow(false);
		Label->SetupAttachment(GetRootComponent());
		Label->RegisterComponent();
	}
}

void AValleyWorld::PlaceShelter(int32 Index)
{
	if (Index < 0 || Index >= Brain.ShelterCount)
	{
		return;
	}
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Cube = Valley::CubeMesh();
	UMaterialInterface* Wood = Valley::Material(TEXT("M_Wood"));
	UMaterialInterface* Hide = Valley::Material(TEXT("M_Hide"));
	if (!Cyl)
	{
		return;
	}
	const FVector G = Terrain ? Terrain->StandAt(Brain.ShelterX[Index], Brain.ShelterY[Index])
							  : FVector(Brain.ShelterX[Index], Brain.ShelterY[Index], AValleyTerrain::OffMeshStandZ);
	Place(Cyl, G + FVector(-70.f, -40.f, 80.f), FRotator::ZeroRotator, FVector(0.14f, 0.14f, 1.55f), Wood, FName(*FString::Printf(TEXT("PoleA%d"), Index)));
	Place(Cyl, G + FVector(70.f, -40.f, 80.f), FRotator::ZeroRotator, FVector(0.14f, 0.14f, 1.55f), Wood, FName(*FString::Printf(TEXT("PoleB%d"), Index)));
	Place(Cyl, G + FVector(0.f, 55.f, 50.f), FRotator(0.f, 0.f, 55.f), FVector(0.12f, 0.12f, 1.7f), Wood, FName(*FString::Printf(TEXT("PoleC%d"), Index)));
	Place(Cyl, G + FVector(-40.f, 20.f, 110.f), FRotator(0.f, 90.f, 18.f), FVector(0.08f, 0.08f, 1.4f), Wood, FName(*FString::Printf(TEXT("Ridge%d"), Index)));
	if (Cube)
	{
		Place(Cube, G + FVector(0.f, 8.f, 128.f), FRotator(-30.f, 0.f, 0.f), FVector(2.3f, 2.1f, 0.07f), Hide, FName(*FString::Printf(TEXT("Roof%d"), Index)));
		Place(Cube, G + FVector(0.f, -20.f, 70.f), FRotator(0.f, 0.f, 8.f), FVector(1.6f, 0.08f, 1.1f), Hide, FName(*FString::Printf(TEXT("Wall%d"), Index)));
	}
}

void AValleyWorld::SpawnSheltersAndFire()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Cube = Valley::CubeMesh();
	UStaticMesh* Sphere = Valley::SphereMesh();
	UMaterialInterface* Wood = Valley::Material(TEXT("M_Wood"));
	UMaterialInterface* Hide = Valley::Material(TEXT("M_Hide"));
	UMaterialInterface* Fire = Valley::Material(TEXT("M_Fire"));
	UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone"));
	if (!Cyl)
	{
		return;
	}

	for (int32 I = 0; I < Brain.ShelterCount; ++I)
	{
		PlaceShelter(I);
	}
	SpawnedShelters = Brain.ShelterCount;

	const FVector FireG = Terrain ? Terrain->StandAt(Brain.FireX, Brain.FireY) : FVector(Brain.FireX, Brain.FireY, 0.f);
	Place(Cyl, FireG + FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator, FVector(0.85f, 0.85f, 0.08f), Stone, TEXT("Hearth"));
	for (int32 S = 0; S < 7; ++S)
	{
		const float Ang = S * 51.4f;
		const FVector Ring = FRotator(0.f, Ang, 0.f).RotateVector(FVector(42.f, 0.f, 8.f));
		Place(Sphere ? Sphere : Cyl, FireG + Ring, FRotator(20.f, Ang, 0.f), FVector(0.18f, 0.14f, 0.12f), Stone, FName(*FString::Printf(TEXT("Ring%d"), S)));
	}
	if (Cube)
	{
		Place(Cube, FireG + FVector(0.f, 0.f, 28.f), FRotator::ZeroRotator, FVector(0.32f, 0.32f, 0.42f), Fire, TEXT("Flame"));
		Place(Cube, FireG + FVector(6.f, -5.f, 50.f), FRotator(12.f, 20.f, 0.f), FVector(0.14f, 0.14f, 0.38f), Fire, TEXT("FlameTip"));
		Place(Cyl, FireG + FVector(90.f, 40.f, 12.f), FRotator(0.f, 20.f, 90.f), FVector(0.12f, 0.12f, 0.7f), Wood, TEXT("LogA"));
		Place(Cyl, FireG + FVector(105.f, 55.f, 18.f), FRotator(0.f, -15.f, 82.f), FVector(0.11f, 0.11f, 0.62f), Wood, TEXT("LogB"));
		Place(Cyl, FireG + FVector(88.f, 62.f, 10.f), FRotator(0.f, 70.f, 90.f), FVector(0.1f, 0.1f, 0.55f), Wood, TEXT("LogC"));
		Place(Cyl, FireG + FVector(-110.f, 30.f, 55.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 1.1f), Wood, TEXT("RackL"));
		Place(Cyl, FireG + FVector(-70.f, 30.f, 55.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 1.1f), Wood, TEXT("RackR"));
		Place(Cube, FireG + FVector(-90.f, 30.f, 100.f), FRotator::ZeroRotator, FVector(0.7f, 0.08f, 0.55f), Hide, TEXT("RackHide"));
		Place(Sphere ? Sphere : Cyl, FireG + FVector(40.f, -80.f, 10.f), FRotator(12.f, 30.f, 0.f), FVector(0.28f, 0.22f, 0.1f), Stone, TEXT("GrindStone"));
		Place(Cyl, FireG + FVector(-40.f, -95.f, 14.f), FRotator(0.f, 35.f, 90.f), FVector(0.16f, 0.16f, 0.22f), Wood, TEXT("SeatLog"));
		Place(Cyl, FireG + FVector(70.f, -110.f, 12.f), FRotator(0.f, -20.f, 88.f), FVector(0.14f, 0.14f, 0.2f), Wood, TEXT("SeatLogB"));
		Place(Sphere ? Sphere : Cyl, FireG + FVector(-55.f, 70.f, 10.f), FRotator::ZeroRotator, FVector(0.16f, 0.14f, 0.08f), Hide, TEXT("WaterSkin"));
		Place(Sphere ? Sphere : Cyl, FireG + FVector(-48.f, 82.f, 8.f), FRotator::ZeroRotator, FVector(0.12f, 0.1f, 0.06f), Hide, TEXT("WaterSkinB"));
		Place(Cube, FireG + FVector(130.f, 20.f, 16.f), FRotator(8.f, 25.f, 0.f), FVector(0.38f, 0.22f, 0.08f), Hide, TEXT("HidePile"));
		Place(Cube, FireG + FVector(148.f, 8.f, 12.f), FRotator(-6.f, 70.f, 4.f), FVector(0.32f, 0.2f, 0.06f), Hide, TEXT("HidePileB"));
		Place(Cyl, FireG + FVector(20.f, 110.f, 8.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 0.05f), Stone, TEXT("Bowl"));
		Place(Sphere ? Sphere : Cyl, FireG + FVector(20.f, 110.f, 14.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 0.05f), Stone, TEXT("BowlLip"));
	}

	FireLight = NewObject<UPointLightComponent>(this, TEXT("FireLight"));
	FireLight->SetWorldLocation(FireG + FVector(0.f, 0.f, 70.f));
	FireLight->SetIntensity(4200.f);
	FireLight->SetLightColor(FLinearColor(1.f, 0.55f, 0.18f));
	FireLight->SetAttenuationRadius(1100.f);
	FireLight->SetCastShadows(true);
	FireLight->SetupAttachment(GetRootComponent());
	FireLight->RegisterComponent();

	for (int32 T = 1; T < Brain.TribeCount; ++T)
	{
		const vg::Tribe& Tribe = Brain.Tribes[T];
		const FVector G = Terrain ? Terrain->StandAt(Tribe.FireX, Tribe.FireY) : FVector(Tribe.FireX, Tribe.FireY, AValleyTerrain::OffMeshStandZ);
		Place(Cyl, G + FVector(0.f, 0.f, 8.f), FRotator::ZeroRotator, FVector(0.7f, 0.7f, 0.06f), Stone, FName(*FString::Printf(TEXT("Hearth%d"), T)));
		if (Cube)
		{
			Place(Cube, G + FVector(0.f, 0.f, 28.f), FRotator::ZeroRotator, FVector(0.28f, 0.28f, 0.36f), Fire, FName(*FString::Printf(TEXT("Flame%d"), T)));
		}
	}
}

void AValleyWorld::EnsureWorkVisuals()
{
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Cube = Valley::CubeMesh();
	UStaticMesh* Sphere = Valley::SphereMesh();
	UMaterialInterface* Wood = Valley::Material(TEXT("M_Wood"));
	UMaterialInterface* Foliage = Valley::Material(TEXT("M_Foliage"));
	UMaterialInterface* Stone = Valley::Material(TEXT("M_Stone"));
	UMaterialInterface* Hide = Valley::Material(TEXT("M_Hide"));
	if (!Cyl)
	{
		return;
	}

	// Kit pieces hang off the stage root so hiding that stage hides the whole module.
	// These are log, thatch, and stone parts. A finished building is not its own imported mesh.
	auto AddPiece = [&](UStaticMeshComponent* Parent, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat, const FName& Name)
	{
		if (!Parent || !Mesh)
		{
			return;
		}
		if (UStaticMeshComponent* Piece = Place(Mesh, Loc, Rot, Scale, Mat, Name))
		{
			Piece->AttachToComponent(Parent, FAttachmentTransformRules::KeepWorldTransform);
		}
	};

	while (HarvestTrunks.Num() < Brain.TreeCount)
	{
		const int32 I = HarvestTrunks.Num();
		const vg::Timber& Tree = Brain.Trees[I];
		const FVector G = Terrain ? Terrain->StandAt(Tree.X, Tree.Y) : FVector(Tree.X, Tree.Y, AValleyTerrain::OffMeshStandZ);
		UStaticMeshComponent* Trunk = Place(Cyl, G + FVector(0.f, 0.f, 90.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 1.8f), Wood,
			FName(*FString::Printf(TEXT("HarvestTrunk%d"), I)));
		UStaticMeshComponent* Crown = Place(Sphere ? Sphere : Cyl, G + FVector(0.f, 0.f, 230.f), FRotator::ZeroRotator, FVector(1.15f, 1.15f, 0.9f), Foliage,
			FName(*FString::Printf(TEXT("HarvestCrown%d"), I)));
		HarvestTrunks.Add(Trunk);
		HarvestCrowns.Add(Crown);
	}
	for (int32 I = 0; I < HarvestTrunks.Num() && I < Brain.TreeCount; ++I)
	{
		if (Brain.Trees[I].Standing)
		{
			continue;
		}
		if (HarvestCrowns.IsValidIndex(I) && HarvestCrowns[I])
		{
			HarvestCrowns[I]->DestroyComponent();
			HarvestCrowns[I] = nullptr;
		}
		if (HarvestTrunks[I])
		{
			HarvestTrunks[I]->DestroyComponent();
			HarvestTrunks[I] = nullptr;
		}
	}

	while (SitePads.Num() < Brain.SiteCount)
	{
		const int32 I = SitePads.Num();
		const vg::WorkSite& Site = Brain.Sites[I];
		const FVector G = Terrain ? Terrain->StandAt(Site.X, Site.Y) : FVector(Site.X, Site.Y, AValleyTerrain::OffMeshStandZ);
		UStaticMeshComponent* Pad = Place(Cube ? Cube : Cyl, G + FVector(0.f, 0.f, 8.f), FRotator::ZeroRotator, FVector(2.6f, 2.4f, 0.1f), Stone,
			FName(*FString::Printf(TEXT("SitePad%d"), I)));
		const FVector Corners[4] = {FVector(-90.f, -80.f, 16.f), FVector(90.f, -80.f, 16.f), FVector(-90.f, 80.f, 16.f), FVector(90.f, 80.f, 16.f)};
		for (int32 C = 0; C < 4; ++C)
		{
			AddPiece(Pad, Cube ? Cube : Cyl, G + Corners[C], FRotator::ZeroRotator, FVector(0.28f, 0.28f, 0.16f), Stone,
				FName(*FString::Printf(TEXT("SiteStone%d_%d"), I, C)));
		}
		UStaticMeshComponent* Frame = Place(Cyl, G + Corners[0] + FVector(0.f, 0.f, 70.f), FRotator::ZeroRotator, FVector(0.14f, 0.14f, 1.5f), Wood,
			FName(*FString::Printf(TEXT("SiteFrame%d"), I)));
		for (int32 C = 1; C < 4; ++C)
		{
			AddPiece(Frame, Cyl, G + Corners[C] + FVector(0.f, 0.f, 70.f), FRotator::ZeroRotator, FVector(0.14f, 0.14f, 1.5f), Wood,
				FName(*FString::Printf(TEXT("SiteLog%d_%d"), I, C)));
		}
		AddPiece(Frame, Cyl, G + FVector(0.f, -80.f, 145.f), FRotator(0.f, 90.f, 90.f), FVector(0.1f, 0.1f, 1.9f), Wood,
			FName(*FString::Printf(TEXT("SiteBeamA%d"), I)));
		AddPiece(Frame, Cyl, G + FVector(0.f, 80.f, 145.f), FRotator(0.f, 90.f, 90.f), FVector(0.1f, 0.1f, 1.9f), Wood,
			FName(*FString::Printf(TEXT("SiteBeamB%d"), I)));
		UStaticMeshComponent* Wall = Place(Cube ? Cube : Cyl, G + FVector(0.f, -80.f, 78.f), FRotator::ZeroRotator, FVector(1.7f, 0.08f, 1.15f), Hide,
			FName(*FString::Printf(TEXT("SiteWall%d"), I)));
		AddPiece(Wall, Cube ? Cube : Cyl, G + FVector(0.f, 80.f, 78.f), FRotator::ZeroRotator, FVector(1.7f, 0.08f, 1.15f), Hide,
			FName(*FString::Printf(TEXT("SiteWallB%d"), I)));
		AddPiece(Wall, Cube ? Cube : Cyl, G + FVector(-90.f, 0.f, 78.f), FRotator(0.f, 90.f, 0.f), FVector(1.5f, 0.08f, 1.15f), Hide,
			FName(*FString::Printf(TEXT("SiteWallC%d"), I)));
		UStaticMeshComponent* Roof = Place(Cube ? Cube : Cyl, G + FVector(0.f, -40.f, 168.f), FRotator(-22.f, 0.f, 0.f), FVector(2.9f, 1.35f, 0.08f), Hide,
			FName(*FString::Printf(TEXT("SiteRoof%d"), I)));
		AddPiece(Roof, Cube ? Cube : Cyl, G + FVector(0.f, 40.f, 168.f), FRotator(22.f, 0.f, 0.f), FVector(2.9f, 1.35f, 0.08f), Hide,
			FName(*FString::Printf(TEXT("SiteRoofB%d"), I)));
		SitePads.Add(Pad);
		SiteFrames.Add(Frame);
		SiteWalls.Add(Wall);
		SiteRoofs.Add(Roof);
	}
	for (int32 I = 0; I < SitePads.Num() && I < Brain.SiteCount; ++I)
	{
		const vg::WorkSite& Site = Brain.Sites[I];
		const int32 Stage = Site.Stage;
		if (SitePads[I])
		{
			SitePads[I]->SetHiddenInGame(Stage < 1, true);
		}
		if (SiteFrames.IsValidIndex(I) && SiteFrames[I])
		{
			SiteFrames[I]->SetHiddenInGame(Stage < 2, true);
		}
		if (SiteWalls.IsValidIndex(I) && SiteWalls[I])
		{
			SiteWalls[I]->SetHiddenInGame(Stage < 3, true);
		}
		if (SiteRoofs.IsValidIndex(I) && SiteRoofs[I])
		{
			SiteRoofs[I]->SetHiddenInGame(Stage < 4, true);
		}
	}
}

void AValleyWorld::SpawnPeople()
{
	for (int32 I = 0; I < Brain.VillagerCount; ++I)
	{
		AValleyVillager* V = GetWorld()->SpawnActor<AValleyVillager>();
		UClass* Presentation = (I == vg::kMetaHumanMilestoneSlot && Brain.Villagers[I].bTeacher)
			? MaraMetaHumanClass.Get()
			: nullptr;
		V->Arm(Brain.Villagers[I], Presentation);
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

void AValleyWorld::EnsureSpawnedPopulation()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	while (Villagers.Num() < Brain.VillagerCount)
	{
		const int32 I = Villagers.Num();
		if (I < 0 || I >= vg::kMaxHumans)
		{
			break;
		}
		AValleyVillager* V = World->SpawnActor<AValleyVillager>();
		if (!V)
		{
			break;
		}
		V->Arm(Brain.Villagers[I], nullptr);
		V->SyncFromSim(Brain.Villagers[I], Terrain, 0.f);
		Villagers.Add(V);
	}
	while (SpawnedShelters < Brain.ShelterCount)
	{
		PlaceShelter(SpawnedShelters);
		SpawnedShelters += 1;
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
			L->SetIntensity(bNight ? 0.55f : FMath::Lerp(16.f, 7.5f, Gold));
			L->SetLightColor(bNight ? FLinearColor(0.28f, 0.36f, 0.58f) : FMath::Lerp(FLinearColor(1.f, 0.96f, 0.86f), FLinearColor(1.f, 0.58f, 0.3f), Gold));
		}
	}
	if (Sky)
	{
		if (USkyLightComponent* S = Sky->GetLightComponent())
		{
			S->SetIntensity(vg::IsNight(Hours) ? 0.28f : 1.05f);
		}
	}
	if (Fog)
	{
		if (UExponentialHeightFogComponent* F = Fog->GetComponent())
		{
			const bool bStorm = Brain.Sky != vg::Weather::Clear;
			F->SetFogDensity(vg::IsNight(Hours) ? 0.028f : (bStorm ? 0.034f : 0.018f));
		}
	}
	if (Post)
	{
		const bool bNight = vg::IsNight(Hours);
		Post->Settings.AutoExposureBias = bNight ? 0.08f : 0.28f;
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
	for (int32 I = 0; I < Trees.Num(); ++I)
	{
		UStaticMeshComponent* Trunk = Trees[I];
		if (Trunk)
		{
			const float Yaw = TreeBaseYaw.IsValidIndex(I) ? TreeBaseYaw[I] : 18.f;
			Trunk->SetWorldRotation(FRotator(Lean, Yaw, 0.f));
		}
	}
}

void AValleyWorld::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	vg::TickWorld(Brain, DeltaSeconds);
	EnsureSpawnedPopulation();
	EnsureWorkVisuals();
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
