#include "ValleyGameMode.h"
#include "ValleyWorld.h"
#include "ValleyPlayerController.h"
#include "ValleyHUD.h"
#include "ValleyGodPawn.h"

AValleyGameMode::AValleyGameMode()
{
	PrimaryActorTick.bCanEverTick = false;
	PlayerControllerClass = AValleyPlayerController::StaticClass();
	HUDClass = AValleyHUD::StaticClass();
	DefaultPawnClass = AValleyGodPawn::StaticClass();
}

void AValleyGameMode::BeginPlay()
{
	Super::BeginPlay();
	Valley = GetWorld()->SpawnActor<AValleyWorld>();
	Valley->BuildValley();
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->SetActorLocation(FVector(40.f, -2100.f, 1280.f));
		}
		PC->SetControlRotation(FRotator(-28.f, 90.f, 0.f));
	}
}
