#include "SiltTruck.h"
#include "SiltTerrain.h"
#include "Camera/CameraComponent.h"
#include "Components/AudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "Sound/SoundWaveProcedural.h"
#include "EngineUtils.h"

namespace
{
	AActor* FindNearestAnchor(UWorld* World, const FVector& From, float MaxDist, AActor* Self, AActor* Partner)
	{
		AActor* Best = nullptr;
		float BestD = MaxDist;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor || Actor == Self)
			{
				continue;
			}
			const bool bTagged = Actor->ActorHasTag(Silt::TagAnchor()) || Actor == Partner;
			if (!bTagged)
			{
				continue;
			}
			const float D = FVector::Dist(From, Actor->GetActorLocation());
			if (D < BestD)
			{
				BestD = D;
				Best = Actor;
			}
		}
		return Best;
	}
}

ASiltTruck::ASiltTruck()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
	AutoPossessAI = EAutoPossessAI::Disabled;

	Chassis = CreateDefaultSubobject<UBoxComponent>(TEXT("Chassis"));
	SetRootComponent(Chassis);
	Chassis->SetBoxExtent(FVector(250.f, 110.f, 70.f));
	Chassis->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Chassis->SetCollisionResponseToAllChannels(ECR_Overlap);
	Chassis->SetGenerateOverlapEvents(true);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(Chassis);
	SpringArm->TargetArmLength = 820.f;
	SpringArm->SocketOffset = FVector(0.f, 0.f, 220.f);
	SpringArm->bDoCollisionTest = false;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 6.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 7.f;
	SpringArm->SetRelativeLocation(FVector(-40.f, 0.f, 40.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
	Camera->SetFieldOfView(72.f);

	EngineAudio = CreateDefaultSubobject<UAudioComponent>(TEXT("EngineAudio"));
	EngineAudio->SetupAttachment(Chassis);
	EngineAudio->bAutoActivate = false;
}

void ASiltTruck::BeginPlay()
{
	Super::BeginPlay();
	EngineWave = NewObject<USoundWaveProcedural>(this, TEXT("EngineWave"));
	EngineWave->SetSampleRate(22050);
	EngineWave->NumChannels = 1;
	EngineWave->Duration = 10000.f;
	EngineWave->bLooping = true;
	EngineWave->OnSoundWaveProceduralUnderflow.BindUObject(this, &ASiltTruck::OnEngineAudioNeeded);
	EngineAudio->SetSound(EngineWave);
	OnEngineAudioNeeded(EngineWave, 22050);
	EngineAudio->Play();
}

void ASiltTruck::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ASiltTruck::Configure(ESiltDriver InDriver, ASiltTerrain* InTerrain)
{
	Driver = InDriver;
	Terrain = InTerrain;
	Tags.Reset();
	Tags.Add(Driver == ESiltDriver::Chief ? Silt::TagChief() : Silt::TagGooch());
	BuildVisuals();
}

UStaticMeshComponent* ASiltTruck::AddPart(const FName& Name, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat)
{
	UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
	Comp->SetStaticMesh(Mesh);
	Comp->SetRelativeLocation(Loc);
	Comp->SetRelativeRotation(Rot);
	Comp->SetRelativeScale3D(Scale);
	Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Comp->SetCastShadow(true);
	if (Mat)
	{
		Comp->SetMaterial(0, Mat);
	}
	Comp->SetupAttachment(Chassis);
	Comp->RegisterComponent();
	return Comp;
}

void ASiltTruck::BuildVisuals()
{
	UStaticMesh* Cube = Silt::CubeMesh();
	UStaticMesh* Sphere = Silt::SphereMesh();
	UStaticMesh* Cyl = Silt::CylinderMesh();
	if (!Cube || !Cyl)
	{
		return;
	}

	const bool bChief = Driver == ESiltDriver::Chief;
	UMaterialInterface* Paint = Silt::Material(bChief ? TEXT("M_TruckChief") : TEXT("M_TruckGooch"));
	UMaterialInterface* Rubber = Silt::Material(TEXT("M_Rubber"));
	UMaterialInterface* Metal = Silt::Material(TEXT("M_Metal"));
	UMaterialInterface* Chrome = Silt::Material(TEXT("M_Chrome"));
	UMaterialInterface* Glass = Silt::Material(TEXT("M_Glass"));
	UMaterialInterface* Light = Silt::Material(TEXT("M_Emissive"));

	// Frame and bed — Chief is a crew-cab long bed, Gooch is a single-cab with a snorkel.
	const float CabLen = bChief ? 220.f : 170.f;
	AddPart(TEXT("Frame"), Cube, FVector(-10.f, 0.f, -8.f), FRotator::ZeroRotator, FVector(5.1f, 1.55f, 0.18f), Metal);
	AddPart(TEXT("Bed"), Cube, FVector(-145.f, 0.f, 28.f), FRotator::ZeroRotator, FVector(2.3f, 1.72f, 0.72f), Paint);
	AddPart(TEXT("BedRailL"), Cube, FVector(-145.f, -82.f, 58.f), FRotator::ZeroRotator, FVector(2.25f, 0.08f, 0.22f), Paint);
	AddPart(TEXT("BedRailR"), Cube, FVector(-145.f, 82.f, 58.f), FRotator::ZeroRotator, FVector(2.25f, 0.08f, 0.22f), Paint);
	AddPart(TEXT("Cab"), Cube, FVector(70.f, 0.f, 62.f), FRotator::ZeroRotator, FVector(CabLen * 0.01f, 1.68f, 1.15f), Paint);
	AddPart(TEXT("Hood"), Cube, FVector(195.f, 0.f, 38.f), FRotator(-6.f, 0.f, 0.f), FVector(1.35f, 1.6f, 0.55f), Paint);
	AddPart(TEXT("Roof"), Cube, FVector(65.f, 0.f, 128.f), FRotator::ZeroRotator, FVector(CabLen * 0.009f, 1.55f, 0.12f), Paint);
	AddPart(TEXT("Windshield"), Cube, FVector(145.f, 0.f, 105.f), FRotator(-28.f, 0.f, 0.f), FVector(0.08f, 1.5f, 0.7f), Glass);
	AddPart(TEXT("GlassL"), Cube, FVector(70.f, -84.f, 95.f), FRotator::ZeroRotator, FVector(1.4f, 0.04f, 0.55f), Glass);
	AddPart(TEXT("GlassR"), Cube, FVector(70.f, 84.f, 95.f), FRotator::ZeroRotator, FVector(1.4f, 0.04f, 0.55f), Glass);
	AddPart(TEXT("Bumper"), Cube, FVector(268.f, 0.f, 8.f), FRotator::ZeroRotator, FVector(0.28f, 1.85f, 0.28f), Chrome);
	AddPart(TEXT("BrushL"), Cube, FVector(275.f, -50.f, 28.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 0.55f), Chrome);
	AddPart(TEXT("BrushR"), Cube, FVector(275.f, 50.f, 28.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 0.55f), Chrome);
	AddPart(TEXT("Grille"), Cube, FVector(262.f, 0.f, 36.f), FRotator::ZeroRotator, FVector(0.06f, 1.15f, 0.42f), Chrome);
	AddPart(TEXT("HeadL"), Sphere, FVector(268.f, -58.f, 32.f), FRotator::ZeroRotator, FVector(0.22f, 0.18f, 0.18f), Light);
	AddPart(TEXT("HeadR"), Sphere, FVector(268.f, 58.f, 32.f), FRotator::ZeroRotator, FVector(0.22f, 0.18f, 0.18f), Light);
	AddPart(TEXT("TailL"), Cube, FVector(-262.f, -70.f, 40.f), FRotator::ZeroRotator, FVector(0.08f, 0.22f, 0.16f), Light);
	AddPart(TEXT("TailR"), Cube, FVector(-262.f, 70.f, 40.f), FRotator::ZeroRotator, FVector(0.08f, 0.22f, 0.16f), Light);
	AddPart(TEXT("MirrorL"), Cube, FVector(145.f, -108.f, 88.f), FRotator::ZeroRotator, FVector(0.18f, 0.28f, 0.16f), Chrome);
	AddPart(TEXT("MirrorR"), Cube, FVector(145.f, 108.f, 88.f), FRotator::ZeroRotator, FVector(0.18f, 0.28f, 0.16f), Chrome);
	AddPart(TEXT("WinchDrum"), Cyl, FVector(280.f, 0.f, 22.f), FRotator(0.f, 0.f, 90.f), FVector(0.28f, 0.28f, 0.35f), Metal);
	AddPart(TEXT("Exhaust"), Cyl, FVector(-40.f, 95.f, 70.f), FRotator(0.f, 0.f, 0.f), FVector(0.14f, 0.14f, 1.1f), Chrome);
	AddPart(TEXT("FlapL"), Cube, FVector(-230.f, -90.f, 10.f), FRotator::ZeroRotator, FVector(0.08f, 0.35f, 0.45f), Silt::Material(TEXT("M_Rubber")));
	AddPart(TEXT("FlapR"), Cube, FVector(-230.f, 90.f, 10.f), FRotator::ZeroRotator, FVector(0.08f, 0.35f, 0.45f), Silt::Material(TEXT("M_Rubber")));

	if (bChief)
	{
		AddPart(TEXT("LightBar"), Cube, FVector(60.f, 0.f, 142.f), FRotator::ZeroRotator, FVector(0.7f, 1.2f, 0.12f), Chrome);
		AddPart(TEXT("BarLightL"), Cube, FVector(60.f, -40.f, 148.f), FRotator::ZeroRotator, FVector(0.2f, 0.18f, 0.08f), Light);
		AddPart(TEXT("BarLightR"), Cube, FVector(60.f, 40.f, 148.f), FRotator::ZeroRotator, FVector(0.2f, 0.18f, 0.08f), Light);
	}
	else
	{
		AddPart(TEXT("Snorkel"), Cyl, FVector(150.f, -88.f, 90.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 1.3f), Paint);
		AddPart(TEXT("Antenna"), Cyl, FVector(-20.f, 80.f, 150.f), FRotator::ZeroRotator, FVector(0.04f, 0.04f, 1.6f), Metal);
	}

	DoorSign = NewObject<UTextRenderComponent>(this, TEXT("DoorSign"));
	DoorSign->SetText(FText::FromString(Silt::DriverName(Driver)));
	DoorSign->SetHorizontalAlignment(EHTA_Center);
	DoorSign->SetWorldSize(18.f);
	DoorSign->SetTextRenderColor(FColor(240, 220, 160));
	DoorSign->SetRelativeLocation(FVector(80.f, -86.f, 70.f));
	DoorSign->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	DoorSign->SetupAttachment(Chassis);
	DoorSign->RegisterComponent();

	CableMesh = AddPart(TEXT("Cable"), Cyl, FVector(280.f, 0.f, 22.f), FRotator::ZeroRotator, FVector(0.04f, 0.04f, 0.04f), Metal);
	CableMesh->SetVisibility(false);

	Wheels.SetNum(4);
	const FVector WheelLoc[4] = {
		FVector(175.f, -95.f, -18.f),
		FVector(175.f, 95.f, -18.f),
		FVector(-175.f, -95.f, -18.f),
		FVector(-175.f, 95.f, -18.f)
	};
	for (int32 I = 0; I < 4; ++I)
	{
		FSiltWheel& W = Wheels[I];
		W.LocalOffset = WheelLoc[I];
		W.bSteered = I < 2;
		W.RadiusCm = 42.f;
		const FName TireName(*FString::Printf(TEXT("Tire%d"), I));
		const FName RimName(*FString::Printf(TEXT("Rim%d"), I));
		W.Tire = AddPart(TireName, Cyl, WheelLoc[I], FRotator(0.f, 0.f, 90.f), FVector(0.84f, 0.84f, 0.28f), Rubber);
		W.Rim = AddPart(RimName, Cyl, WheelLoc[I], FRotator(0.f, 0.f, 90.f), FVector(0.5f, 0.5f, 0.3f), Chrome);
	}
}

void ASiltTruck::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Terrain)
	{
		return;
	}

	if (Input.bAirDown)
	{
		PSI = FMath::Max(12.f, PSI - 8.f * DeltaSeconds);
	}
	if (Input.bAirUp)
	{
		PSI = FMath::Min(36.f, PSI + 8.f * DeltaSeconds);
	}
	if (Input.bWinchToggle)
	{
		TryToggleWinch();
		Input.bWinchToggle = false;
	}
	if (Input.bInteract)
	{
		TryInteract();
		Input.bInteract = false;
	}

	if (Input.bReset)
	{
		ResetHold += DeltaSeconds;
		if (ResetHold > 1.2f)
		{
			ResetUpright();
			ResetHold = 0.f;
		}
	}
	else
	{
		ResetHold = 0.f;
	}

	const int32 Steps = FMath::Clamp(FMath::CeilToInt(DeltaSeconds / 0.012f), 1, 6);
	const float Sub = DeltaSeconds / Steps;
	for (int32 I = 0; I < Steps; ++I)
	{
		Simulate(Sub);
	}

	UpdateVisuals(DeltaSeconds);
	UpdateAudio(DeltaSeconds);

	if (CarriedCrate)
	{
		CarriedCrate->SetActorLocation(GetActorTransform().TransformPosition(FVector(-150.f, 0.f, 95.f)));
		CarriedCrate->SetActorRotation(GetActorRotation());
	}
}

void ASiltTruck::Simulate(float Dt)
{
	SimulateEngine(Dt);

	FVector Force = FVector(0.f, 0.f, MassKg * -980.f);
	FVector Torque = FVector::ZeroVector;
	IntegrateWheels(Dt, Force, Torque);
	SimulateWinch(Dt, Force);

	// Light air / splash drag so it still feels heavy.
	Force += -Velocity * (MassKg * 0.18f);

	const FVector Accel = Force / MassKg;
	Velocity += Accel * Dt;
	Velocity = Velocity.GetClampedToMaxSize(4200.f);

	const FVector Inertia(1.8e8f, 3.4e8f, 3.1e8f);
	AngularVel.X += (Torque.X / Inertia.X) * Dt;
	AngularVel.Y += (Torque.Y / Inertia.Y) * Dt;
	AngularVel.Z += (Torque.Z / Inertia.Z) * Dt;
	AngularVel *= FMath::Clamp(1.f - 1.8f * Dt, 0.2f, 1.f);

	FVector NewLoc = GetActorLocation() + Velocity * Dt;
	FQuat Q = GetActorQuat();
	const FVector DeltaAng = AngularVel * Dt;
	if (!DeltaAng.IsNearlyZero())
	{
		Q = FQuat(DeltaAng.GetSafeNormal(), DeltaAng.Size()) * Q;
		Q.Normalize();
	}
	SetActorLocationAndRotation(NewLoc, Q.Rotator());
}

void ASiltTruck::SimulateEngine(float Dt)
{
	const bool bReverse = Input.Throttle < -0.15f && FVector::DotProduct(Velocity, GetActorForwardVector()) < 120.f;
	if (bReverse)
	{
		Gear = ESiltGear::Reverse;
	}
	else if (FMath::Abs(Input.Throttle) < 0.04f && Velocity.Size() < 80.f && Input.Brake > 0.6f)
	{
		Gear = ESiltGear::Neutral;
	}
	else if (PSI <= 18.f || Wheels.Num() > 0 && Wheels[0].SinkCm > 18.f)
	{
		Gear = ESiltGear::Low;
	}
	else
	{
		Gear = ESiltGear::Drive;
	}

	const float TargetRPM = 750.f + FMath::Abs(Input.Throttle) * 3200.f + Velocity.Size() * 0.35f;
	RPM = FMath::FInterpTo(RPM, TargetRPM, Dt, 3.2f);
}

void ASiltTruck::IntegrateWheels(float Dt, FVector& OutForce, FVector& OutTorque)
{
	const FTransform XF = GetActorTransform();
	const FVector Forward = GetActorForwardVector();
	const FVector Right = GetActorRightVector();
	const float TargetSteer = Input.Steer * 34.f;
	SteerAngle = FMath::FInterpTo(SteerAngle, TargetSteer, Dt, 6.f);

	const float GearMul = (Gear == ESiltGear::Low) ? 1.85f : (Gear == ESiltGear::Reverse ? -0.7f : (Gear == ESiltGear::Neutral ? 0.f : 1.f));
	const float Drive = FMath::Clamp(Input.Throttle, -1.f, 1.f) * GearMul;
	const float MaxDrive = 780000.f;

	int32 Grounded = 0;
	for (FSiltWheel& W : Wheels)
	{
		const FVector WorldRest = XF.TransformPosition(W.LocalOffset);
		const FSiltGroundHit Hit = Terrain->Query(WorldRest);
		W.Surface = Hit.Surface;
		W.SinkCm = Hit.SinkCm;
		W.ContactNormal = Hit.Normal;
		W.SteerAngle = W.bSteered ? SteerAngle : 0.f;

		const float GroundZ = Hit.Position.Z + W.RadiusCm - Hit.SinkCm * 0.35f;
		const float Compression = GroundZ - WorldRest.Z;
		W.bGrounded = Compression > -W.RestLengthCm;
		if (!W.bGrounded)
		{
			W.Compression = 0.f;
			continue;
		}
		++Grounded;

		W.LastCompression = W.Compression;
		W.Compression = FMath::Clamp(Compression + W.RestLengthCm, 0.f, W.RestLengthCm + 18.f);
		const float CompVel = (W.Compression - W.LastCompression) / FMath::Max(Dt, 0.001f);
		const float SpringF = W.Spring * W.Compression + W.Damper * CompVel;
		const FVector SuspForce = Hit.Normal * SpringF;
		W.ContactPos = FVector(WorldRest.X, WorldRest.Y, Hit.Position.Z);

		const float Fz = FMath::Max(SpringF, 200.f);
		float Mu = 0.95f;
		switch (Hit.Surface)
		{
		case ESiltSurface::Pavement: Mu = 1.05f; break;
		case ESiltSurface::Gravel: Mu = 0.72f; break;
		case ESiltSurface::Grass: Mu = 0.5f; break;
		case ESiltSurface::Water: Mu = 0.12f; break;
		case ESiltSurface::Wood: Mu = 0.4f; break;
		case ESiltSurface::Mud:
		default:
			{
				const float AirDown = FMath::Clamp((28.f - PSI) / 16.f, -0.4f, 1.f);
				Mu = 0.20f + 0.22f * AirDown;
				Mu *= FMath::Clamp(1.f - Hit.SinkCm / 110.f, 0.18f, 1.f);
				break;
			}
		}

		FVector WheelFwd = Forward.RotateAngleAxis(W.SteerAngle, FVector::UpVector);
		WheelFwd = FVector::VectorPlaneProject(WheelFwd, Hit.Normal).GetSafeNormal();
		FVector WheelRight = FVector::CrossProduct(Hit.Normal, WheelFwd).GetSafeNormal();

		const FVector PointVel = Velocity + FVector::CrossProduct(AngularVel, W.ContactPos - GetActorLocation());
		const float LongVel = FVector::DotProduct(PointVel, WheelFwd);
		const float LatVel = FVector::DotProduct(PointVel, WheelRight);

		const float WheelSpeed = W.AngularVel * W.RadiusCm;
		const float SlipDenom = FMath::Max(220.f, FMath::Abs(LongVel) + FMath::Abs(WheelSpeed));
		const float SlipRatio = (WheelSpeed - LongVel) / SlipDenom;
		W.Slip = FMath::Abs(SlipRatio) + FMath::Abs(LatVel) / SlipDenom;

		// Peak then drop — flooring it in mud lights the tires and they dig.
		const float LongShape = FMath::Clamp(SlipRatio * 7.f, -1.6f, 1.6f);
		const float PeakDrop = 1.f - 0.55f * FMath::Clamp(FMath::Abs(SlipRatio) - 0.28f, 0.f, 1.f);
		const float Fx = Fz * Mu * LongShape * PeakDrop;
		const float Fy = Fz * Mu * FMath::Clamp(-LatVel / 280.f, -1.2f, 1.2f);

		FVector TireForce = WheelFwd * Fx + WheelRight * Fy + SuspForce;

		const float SinkDrag = (Hit.Surface == ESiltSurface::Mud || Hit.Surface == ESiltSurface::Water)
			? Hit.SinkCm * 420.f + (Hit.Surface == ESiltSurface::Water ? 18000.f : 0.f)
			: 0.f;
		TireForce += -PointVel.GetSafeNormal() * SinkDrag * FMath::Clamp(PointVel.Size() / 200.f, 0.f, 3.f);

		if (Input.bHandbrake && !W.bSteered)
		{
			TireForce += -WheelFwd * Fz * Mu * 1.1f * FMath::Sign(LongVel);
		}
		else
		{
			TireForce += WheelFwd * Drive * (MaxDrive * 0.25f);
			if (Input.Brake > 0.05f)
			{
				TireForce += -WheelFwd * Input.Brake * Fz * Mu * 1.35f * FMath::Sign(LongVel);
			}
		}

		OutForce += TireForce;
		OutTorque += FVector::CrossProduct(W.ContactPos - GetActorLocation(), TireForce);

		const float DriveOmega = Drive * 38.f;
		const float GroundOmega = LongVel / W.RadiusCm;
		W.AngularVel = FMath::FInterpTo(W.AngularVel, GroundOmega + DriveOmega * (1.f + W.Slip * 1.4f), Dt, 8.f);

		Terrain->ApplyTire(W.ContactPos, Fz, W.Slip * FMath::Abs(Drive), PSI, Dt);
	}

	if (Grounded == 0)
	{
		// Keep a little angular damping in the air so it doesn't tumble forever.
		AngularVel *= (1.f - 0.4f * Dt);
	}
}

void ASiltTruck::SimulateWinch(float Dt, FVector& OutForce)
{
	if (Input.bWinchIn && !WinchTarget)
	{
		FindWinchTarget();
	}

	if (!WinchTarget)
	{
		if (CableMesh)
		{
			CableMesh->SetVisibility(false);
		}
		return;
	}

	const FVector From = GetFrontHook();
	const FVector To = WinchTarget->GetActorLocation();
	const float Dist = FVector::Dist(From, To);
	if (Dist > 2800.f)
	{
		WinchTarget = nullptr;
		return;
	}

	if (WinchLength <= 0.f)
	{
		WinchLength = Dist;
	}
	if (Input.bWinchIn)
	{
		WinchLength = FMath::Max(180.f, WinchLength - 220.f * Dt);
	}
	if (Input.bWinchOut)
	{
		WinchLength = FMath::Min(2600.f, WinchLength + 220.f * Dt);
	}

	const float Stretch = Dist - WinchLength;
	if (Stretch > 0.f)
	{
		const FVector Dir = (To - From).GetSafeNormal();
		const float Spring = 14000.f * Stretch + 1800.f * FVector::DotProduct(Velocity, Dir);
		const float Clamped = FMath::Clamp(Spring, -45000.f, 52000.f);
		OutForce += Dir * Clamped;
		if (ASiltTruck* Other = Cast<ASiltTruck>(WinchTarget))
		{
			Other->Velocity += -Dir * (Clamped / FMath::Max(Other->MassKg, 1.f)) * Dt;
		}
	}

	if (CableMesh)
	{
		const FVector Mid = (From + To) * 0.5f;
		const FVector Delta = To - From;
		CableMesh->SetWorldLocation(Mid);
		CableMesh->SetWorldRotation(FRotationMatrix::MakeFromZ(Delta.GetSafeNormal()).Rotator());
		CableMesh->SetWorldScale3D(FVector(0.045f, 0.045f, Dist / 100.f));
		CableMesh->SetVisibility(true);
	}
}

void ASiltTruck::FindWinchTarget()
{
	WinchTarget = FindNearestAnchor(GetWorld(), GetFrontHook(), 2000.f, this, Partner.Get());
	WinchLength = 0.f;
}

void ASiltTruck::TryToggleWinch()
{
	if (WinchTarget)
	{
		WinchTarget = nullptr;
		WinchLength = 0.f;
		if (CableMesh)
		{
			CableMesh->SetVisibility(false);
		}
		return;
	}
	FindWinchTarget();
}

void ASiltTruck::TryInteract()
{
	if (CarriedCrate)
	{
		return;
	}
	for (TActorIterator<AActor> It(GetWorld()); It; ++It)
	{
		if (!It->ActorHasTag(Silt::TagCrate()))
		{
			continue;
		}
		if (FVector::Dist(GetActorLocation(), It->GetActorLocation()) < 420.f)
		{
			AttachCrate(*It);
			break;
		}
	}
}

void ASiltTruck::AttachCrate(AActor* Crate)
{
	CarriedCrate = Crate;
}

FVector ASiltTruck::GetFrontHook() const
{
	return GetActorTransform().TransformPosition(FVector(290.f, 0.f, 20.f));
}

FVector ASiltTruck::GetRearHook() const
{
	return GetActorTransform().TransformPosition(FVector(-270.f, 0.f, 20.f));
}

float ASiltTruck::GetSpeedMPH() const
{
	return Velocity.Size() * 0.0223694f; // cm/s to mph
}

FString ASiltTruck::GetWinchLabel() const
{
	if (!WinchTarget)
	{
		return TEXT("WINCH FREE");
	}
	if (Cast<ASiltTruck>(WinchTarget))
	{
		return FString::Printf(TEXT("WINCH -> %s"), Silt::DriverName(Cast<ASiltTruck>(WinchTarget)->GetDriver()));
	}
	return FString::Printf(TEXT("WINCH -> %s"), *WinchTarget->GetActorNameOrLabel());
}

void ASiltTruck::ResetUpright()
{
	const FVector Loc = GetActorLocation();
	const float Z = Terrain ? Terrain->Query(Loc).Position.Z + 90.f : Loc.Z + 80.f;
	SetActorLocation(FVector(Loc.X, Loc.Y, Z));
	SetActorRotation(FRotator(0.f, GetActorRotation().Yaw, 0.f));
	Velocity = FVector::ZeroVector;
	AngularVel = FVector::ZeroVector;
}

void ASiltTruck::UpdateVisuals(float Dt)
{
	const FTransform XF = GetActorTransform();
	for (FSiltWheel& W : Wheels)
	{
		if (!W.Tire)
		{
			continue;
		}
		FVector Loc = XF.TransformPosition(W.LocalOffset);
		if (W.bGrounded)
		{
			Loc.Z = W.ContactPos.Z + W.RadiusCm;
		}
		const FRotator SteerRot(0.f, W.SteerAngle, 0.f);
		const FRotator Spin(FMath::RadiansToDegrees(W.AngularVel) * Dt, 0.f, 0.f);
		W.Tire->SetWorldLocation(Loc);
		W.Tire->AddLocalRotation(FRotator(0.f, 0.f, FMath::RadiansToDegrees(W.AngularVel) * Dt));
		W.Tire->SetRelativeRotation(FRotator(0.f, W.SteerAngle, 90.f) + FRotator(FMath::RadiansToDegrees(W.AngularVel) * 0.f, 0.f, 0.f));
		// Keep the cylinder on its side and steer in yaw.
		const float SpinDeg = FMath::RadiansToDegrees(W.AngularVel);
		W.Tire->SetRelativeRotation(FRotator(SpinDeg, W.SteerAngle, 90.f));
		if (W.Rim)
		{
			W.Rim->SetWorldLocation(Loc);
			W.Rim->SetRelativeRotation(FRotator(SpinDeg, W.SteerAngle, 90.f));
		}
		(void)Spin;
		(void)SteerRot;
	}
}

void ASiltTruck::UpdateAudio(float Dt)
{
	if (EngineAudio)
	{
		EngineAudio->SetPitchMultiplier(0.7f + RPM / 4500.f);
		EngineAudio->SetVolumeMultiplier(0.18f + FMath::Abs(Input.Throttle) * 0.35f);
	}
}

void ASiltTruck::OnEngineAudioNeeded(USoundWaveProcedural* Wave, int32 SamplesRequired)
{
	TArray<uint8> PCM;
	PCM.SetNumUninitialized(SamplesRequired * sizeof(int16));
	int16* Samples = reinterpret_cast<int16*>(PCM.GetData());
	const float Freq = 38.f + RPM * 0.04f;
	for (int32 I = 0; I < SamplesRequired; ++I)
	{
		AudioPhase += 2.f * PI * Freq / 22050.f;
		if (AudioPhase > 2.f * PI)
		{
			AudioPhase -= 2.f * PI;
		}
		const float Diesel = FMath::Sin(AudioPhase) * 0.45f + FMath::Sin(AudioPhase * 0.5f) * 0.25f;
		const float Noise = (FMath::FRand() - 0.5f) * 0.18f;
		Samples[I] = static_cast<int16>(FMath::Clamp(Diesel + Noise, -1.f, 1.f) * 8000.f);
	}
	Wave->QueueAudio(PCM.GetData(), PCM.Num());
}
