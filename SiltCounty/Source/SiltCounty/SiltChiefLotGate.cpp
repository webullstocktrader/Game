#include "SiltChiefLotGate.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Font.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "SiltCounty.h"
#include "SiltCountyGameState.h"
#include "SiltTypes.h"
#include "UObject/ConstructorHelpers.h"

ASiltChiefLotGate::ASiltChiefLotGate()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;
	Tags.Add(TEXT("SiltCounty"));

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
	UStaticMesh* Cylinder = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;

	auto MakePost = [this, Cylinder, Root](const TCHAR* Name, float Y)
	{
		UStaticMeshComponent* Post = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Post->SetupAttachment(Root);
		Post->SetMobility(EComponentMobility::Movable);
		Post->SetStaticMesh(Cylinder);
		Post->SetRelativeLocation(FVector(0.f, Y, 85.f));
		Post->SetRelativeScale3D(FVector(0.22f, 0.22f, 1.7f));
		Post->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Post->SetCollisionProfileName(TEXT("BlockAll"));
	};
	MakePost(TEXT("PostSouth"), -230.f);
	MakePost(TEXT("PostNorth"), 230.f);

	Hinge = CreateDefaultSubobject<USceneComponent>(TEXT("Hinge"));
	Hinge->SetMobility(EComponentMobility::Movable);
	Hinge->SetupAttachment(Root);
	Hinge->SetRelativeLocation(FVector(0.f, -220.f, 0.f));

	auto MakeBar = [this, Cube](const TCHAR* Name, float Z) -> UStaticMeshComponent*
	{
		UStaticMeshComponent* Bar = CreateDefaultSubobject<UStaticMeshComponent>(Name);
		Bar->SetMobility(EComponentMobility::Movable);
		Bar->SetupAttachment(Hinge);
		Bar->SetStaticMesh(Cube);
		Bar->SetRelativeLocation(FVector(0.f, 220.f, Z));
		Bar->SetRelativeScale3D(FVector(0.1f, 4.4f, 0.1f));
		Bar->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		Bar->SetCollisionProfileName(TEXT("BlockAll"));
		return Bar;
	};
	BarLow = MakeBar(TEXT("BarLow"), 70.f);
	BarHigh = MakeBar(TEXT("BarHigh"), 130.f);

	Sign = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Sign"));
	Sign->SetMobility(EComponentMobility::Movable);
	Sign->SetupAttachment(Root);
	Sign->SetRelativeLocation(FVector(40.f, 0.f, 260.f));
	Sign->SetHorizontalAlignment(EHTA_Center);
	Sign->SetWorldSize(90.f);
	Sign->SetTextRenderColor(FColor(255, 196, 64));
	Sign->SetText(FText::FromString(TEXT("CHIEF'S TRUCK PAD — LOCKED")));
}

void ASiltChiefLotGate::BeginPlay()
{
	Super::BeginPlay();

	UMaterialInterface* Paint = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SiltCounty/Materials/M_TruckPaint.M_TruckPaint"));
	if (!Paint)
	{
		Paint = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (Paint)
	{
		if (UMaterialInstanceDynamic* Metal = UMaterialInstanceDynamic::Create(Paint, this))
		{
			Metal->SetVectorParameterValue(TEXT("PaintColor"), FLinearColor(0.12f, 0.12f, 0.13f));
			Metal->SetScalarParameterValue(TEXT("Roughness"), 0.4f);
			if (BarLow)
			{
				BarLow->SetMaterial(0, Metal);
			}
			if (BarHigh)
			{
				BarHigh->SetMaterial(0, Metal);
			}
		}
	}
	if (UFont* Font = LoadObject<UFont>(nullptr, TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField")))
	{
		if (Sign)
		{
			Sign->SetFont(Font);
		}
	}
}

void ASiltChiefLotGate::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bOpen || !GetWorld())
	{
		return;
	}

	if (!bOpening)
	{
		const ASiltCountyGameState* GameState = GetWorld()->GetGameState<ASiltCountyGameState>();
		if (!GameState)
		{
			return;
		}
		const bool bUnlocked = GameState->IsChiefGarageUnlocked()
			|| GameState->GetCashCollected() >= SiltGarage::ChiefGarageUnlockPrice;
		if (!bUnlocked)
		{
			return;
		}

		bOpening = true;
		if (BarLow)
		{
			BarLow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (BarHigh)
		{
			BarHigh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (Sign)
		{
			Sign->SetText(FText::FromString(TEXT("CHIEF'S TRUCK PAD")));
			Sign->SetTextRenderColor(FColor(180, 220, 170));
		}
		UE_LOG(LogSiltCounty, Display, TEXT("Chief truck pad gate opened (cash $%.2f, unlocked %d)."), GameState->GetCashCollected(), GameState->IsChiefGarageUnlocked() ? 1 : 0);
	}

	OpenAlpha = FMath::Min(1.f, OpenAlpha + DeltaSeconds * 1.5f);
	if (Hinge)
	{
		Hinge->SetRelativeRotation(FRotator(0.f, OpenAlpha * 100.f, 0.f));
	}
	if (OpenAlpha >= 1.f)
	{
		bOpen = true;
		SetActorTickEnabled(false);
	}
}
