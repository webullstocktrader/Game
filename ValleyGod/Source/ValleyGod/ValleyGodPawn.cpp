#include "ValleyGodPawn.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerCameraManager.h"

AValleyGodPawn::AValleyGodPawn()
{
	bAddDefaultMovementBindings = false;
	bCanBeDamaged = false;
	bUseControllerRotationPitch = true;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	SetCanBeDamaged(false);
}

void AValleyGodPawn::BeginPlay()
{
	Super::BeginPlay();
	HideWatcherBody();
	GoOverview();
}

void AValleyGodPawn::HideWatcherBody()
{
	if (UStaticMeshComponent* Mesh = GetMeshComponent())
	{
		Mesh->SetHiddenInGame(true);
		Mesh->SetVisibility(false);
		Mesh->SetCastShadow(false);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetStaticMesh(nullptr);
	}
	if (USphereComponent* Col = GetCollisionComponent())
	{
		Col->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Col->SetHiddenInGame(true);
		Col->SetSphereRadius(12.f);
	}
	SetActorEnableCollision(false);
}

void AValleyGodPawn::SetFlyInput(float Forward, float Right, float Up, bool bInSprint)
{
	ForwardAxis = Forward;
	RightAxis = Right;
	UpAxis = Up;
	bSprint = bInSprint;
}

void AValleyGodPawn::AddLook(float Yaw, float Pitch)
{
	AddControllerYawInput(Yaw);
	AddControllerPitchInput(Pitch);
}

void AValleyGodPawn::AddZoom(float Wheel)
{
	if (FMath::IsNearlyZero(Wheel))
	{
		return;
	}
	Fov = FMath::Clamp(Fov - Wheel * 6.f, 42.f, 110.f);
	FlySpeed = FMath::Clamp(FlySpeed + Wheel * 400.f, 700.f, 24000.f);
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->SetFOV(Fov);
		}
	}
}

void AValleyGodPawn::FollowActor(AActor* Target, float DeltaSeconds)
{
	if (!Target)
	{
		bFollowing = false;
		return;
	}
	FollowTarget = Target;
	bFollowing = true;
	const FVector Desired = Target->GetActorLocation() + FVector(-280.f, -40.f, 220.f);
	SetActorLocation(FMath::VInterpTo(GetActorLocation(), Desired, DeltaSeconds, 3.5f));
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		const FRotator Look = (Target->GetActorLocation() - GetActorLocation()).Rotation();
		PC->SetControlRotation(FMath::RInterpTo(PC->GetControlRotation(), Look, DeltaSeconds, 4.f));
	}
}

void AValleyGodPawn::StopFollow()
{
	bFollowing = false;
	FollowTarget = nullptr;
}

void AValleyGodPawn::GoOverview()
{
	StopFollow();
	// Straight down on the whole miniature Earth: valley, ocean, and the other seven lands.
	SetActorLocation(FVector(0.f, 400.f, 30000.f));
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetControlRotation(FRotator(-88.f, 0.f, 0.f));
	}
}

void AValleyGodPawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	HideWatcherBody();

	if (bFollowing && FollowTarget.IsValid())
	{
		FollowActor(FollowTarget.Get(), DeltaSeconds);
		return;
	}

	const float Speed = bSprint ? FlySpeed * 2.4f : FlySpeed;
	AddMovementInput(GetActorForwardVector(), ForwardAxis);
	AddMovementInput(GetActorRightVector(), RightAxis);
	AddMovementInput(FVector::UpVector, UpAxis);

	if (UFloatingPawnMovement* Move = Cast<UFloatingPawnMovement>(GetMovementComponent()))
	{
		Move->MaxSpeed = Speed;
		Move->Acceleration = Speed * 4.f;
		Move->Deceleration = Speed * 4.f;
	}
}
