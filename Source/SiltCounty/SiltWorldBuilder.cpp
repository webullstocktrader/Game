#include "SiltWorldBuilder.h"
#include "SiltTerrain.h"
#include "SiltTruck.h"
#include "SiltProp.h"
#include "SiltDeer.h"
#include "SiltTypes.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMeshActor.h"
#include "Components/VolumetricCloudComponent.h"
#include "GameFramework/PlayerController.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

ASiltWorldBuilder::ASiltWorldBuilder()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

FVector ASiltWorldBuilder::GetGarageLookAt() const
{
	if (Chief && Gooch)
	{
		return (Chief->GetActorLocation() + Gooch->GetActorLocation()) * 0.5f;
	}
	return GetActorLocation();
}

void ASiltWorldBuilder::BuildSlice()
{
	StripTemplateActors();
	Terrain = GetWorld()->SpawnActor<ASiltTerrain>(FVector::ZeroVector, FRotator::ZeroRotator);
	Terrain->BuildCounty();

	SpawnAtmosphere();
	SpawnGarage();
	SpawnTreesAndStumps();
	SpawnTown();
	SpawnRain();

	const FVector Garage = Terrain->GetGarageCenter();
	const FSiltGroundHit Pad = Terrain->Query(Garage);
	const FVector ChiefPos(Garage.X + 80.f, Garage.Y - 280.f, Pad.Position.Z + 70.f);
	const FVector GoochPos(Garage.X + 80.f, Garage.Y + 280.f, Pad.Position.Z + 70.f);

	Chief = GetWorld()->SpawnActor<ASiltTruck>(ChiefPos, FRotator(0.f, 0.f, 0.f));
	Gooch = GetWorld()->SpawnActor<ASiltTruck>(GoochPos, FRotator(0.f, 0.f, 0.f));
	Chief->Configure(ESiltDriver::Chief, Terrain);
	Gooch->Configure(ESiltDriver::Gooch, Terrain);
	Chief->SetPartner(Gooch);
	Gooch->SetPartner(Chief);

	const FVector Ford = Terrain->GetFordCenter();
	const FSiltGroundHit FordHit = Terrain->Query(Ford);
	ASiltAnchor* Stranded = GetWorld()->SpawnActor<ASiltAnchor>();
	Stranded->BuildStranded(FVector(Ford.X + 220.f, Ford.Y - 80.f, FordHit.Position.Z + 40.f), FRotator(8.f, 40.f, 12.f));

	Crate = GetWorld()->SpawnActor<ASiltCrate>();
	Crate->Place(FVector(Ford.X - 80.f, Ford.Y + 60.f, FordHit.Position.Z + 50.f));

	const FVector LayBy = Terrain->GetHighwayLayBy();
	const FSiltGroundHit LayHit = Terrain->Query(LayBy);
	DropZone = GetWorld()->SpawnActor<ASiltDropZone>();
	DropZone->Place(FVector(LayBy.X, LayBy.Y, LayHit.Position.Z + 6.f));

	auto SpawnDeer = [&](const FVector2D& XY)
	{
		const FSiltGroundHit Hit = Terrain->Query(FVector(XY.X, XY.Y, 0.f));
		ASiltDeer* Deer = GetWorld()->SpawnActor<ASiltDeer>();
		const FVector Start(XY.X, XY.Y, Hit.Position.Z);
		Deer->Arm(Start, Start + FVector(800.f, 400.f, 0.f), Start + FVector(-600.f, 900.f, 0.f));
	};
	SpawnDeer(FVector2D(2200.f, 5200.f));
	SpawnDeer(FVector2D(-1400.f, 6400.f));
	SpawnDeer(FVector2D(1500.f, -4800.f));
}

void ASiltWorldBuilder::StripTemplateActors()
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

UStaticMeshComponent* ASiltWorldBuilder::PlaceMesh(UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat, const FName& Name)
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

void ASiltWorldBuilder::SpawnAtmosphere()
{
	ADirectionalLight* Sun = GetWorld()->SpawnActor<ADirectionalLight>(FVector::ZeroVector, FRotator(-48.f, 210.f, 0.f));
	if (UDirectionalLightComponent* L = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
	{
		L->SetIntensity(4.2f);
		L->SetLightColor(FLinearColor(0.72f, 0.78f, 0.86f));
		L->SetAtmosphereSunLight(true);
		L->SetDynamicShadowDistanceMovableLight(40000.f);
	}

	ASkyLight* Sky = GetWorld()->SpawnActor<ASkyLight>();
	if (USkyLightComponent* S = Sky->GetLightComponent())
	{
		S->SetIntensity(1.15f);
		S->bRealTimeCapture = true;
		S->SetLightColor(FLinearColor(0.65f, 0.72f, 0.78f));
	}

	GetWorld()->SpawnActor<ASkyAtmosphere>();
	GetWorld()->SpawnActor<AVolumetricCloud>();

	AExponentialHeightFog* Fog = GetWorld()->SpawnActor<AExponentialHeightFog>(FVector(0.f, 0.f, 200.f), FRotator::ZeroRotator);
	if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
	{
		FogComp->SetFogDensity(0.028f);
		FogComp->FogHeightFalloff = 0.12f;
		FogComp->SetVolumetricFog(true);
		FogComp->VolumetricFogExtinctionScale = 0.8f;
	}

	APostProcessVolume* PP = GetWorld()->SpawnActor<APostProcessVolume>();
	PP->bUnbound = true;
	FPostProcessSettings& P = PP->Settings;
	P.bOverride_AutoExposureMethod = true;
	P.AutoExposureMethod = AEM_Manual;
	P.bOverride_AutoExposureBias = true;
	P.AutoExposureBias = 0.15f;
	P.bOverride_ColorSaturation = true;
	P.ColorSaturation = FVector4(0.86f, 0.88f, 0.84f, 1.f);
	P.bOverride_ColorContrast = true;
	P.ColorContrast = FVector4(1.12f, 1.1f, 1.08f, 1.f);
	P.bOverride_VignetteIntensity = true;
	P.VignetteIntensity = 0.42f;
	P.bOverride_SceneFringeIntensity = true;
	P.SceneFringeIntensity = 0.4f;
	P.bOverride_FilmGrainIntensity = true;
	P.FilmGrainIntensity = 0.08f;
	P.bOverride_BloomIntensity = true;
	P.BloomIntensity = 0.35f;
}

void ASiltWorldBuilder::SpawnGarage()
{
	UStaticMesh* Cube = Silt::CubeMesh();
	if (!Cube || !Terrain)
	{
		return;
	}
	UMaterialInterface* Metal = Silt::Material(TEXT("M_Metal"));
	UMaterialInterface* Gravel = Silt::Material(TEXT("M_Gravel"));
	const FVector C = Terrain->GetGarageCenter();
	const FSiltGroundHit Hit = Terrain->Query(C);
	const float Z = Hit.Position.Z;

	PlaceMesh(Cube, FVector(C.X - 200.f, C.Y, Z + 10.f), FRotator::ZeroRotator, FVector(22.f, 18.f, 0.2f), Gravel, TEXT("GaragePad"));
	PlaceMesh(Cube, FVector(C.X - 400.f, C.Y, Z + 280.f), FRotator::ZeroRotator, FVector(16.f, 18.f, 0.25f), Metal, TEXT("GarageRoof"));
	PlaceMesh(Cube, FVector(C.X - 400.f, C.Y - 900.f, Z + 200.f), FRotator::ZeroRotator, FVector(16.f, 0.25f, 4.2f), Metal, TEXT("WallS"));
	PlaceMesh(Cube, FVector(C.X - 400.f, C.Y + 900.f, Z + 200.f), FRotator::ZeroRotator, FVector(16.f, 0.25f, 4.2f), Metal, TEXT("WallN"));
	PlaceMesh(Cube, FVector(C.X - 1180.f, C.Y, Z + 200.f), FRotator::ZeroRotator, FVector(0.25f, 18.f, 4.2f), Metal, TEXT("WallW"));
	// Open east wall — rolling door frame only.
	PlaceMesh(Cube, FVector(C.X + 380.f, C.Y - 900.f, Z + 200.f), FRotator::ZeroRotator, FVector(0.25f, 0.25f, 4.2f), Metal, TEXT("DoorPostS"));
	PlaceMesh(Cube, FVector(C.X + 380.f, C.Y + 900.f, Z + 200.f), FRotator::ZeroRotator, FVector(0.25f, 0.25f, 4.2f), Metal, TEXT("DoorPostN"));
	PlaceMesh(Cube, FVector(C.X + 380.f, C.Y, Z + 410.f), FRotator::ZeroRotator, FVector(0.25f, 18.f, 0.25f), Metal, TEXT("DoorLintel"));

	KeyRackLocation = FVector(C.X - 1100.f, C.Y - 400.f, Z + 160.f);
	PlaceMesh(Cube, KeyRackLocation, FRotator::ZeroRotator, FVector(0.12f, 1.1f, 0.7f), Metal, TEXT("KeyRack"));
	PlaceMesh(Silt::CylinderMesh(), KeyRackLocation + FVector(8.f, -20.f, 10.f), FRotator(0.f, 0.f, 90.f), FVector(0.08f, 0.08f, 0.12f), Silt::Material(TEXT("M_Chrome")), TEXT("KeyChief"));
	PlaceMesh(Silt::CylinderMesh(), KeyRackLocation + FVector(8.f, 20.f, 10.f), FRotator(0.f, 0.f, 90.f), FVector(0.08f, 0.08f, 0.12f), Silt::Material(TEXT("M_Chrome")), TEXT("KeyGooch"));

	UTextRenderComponent* Sign = NewObject<UTextRenderComponent>(this, TEXT("ShopSign"));
	Sign->SetText(FText::FromString(TEXT("CHIEF & GOOCH  ·  SHOP")));
	Sign->SetWorldSize(28.f);
	Sign->SetTextRenderColor(FColor(220, 180, 90));
	Sign->SetHorizontalAlignment(EHTA_Center);
	Sign->SetWorldLocation(FVector(C.X + 390.f, C.Y, Z + 360.f));
	Sign->SetWorldRotation(FRotator(0.f, 90.f, 0.f));
	Sign->SetupAttachment(GetRootComponent());
	Sign->RegisterComponent();

	UTextRenderComponent* Hwy = NewObject<UTextRenderComponent>(this, TEXT("HwySign"));
	Hwy->SetText(FText::FromString(TEXT("HIGHWAY 6")));
	Hwy->SetWorldSize(42.f);
	Hwy->SetTextRenderColor(FColor(230, 230, 230));
	Hwy->SetHorizontalAlignment(EHTA_Center);
	const FVector HwyLoc = Terrain->GetHighwayLayBy() + FVector(-600.f, 0.f, 220.f);
	Hwy->SetWorldLocation(FVector(8000.f, -2000.f, Terrain->Query(FVector(8000.f, -2000.f, 0.f)).Position.Z + 220.f));
	Hwy->SetWorldRotation(FRotator(0.f, 90.f, 0.f));
	Hwy->SetupAttachment(GetRootComponent());
	Hwy->RegisterComponent();
	(void)HwyLoc;
}

void ASiltWorldBuilder::SpawnTreesAndStumps()
{
	UStaticMesh* Cyl = Silt::CylinderMesh();
	if (!Cyl || !Terrain)
	{
		return;
	}
	UMaterialInterface* Bark = Silt::Material(TEXT("M_Bark"));
	UMaterialInterface* Leaf = Silt::Material(TEXT("M_Foliage"));
	int32 TreeN = 0;
	int32 StumpN = 0;
	FRandomStream Rng(613);
	for (int32 I = 0; I < 220; ++I)
	{
		const float X = Rng.FRandRange(-16000.f, 16000.f);
		const float Y = Rng.FRandRange(-16000.f, 16000.f);
		if (FMath::Abs(X - 8000.f) < 900.f)
		{
			continue;
		}
		if (FMath::Abs(X + 12000.f) < 2200.f && FMath::Abs(Y) < 2000.f)
		{
			continue;
		}
		const FSiltGroundHit Hit = Terrain->Query(FVector(X, Y, 0.f));
		if (Hit.Surface == ESiltSurface::Pavement || Hit.Surface == ESiltSurface::Water)
		{
			continue;
		}
		const float H = 2.8f + Rng.FRand() * 2.4f;
		PlaceMesh(Cyl, FVector(X, Y, Hit.Position.Z + H * 50.f), FRotator::ZeroRotator, FVector(0.28f, 0.28f, H), Bark, FName(*FString::Printf(TEXT("Trunk%d"), TreeN)));
		PlaceMesh(Cyl, FVector(X, Y, Hit.Position.Z + H * 100.f + 80.f), FRotator::ZeroRotator, FVector(1.6f + Rng.FRand(), 1.6f, 1.1f), Leaf, FName(*FString::Printf(TEXT("Canopy%d"), TreeN)));
		++TreeN;

		if (StumpN < 8 && I % 17 == 0)
		{
			ASiltAnchor* Stump = GetWorld()->SpawnActor<ASiltAnchor>();
			Stump->BuildStump(FVector(X + 180.f, Y - 90.f, Hit.Position.Z + 20.f));
			++StumpN;
		}
	}
}

void ASiltWorldBuilder::SpawnTown()
{
	UStaticMesh* Cube = Silt::CubeMesh();
	if (!Cube || !Terrain)
	{
		return;
	}
	const FVector Town(12000.f, 7200.f, 0.f);
	const FSiltGroundHit Hit = Terrain->Query(Town);
	UMaterialInterface* Metal = Silt::Material(TEXT("M_Metal"));
	UMaterialInterface* Wood = Silt::Material(TEXT("M_Wood"));
	PlaceMesh(Cube, FVector(Town.X, Town.Y, Hit.Position.Z + 140.f), FRotator::ZeroRotator, FVector(4.2f, 3.1f, 2.8f), Metal, TEXT("FeedStore"));
	PlaceMesh(Cube, FVector(Town.X + 700.f, Town.Y - 200.f, Hit.Position.Z + 110.f), FRotator(0.f, 15.f, 0.f), FVector(3.2f, 2.4f, 2.2f), Wood, TEXT("Chapel"));
	PlaceMesh(Cube, FVector(Town.X - 600.f, Town.Y + 180.f, Hit.Position.Z + 90.f), FRotator(0.f, -8.f, 0.f), FVector(2.4f, 2.8f, 1.8f), Metal, TEXT("GasShed"));

	UTextRenderComponent* Sign = NewObject<UTextRenderComponent>(this, TEXT("SiltSign"));
	Sign->SetText(FText::FromString(TEXT("SILT")));
	Sign->SetWorldSize(64.f);
	Sign->SetTextRenderColor(FColor(200, 40, 40));
	Sign->SetHorizontalAlignment(EHTA_Center);
	Sign->SetWorldLocation(FVector(Town.X, Town.Y + 400.f, Hit.Position.Z + 260.f));
	Sign->SetWorldRotation(FRotator(0.f, 180.f, 0.f));
	Sign->SetupAttachment(GetRootComponent());
	Sign->RegisterComponent();
}

void ASiltWorldBuilder::SpawnRain()
{
	UStaticMesh* Cube = Silt::CubeMesh();
	UMaterialInterface* Water = Silt::Material(TEXT("M_Water"));
	if (!Cube)
	{
		return;
	}
	for (int32 I = 0; I < 90; ++I)
	{
		UStaticMeshComponent* Streak = NewObject<UStaticMeshComponent>(this, FName(*FString::Printf(TEXT("Rain%d"), I)));
		Streak->SetStaticMesh(Cube);
		Streak->SetWorldScale3D(FVector(0.02f, 0.02f, 0.55f));
		Streak->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Streak->SetCastShadow(false);
		if (Water)
		{
			Streak->SetMaterial(0, Water);
		}
		Streak->SetupAttachment(GetRootComponent());
		Streak->RegisterComponent();
		RainStreaks.Add(Streak);
	}
}

void ASiltWorldBuilder::UpdateRain(float DeltaSeconds)
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}
	const FVector Cam = PC->PlayerCameraManager->GetCameraLocation();
	FRandomStream Rng(FMath::FloorToInt(GetWorld()->TimeSeconds * 40.f));
	for (int32 I = 0; I < RainStreaks.Num(); ++I)
	{
		UStaticMeshComponent* Streak = RainStreaks[I];
		FVector Loc = Streak->GetComponentLocation();
		Loc.Z -= 2800.f * DeltaSeconds;
		if (Loc.Z < Cam.Z - 400.f || RainClock < 0.05f)
		{
			Loc = Cam + FVector(Rng.FRandRange(-1800.f, 1800.f), Rng.FRandRange(-1800.f, 1800.f), Rng.FRandRange(200.f, 1400.f));
		}
		Streak->SetWorldLocation(Loc);
	}
	RainClock += DeltaSeconds;
}

void ASiltWorldBuilder::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateRain(DeltaSeconds);
}
