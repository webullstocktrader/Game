#include "SiltContractActor.h"

#include "Components/BoxComponent.h"
#include "Components/TextRenderComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Font.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "SiltCountyGameState.h"
#include "SiltTerrain.h"
#include "SiltTruckPawn.h"
#include "UObject/ConstructorHelpers.h"

ASiltContractActor::ASiltContractActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;
	NetUpdateFrequency = 30.f;
	Tags.Add(TEXT("SiltCounty"));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
	UStaticMesh* Cylinder = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;

	Van = CreateDefaultSubobject<UBoxComponent>(TEXT("Van"));
	SetRootComponent(Van);
	Van->SetBoxExtent(FVector(130.f, 58.f, 45.f));
	Van->SetSimulatePhysics(false);
	Van->SetEnableGravity(false);
	Van->SetCollisionProfileName(TEXT("PhysicsActor"));
	Van->SetIsReplicated(true);
	Van->SetMassOverrideInKg(NAME_None, 1900.f, true);
	Van->SetLinearDamping(1.4f);
	Van->SetAngularDamping(2.5f);

	VanBody = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VanBody"));
	VanBody->SetupAttachment(Van);
	VanBody->SetStaticMesh(Cube);
	VanBody->SetRelativeScale3D(FVector(2.6f, 1.16f, 0.9f));
	VanBody->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Cabin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cabin"));
	Cabin->SetupAttachment(Van);
	Cabin->SetStaticMesh(Cube);
	Cabin->SetRelativeLocation(FVector(55.f, 0.f, 70.f));
	Cabin->SetRelativeScale3D(FVector(1.15f, 1.05f, 0.7f));
	Cabin->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Person = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Person"));
	Person->SetupAttachment(Van);
	Person->SetStaticMesh(Cylinder);
	Person->SetRelativeLocation(FVector(-10.f, 0.f, 150.f));
	Person->SetRelativeScale3D(FVector(0.32f, 0.32f, 0.85f));
	Person->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HelpText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("HelpText"));
	HelpText->SetupAttachment(Van);
	HelpText->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	HelpText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	HelpText->SetHorizontalAlignment(EHTA_Center);
	HelpText->SetWorldSize(48.f);
	HelpText->SetText(FText::FromString(TEXT("HELP")));
	HelpText->SetTextRenderColor(FColor(255, 220, 80));
}

void ASiltContractActor::BeginPlay()
{
	Super::BeginPlay();
	ApplyPhysicsRole();
	RefreshText();

	UMaterialInterface* Paint = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SiltCounty/Materials/M_TruckPaint.M_TruckPaint"));
	if (!Paint)
	{
		Paint = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}
	if (Paint)
	{
		if (UMaterialInstanceDynamic* VanMid = UMaterialInstanceDynamic::Create(Paint, this))
		{
			VanMid->SetVectorParameterValue(TEXT("PaintColor"), FLinearColor(0.05f, 0.12f, 0.28f));
			VanMid->SetScalarParameterValue(TEXT("Roughness"), 0.4f);
			VanBody->SetMaterial(0, VanMid);
			Cabin->SetMaterial(0, VanMid);
		}
		if (UMaterialInstanceDynamic* PersonMid = UMaterialInstanceDynamic::Create(Paint, this))
		{
			PersonMid->SetVectorParameterValue(TEXT("PaintColor"), FLinearColor(0.55f, 0.18f, 0.08f));
			PersonMid->SetScalarParameterValue(TEXT("Roughness"), 0.7f);
			Person->SetMaterial(0, PersonMid);
		}
	}
	if (UFont* Font = LoadObject<UFont>(nullptr, TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField")))
	{
		HelpText->SetFont(Font);
	}
}

void ASiltContractActor::ApplyPhysicsRole()
{
	if (!Van)
	{
		return;
	}
	const bool bSimulate = HasAuthority() && State != ESiltContractState::Complete;
	Van->SetSimulatePhysics(bSimulate);
	Van->SetEnableGravity(bSimulate);
	if (bSimulate)
	{
		Van->WakeAllRigidBodies();
	}
}

void ASiltContractActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASiltContractActor, State);
	DOREPLIFETIME(ASiltContractActor, HookedTruck);
}

void ASiltContractActor::OnRep_State()
{
	RefreshText();
	ApplyPhysicsRole();
}

void ASiltContractActor::RefreshText() const
{
	if (!HelpText)
	{
		return;
	}
	switch (State)
	{
	case ESiltContractState::Towing:
		HelpText->SetText(FText::FromString(TEXT("HOOKED")));
		break;
	case ESiltContractState::Complete:
		HelpText->SetText(FText::FromString(TEXT("THANKS")));
		break;
	default:
		HelpText->SetText(FText::FromString(TEXT("HELP")));
		break;
	}
}

void ASiltContractActor::TryHook(ASiltTruckPawn* Truck)
{
	if (!HasAuthority() || !Truck || State != ESiltContractState::Available)
	{
		return;
	}
	if (FVector::Dist(Truck->GetActorLocation(), GetActorLocation()) > HookRange)
	{
		return;
	}
	HookedTruck = Truck;
	State = ESiltContractState::Towing;
	RefreshText();
	if (Van)
	{
		Van->WakeAllRigidBodies();
	}
	ForceNetUpdate();
}

void ASiltContractActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!HasAuthority())
	{
		if (State == ESiltContractState::Towing && HookedTruck)
		{
			DrawDebugLine(GetWorld(), HookedTruck->GetHitchWorld(), GetActorLocation(), FColor(196, 150, 64), false, -1.f, 0, 5.f);
		}
		return;
	}

	if (State == ESiltContractState::Locked)
	{
		if (const ASiltCountyGameState* GameState = GetWorld()->GetGameState<ASiltCountyGameState>())
		{
			if (GameState->IsGameplay())
			{
				State = ESiltContractState::Available;
				RefreshText();
				ForceNetUpdate();
			}
		}
		return;
	}

	if (State == ESiltContractState::Towing)
	{
		StepTow(DeltaSeconds);
		TryComplete();
		if (HookedTruck)
		{
			DrawDebugLine(GetWorld(), HookedTruck->GetHitchWorld(), GetActorLocation(), FColor(196, 150, 64), false, -1.f, 0, 5.f);
		}
	}
}

void ASiltContractActor::StepTow(float DeltaSeconds)
{
	if (!HookedTruck || !Van || !Van->IsSimulatingPhysics())
	{
		return;
	}

	const FVector Hitch = HookedTruck->GetHitchWorld();
	const FVector Anchor = Hitch - HookedTruck->GetActorForwardVector() * 620.f;
	const FVector Delta = Anchor - Van->GetComponentLocation();
	const float Distance = Delta.Size();
	if (Distance > 40.f)
	{
		const FVector Direction = Delta / Distance;
		const float Stretch = FMath::Max(0.f, Distance - 180.f);
		FVector Force = Direction * Stretch * 6500.f - Van->GetComponentVelocity() * 700.f;
		Force = Force.GetClampedToMaxSize(2200000.f);
		Van->AddForce(Force);
	}

	const ESiltSurface Surface = SiltTerrain::SampleSurface(GetActorLocation().X, GetActorLocation().Y);
	const float Drag = (Surface == ESiltSurface::Asphalt || Surface == ESiltSurface::Road || Surface == ESiltSurface::Gravel) ? 350.f : (Surface == ESiltSurface::DeepMud || Surface == ESiltSurface::Water) ? 3200.f : 1400.f;
	Van->AddForce(-Van->GetComponentVelocity() * Drag);
	(void)DeltaSeconds;
}

void ASiltContractActor::TryComplete()
{
	const FVector Drop = SiltTerrain::GetDropZone();
	if (FVector::Dist2D(GetActorLocation(), Drop) > 1600.f)
	{
		return;
	}
	if (GetVelocity().Size() > 250.f)
	{
		return;
	}

	State = ESiltContractState::Complete;
	HookedTruck = nullptr;
	if (Van)
	{
		Van->SetSimulatePhysics(false);
		Van->SetEnableGravity(false);
		SetActorLocationAndRotation(Drop + FVector(0.f, 0.f, 80.f), FRotator(0.f, 90.f, 0.f), false, nullptr, ETeleportType::TeleportPhysics);
	}
	RefreshText();
	ForceNetUpdate();
}
