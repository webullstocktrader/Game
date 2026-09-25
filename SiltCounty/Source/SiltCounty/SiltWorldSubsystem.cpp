#include "SiltWorldSubsystem.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/VolumetricCloudComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/Font.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyAtmosphere.h"
#include "Engine/SkyLight.h"
#include "Engine/SphereReflectionCapture.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/TextRenderActor.h"
#include "Engine/VolumetricCloud.h"
#include "Engine/World.h"
#include "Engine/WorldSettings.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "HAL/PlatformTime.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "SiltCounty.h"
#include "SiltGroundChunk.h"
#include "SiltRainActor.h"
#include "SiltTerrain.h"

namespace
{
	void TagSilt(AActor* Actor)
	{
		if (Actor)
		{
			Actor->Tags.AddUnique(TEXT("SiltCounty"));
		}
	}

	template <typename TActor>
	void DestroyUntagged(UWorld& World)
	{
		TArray<AActor*> Pending;
		for (TActorIterator<TActor> It(&World); It; ++It)
		{
			if (!It->Tags.Contains(TEXT("SiltCounty")))
			{
				Pending.Add(*It);
			}
		}
		for (AActor* Actor : Pending)
		{
			Actor->Destroy();
		}
	}

	UStaticMesh* EngineMesh(const TCHAR* Path)
	{
		return LoadObject<UStaticMesh>(nullptr, Path);
	}

	AStaticMeshActor* SpawnMesh(UWorld& World, UStaticMesh* Mesh, const FTransform& Xform, UMaterialInterface* Material, FName Name)
	{
		if (!Mesh)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.Name = Name;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AStaticMeshActor* Actor = World.SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Xform, Params);
		if (!Actor)
		{
			return nullptr;
		}

		TagSilt(Actor);
		UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetStaticMesh(Mesh);
		Comp->SetWorldTransform(Xform);
		if (Material)
		{
			Comp->SetMaterial(0, Material);
		}
		Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Comp->SetCollisionProfileName(TEXT("BlockAll"));
		return Actor;
	}

	UMaterialInstanceDynamic* Tint(UMaterialInterface* Base, const FLinearColor& Color, UObject* Outer, float Roughness)
	{
		if (!Base)
		{
			return nullptr;
		}
		UMaterialInstanceDynamic* Mid = UMaterialInstanceDynamic::Create(Base, Outer);
		if (Mid)
		{
			Mid->SetVectorParameterValue(TEXT("PaintColor"), Color);
			Mid->SetScalarParameterValue(TEXT("Roughness"), Roughness);
		}
		return Mid;
	}
}

void USiltWorldSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	EnsureBuilt();
}

void USiltWorldSubsystem::EnsureBuilt()
{
	if (bBuilt)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	if (World->WorldType != EWorldType::Game && World->WorldType != EWorldType::PIE)
	{
		return;
	}

	bBuilt = true;
	if (AWorldSettings* Settings = World->GetWorldSettings())
	{
		Settings->bForceNoPrecomputedLighting = true;
		Settings->KillZ = -100000.f;
		Settings->bEnableWorldBoundsChecks = false;
	}

	const double Started = FPlatformTime::Seconds();
	ClearTemplateActors(*World);
	BuildTerrain(*World);
	BuildDressing(*World);
	BuildWeather(*World);
	UE_LOG(LogSiltCounty, Display, TEXT("Silt County world built in %.2fs"), FPlatformTime::Seconds() - Started);
}

void USiltWorldSubsystem::ClearTemplateActors(UWorld& World) const
{
	DestroyUntagged<AStaticMeshActor>(World);
	DestroyUntagged<ADirectionalLight>(World);
	DestroyUntagged<ASkyLight>(World);
	DestroyUntagged<AExponentialHeightFog>(World);
	DestroyUntagged<ASkyAtmosphere>(World);
	DestroyUntagged<AVolumetricCloud>(World);
	DestroyUntagged<APostProcessVolume>(World);
	DestroyUntagged<APlayerStart>(World);
	DestroyUntagged<ASphereReflectionCapture>(World);
	DestroyUntagged<APointLight>(World);
}

UMaterialInterface* USiltWorldSubsystem::LoadMat(const TCHAR* ProjectPath, const TCHAR* Fallback) const
{
	if (UMaterialInterface* Material = LoadObject<UMaterialInterface>(nullptr, ProjectPath))
	{
		return Material;
	}
	UE_LOG(LogSiltCounty, Warning, TEXT("Missing %s — using fallback. Open the editor once so Silt materials can be generated."), ProjectPath);
	return LoadObject<UMaterialInterface>(nullptr, Fallback);
}

bool USiltWorldSubsystem::ChunkNeedsDetail(float CenterX, float CenterY) const
{
	if (SiltTerrain::DistanceToRoad(CenterX, CenterY) < 80000.f)
	{
		return true;
	}
	const FVector2D Contract = SiltTerrain::GetContractXY();
	if (FVector2D::Distance(FVector2D(CenterX, CenterY), Contract) < 90000.f)
	{
		return true;
	}
	if (FVector2D::Distance(FVector2D(CenterX, CenterY), FVector2D(0.f, 80000.f)) < 90000.f)
	{
		return true;
	}
	return FVector2D::Distance(FVector2D(CenterX, CenterY), FVector2D(0.f, 52000.f)) < 70000.f;
}

void USiltWorldSubsystem::BuildTerrain(UWorld& World) const
{
	UMaterialInterface* GroundMat = LoadMat(
		TEXT("/Game/SiltCounty/Materials/M_WetGround.M_WetGround"),
		TEXT("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"));
	UMaterialInterface* WaterMat = LoadMat(
		TEXT("/Game/SiltCounty/Materials/M_FloodWater.M_FloodWater"),
		TEXT("/Engine/EngineMaterials/WorldGridMaterial.WorldGridMaterial"));

	constexpr float Chunk = 100000.f;
	int32 Count = 0;
	for (float X = SiltTerrain::GetCountyMin(); X < SiltTerrain::GetCountyMax() - 1.f; X += Chunk)
	{
		for (float Y = SiltTerrain::GetCountyMin(); Y < SiltTerrain::GetCountyMax() - 1.f; Y += Chunk)
		{
			const float CenterX = X + Chunk * 0.5f;
			const float CenterY = Y + Chunk * 0.5f;
			const float Step = ChunkNeedsDetail(CenterX, CenterY) ? 1000.f : 10000.f;

			FActorSpawnParameters Params;
			Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			ASiltGroundChunk* GroundChunk = World.SpawnActor<ASiltGroundChunk>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
			if (!GroundChunk)
			{
				continue;
			}
			GroundChunk->Build(FVector2D(X, Y), FVector2D(X + Chunk, Y + Chunk), Step, GroundMat, WaterMat);
			++Count;
			if (Count % 16 == 0)
			{
				UE_LOG(LogSiltCounty, Display, TEXT("Silt County terrain %d chunks..."), Count);
			}
		}
	}

	UE_LOG(LogSiltCounty, Display, TEXT("Silt County terrain chunks: %d (8km x 8km)"), Count);
}

void USiltWorldSubsystem::BuildDressing(UWorld& World) const
{
	UMaterialInterface* Paint = LoadMat(
		TEXT("/Game/SiltCounty/Materials/M_TruckPaint.M_TruckPaint"),
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UMaterialInterface* BeaconMat = LoadMat(
		TEXT("/Game/SiltCounty/Materials/M_Beacon.M_Beacon"),
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UStaticMesh* Cube = EngineMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cylinder = EngineMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	auto Place = [&](UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, const FLinearColor& Color, float Roughness, bool bBlock)
	{
		const FTransform Xform(Rotation, Location, Scale);
		UMaterialInstanceDynamic* Mid = Tint(Paint, Color, this, Roughness);
		AStaticMeshActor* Actor = SpawnMesh(World, Mesh, Xform, Mid, NAME_None);
		if (Actor && !bBlock)
		{
			Actor->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		return Actor;
	};

	auto Label = [&](const FVector& Location, const FString& Text, const FColor& Color, float Size, float Yaw = 90.f)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ATextRenderActor* Sign = World.SpawnActor<ATextRenderActor>(Location, FRotator(0.f, Yaw, 0.f), Params);
		if (!Sign)
		{
			return;
		}
		TagSilt(Sign);
		if (UTextRenderComponent* Render = Sign->GetTextRender())
		{
			Render->SetHorizontalAlignment(EHTA_Center);
			Render->SetWorldSize(Size);
			Render->SetTextRenderColor(Color);
			Render->SetText(FText::FromString(Text));
			if (UFont* Font = LoadObject<UFont>(nullptr, TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField")))
			{
				Render->SetFont(Font);
			}
		}
	};

	const float GarageZ = SiltTerrain::SampleHeight(0.f, 84000.f);
	const FLinearColor Concrete(0.22f, 0.21f, 0.19f);
	const FLinearColor Timber(0.18f, 0.10f, 0.06f);
	const FLinearColor Roof(0.12f, 0.13f, 0.14f);
	const FLinearColor DampWall(0.30f, 0.28f, 0.24f);
	const FLinearColor ShopRoofTint(0.52f, 0.38f, 0.26f);
	const FLinearColor BenchWood(0.46f, 0.28f, 0.14f);
	const FLinearColor BenchSteel(0.58f, 0.46f, 0.30f);
	const TCHAR* ShopPaintPath = TEXT("/Game/SiltCounty/Materials/M_TruckPaint.M_TruckPaint");
	UMaterialInterface* ShopWallMat = LoadMat(TEXT("/Game/SiltCounty/Materials/M_ShopWall.M_ShopWall"), ShopPaintPath);
	UMaterialInterface* ShopFloorMat = LoadMat(TEXT("/Game/SiltCounty/Materials/M_ShopFloor.M_ShopFloor"), ShopPaintPath);

	auto ApplyShopMat = [&](AStaticMeshActor* Actor, UMaterialInterface* Material)
	{
		if (Actor && Material && Material != Paint)
		{
			Actor->GetStaticMeshComponent()->SetMaterial(0, Material);
		}
	};

	// Lock B shop shell stays on this footprint. Truck spawns and the bay light stay put.
	ApplyShopMat(Place(Cube, FVector(-1100.f, 86000.f, GarageZ + 280.f), FRotator::ZeroRotator, FVector(0.5f, 4.6f, 5.6f), DampWall, 0.32f, true), ShopWallMat);
	ApplyShopMat(Place(Cube, FVector(1100.f, 86000.f, GarageZ + 280.f), FRotator::ZeroRotator, FVector(0.5f, 4.6f, 5.6f), DampWall, 0.32f, true), ShopWallMat);
	ApplyShopMat(Place(Cube, FVector(0.f, 90000.f, GarageZ + 280.f), FRotator::ZeroRotator, FVector(22.f, 0.5f, 5.6f), DampWall, 0.32f, true), ShopWallMat);
	Place(Cube, FVector(0.f, 86000.f, GarageZ + 620.f), FRotator::ZeroRotator, FVector(24.f, 9.f, 0.35f), ShopRoofTint, 0.46f, true);

	auto PlaceShop = [&](UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, UMaterialInterface* Material, const FLinearColor& FallbackColor, float Roughness, FName Name)
	{
		UMaterialInterface* Surface = Material;
		if (!Surface || Surface == Paint)
		{
			Surface = Tint(Paint, FallbackColor, this, Roughness);
		}
		const FTransform Xform(FRotator::ZeroRotator, Location, Scale);
		return SpawnMesh(World, Mesh, Xform, Surface, Name);
	};

	PlaceShop(Cube, FVector(0.f, 86000.f, GarageZ + 8.f), FVector(20.f, 7.6f, 0.16f), ShopFloorMat, FLinearColor(0.18f, 0.17f, 0.15f), 0.12f, TEXT("ShopFloor"));
	const FVector Bench(-860.f, 86080.f, GarageZ);
	PlaceShop(Cube, Bench + FVector(0.f, 0.f, 42.f), FVector(1.15f, 2.0f, 0.72f), nullptr, FLinearColor(0.32f, 0.20f, 0.10f), 0.58f, TEXT("ShopBench"));
	PlaceShop(Cube, Bench + FVector(0.f, 0.f, 84.f), FVector(1.35f, 2.2f, 0.10f), nullptr, BenchWood, 0.48f, TEXT("ShopBenchTop"));
	PlaceShop(Cube, Bench + FVector(0.f, -40.f, 96.f), FVector(0.42f, 0.55f, 0.06f), nullptr, BenchSteel, 0.28f, TEXT("ShopBenchTray"));

	const FVector TownCenter(0.f, 52000.f, SiltTerrain::SampleHeight(0.f, 52000.f));
	const FVector HouseOffsets[] = {
		FVector(-3500.f, 1800.f, 0.f), FVector(4200.f, 600.f, 0.f), FVector(-1800.f, -2600.f, 0.f),
		FVector(2600.f, -1800.f, 0.f), FVector(700.f, 3200.f, 0.f), FVector(-4800.f, -400.f, 0.f)
	};
	int32 HouseIndex = 0;
	for (const FVector& Offset : HouseOffsets)
	{
		const FVector Loc = TownCenter + Offset;
		const float Ground = SiltTerrain::SampleHeight(Loc.X, Loc.Y);
		const float Height = 280.f + static_cast<float>(HouseIndex % 3) * 70.f;
		const bool bFlooded = Ground < SiltTerrain::GetWaterLevel() + 40.f;
		Place(
			Cube,
			FVector(Loc.X, Loc.Y, Ground + Height * 0.5f - (bFlooded ? 40.f : 0.f)),
			FRotator(0.f, HouseIndex * 18.f, bFlooded ? 4.f : 0.f),
			FVector(4.2f + HouseIndex * 0.15f, 3.4f, Height / 100.f),
			bFlooded ? FLinearColor(0.10f, 0.09f, 0.08f) : Timber,
			bFlooded ? 0.22f : 0.58f,
			true);
		++HouseIndex;
	}
	Label(FVector(0.f, 56000.f, SiltTerrain::GetWaterLevel() + 900.f), TEXT("SOUTH TOWN  —  FLOODED"), FColor(210, 220, 230), 160.f);

	const TCHAR* TruckPaintPath = TEXT("/Game/SiltCounty/Materials/M_TruckPaint.M_TruckPaint");
	UMaterialInterface* TownBrick = LoadMat(TEXT("/Game/SiltCounty/Materials/M_TownBrick.M_TownBrick"), TruckPaintPath);
	UMaterialInterface* TownClapboard = LoadMat(TEXT("/Game/SiltCounty/Materials/M_TownClapboard.M_TownClapboard"), TruckPaintPath);
	UMaterialInterface* ConcreteBlock = LoadMat(TEXT("/Game/SiltCounty/Materials/M_ConcreteBlock.M_ConcreteBlock"), TruckPaintPath);
	UMaterialInterface* Municipal = LoadMat(TEXT("/Game/SiltCounty/Materials/M_MunicipalPaint.M_MunicipalPaint"), TruckPaintPath);

	auto PlaceMat = [&](UStaticMesh* Mesh, const FVector& Location, const FRotator& Rotation, const FVector& Scale, UMaterialInterface* Material, const FLinearColor& FallbackColor, FName Name)
	{
		UMaterialInterface* Surface = Material;
		if (!Surface || Surface == Paint)
		{
			Surface = Tint(Paint, FallbackColor, this, 0.72f);
		}
		const FTransform Xform(Rotation, Location, Scale);
		return SpawnMesh(World, Mesh, Xform, Surface, Name);
	};

	// Pass A exterior landmarks. Garage blockout at Y≈86000 stays above.
	{
		const FVector BankAnchor = TownCenter + FVector(-4200.f, 900.f, 0.f);
		const float BankZ = SiltTerrain::SampleHeight(BankAnchor.X, BankAnchor.Y);
		const FVector Bank(BankAnchor.X, BankAnchor.Y, BankZ);
		const FLinearColor BrickFallback(0.34f, 0.13f, 0.09f);
		const FLinearColor BlockFallback(0.40f, 0.39f, 0.36f);
		PlaceMat(Cube, Bank + FVector(0.f, 0.f, 22.5f), FRotator::ZeroRotator, FVector(8.2f, 5.6f, 0.45f), ConcreteBlock, BlockFallback, TEXT("CountyTrustPlinth"));
		PlaceMat(Cube, Bank + FVector(0.f, 0.f, 255.f), FRotator::ZeroRotator, FVector(7.5f, 5.0f, 4.2f), TownBrick, BrickFallback, TEXT("CountyTrustBank"));
		const FVector PierSW(-280.f, -180.f, 0.f);
		const FVector PierNE(280.f, 180.f, 0.f);
		PlaceMat(Cube, Bank + PierSW + FVector(0.f, 0.f, 27.5f), FRotator::ZeroRotator, FVector(1.5f, 1.5f, 0.55f), ConcreteBlock, BlockFallback, TEXT("CountyTrustBaseSW"));
		PlaceMat(Cube, Bank + PierNE + FVector(0.f, 0.f, 27.5f), FRotator::ZeroRotator, FVector(1.5f, 1.5f, 0.55f), ConcreteBlock, BlockFallback, TEXT("CountyTrustBaseNE"));
		PlaceMat(Cube, Bank + PierSW + FVector(0.f, 0.f, 385.f), FRotator::ZeroRotator, FVector(1.15f, 1.15f, 6.6f), TownBrick, BrickFallback, TEXT("CountyTrustPierSW"));
		PlaceMat(Cube, Bank + PierNE + FVector(0.f, 0.f, 385.f), FRotator::ZeroRotator, FVector(1.15f, 1.15f, 6.6f), TownBrick, BrickFallback, TEXT("CountyTrustPierNE"));
		Label(Bank + FVector(0.f, 310.f, 500.f), TEXT("COUNTY TRUST"), FColor(236, 224, 196), 78.f);

		const FVector HallAnchor = TownCenter + FVector(3900.f, 1100.f, 0.f);
		const float HallZ = SiltTerrain::SampleHeight(HallAnchor.X, HallAnchor.Y);
		const FVector Hall(HallAnchor.X, HallAnchor.Y, HallZ);
		const FLinearColor ClapboardFallback(0.38f, 0.31f, 0.22f);
		PlaceMat(Cube, Hall + FVector(0.f, 0.f, 240.f), FRotator::ZeroRotator, FVector(6.5f, 5.5f, 4.8f), TownClapboard, ClapboardFallback, TEXT("TownHall"));
		const FVector TurretNW(-260.f, 220.f, 0.f);
		const FVector TurretNE(260.f, 220.f, 0.f);
		PlaceMat(Cylinder, Hall + TurretNW + FVector(0.f, 0.f, 450.f), FRotator::ZeroRotator, FVector(1.6f, 1.6f, 9.f), TownClapboard, ClapboardFallback, TEXT("TownHallTurretNW"));
		PlaceMat(Cylinder, Hall + TurretNE + FVector(0.f, 0.f, 450.f), FRotator::ZeroRotator, FVector(1.6f, 1.6f, 9.f), TownClapboard, ClapboardFallback, TEXT("TownHallTurretNE"));
		const FLinearColor Civic(0.36f, 0.42f, 0.38f);
		auto PlaceCivic = [&](UStaticMesh* Mesh, const FVector& Location, const FVector& Scale, FName Name)
		{
			UMaterialInterface* Surface = Tint(Municipal ? Municipal : Paint, Civic, this, 0.48f);
			const FTransform Xform(FRotator::ZeroRotator, Location, Scale);
			SpawnMesh(World, Mesh, Xform, Surface, Name);
		};
		PlaceCivic(Cylinder, Hall + TurretNW + FVector(0.f, 0.f, 935.f), FVector(2.05f, 2.05f, 0.55f), TEXT("TownHallCapNW"));
		PlaceCivic(Cylinder, Hall + TurretNE + FVector(0.f, 0.f, 935.f), FVector(2.05f, 2.05f, 0.55f), TEXT("TownHallCapNE"));
		PlaceCivic(Cube, Hall + FVector(0.f, 0.f, 488.f), FVector(6.9f, 5.9f, 0.16f), TEXT("TownHallTrim"));
		Label(Hall + FVector(0.f, 340.f, 620.f), TEXT("TOWN HALL"), FColor(236, 228, 206), 84.f);
	}

	// Town silhouettes. House boxes, County Trust, and Town Hall stay secondary.
	const FLinearColor Galvanized(0.73f, 0.74f, 0.71f);
	const FLinearColor Barn(0.34f, 0.13f, 0.07f);
	const FLinearColor Brick(0.58f, 0.24f, 0.16f);
	const FLinearColor TankWhite(0.80f, 0.82f, 0.84f);
	const FLinearColor Steel(0.22f, 0.23f, 0.25f);

	const FVector Coop(9800.f, 58500.f, 0.f);
	const float CoopZ = SiltTerrain::SampleHeight(Coop.X, Coop.Y);
	constexpr float SiloHeightCm = 4800.f;
	constexpr float SiloSpacing = 780.f;
	Place(Cube, FVector(Coop.X, Coop.Y, CoopZ + 40.f), FRotator::ZeroRotator, FVector(10.f, 34.f, 0.8f), Concrete, 0.62f, true);
	for (int32 Silo = 0; Silo < 4; ++Silo)
	{
		const float SiloY = Coop.Y + (static_cast<float>(Silo) - 1.5f) * SiloSpacing;
		Place(Cylinder, FVector(Coop.X, SiloY, CoopZ + SiloHeightCm * 0.5f), FRotator::ZeroRotator, FVector(6.2f, 6.2f, SiloHeightCm / 100.f), Galvanized, 0.32f, true);
		Place(Cylinder, FVector(Coop.X, SiloY, CoopZ + SiloHeightCm - 280.f), FRotator::ZeroRotator, FVector(6.45f, 6.45f, 0.55f), FLinearColor(0.42f, 0.20f, 0.10f), 0.55f, false);
	}
	Place(Cube, FVector(Coop.X, Coop.Y, CoopZ + SiloHeightCm + 220.f), FRotator::ZeroRotator, FVector(8.f, 30.f, 5.2f), Barn, 0.58f, true);
	Place(Cube, FVector(Coop.X + 520.f, Coop.Y, CoopZ + 2700.f), FRotator::ZeroRotator, FVector(2.4f, 2.6f, 54.f), Barn, 0.5f, true);
	Label(FVector(Coop.X - 900.f, Coop.Y, CoopZ + SiloHeightCm + 980.f), TEXT("APACHE FARMERS COOP"), FColor(230, 210, 140), 210.f, 180.f);

	const FVector TowerBase(-11000.f, 58500.f, 0.f);
	const float TowerZ = SiltTerrain::SampleHeight(TowerBase.X, TowerBase.Y);
	constexpr float LegHeight = 2800.f;
	constexpr float LegSpread = 320.f;
	const FVector LegOffsets[] = {
		FVector(-LegSpread, -LegSpread, 0.f),
		FVector(LegSpread, -LegSpread, 0.f),
		FVector(-LegSpread, LegSpread, 0.f),
		FVector(LegSpread, LegSpread, 0.f)
	};
	for (const FVector& Leg : LegOffsets)
	{
		Place(
			Cylinder,
			FVector(TowerBase.X, TowerBase.Y, TowerZ + LegHeight * 0.5f) + Leg,
			FRotator::ZeroRotator,
			FVector(0.7f, 0.7f, LegHeight / 100.f),
			Steel,
			0.45f,
			true);
	}
	Place(Cylinder, FVector(TowerBase.X, TowerBase.Y, TowerZ + LegHeight), FRotator::ZeroRotator, FVector(8.2f, 8.2f, 0.35f), FLinearColor(0.30f, 0.31f, 0.33f), 0.4f, true);
	Place(Cylinder, FVector(TowerBase.X, TowerBase.Y, TowerZ + LegHeight + 260.f), FRotator::ZeroRotator, FVector(7.2f, 7.2f, 4.8f), TankWhite, 0.28f, true);
	Place(Cylinder, FVector(TowerBase.X, TowerBase.Y, TowerZ + LegHeight + 560.f), FRotator::ZeroRotator, FVector(5.4f, 5.4f, 1.4f), FLinearColor(0.55f, 0.57f, 0.60f), 0.35f, true);
	Label(FVector(TowerBase.X + 700.f, TowerBase.Y, TowerZ + LegHeight + 1100.f), TEXT("WATER TOWER"), FColor(210, 225, 235), 180.f, 0.f);

	const FVector School(15800.f, 49000.f, 0.f);
	const float SchoolZ = SiltTerrain::SampleHeight(School.X, School.Y);
	Place(Cube, FVector(School.X, School.Y, SchoolZ + 340.f), FRotator::ZeroRotator, FVector(16.f, 92.f, 6.8f), Brick, 0.62f, true);
	Place(Cube, FVector(School.X, School.Y, SchoolZ + 710.f), FRotator::ZeroRotator, FVector(18.f, 94.f, 0.45f), Roof, 0.42f, true);
	Place(Cube, FVector(School.X - 820.f, School.Y, SchoolZ + 520.f), FRotator::ZeroRotator, FVector(0.35f, 90.f, 1.15f), FLinearColor(0.86f, 0.82f, 0.70f), 0.5f, false);
	Place(Cube, FVector(School.X + 200.f, School.Y - 5200.f, SchoolZ + 420.f), FRotator::ZeroRotator, FVector(22.f, 18.f, 8.4f), FLinearColor(0.48f, 0.20f, 0.14f), 0.55f, true);
	Place(Cylinder, FVector(School.X - 1400.f, School.Y + 4200.f, SchoolZ + 700.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 14.f), FLinearColor(0.75f, 0.76f, 0.74f), 0.3f, false);
	Place(Cube, FVector(School.X - 1580.f, School.Y + 4200.f, SchoolZ + 1360.f), FRotator::ZeroRotator, FVector(0.9f, 0.12f, 0.55f), FLinearColor(0.55f, 0.08f, 0.06f), 0.4f, false);
	Label(FVector(School.X - 1700.f, School.Y, SchoolZ + 1200.f), TEXT("BOONE-APACHE HIGH SCHOOL"), FColor(245, 236, 210), 190.f, 180.f);

	// Fiction locked lot. No street address and no house mesh.
	const FVector Lot(-6200.f, 70500.f, 0.f);
	const float LotZ = SiltTerrain::SampleHeight(Lot.X, Lot.Y);
	constexpr float LotHalfX = 640.f;
	constexpr float LotHalfY = 440.f;
	Place(Cube, FVector(Lot.X, Lot.Y, LotZ + 10.f), FRotator::ZeroRotator, FVector(14.f, 10.f, 0.16f), FLinearColor(0.30f, 0.29f, 0.26f), 0.72f, true);
	const FVector LotCorners[] = {
		FVector(-LotHalfX, -LotHalfY, 0.f),
		FVector(LotHalfX, -LotHalfY, 0.f),
		FVector(-LotHalfX, LotHalfY, 0.f),
		FVector(LotHalfX, LotHalfY, 0.f)
	};
	for (const FVector& Corner : LotCorners)
	{
		Place(
			Cylinder,
			FVector(Lot.X, Lot.Y, LotZ + 90.f) + Corner,
			FRotator::ZeroRotator,
			FVector(0.38f, 0.38f, 1.8f),
			FLinearColor(0.45f, 0.075f, 0.05f),
			0.55f,
			true);
	}
	Place(Cube, FVector(Lot.X + LotHalfX, Lot.Y, LotZ + 70.f), FRotator::ZeroRotator, FVector(0.12f, 8.6f, 0.12f), FLinearColor(0.12f, 0.12f, 0.13f), 0.4f, true);
	Place(Cube, FVector(Lot.X + LotHalfX, Lot.Y, LotZ + 130.f), FRotator::ZeroRotator, FVector(0.12f, 8.6f, 0.12f), FLinearColor(0.12f, 0.12f, 0.13f), 0.4f, true);
	Label(FVector(Lot.X + LotHalfX + 80.f, Lot.Y, LotZ + 360.f), TEXT("CHIEF'S PERSONAL GARAGE — LOCKED"), FColor(255, 196, 64), 120.f, 0.f);

	const FVector Bridge(6000.f, 24000.f, SiltTerrain::SampleHeight(6000.f, 24000.f));
	Place(Cube, Bridge + FVector(-900.f, 1800.f, 80.f), FRotator(0.f, 70.f, -12.f), FVector(6.f, 1.6f, 0.35f), Concrete, 0.5f, true);
	Place(Cube, Bridge + FVector(900.f, -1600.f, 40.f), FRotator(0.f, 70.f, 14.f), FVector(5.f, 1.6f, 0.35f), Concrete, 0.5f, true);
	Label(Bridge + FVector(0.f, 0.f, 700.f), TEXT("BRIDGE OUT"), FColor(220, 80, 60), 180.f);

	const FVector Culvert(-1500.f, 40000.f, SiltTerrain::SampleHeight(-1500.f, 40000.f));
	Place(Cylinder, Culvert + FVector(0.f, 400.f, 40.f), FRotator(90.f, 0.f, 90.f), FVector(1.3f, 1.3f, 2.4f), FLinearColor(0.25f, 0.22f, 0.18f), 0.45f, true);
	Place(Cylinder, Culvert + FVector(0.f, -400.f, 20.f), FRotator(80.f, 15.f, 90.f), FVector(1.1f, 1.1f, 2.0f), FLinearColor(0.18f, 0.16f, 0.13f), 0.35f, false);
	Label(Culvert + FVector(0.f, 0.f, 500.f), TEXT("CULVERT"), FColor(190, 190, 170), 120.f);

	const FVector Drop = SiltTerrain::GetDropZone();
	if (AStaticMeshActor* Marker = Place(Cylinder, Drop + FVector(0.f, 0.f, 1600.f), FRotator::ZeroRotator, FVector(0.35f, 0.35f, 32.f), FLinearColor(1.f, 0.7f, 0.2f), 0.2f, false))
	{
		if (UMaterialInstanceDynamic* Beam = Tint(BeaconMat ? BeaconMat : Paint, FLinearColor(1.f, 0.65f, 0.15f), Marker, 0.2f))
		{
			Marker->GetStaticMeshComponent()->SetMaterial(0, Beam);
		}
	}
	Label(Drop + FVector(0.f, 0.f, 360.f), TEXT("DROP ZONE"), FColor(255, 210, 90), 140.f);

	if (APointLight* Bay = World.SpawnActor<APointLight>(FVector(0.f, 84000.f, GarageZ + 480.f), FRotator::ZeroRotator))
	{
		TagSilt(Bay);
		if (UPointLightComponent* Light = Cast<UPointLightComponent>(Bay->GetLightComponent()))
		{
			Light->SetMobility(EComponentMobility::Movable);
			Light->SetIntensity(8000.f);
			Light->SetAttenuationRadius(2800.f);
			Light->SetLightColor(FLinearColor(1.f, 0.85f, 0.65f));
		}
	}
}

void USiltWorldSubsystem::BuildWeather(UWorld& World) const
{
	const FRotator SunRot(-48.f, 35.f, 0.f);
	if (ADirectionalLight* Sun = World.SpawnActor<ADirectionalLight>(FVector(0.f, 0.f, 2000.f), SunRot))
	{
		TagSilt(Sun);
		if (UDirectionalLightComponent* Light = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			Light->SetMobility(EComponentMobility::Movable);
			Light->SetIntensity(10.f);
			Light->SetLightColor(FLinearColor(0.72f, 0.78f, 0.88f));
			Light->SetAtmosphereSunLight(true);
			Light->SetCastShadows(true);
			Light->DynamicShadowDistanceMovableLight = 80000.f;
			Light->CascadeDistributionExponent = 2.2f;
			Light->ContactShadowLength = 0.12f;
		}
	}

	if (ASkyAtmosphere* Atmosphere = World.SpawnActor<ASkyAtmosphere>())
	{
		TagSilt(Atmosphere);
	}

	if (ASkyLight* Sky = World.SpawnActor<ASkyLight>())
	{
		TagSilt(Sky);
		if (USkyLightComponent* Light = Sky->GetLightComponent())
		{
			Light->SetMobility(EComponentMobility::Movable);
			Light->SetIntensity(1.35f);
			Light->SetRealTimeCapture(true);
			Light->RecaptureSky();
			Light->bLowerHemisphereIsBlack = false;
			Light->LowerHemisphereColor = FLinearColor(0.08f, 0.09f, 0.08f);
		}
	}

	if (AVolumetricCloud* Clouds = World.SpawnActor<AVolumetricCloud>())
	{
		TagSilt(Clouds);
		if (UVolumetricCloudComponent* Comp = Clouds->FindComponentByClass<UVolumetricCloudComponent>())
		{
			Comp->LayerBottomAltitude = 0.8f;
			Comp->LayerHeight = 5.5f;
			if (UMaterialInterface* CloudMat = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/EngineSky/VolumetricClouds/m_SimpleVolumetricCloud_Inst.m_SimpleVolumetricCloud_Inst")))
			{
				Comp->SetMaterial(CloudMat);
			}
		}
	}

	if (AExponentialHeightFog* Fog = World.SpawnActor<AExponentialHeightFog>())
	{
		TagSilt(Fog);
		if (UExponentialHeightFogComponent* Comp = Fog->GetComponent())
		{
			Comp->SetFogDensity(0.017f);
			Comp->SetFogHeightFalloff(0.18f);
			Comp->SetFogInscatteringColor(FLinearColor(0.50f, 0.46f, 0.40f));
			Comp->SetFogMaxOpacity(0.92f);
			Comp->SetStartDistance(800.f);
			Comp->bEnableVolumetricFog = true;
			Comp->VolumetricFogScatteringDistribution = 0.35f;
			Comp->VolumetricFogDistance = 8000.f;
			Comp->VolumetricFogAlbedo = FColor(187, 181, 170);
		}
	}

	if (APostProcessVolume* Post = World.SpawnActor<APostProcessVolume>())
	{
		TagSilt(Post);
		Post->bUnbound = true;
		Post->Settings.bOverride_ColorSaturation = true;
		Post->Settings.ColorSaturation = FVector4(0.78f, 0.86f, 0.84f, 1.f);
		Post->Settings.bOverride_ColorContrast = true;
		Post->Settings.ColorContrast = FVector4(1.05f, 1.04f, 1.02f, 1.f);
		Post->Settings.bOverride_ColorGamma = true;
		Post->Settings.ColorGamma = FVector4(0.98f, 1.0f, 1.02f, 1.f);
		Post->Settings.bOverride_AutoExposureBias = true;
		Post->Settings.AutoExposureBias = -0.15f;
		Post->Settings.bOverride_VignetteIntensity = true;
		Post->Settings.VignetteIntensity = 0.28f;
		Post->Settings.bOverride_BloomIntensity = true;
		Post->Settings.BloomIntensity = 0.35f;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	World.SpawnActor<ASiltRainActor>(FVector::ZeroVector, FRotator::ZeroRotator, Params);
}
