#include "ValleyVillager.h"
#include "ValleyTypes.h"
#include "ValleyTerrain.h"
#include "Sim/ValleySim.h"
#include "Sim/ValleyPalette.h"
#include "Components/CapsuleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"

AValleyVillager::AValleyVillager()
{
	PrimaryActorTick.bCanEverTick = true;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	ClickProbe = CreateDefaultSubobject<UCapsuleComponent>(TEXT("ClickProbe"));
	ClickProbe->SetupAttachment(Root);
	ClickProbe->InitCapsuleSize(36.f, 92.f);
	ClickProbe->SetRelativeLocation(FVector(0.f, 0.f, 92.f));
	ClickProbe->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	ClickProbe->SetCollisionResponseToAllChannels(ECR_Ignore);
	ClickProbe->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ClickProbe->SetGenerateOverlapEvents(false);
	ClickProbe->SetHiddenInGame(true);
	ClickProbe->SetCanEverAffectNavigation(false);
}

void AValleyVillager::Arm(const vg::Villager& Sim, UClass* PresentationClass)
{
	VillagerId = Sim.Id;
	bWoman = Sim.Body == vg::Sex::Female;
	AttachLabels(Sim);
	if (PresentationClass)
	{
		SpawnPresentation(PresentationClass);
	}
	if (!Presentation)
	{
		BuildBody(Sim);
	}
}

void AValleyVillager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Presentation)
	{
		Presentation->Destroy();
		Presentation = nullptr;
	}
	Super::EndPlay(EndPlayReason);
}

void AValleyVillager::SpawnPresentation(UClass* PresentationClass)
{
	if (!PresentationClass || !GetWorld())
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AActor* Spawned = GetWorld()->SpawnActor<AActor>(PresentationClass, GetActorTransform(), Params);
	if (!Spawned)
	{
		UE_LOG(LogTemp, Warning, TEXT("Valley God: Mara MetaHuman spawn failed for %s"), *PresentationClass->GetName());
		return;
	}

	if (USceneComponent* SpawnRoot = Spawned->GetRootComponent())
	{
		SpawnRoot->SetMobility(EComponentMobility::Movable);
	}

	ACharacter* Character = Cast<ACharacter>(Spawned);
	float FeetToCenter = 0.f;
	if (Character)
	{
		if (UCharacterMovementComponent* Move = Character->GetCharacterMovement())
		{
			Move->DisableMovement();
			Move->SetComponentTickEnabled(false);
		}
		if (UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
		{
			Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			FeetToCenter = Capsule->GetScaledCapsuleHalfHeight();
		}
	}

	TArray<UPrimitiveComponent*> Prims;
	Spawned->GetComponents<UPrimitiveComponent>(Prims, true);
	for (UPrimitiveComponent* Prim : Prims)
	{
		if (Prim)
		{
			Prim->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}

	const bool bAttached = Spawned->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	if (!bAttached)
	{
		UE_LOG(LogTemp, Warning, TEXT("Valley God: Mara MetaHuman attach failed for %s; using procedural body"), *PresentationClass->GetName());
		Spawned->Destroy();
		return;
	}

	Spawned->SetActorRelativeRotation(FRotator::ZeroRotator);
	Spawned->SetActorRelativeScale3D(FVector::OneVector);
	Spawned->SetActorRelativeLocation(FVector(0.f, 0.f, FeetToCenter));
	Presentation = Spawned;
	UE_LOG(LogTemp, Display, TEXT("Valley God: %s using MetaHuman %s"), *GetName(), *PresentationClass->GetName());
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

	const vg::PersonLook& L = vg::PersonLookAt(Sim.Id);
	bWoman = L.Woman;
	HeightScale = L.Height;
	SetActorScale3D(FVector(HeightScale));

	const FLinearColor Skin(L.SkinR, L.SkinG, L.SkinB);
	const FLinearColor Cloth(L.ClothR, L.ClothG, L.ClothB);
	const FLinearColor Hair(L.HairR, L.HairG, L.HairB);
	const FLinearColor Hide = Cloth * 0.82f + FLinearColor(0.06f, 0.04f, 0.02f);

	UMaterialInterface* SkinUse = Valley::Tint(this, Valley::Material(TEXT("M_SkinWarm")), Skin, TEXT("SkinDyn"));
	UMaterialInterface* ClothUse = Valley::Tint(this, Valley::Material(TEXT("M_ClothOchre")), Cloth, TEXT("ClothDyn"));
	UMaterialInterface* HairUse = Valley::Tint(this, Valley::Material(TEXT("M_Hair")), Hair, TEXT("HairDyn"));
	UMaterialInterface* HideUse = Valley::Tint(this, Valley::Material(TEXT("M_Hide")), Hide, TEXT("HideDyn"));
	UMaterialInterface* EyeUse = Valley::Material(TEXT("M_Eye"));
	UMaterialInterface* WoodUse = Valley::Material(TEXT("M_Wood"));

	const bool bF = bWoman;
	const float Shoulder = L.Shoulder;
	const float Hip = L.Hip;
	const float Torso = L.Torso;
	const float HeadS = L.Head;
	const float LegX = Hip * 18.f;
	const float ArmY = Shoulder * 40.f;

	// Bare legs under the tunic so the body is not one capsule.
	AddPart(TEXT("FootL"), Sphere, FVector(6.f, -LegX, 7.f), FRotator::ZeroRotator, FVector(0.16f, 0.09f, 0.06f), HideUse);
	AddPart(TEXT("FootR"), Sphere, FVector(6.f, LegX, 7.f), FRotator::ZeroRotator, FVector(0.16f, 0.09f, 0.06f), HideUse);
	AddPart(TEXT("CalfL"), Cyl, FVector(1.f, -LegX, 26.f), FRotator::ZeroRotator, FVector(bF ? 0.10f : 0.12f, bF ? 0.10f : 0.12f, 0.30f), SkinUse);
	AddPart(TEXT("CalfR"), Cyl, FVector(1.f, LegX, 26.f), FRotator::ZeroRotator, FVector(bF ? 0.10f : 0.12f, bF ? 0.10f : 0.12f, 0.30f), SkinUse);
	ThighL = AddPart(TEXT("ThighL"), Cyl, FVector(0.f, -LegX, 58.f), FRotator::ZeroRotator, FVector(bF ? 0.13f : 0.15f, bF ? 0.13f : 0.15f, 0.32f), SkinUse);
	ThighR = AddPart(TEXT("ThighR"), Cyl, FVector(0.f, LegX, 58.f), FRotator::ZeroRotator, FVector(bF ? 0.13f : 0.15f, bF ? 0.13f : 0.15f, 0.32f), SkinUse);

	AddPart(TEXT("Pelvis"), Cyl, FVector(0.f, 0.f, 82.f), FRotator::ZeroRotator, FVector(Hip * 0.92f, Hip * 0.72f, 0.16f), HideUse);
	AddPart(TEXT("Abdomen"), Cyl, FVector(0.f, 0.f, 98.f), FRotator::ZeroRotator, FVector(Torso * 0.78f, Torso * 0.58f, 0.22f), SkinUse);
	AddPart(TEXT("Chest"), Cyl, FVector(1.f, 0.f, 118.f), FRotator::ZeroRotator, FVector(Torso * 0.95f, Torso * 0.62f, 0.24f), SkinUse);

	if (bF)
	{
		AddPart(TEXT("Skirt"), Cyl, FVector(0.f, 0.f, 52.f), FRotator::ZeroRotator, FVector(Hip * 1.02f, Hip * 0.82f, 0.58f), HideUse);
		if (Cone)
		{
			AddPart(TEXT("SkirtHem"), Cone, FVector(0.f, 0.f, 28.f), FRotator(180.f, 0.f, 0.f), FVector(Hip * 1.18f, Hip * 0.95f, 0.28f), HideUse);
		}
		AddPart(TEXT("Bodice"), Cyl, FVector(0.f, 0.f, 112.f), FRotator::ZeroRotator, FVector(Torso * 1.02f, Torso * 0.72f, 0.38f), ClothUse);
		AddPart(TEXT("Wrap"), Cyl, FVector(2.f, 0.f, 108.f), FRotator(8.f, 0.f, 0.f), FVector(Torso * 1.08f, Torso * 0.78f, 0.12f), HideUse);
		AddPart(TEXT("Waist"), Cyl, FVector(0.f, 0.f, 84.f), FRotator::ZeroRotator, FVector(Torso * 0.95f, Torso * 0.7f, 0.08f), ClothUse);
	}
	else
	{
		AddPart(TEXT("Tunic"), Cyl, FVector(0.f, 0.f, 88.f), FRotator::ZeroRotator, FVector(Torso * 1.05f, Torso * 0.68f, 0.72f), ClothUse);
		AddPart(TEXT("Belt"), Cyl, FVector(0.f, 0.f, 64.f), FRotator::ZeroRotator, FVector(Torso * 1.12f, Torso * 0.74f, 0.07f), HideUse);
		AddPart(TEXT("ShoulderL"), Sphere, FVector(2.f, -ArmY * 0.72f, 132.f), FRotator::ZeroRotator, FVector(0.18f, 0.16f, 0.14f), ClothUse);
		AddPart(TEXT("ShoulderR"), Sphere, FVector(2.f, ArmY * 0.72f, 132.f), FRotator::ZeroRotator, FVector(0.18f, 0.16f, 0.14f), ClothUse);
	}

	AddPart(TEXT("Neck"), Cyl, FVector(0.f, 0.f, 138.f), FRotator::ZeroRotator, FVector(bF ? 0.11f : 0.13f, bF ? 0.11f : 0.13f, 0.14f), SkinUse);
	AddPart(TEXT("Head"), Sphere, FVector(2.f, 0.f, 156.f), FRotator::ZeroRotator, FVector(HeadS, HeadS * 0.88f, HeadS * 1.05f), SkinUse);
	AddPart(TEXT("Jaw"), Sphere, FVector(8.f, 0.f, 146.f), FRotator::ZeroRotator, FVector(HeadS * 0.55f, HeadS * 0.62f, HeadS * 0.38f), SkinUse);
	AddPart(TEXT("Nose"), Sphere, FVector(16.f, 0.f, 156.f), FRotator::ZeroRotator, FVector(0.06f, 0.045f, 0.05f), SkinUse);
	AddPart(TEXT("EyeL"), Sphere, FVector(14.f, -7.f, 160.f), FRotator::ZeroRotator, FVector(0.045f, 0.035f, 0.03f), EyeUse);
	AddPart(TEXT("EyeR"), Sphere, FVector(14.f, 7.f, 160.f), FRotator::ZeroRotator, FVector(0.045f, 0.035f, 0.03f), EyeUse);

	ArmL = AddPart(TEXT("ArmL"), Cyl, FVector(3.f, -ArmY, 118.f), FRotator(12.f, 0.f, 10.f), FVector(bF ? 0.08f : 0.11f, bF ? 0.08f : 0.11f, 0.28f), SkinUse);
	ArmR = AddPart(TEXT("ArmR"), Cyl, FVector(3.f, ArmY, 118.f), FRotator(-12.f, 0.f, -10.f), FVector(bF ? 0.08f : 0.11f, bF ? 0.08f : 0.11f, 0.28f), SkinUse);
	AddPart(TEXT("ForeL"), Cyl, FVector(8.f, -ArmY - 4.f, 92.f), FRotator(18.f, 0.f, 8.f), FVector(bF ? 0.07f : 0.09f, bF ? 0.07f : 0.09f, 0.24f), SkinUse);
	AddPart(TEXT("ForeR"), Cyl, FVector(8.f, ArmY + 4.f, 92.f), FRotator(-18.f, 0.f, -8.f), FVector(bF ? 0.07f : 0.09f, bF ? 0.07f : 0.09f, 0.24f), SkinUse);
	AddPart(TEXT("HandL"), Sphere, FVector(14.f, -ArmY - 8.f, 74.f), FRotator::ZeroRotator, FVector(0.08f, 0.06f, 0.05f), SkinUse);
	AddPart(TEXT("HandR"), Sphere, FVector(14.f, ArmY + 8.f, 74.f), FRotator::ZeroRotator, FVector(0.08f, 0.06f, 0.05f), SkinUse);

	switch (L.HairStyle)
	{
	case 1: // bun
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 168.f), FRotator::ZeroRotator, FVector(HeadS * 0.95f, HeadS * 0.9f, 0.14f), HairUse);
		AddPart(TEXT("HairBun"), Sphere, FVector(-10.f, 0.f, 174.f), FRotator::ZeroRotator, FVector(0.20f, 0.20f, 0.18f), HairUse);
		break;
	case 2: // long
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 168.f), FRotator::ZeroRotator, FVector(HeadS * 1.02f, HeadS * 0.95f, 0.16f), HairUse);
		AddPart(TEXT("HairL"), Cyl, FVector(-10.f, -9.f, 132.f), FRotator(14.f, 0.f, 0.f), FVector(0.08f, 0.08f, 0.52f), HairUse);
		AddPart(TEXT("HairR"), Cyl, FVector(-10.f, 9.f, 132.f), FRotator(14.f, 0.f, 0.f), FVector(0.08f, 0.08f, 0.52f), HairUse);
		break;
	case 3: // tied
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 166.f), FRotator::ZeroRotator, FVector(HeadS * 0.9f, HeadS * 0.85f, 0.12f), HairUse);
		AddPart(TEXT("HairTail"), Cyl, FVector(-16.f, 0.f, 146.f), FRotator(50.f, 0.f, 0.f), FVector(0.07f, 0.07f, 0.32f), HairUse);
		break;
	case 4: // thick
		AddPart(TEXT("Hair"), Sphere, FVector(-4.f, 0.f, 170.f), FRotator::ZeroRotator, FVector(HeadS * 1.12f, HeadS * 1.05f, 0.24f), HairUse);
		break;
	case 5: // streak-bun
		AddPart(TEXT("HairCap"), Sphere, FVector(-2.f, 0.f, 168.f), FRotator::ZeroRotator, FVector(HeadS * 0.95f, HeadS * 0.9f, 0.14f), HairUse);
		AddPart(TEXT("Bun"), Sphere, FVector(-12.f, 0.f, 176.f), FRotator::ZeroRotator, FVector(0.18f, 0.18f, 0.16f), HairUse);
		AddPart(TEXT("Streak"), Cyl, FVector(4.f, 10.f, 164.f), FRotator(70.f, 20.f, 0.f), FVector(0.04f, 0.04f, 0.16f), HairUse);
		break;
	default: // crop
		AddPart(TEXT("Hair"), Sphere, FVector(-2.f, 0.f, 168.f), FRotator::ZeroRotator, FVector(HeadS * 0.92f, HeadS * 0.86f, 0.12f), HairUse);
		break;
	}

	if (L.Beard)
	{
		AddPart(TEXT("Beard"), Sphere, FVector(10.f, 0.f, 142.f), FRotator::ZeroRotator, FVector(0.16f, 0.18f, 0.14f), HairUse);
	}

	if (Cone && Sim.Role == vg::Habit::Hunter)
	{
		AddPart(TEXT("Spear"), Cyl, FVector(18.f, ArmY + 6.f, 96.f), FRotator(8.f, 0.f, 12.f), FVector(0.035f, 0.035f, 1.15f), WoodUse);
		AddPart(TEXT("SpearTip"), Cone ? Cone : Cyl, FVector(22.f, ArmY + 8.f, 154.f), FRotator(8.f, 0.f, 12.f), FVector(0.06f, 0.06f, 0.14f), Valley::Material(TEXT("M_Stone")));
	}
}

void AValleyVillager::AttachLabels(const vg::Villager& Sim)
{
	if (Speech && Nameplate)
	{
		Nameplate->SetText(FText::FromString(UTF8_TO_TCHAR(Sim.Name)));
		return;
	}

	Speech = NewObject<UTextRenderComponent>(this, TEXT("Speech"));
	Speech->SetText(FText::GetEmpty());
	Speech->SetWorldSize(14.f);
	Speech->SetTextRenderColor(FColor(250, 236, 210));
	Speech->SetHorizontalAlignment(EHTA_Center);
	Speech->SetVerticalAlignment(EVRTA_TextBottom);
	Speech->SetRelativeLocation(FVector(0.f, 0.f, 220.f));
	Speech->SetupAttachment(GetRootComponent());
	Speech->RegisterComponent();

	Nameplate = NewObject<UTextRenderComponent>(this, TEXT("Nameplate"));
	Nameplate->SetText(FText::FromString(UTF8_TO_TCHAR(Sim.Name)));
	Nameplate->SetWorldSize(11.f);
	Nameplate->SetTextRenderColor(FColor(210, 190, 140));
	Nameplate->SetHorizontalAlignment(EHTA_Center);
	Nameplate->SetVerticalAlignment(EVRTA_TextBottom);
	Nameplate->SetRelativeLocation(FVector(0.f, 0.f, 198.f));
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

	if (bMoving && !Presentation)
	{
		WalkPhase = WorldTime * (Sim.Current == vg::Activity::Panic ? 14.f : 8.f);
		Loc.Z += FMath::Abs(FMath::Sin(WalkPhase)) * 6.f;
		if (ArmL && ArmR)
		{
			ArmL->SetRelativeRotation(FRotator(18.f * FMath::Sin(WalkPhase), 0.f, 10.f));
			ArmR->SetRelativeRotation(FRotator(-18.f * FMath::Sin(WalkPhase), 0.f, -10.f));
		}
		if (ThighL && ThighR)
		{
			ThighL->SetRelativeRotation(FRotator(-12.f * FMath::Sin(WalkPhase), 0.f, 0.f));
			ThighR->SetRelativeRotation(FRotator(12.f * FMath::Sin(WalkPhase), 0.f, 0.f));
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
