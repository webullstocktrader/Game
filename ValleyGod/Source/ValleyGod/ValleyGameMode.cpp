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
		if (AValleyGodPawn* Watcher = Cast<AValleyGodPawn>(PC->GetPawn()))
		{
			Watcher->GoOverview();
		}
	}
}
