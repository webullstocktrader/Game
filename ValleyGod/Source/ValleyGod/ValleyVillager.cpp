#include "ValleyVillager.h"
#include "ValleyTypes.h"
#include "ValleyTerrain.h"
#include "Sim/ValleySim.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"

AValleyVillager::AValleyVillager()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

void AValleyVillager::Arm(int32 InId, const FLinearColor& Skin, const FLinearColor& Cloth, const FLinearColor& Hair)
{
	VillagerId = InId;
	BuildBody(Skin, Cloth, Hair);
}

UStaticMeshComponent* AValleyVillager::AddPart(const FName& Name, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat)
{
	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
	Comp->SetStaticMesh(Mesh);
	Comp->SetRelativeLocation(Loc);
	Comp->SetRelativeRotation(Rot);
	Comp->SetRelativeScale3D(Scale);
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Comp->SetCollisionResponseToAllChannels(ECR_Ignore);
	Comp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	if (Mat)
	{
		Comp->SetMaterial(0, Mat);
	}
	Comp->SetupAttachment(GetRootComponent());
	Comp->RegisterComponent();
	Parts.Add(Comp);
	return Comp;
}

void AValleyVillager::BuildBody(const FLinearColor& Skin, const FLinearColor& Cloth, const FLinearColor& Hair)
{
	UStaticMesh* Sphere = Valley::SphereMesh();
	UStaticMesh* Cyl = Valley::CylinderMesh();
	if (!Sphere || !Cyl)
	{
		return;
	}

	UMaterialInterface* SkinMat = Valley::Material(TEXT("M_SkinWarm"));
	UMaterialInterface* ClothMat = Valley::Material(TEXT("M_ClothOchre"));
	UMaterialInterface* HairMat = Valley::Material(TEXT("M_Hair"));

	auto Tint = [this](UMaterialInterface* Parent, const FLinearColor& Color, const FName& Name) -> UMaterialInterface*
	{
		if (!Parent)
		{
			return Valley::FallbackMaterial();
		}
		UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(Parent, this, Name);
		if (Dyn)
		{
			Dyn->SetVectorParameterValue(TEXT("BaseColor"), Color);
			return Dyn;
		}
		return Parent;
	};

	UMaterialInterface* SkinUse = Tint(SkinMat, Skin, TEXT("SkinDyn"));
	UMaterialInterface* ClothUse = Tint(ClothMat, Cloth, TEXT("ClothDyn"));
	UMaterialInterface* HairUse = Tint(HairMat, Hair, TEXT("HairDyn"));

	AddPart(TEXT("Tunic"), Cyl, FVector(0.f, 0.f, 78.f), FRotator::ZeroRotator, FVector(0.42f, 0.32f, 0.72f), ClothUse);
	AddPart(TEXT("Pelvis"), Cyl, FVector(0.f, 0.f, 42.f), FRotator::ZeroRotator, FVector(0.34f, 0.26f, 0.28f), ClothUse);
	AddPart(TEXT("Head"), Sphere, FVector(0.f, 0.f, 128.f), FRotator::ZeroRotator, FVector(0.32f, 0.28f, 0.32f), SkinUse);
	AddPart(TEXT("Hair"), Sphere, FVector(-4.f, 0.f, 140.f), FRotator::ZeroRotator, FVector(0.28f, 0.26f, 0.16f), HairUse);
	AddPart(TEXT("LegL"), Cyl, FVector(0.f, -8.f, 18.f), FRotator::ZeroRotator, FVector(0.11f, 0.11f, 0.36f), SkinUse);
	AddPart(TEXT("LegR"), Cyl, FVector(0.f, 8.f, 18.f), FRotator::ZeroRotator, FVector(0.11f, 0.11f, 0.36f), SkinUse);
	ArmL = AddPart(TEXT("ArmL"), Cyl, FVector(0.f, -18.f, 88.f), FRotator(12.f, 0.f, 0.f), FVector(0.08f, 0.08f, 0.32f), SkinUse);
	ArmR = AddPart(TEXT("ArmR"), Cyl, FVector(0.f, 18.f, 88.f), FRotator(-12.f, 0.f, 0.f), FVector(0.08f, 0.08f, 0.32f), SkinUse);

	Speech = NewObject<UTextRenderComponent>(this, TEXT("Speech"));
	Speech->SetText(FText::GetEmpty());
	Speech->SetWorldSize(16.f);
	Speech->SetTextRenderColor(FColor(250, 236, 210));
	Speech->SetHorizontalAlignment(EHTA_Center);
	Speech->SetVerticalAlignment(EVRTA_TextBottom);
	Speech->SetRelativeLocation(FVector(0.f, 0.f, 190.f));
	Speech->SetupAttachment(GetRootComponent());
	Speech->RegisterComponent();
}

void AValleyVillager::SyncFromSim(const vg::Villager& Sim, AValleyTerrain* Terrain, float WorldTime)
{
	FVector Loc(Sim.X, Sim.Y, 0.f);
	if (Terrain)
	{
		Loc.Z = Terrain->HeightAt(Sim.X, Sim.Y);
	}

	const bool bSleep = Sim.Current == vg::Activity::Sleep;
	const bool bMoving = Sim.Current == vg::Activity::Walk || Sim.Current == vg::Activity::Hunt
		|| Sim.Current == vg::Activity::Panic || Sim.Current == vg::Activity::Shelter
		|| Sim.Current == vg::Activity::HighGround || Sim.Current == vg::Activity::Eat;

	if (bMoving)
	{
		WalkPhase = WorldTime * (Sim.Current == vg::Activity::Panic ? 14.f : 8.f);
		Loc.Z += FMath::Abs(FMath::Sin(WalkPhase)) * 6.f;
		if (ArmL && ArmR)
		{
			ArmL->SetRelativeRotation(FRotator(18.f * FMath::Sin(WalkPhase), 0.f, 0.f));
			ArmR->SetRelativeRotation(FRotator(-18.f * FMath::Sin(WalkPhase), 0.f, 0.f));
		}
	}

	if (bSleep)
	{
		SetActorLocation(Loc + FVector(0.f, 0.f, 18.f));
		SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(Sim.Heading), 78.f));
	}
	else
	{
		SetActorLocation(Loc);
		SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(Sim.Heading), 0.f));
	}

	if (Speech)
	{
		const FString Line = UTF8_TO_TCHAR(Sim.Speech ? Sim.Speech : "");
		Speech->SetText(Line.IsEmpty() ? FText::GetEmpty() : FText::FromString(Line));
		Speech->SetVisibility(!Line.IsEmpty());
	}
}

void AValleyVillager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Speech || !Speech->IsVisible())
	{
		return;
	}
	if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
	{
		if (PC->PlayerCameraManager)
		{
			const FVector Cam = PC->PlayerCameraManager->GetCameraLocation();
			const FVector ToCam = Cam - Speech->GetComponentLocation();
			Speech->SetWorldRotation(ToCam.Rotation());
		}
	}
}
