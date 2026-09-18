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

void AValleyVillager::Arm(const vg::Villager& Sim)
{
	VillagerId = Sim.Id;
	bWoman = Sim.Body == vg::Sex::Female;
	BuildBody(Sim);
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
	Comp->SetCastShadow(true);
	if (Mat)
	{
		Comp->SetMaterial(0, Mat);
	}
	Comp->SetupAttachment(GetRootComponent());
	Comp->RegisterComponent();
	Parts.Add(Comp);
	return Comp;
}

void AValleyVillager::BuildBody(const vg::Villager& Sim)
{
	UStaticMesh* Sphere = Valley::SphereMesh();
	UStaticMesh* Cyl = Valley::CylinderMesh();
	UStaticMesh* Cone = Valley::ConeMesh();
	if (!Sphere || !Cyl)
	{
		return;
	}

	struct FLook
	{
		float Height;
		float Shoulder;
		float Hip;
		float Torso;
		FLinearColor Skin;
		FLinearColor Cloth;
		FLinearColor Hair;
		int32 HairStyle;
		bool bBeard;
	};

	// Distinct adult looks. HairStyle: 0 crop, 1 bun, 2 long, 3 tied, 4 thick, 5 streak-bun.
	const FLook Looks[8] = {
		{ 0.96f, 0.34f, 0.40f, 0.30f, FLinearColor(0.50f, 0.34f, 0.22f), FLinearColor(0.42f, 0.24f, 0.08f), FLinearColor(0.07f, 0.04f, 0.02f), 0, false }, // Mara
		{ 1.00f, 0.36f, 0.46f, 0.34f, FLinearColor(0.62f, 0.42f, 0.28f), FLinearColor(0.38f, 0.12f, 0.08f), FLinearColor(0.12f, 0.06f, 0.02f), 2, false }, // Nima
		{ 0.93f, 0.32f, 0.38f, 0.28f, FLinearColor(0.28f, 0.16f, 0.10f), FLinearColor(0.16f, 0.14f, 0.12f), FLinearColor(0.22f, 0.20f, 0.18f), 5, false }, // Lira
		{ 1.04f, 0.35f, 0.44f, 0.32f, FLinearColor(0.22f, 0.12f, 0.08f), FLinearColor(0.18f, 0.10f, 0.05f), FLinearColor(0.03f, 0.02f, 0.015f), 4, false }, // Sable
		{ 1.08f, 0.50f, 0.34f, 0.42f, FLinearColor(0.42f, 0.28f, 0.16f), FLinearColor(0.14f, 0.09f, 0.05f), FLinearColor(0.05f, 0.03f, 0.02f), 0, true },  // Flint
		{ 1.12f, 0.56f, 0.40f, 0.50f, FLinearColor(0.55f, 0.40f, 0.28f), FLinearColor(0.10f, 0.08f, 0.06f), FLinearColor(0.10f, 0.08f, 0.06f), 4, true },  // Oak
		{ 1.02f, 0.42f, 0.32f, 0.34f, FLinearColor(0.36f, 0.26f, 0.16f), FLinearColor(0.16f, 0.18f, 0.10f), FLinearColor(0.08f, 0.05f, 0.03f), 3, false }, // Reed
		{ 1.05f, 0.48f, 0.38f, 0.44f, FLinearColor(0.48f, 0.30f, 0.20f), FLinearColor(0.28f, 0.10f, 0.05f), FLinearColor(0.06f, 0.03f, 0.02f), 0, false }  // Bram
	};

	const int32 Idx = FMath::Clamp(Sim.Id, 0, 7);
	const FLook& L = Looks[Idx];
	HeightScale = L.Height;
	SetActorScale3D(FVector(HeightScale));

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

	UMaterialInterface* SkinUse = Tint(Valley::Material(TEXT("M_SkinWarm")), L.Skin, TEXT("SkinDyn"));
	UMaterialInterface* ClothUse = Tint(Valley::Material(TEXT("M_ClothOchre")), L.Cloth, TEXT("ClothDyn"));
	UMaterialInterface* HairUse = Tint(Valley::Material(TEXT("M_Hair")), L.Hair, TEXT("HairDyn"));
	UMaterialInterface* HideUse = Tint(Valley::Material(TEXT("M_Hide")), L.Cloth * 0.85f, TEXT("HideDyn"));

	const bool bF = bWoman;
	const float Shoulder = L.Shoulder;
	const float Hip = L.Hip;
	const float Torso = L.Torso;

	// Hide tunic covers torso. Female: fitted bodice + longer skirt. Male: longer straight tunic.
	if (bF)
	{
		AddPart(TEXT("Skirt"), Cyl, FVector(0.f, 0.f, 46.f), FRotator::ZeroRotator, FVector(Hip, Hip * 0.78f, 0.72f), HideUse);
		if (Cone)
		{
			AddPart(TEXT("SkirtFlare"), Cone, FVector(0.f, 0.f, 28.f), FRotator(180.f, 0.f, 0.f), FVector(Hip * 1.15f, Hip * 0.9f, 0.35f), HideUse);
		}
		AddPart(TEXT("Bodice"), Cyl, FVector(0.f, 0.f, 96.f), FRotator::ZeroRotator, FVector(Torso * 0.95f, Torso * 0.72f, 0.52f), ClothUse);
		// Clothed chest volume — same hide as the wrap, not skin.
		AddPart(TEXT("WrapChest"), Sphere, FVector(4.f, 0.f, 108.f), FRotator::ZeroRotator, FVector(0.28f, 0.38f, 0.18f), ClothUse);
		AddPart(TEXT("Waist"), Cyl, FVector(0.f, 0.f, 72.f), FRotator::ZeroRotator, FVector(Torso * 0.72f, Torso * 0.62f, 0.18f), ClothUse);
	}
	else
	{
		AddPart(TEXT("Tunic"), Cyl, FVector(0.f, 0.f, 78.f), FRotator::ZeroRotator, FVector(Torso, Torso * 0.62f, 0.95f), ClothUse);
		AddPart(TEXT("Belt"), Cyl, FVector(0.f, 0.f, 58.f), FRotator::ZeroRotator, FVector(Torso * 1.05f, Torso * 0.68f, 0.08f), HideUse);
		AddPart(TEXT("ShoulderL"), Sphere, FVector(2.f, -Shoulder * 28.f, 118.f), FRotator::ZeroRotator, FVector(0.16f, 0.18f, 0.14f), ClothUse);
		AddPart(TEXT("ShoulderR"), Sphere, FVector(2.f, Shoulder * 28.f, 118.f), FRotator::ZeroRotator, FVector(0.16f, 0.18f, 0.14f), ClothUse);
	}

	AddPart(TEXT("Pelvis"), Cyl, FVector(0.f, 0.f, 36.f), FRotator::ZeroRotator, FVector(Hip * 0.85f, Hip * 0.7f, 0.22f), HideUse);
	AddPart(TEXT("Neck"), Cyl, FVector(0.f, 0.f, 128.f), FRotator::ZeroRotator, FVector(bF ? 0.11f : 0.14f, bF ? 0.11f : 0.14f, 0.16f), SkinUse);
	AddPart(TEXT("Head"), Sphere, FVector(0.f, 0.f, 148.f), FRotator::ZeroRotator, FVector(bF ? 0.30f : 0.33f, bF ? 0.26f : 0.29f, bF ? 0.32f : 0.34f), SkinUse);
	AddPart(TEXT("FootL"), Sphere, FVector(4.f, -Hip * 18.f, 6.f), FRotator::ZeroRotator, FVector(0.14f, 0.08f, 0.06f), HideUse);
	AddPart(TEXT("FootR"), Sphere, FVector(4.f, Hip * 18.f, 6.f), FRotator::ZeroRotator, FVector(0.14f, 0.08f, 0.06f), HideUse);

	const float LegX = Hip * 16.f;
	AddPart(TEXT("LegL"), Cyl, FVector(0.f, -LegX, 18.f), FRotator::ZeroRotator, FVector(bF ? 0.10f : 0.13f, bF ? 0.10f : 0.13f, 0.34f), SkinUse);
	AddPart(TEXT("LegR"), Cyl, FVector(0.f, LegX, 18.f), FRotator::ZeroRotator, FVector(bF ? 0.10f : 0.13f, bF ? 0.10f : 0.13f, 0.34f), SkinUse);
	ArmL = AddPart(TEXT("ArmL"), Cyl, FVector(2.f, -Shoulder * 38.f, 100.f), FRotator(12.f, 0.f, 8.f), FVector(bF ? 0.07f : 0.10f, bF ? 0.07f : 0.10f, 0.38f), SkinUse);
	ArmR = AddPart(TEXT("ArmR"), Cyl, FVector(2.f, Shoulder * 38.f, 100.f), FRotator(-12.f, 0.f, -8.f), FVector(bF ? 0.07f : 0.10f, bF ? 0.07f : 0.10f, 0.38f), SkinUse);

	// Hair — readable from a fly camera.
	switch (L.HairStyle)
	{
	case 1: // bun
		AddPart(TEXT("Hair"), Sphere, FVector(-6.f, 0.f, 162.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 0.20f), HairUse);
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 158.f), FRotator::ZeroRotator, FVector(0.28f, 0.26f, 0.12f), HairUse);
		break;
	case 2: // long
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 158.f), FRotator::ZeroRotator, FVector(0.30f, 0.28f, 0.14f), HairUse);
		AddPart(TEXT("HairL"), Cyl, FVector(-8.f, -8.f, 128.f), FRotator(12.f, 0.f, 0.f), FVector(0.08f, 0.08f, 0.45f), HairUse);
		AddPart(TEXT("HairR"), Cyl, FVector(-8.f, 8.f, 128.f), FRotator(12.f, 0.f, 0.f), FVector(0.08f, 0.08f, 0.45f), HairUse);
		break;
	case 3: // tied
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 156.f), FRotator::ZeroRotator, FVector(0.26f, 0.24f, 0.10f), HairUse);
		AddPart(TEXT("HairTail"), Cyl, FVector(-14.f, 0.f, 140.f), FRotator(55.f, 0.f, 0.f), FVector(0.07f, 0.07f, 0.28f), HairUse);
		break;
	case 4: // thick
		AddPart(TEXT("Hair"), Sphere, FVector(-4.f, 0.f, 160.f), FRotator::ZeroRotator, FVector(0.34f, 0.32f, 0.22f), HairUse);
		break;
	case 5: // streak-bun
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 158.f), FRotator::ZeroRotator, FVector(0.28f, 0.26f, 0.12f), HairUse);
		AddPart(TEXT("Bun"), Sphere, FVector(-10.f, 0.f, 164.f), FRotator::ZeroRotator, FVector(0.18f, 0.18f, 0.16f), HairUse);
		break;
	default: // crop
		AddPart(TEXT("Hair"), Sphere, FVector(-2.f, 0.f, 158.f), FRotator::ZeroRotator, FVector(0.26f, 0.24f, 0.10f), HairUse);
		break;
	}

	if (L.bBeard)
	{
		AddPart(TEXT("Beard"), Sphere, FVector(8.f, 0.f, 138.f), FRotator::ZeroRotator, FVector(0.16f, 0.18f, 0.14f), HairUse);
	}

	if (Cone && Sim.Role == vg::Habit::Hunter)
	{
		AddPart(TEXT("Spear"), Cyl, FVector(18.f, Shoulder * 42.f, 90.f), FRotator(8.f, 0.f, 12.f), FVector(0.03f, 0.03f, 1.1f), Valley::Material(TEXT("M_Wood")));
	}

	Speech = NewObject<UTextRenderComponent>(this, TEXT("Speech"));
	Speech->SetText(FText::GetEmpty());
	Speech->SetWorldSize(14.f);
	Speech->SetTextRenderColor(FColor(250, 236, 210));
	Speech->SetHorizontalAlignment(EHTA_Center);
	Speech->SetVerticalAlignment(EVRTA_TextBottom);
	Speech->SetRelativeLocation(FVector(0.f, 0.f, 210.f));
	Speech->SetupAttachment(GetRootComponent());
	Speech->RegisterComponent();

	Nameplate = NewObject<UTextRenderComponent>(this, TEXT("Nameplate"));
	Nameplate->SetText(FText::FromString(UTF8_TO_TCHAR(Sim.Name)));
	Nameplate->SetWorldSize(11.f);
	Nameplate->SetTextRenderColor(FColor(210, 190, 140));
	Nameplate->SetHorizontalAlignment(EHTA_Center);
	Nameplate->SetVerticalAlignment(EVRTA_TextBottom);
	Nameplate->SetRelativeLocation(FVector(0.f, 0.f, 188.f));
	Nameplate->SetupAttachment(GetRootComponent());
	Nameplate->RegisterComponent();
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
			ArmL->SetRelativeRotation(FRotator(18.f * FMath::Sin(WalkPhase), 0.f, 8.f));
			ArmR->SetRelativeRotation(FRotator(-18.f * FMath::Sin(WalkPhase), 0.f, -8.f));
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
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC || !PC->PlayerCameraManager)
	{
		return;
	}
	const FVector Cam = PC->PlayerCameraManager->GetCameraLocation();
	auto Face = [&](UTextRenderComponent* Text)
	{
		if (!Text || !Text->IsVisible())
		{
			return;
		}
		Text->SetWorldRotation((Cam - Text->GetComponentLocation()).Rotation());
	};
	Face(Speech);
	Face(Nameplate);
}
