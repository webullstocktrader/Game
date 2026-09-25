#include "SiltTruckPawn.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Net/UnrealNetwork.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "SiltContractActor.h"
#include "SiltCountyGameState.h"
#include "SiltTerrain.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	constexpr float TruckMassKg = 3800.f;
	constexpr float WheelRadius = 48.f;
	constexpr float RestAxleDrop = 28.f;
	constexpr float SuspensionTravel = 24.f;
	constexpr float TraceLength = RestAxleDrop + SuspensionTravel + WheelRadius + 30.f;
	constexpr float SteerDegrees = 32.f;

	const FVector WheelMounts[4] = {
		FVector(165.f, -100.f, -40.f),
		FVector(165.f, 100.f, -40.f),
		FVector(-170.f, -100.f, -40.f),
		FVector(-170.f, 100.f, -40.f)
	};

	void SurfaceTuning(ESiltSurface Surface, float& Grip, float& Drag, float& SpringScale, float& MaxSpeed)
	{
		switch (Surface)
		{
		case ESiltSurface::Road:
			Grip = 1.2f; Drag = 0.045f; SpringScale = 1.f; MaxSpeed = 2300.f; break;
		case ESiltSurface::Dirt:
			Grip = 0.82f; Drag = 0.16f; SpringScale = 0.9f; MaxSpeed = 1500.f; break;
		case ESiltSurface::Mud:
			// Softer than dirt (0.9) but still holds the axle. Was 0.5, which let the truck fall through.
			Grip = 0.45f; Drag = 0.85f; SpringScale = 0.82f; MaxSpeed = 750.f; break;
		case ESiltSurface::DeepMud:
			Grip = 0.28f; Drag = 1.6f; SpringScale = 0.68f; MaxSpeed = 480.f; break;
		case ESiltSurface::Water:
		default:
			Grip = 0.14f; Drag = 1.7f; SpringScale = 0.38f; MaxSpeed = 320.f; break;
		}
	}

	int32 SurfaceRank(ESiltSurface Surface)
	{
		switch (Surface)
		{
		case ESiltSurface::Water: return 4;
		case ESiltSurface::DeepMud: return 3;
		case ESiltSurface::Mud: return 2;
		case ESiltSurface::Dirt: return 1;
		default: return 0;
		}
	}
}

ASiltTruckPawn::ASiltTruckPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	NetUpdateFrequency = 45.f;
	MinNetUpdateFrequency = 20.f;
	bAlwaysRelevant = true;
	AutoPossessAI = EAutoPossessAI::Disabled;
	bUseControllerRotationYaw = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderFinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cube = CubeFinder.Succeeded() ? CubeFinder.Object : nullptr;
	UStaticMesh* Cylinder = CylinderFinder.Succeeded() ? CylinderFinder.Object : nullptr;

	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	SetRootComponent(Body);
	Body->SetBoxExtent(FVector(250.f, 105.f, 40.f));
	Body->SetSimulatePhysics(false);
	Body->SetEnableGravity(false);
	Body->SetCollisionProfileName(TEXT("PhysicsActor"));
	Body->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Body->SetIsReplicated(true);
	Body->SetMassOverrideInKg(NAME_None, TruckMassKg, true);
	Body->SetLinearDamping(0.08f);
	Body->SetAngularDamping(1.1f);
	Body->SetCenterOfMass(FVector(0.f, 0.f, -25.f));
	Body->SetNotifyRigidBodyCollision(true);

	VisualBody = MakeVisual(TEXT("VisualBody"), Cube, FVector::ZeroVector, FVector(5.0f, 2.1f, 0.8f), FRotator::ZeroRotator);
	Cab = MakeVisual(TEXT("Cab"), Cube, FVector(30.f, 0.f, 85.f), FVector(1.7f, 1.95f, 1.15f), FRotator::ZeroRotator);
	Hood = MakeVisual(TEXT("Hood"), Cube, FVector(175.f, 0.f, 18.f), FVector(1.45f, 2.0f, 0.32f), FRotator::ZeroRotator);
	Bed = MakeVisual(TEXT("Bed"), Cube, FVector(-80.f, 0.f, 48.f), FVector(2.3f, 2.0f, 0.28f), FRotator::ZeroRotator);
	Bumper = MakeVisual(TEXT("Bumper"), Cube, FVector(248.f, 0.f, -8.f), FVector(0.28f, 2.15f, 0.32f), FRotator::ZeroRotator);
	Stack = MakeVisual(TEXT("Stack"), Cylinder, FVector(-30.f, 78.f, 130.f), FVector(0.16f, 0.16f, 1.3f), FRotator::ZeroRotator);
	LightBar = MakeVisual(TEXT("LightBar"), Cube, FVector(10.f, 0.f, 155.f), FVector(0.18f, 1.15f, 0.12f), FRotator::ZeroRotator);

	WheelFL = MakeVisual(TEXT("WheelFL"), Cylinder, WheelMounts[0], FVector(0.96f, 0.96f, 0.42f), FRotator(0.f, 0.f, 90.f));
	WheelFR = MakeVisual(TEXT("WheelFR"), Cylinder, WheelMounts[1], FVector(0.96f, 0.96f, 0.42f), FRotator(0.f, 0.f, 90.f));
	WheelRL = MakeVisual(TEXT("WheelRL"), Cylinder, WheelMounts[2], FVector(0.96f, 0.96f, 0.42f), FRotator(0.f, 0.f, 90.f));
	WheelRR = MakeVisual(TEXT("WheelRR"), Cylinder, WheelMounts[3], FVector(0.96f, 0.96f, 0.42f), FRotator(0.f, 0.f, 90.f));
	Wheels[0] = WheelFL;
	Wheels[1] = WheelFR;
	Wheels[2] = WheelRL;
	Wheels[3] = WheelRR;

	NameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameText"));
	NameText->SetupAttachment(Body);
	NameText->SetRelativeLocation(FVector(-20.f, 0.f, 190.f));
	NameText->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	NameText->SetHorizontalAlignment(EHTA_Center);
	NameText->SetWorldSize(42.f);
	NameText->SetTextRenderColor(FColor::White);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Body);
	SpringArm->SetRelativeLocation(FVector(-20.f, 0.f, 90.f));
	SpringArm->SetRelativeRotation(FRotator(-16.f, 0.f, 0.f));
	SpringArm->TargetArmLength = 820.f;
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeSize = 18.f;
	SpringArm->bInheritPitch = false;
	SpringArm->bInheritYaw = true;
	SpringArm->bInheritRoll = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 8.f;
	SpringArm->CameraLagMaxDistance = 140.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->SetFieldOfView(78.f);

	auto MakeHeadlight = [this](const FName& Name, float Y) -> USpotLightComponent*
	{
		USpotLightComponent* Spot = CreateDefaultSubobject<USpotLightComponent>(Name);
		Spot->SetupAttachment(Body);
		Spot->SetRelativeLocation(FVector(250.f, Y, 15.f));
		Spot->SetRelativeRotation(FRotator(-6.f, 0.f, 0.f));
		Spot->SetIntensity(18000.f);
		Spot->SetAttenuationRadius(7000.f);
		Spot->SetInnerConeAngle(12.f);
		Spot->SetOuterConeAngle(28.f);
		Spot->SetLightColor(FLinearColor(1.f, 0.92f, 0.75f));
		Spot->SetCastShadows(false);
		Spot->VolumetricScatteringIntensity = 2.5f;
		return Spot;
	};
	HeadlightL = MakeHeadlight(TEXT("HeadlightL"), -55.f);
	HeadlightR = MakeHeadlight(TEXT("HeadlightR"), 55.f);
}

UStaticMeshComponent* ASiltTruckPawn::MakeVisual(const FName& Name, UStaticMesh* Mesh, const FVector& RelativeLocation, const FVector& Scale, const FRotator& RelativeRotation)
{
	UStaticMeshComponent* Visual = CreateDefaultSubobject<UStaticMeshComponent>(Name);
	Visual->SetupAttachment(Body);
	Visual->SetStaticMesh(Mesh);
	Visual->SetRelativeLocation(RelativeLocation);
	Visual->SetRelativeScale3D(Scale);
	Visual->SetRelativeRotation(RelativeRotation);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetCastShadow(true);
	return Visual;
}

void ASiltTruckPawn::ConfigureIdentity(int32 InIndex, const FString& InName, const FLinearColor& InPaint)
{
	TruckIndex = InIndex;
	DriverName = InName;
	PaintColor = InPaint;
	if (HasActorBegunPlay())
	{
		ApplyCosmetics();
	}
}

void ASiltTruckPawn::BeginPlay()
{
	Super::BeginPlay();
	ApplyPhysicsRole();
	ApplyCosmetics();

	if (HasAuthority() && Body)
	{
		UPhysicalMaterial* MudHull = NewObject<UPhysicalMaterial>(this, TEXT("MudHull"));
		MudHull->Friction = 0.4f;
		MudHull->Restitution = 0.f;
		MudHull->FrictionCombineMode = EFrictionCombineMode::Min;
		Body->SetPhysMaterialOverride(MudHull);
	}
}

void ASiltTruckPawn::ApplyPhysicsRole()
{
	if (!Body)
	{
		return;
	}
	const bool bSimulate = HasAuthority();
	Body->SetSimulatePhysics(bSimulate);
	Body->SetEnableGravity(bSimulate);
	if (bSimulate)
	{
		Body->WakeAllRigidBodies();
	}
}

void ASiltTruckPawn::ApplyCosmetics()
{
	UMaterialInterface* PaintBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/SiltCounty/Materials/M_TruckPaint.M_TruckPaint"));
	if (!PaintBase)
	{
		PaintBase = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	}

	UMaterialInstanceDynamic* BodyMid = PaintBase ? UMaterialInstanceDynamic::Create(PaintBase, this) : nullptr;
	if (BodyMid)
	{
		BodyMid->SetVectorParameterValue(TEXT("PaintColor"), PaintColor);
		BodyMid->SetScalarParameterValue(TEXT("Roughness"), 0.34f);
	}
	UMaterialInstanceDynamic* DarkMid = PaintBase ? UMaterialInstanceDynamic::Create(PaintBase, this) : nullptr;
	if (DarkMid)
	{
		DarkMid->SetVectorParameterValue(TEXT("PaintColor"), FLinearColor(0.015f, 0.015f, 0.015f));
		DarkMid->SetScalarParameterValue(TEXT("Roughness"), 0.62f);
	}
	UMaterialInstanceDynamic* TrimMid = PaintBase ? UMaterialInstanceDynamic::Create(PaintBase, this) : nullptr;
	if (TrimMid)
	{
		TrimMid->SetVectorParameterValue(TEXT("PaintColor"), FLinearColor(0.08f, 0.08f, 0.075f));
		TrimMid->SetScalarParameterValue(TEXT("Roughness"), 0.45f);
	}

	UStaticMeshComponent* PaintedParts[] = { VisualBody, Cab, Hood, Bed, Bumper, Stack, LightBar };
	for (UStaticMeshComponent* Part : PaintedParts)
	{
		if (Part && BodyMid)
		{
			Part->SetMaterial(0, BodyMid);
		}
	}
	if (Bumper && TrimMid)
	{
		Bumper->SetMaterial(0, TrimMid);
	}
	for (UStaticMeshComponent* Wheel : Wheels)
	{
		if (Wheel && DarkMid)
		{
			Wheel->SetMaterial(0, DarkMid);
		}
	}

	if (Stack)
	{
		Stack->SetVisibility(TruckIndex == 1);
	}
	if (LightBar)
	{
		LightBar->SetVisibility(TruckIndex == 0);
	}
	if (NameText)
	{
		NameText->SetText(FText::FromString(DriverName.IsEmpty() ? TEXT("TRUCK") : DriverName));
		if (UFont* Font = LoadObject<UFont>(nullptr, TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField")))
		{
			NameText->SetFont(Font);
		}
	}
}

void ASiltTruckPawn::OnRep_Identity()
{
	ApplyCosmetics();
}

void ASiltTruckPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASiltTruckPawn, TruckIndex);
	DOREPLIFETIME(ASiltTruckPawn, DriverName);
	DOREPLIFETIME(ASiltTruckPawn, PaintColor);
	DOREPLIFETIME(ASiltTruckPawn, NetSteer);
}

void ASiltTruckPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis(TEXT("SiltThrottle"), this, &ASiltTruckPawn::InputThrottle);
	PlayerInputComponent->BindAxis(TEXT("SiltSteer"), this, &ASiltTruckPawn::InputSteer);
	PlayerInputComponent->BindAxis(TEXT("SiltBrake"), this, &ASiltTruckPawn::InputBrake);
	PlayerInputComponent->BindAxis(TEXT("SiltLookYaw"), this, &ASiltTruckPawn::InputLookYaw);
	PlayerInputComponent->BindAxis(TEXT("SiltLookPitch"), this, &ASiltTruckPawn::InputLookPitch);
	PlayerInputComponent->BindAction(TEXT("SiltHandbrake"), IE_Pressed, this, &ASiltTruckPawn::InputHandbrakePressed);
	PlayerInputComponent->BindAction(TEXT("SiltHandbrake"), IE_Released, this, &ASiltTruckPawn::InputHandbrakeReleased);
	PlayerInputComponent->BindAction(TEXT("SiltReset"), IE_Pressed, this, &ASiltTruckPawn::InputReset);
	PlayerInputComponent->BindAction(TEXT("SiltHook"), IE_Pressed, this, &ASiltTruckPawn::InputHook);
}

void ASiltTruckPawn::InputThrottle(float Value) { ThrottleInput = FMath::Clamp(Value, -1.f, 1.f); }
void ASiltTruckPawn::InputSteer(float Value) { SteerInput = FMath::Clamp(Value, -1.f, 1.f); }
void ASiltTruckPawn::InputBrake(float Value) { BrakeInput = FMath::Clamp(Value, 0.f, 1.f); }
void ASiltTruckPawn::InputHandbrakePressed() { bHandbrakeInput = true; }
void ASiltTruckPawn::InputHandbrakeReleased() { bHandbrakeInput = false; }

void ASiltTruckPawn::InputLookYaw(float Value)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->IsInputKeyDown(EKeys::RightMouseButton))
		{
			CameraYawOffset = FMath::Clamp(CameraYawOffset + Value, -140.f, 140.f);
		}
		else
		{
			const float Pad = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightX);
			if (FMath::Abs(Pad) > 0.08f)
			{
				CameraYawOffset = FMath::Clamp(CameraYawOffset + Pad * 80.f * GetWorld()->GetDeltaSeconds(), -140.f, 140.f);
			}
		}
	}
}

void ASiltTruckPawn::InputLookPitch(float Value)
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->IsInputKeyDown(EKeys::RightMouseButton))
		{
			CameraPitchOffset = FMath::Clamp(CameraPitchOffset + Value, -35.f, 18.f);
		}
		else
		{
			const float Pad = PC->GetInputAnalogKeyState(EKeys::Gamepad_RightY);
			if (FMath::Abs(Pad) > 0.08f)
			{
				CameraPitchOffset = FMath::Clamp(CameraPitchOffset + Pad * -50.f * GetWorld()->GetDeltaSeconds(), -35.f, 18.f);
			}
		}
	}
}

void ASiltTruckPawn::InputReset()
{
	if (HasAuthority())
	{
		ResetUpright();
	}
	else
	{
		ServerReset();
	}
}

void ASiltTruckPawn::InputHook()
{
	if (HasAuthority())
	{
		ServerHook_Implementation();
	}
	else
	{
		ServerHook();
	}
}

void ASiltTruckPawn::ServerDrive_Implementation(float Throttle, float Steer, float Brake, bool bHandbrake)
{
	ThrottleInput = FMath::Clamp(Throttle, -1.f, 1.f);
	SteerInput = FMath::Clamp(Steer, -1.f, 1.f);
	BrakeInput = FMath::Clamp(Brake, 0.f, 1.f);
	bHandbrakeInput = bHandbrake;
}

void ASiltTruckPawn::ServerReset_Implementation()
{
	ResetUpright();
}

void ASiltTruckPawn::ServerHook_Implementation()
{
	if (!GetWorld())
	{
		return;
	}
	for (TActorIterator<ASiltContractActor> It(GetWorld()); It; ++It)
	{
		It->TryHook(this);
		break;
	}
}

bool ASiltTruckPawn::IsDrivingAllowed() const
{
	const UWorld* World = GetWorld();
	const ASiltCountyGameState* GameState = World ? World->GetGameState<ASiltCountyGameState>() : nullptr;
	return GameState && GameState->IsGameplay() && GetController() != nullptr;
}

FVector ASiltTruckPawn::GetHitchWorld() const
{
	return GetActorTransform().TransformPosition(FVector(-300.f, 0.f, 15.f));
}

float ASiltTruckPawn::GetSpeedKmh() const
{
	const float CmPerSecond = Body ? Body->GetComponentVelocity().Size() : GetVelocity().Size();
	return CmPerSecond * 0.036f;
}

void ASiltTruckPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DeltaSeconds = FMath::Min(DeltaSeconds, 0.05f);

	if (IsLocallyControlled() && !HasAuthority())
	{
		ServerDrive(ThrottleInput, SteerInput, BrakeInput, bHandbrakeInput);
	}

	TArray<FWheelQuery, TInlineAllocator<4>> WheelHits;
	QueryWheels(WheelHits);

	if (HasAuthority())
	{
		StepVehicle(DeltaSeconds, WheelHits);
		if (GetActorLocation().Z < -2500.f)
		{
			ResetUpright();
		}
	}

	UpdateWheelVisuals(DeltaSeconds, WheelHits);

	const bool bLookStick =
		(Cast<APlayerController>(GetController()) &&
		 (FMath::Abs(Cast<APlayerController>(GetController())->GetInputAnalogKeyState(EKeys::Gamepad_RightX)) > 0.08f ||
		  Cast<APlayerController>(GetController())->IsInputKeyDown(EKeys::RightMouseButton)));
	if (!bLookStick)
	{
		CameraYawOffset = FMath::FInterpTo(CameraYawOffset, 0.f, DeltaSeconds, 2.5f);
		CameraPitchOffset = FMath::FInterpTo(CameraPitchOffset, 0.f, DeltaSeconds, 2.5f);
	}
	if (SpringArm)
	{
		SpringArm->SetRelativeRotation(FRotator(-16.f + CameraPitchOffset, CameraYawOffset, 0.f));
	}
	if (Camera && IsLocallyControlled())
	{
		Camera->SetFieldOfView(FMath::Lerp(78.f, 88.f, FMath::Clamp(GetSpeedKmh() / 70.f, 0.f, 1.f)));
	}
}

void ASiltTruckPawn::QueryWheels(TArray<FWheelQuery, TInlineAllocator<4>>& OutWheels) const
{
	OutWheels.Reset();
	if (!Body || !GetWorld())
	{
		return;
	}

	const FTransform BodyXform = Body->GetComponentTransform();
	const FVector Up = Body->GetUpVector();
	const FVector Down = -Up;
	const float SteerAngle = (IsLocallyControlled() ? SteerInput : NetSteer) * SteerDegrees;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(SiltWheel), false, this);

	for (int32 Index = 0; Index < 4; ++Index)
	{
		FWheelQuery Query;
		Query.bFront = Index < 2;
		Query.MountWorld = BodyXform.TransformPosition(WheelMounts[Index]);
		const FRotator SteerRot(0.f, Query.bFront ? SteerAngle : 0.f, 0.f);
		Query.WheelForward = BodyXform.TransformVectorNoScale(SteerRot.RotateVector(FVector::ForwardVector)).GetSafeNormal();
		Query.WheelRight = BodyXform.TransformVectorNoScale(SteerRot.RotateVector(FVector::RightVector)).GetSafeNormal();

		FHitResult Hit;
		Query.bGrounded = GetWorld()->LineTraceSingleByChannel(
			Hit,
			Query.MountWorld,
			Query.MountWorld + Down * TraceLength,
			ECC_WorldStatic,
			Params);
		if (Query.bGrounded)
		{
			Query.HitNormal = Hit.ImpactNormal;
			Query.AxleDrop = FMath::Clamp(Hit.Distance - WheelRadius, 0.f, RestAxleDrop + SuspensionTravel);
			Query.Surface = SiltTerrain::SampleSurface(Hit.ImpactPoint.X, Hit.ImpactPoint.Y);
			Query.PointVelocity = Body->IsSimulatingPhysics()
				? Body->GetPhysicsLinearVelocityAtPoint(Query.MountWorld)
				: Body->GetComponentVelocity();
		}
		else
		{
			Query.AxleDrop = RestAxleDrop + SuspensionTravel;
			Query.Surface = SiltTerrain::SampleSurface(Query.MountWorld.X, Query.MountWorld.Y);
		}
		OutWheels.Add(Query);
	}
}

void ASiltTruckPawn::StepVehicle(float DeltaSeconds, const TArray<FWheelQuery, TInlineAllocator<4>>& WheelHits)
{
	if (!Body || !Body->IsSimulatingPhysics())
	{
		return;
	}

	const bool bDrive = IsDrivingAllowed();
	const float Throttle = bDrive ? ThrottleInput : 0.f;
	const float Brake = bDrive ? BrakeInput : 1.f;
	const bool bHandbrake = bDrive ? bHandbrakeInput : true;
	NetSteer = bDrive ? SteerInput : 0.f;

	const float Weight = TruckMassKg * 980.f;
	const float WeightPerWheel = Weight / 4.f;
	const float SpringK = WeightPerWheel / 12.f;
	const float Damper = 2400.f;
	const FVector Up = Body->GetUpVector();

	ESiltSurface Worst = ESiltSurface::Road;
	float SinkSum = 0.f;
	int32 Grounded = 0;

	for (const FWheelQuery& Wheel : WheelHits)
	{
		if (!Wheel.bGrounded)
		{
			continue;
		}
		++Grounded;
		if (SurfaceRank(Wheel.Surface) > SurfaceRank(Worst))
		{
			Worst = Wheel.Surface;
		}

		float Grip = 1.f;
		float Drag = 0.1f;
		float SpringScale = 1.f;
		float MaxSpeed = 1000.f;
		SurfaceTuning(Wheel.Surface, Grip, Drag, SpringScale, MaxSpeed);
		if (bHandbrake)
		{
			Grip *= 0.35f;
		}

		const float ExtraSink = FMath::Max(0.f, RestAxleDrop - Wheel.AxleDrop);
		SinkSum += FMath::Clamp(ExtraSink / 20.f, 0.f, 1.f);

		const float Error = RestAxleDrop - Wheel.AxleDrop;
		const float VelUp = FVector::DotProduct(Wheel.PointVelocity, Up);
		float SpringForce = (WeightPerWheel + Error * SpringK) * SpringScale - VelUp * Damper;
		SpringForce = FMath::Clamp(SpringForce, 0.f, WeightPerWheel * 3.5f);
		Body->AddForceAtLocation(Up * SpringForce, Wheel.MountWorld);

		if (Wheel.Surface == ESiltSurface::DeepMud || Wheel.Surface == ESiltSurface::Mud)
		{
			// Shallow settle only. 0.45 / 0.18 plus the soft spring crushed the axle through the mud.
			Body->AddForceAtLocation(-Up * WeightPerWheel * (Wheel.Surface == ESiltSurface::DeepMud ? 0.10f : 0.05f), Wheel.MountWorld);
		}

		const float ForwardSpeed = FVector::DotProduct(Wheel.PointVelocity, Wheel.WheelForward);
		const float EnginePerWheel = 300000.f;
		float Drive = 0.f;
		if (Throttle > 0.05f && ForwardSpeed < MaxSpeed)
		{
			Drive = Throttle * EnginePerWheel * Grip;
		}
		else if (Throttle < -0.05f && ForwardSpeed > -MaxSpeed * 0.45f)
		{
			Drive = Throttle * EnginePerWheel * Grip * (ForwardSpeed > 80.f ? 1.f : 0.55f);
		}

		if (Brake > 0.05f || bHandbrake)
		{
			const float BrakeScale = (bHandbrake ? 3.2f : 2.f) * FMath::Max(Brake, bHandbrake ? 1.f : Brake);
			Body->AddForceAtLocation(-Wheel.WheelForward * FMath::Sign(ForwardSpeed) * FMath::Abs(ForwardSpeed) * 900.f * BrakeScale, Wheel.MountWorld);
			Drive = 0.f;
		}

		const float MaxFriction = WeightPerWheel * FMath::Max(Grip, 0.05f) * 1.15f;
		Drive = FMath::Clamp(Drive, -MaxFriction, MaxFriction);
		Body->AddForceAtLocation(Wheel.WheelForward * Drive, Wheel.MountWorld);

		const float LateralSpeed = FVector::DotProduct(Wheel.PointVelocity, Wheel.WheelRight);
		const float Lateral = FMath::Clamp(-LateralSpeed * 8500.f * FMath::Max(Grip, 0.05f), -MaxFriction, MaxFriction);
		Body->AddForceAtLocation(Wheel.WheelRight * Lateral, Wheel.MountWorld);

		const float PlanarSpeed = Wheel.PointVelocity.Size2D();
		if (PlanarSpeed > 30.f)
		{
			const FVector DragDir = FVector(Wheel.PointVelocity.X, Wheel.PointVelocity.Y, 0.f) / PlanarSpeed;
			FVector DragForce = -DragDir * PlanarSpeed * PlanarSpeed * Drag;
			DragForce = DragForce.GetClampedToMaxSize(WeightPerWheel * 2.f);
			Body->AddForceAtLocation(DragForce, Wheel.MountWorld);
		}
	}

	CurrentSurface = Grounded > 0 ? Worst : SiltTerrain::SampleSurface(GetActorLocation().X, GetActorLocation().Y);
	SinkAlpha = Grounded > 0 ? SinkSum / static_cast<float>(Grounded) : 0.f;

	const float Depth = SiltTerrain::GetWaterLevel() - Body->GetComponentLocation().Z + 40.f;
	if (Depth > 0.f)
	{
		Body->AddForce(FVector::UpVector * FMath::Min(Depth, 220.f) * 18000.f);
		Body->AddForce(-Body->GetComponentVelocity() * 900.f);
		if (CurrentSurface == ESiltSurface::Road || CurrentSurface == ESiltSurface::Dirt)
		{
			CurrentSurface = ESiltSurface::Water;
		}
	}

	if (Grounded == 0)
	{
		Body->SetLinearDamping(0.2f);
	}
	else if (CurrentSurface == ESiltSurface::DeepMud || CurrentSurface == ESiltSurface::Water)
	{
		Body->SetLinearDamping(0.85f);
		Body->SetAngularDamping(2.4f);
	}
	else if (CurrentSurface == ESiltSurface::Mud)
	{
		Body->SetLinearDamping(0.45f);
		Body->SetAngularDamping(1.6f);
	}
	else
	{
		Body->SetLinearDamping(0.08f);
		Body->SetAngularDamping(1.05f);
	}

	(void)DeltaSeconds;
}

void ASiltTruckPawn::UpdateWheelVisuals(float DeltaSeconds, const TArray<FWheelQuery, TInlineAllocator<4>>& WheelHits)
{
	if (!Body)
	{
		return;
	}

	const float ForwardSpeed = FVector::DotProduct(GetVelocity(), GetActorForwardVector());
	WheelSpin = FMath::Fmod(WheelSpin + (ForwardSpeed / WheelRadius) * FMath::RadiansToDegrees(DeltaSeconds), 360.f);
	const float SteerAngle = (IsLocallyControlled() ? SteerInput : NetSteer) * SteerDegrees;

	if (!HasAuthority())
	{
		ESiltSurface Worst = ESiltSurface::Road;
		float SinkSum = 0.f;
		int32 Grounded = 0;
		for (const FWheelQuery& Wheel : WheelHits)
		{
			if (!Wheel.bGrounded)
			{
				continue;
			}
			++Grounded;
			if (SurfaceRank(Wheel.Surface) > SurfaceRank(Worst))
			{
				Worst = Wheel.Surface;
			}
			SinkSum += FMath::Clamp(FMath::Max(0.f, RestAxleDrop - Wheel.AxleDrop) / 20.f, 0.f, 1.f);
		}
		if (Grounded > 0)
		{
			CurrentSurface = Worst;
			SinkAlpha = SinkSum / static_cast<float>(Grounded);
		}
	}

	const FTransform BodyXform = Body->GetComponentTransform();
	const FVector Down = -Body->GetUpVector();
	for (int32 Index = 0; Index < WheelHits.Num() && Index < 4; ++Index)
	{
		UStaticMeshComponent* Wheel = Wheels[Index];
		if (!Wheel)
		{
			continue;
		}
		const FWheelQuery& Query = WheelHits[Index];
		const FVector WorldWheel = Query.MountWorld + Down * Query.AxleDrop;
		Wheel->SetRelativeLocation(BodyXform.InverseTransformPosition(WorldWheel));
		const float Yaw = Query.bFront ? SteerAngle : 0.f;
		Wheel->SetRelativeRotation(FRotator(0.f, Yaw, 90.f + WheelSpin));
	}
}

void ASiltTruckPawn::ResetUpright()
{
	if (!HasAuthority() || !Body)
	{
		return;
	}
	const FVector Loc = GetActorLocation();
	const float Height = SiltTerrain::SampleHeight(Loc.X, Loc.Y);
	const FRotator Yaw(0.f, GetActorRotation().Yaw, 0.f);
	SetActorLocationAndRotation(FVector(Loc.X, Loc.Y, Height + 220.f), Yaw, false, nullptr, ETeleportType::TeleportPhysics);
	Body->SetPhysicsLinearVelocity(FVector::ZeroVector);
	Body->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
}
