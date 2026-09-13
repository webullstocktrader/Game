#include "SiltCountyGameMode.h"
#include "SiltCountyGameState.h"
#include "SiltCountyGameInstance.h"
#include "SiltCountyPlayerController.h"
#include "SiltCountyHUD.h"
#include "SiltWorldBuilder.h"
#include "SiltIntroDirector.h"
#include "SiltTruck.h"
#include "SiltProp.h"
#include "GameFramework/SpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"
#include "Engine/GameViewportClient.h"

ASiltCountyGameMode::ASiltCountyGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PlayerControllerClass = ASiltCountyPlayerController::StaticClass();
	HUDClass = ASiltCountyHUD::StaticClass();
	GameStateClass = ASiltCountyGameState::StaticClass();
	DefaultPawnClass = ASpectatorPawn::StaticClass();
}

void ASiltCountyGameMode::BeginPlay()
{
	Super::BeginPlay();

	CountyWorld = GetWorld()->SpawnActor<ASiltWorldBuilder>();
	CountyWorld->BuildSlice();

	Intro = GetWorld()->SpawnActor<ASiltIntroDirector>();
	Intro->StartIntro(CountyWorld, this);

	if (ASiltCountyPlayerController* PC = Cast<ASiltCountyPlayerController>(GetWorld()->GetFirstPlayerController()))
	{
		PC->BindIntro(Intro);
	}

	if (ASiltCountyGameState* GS = GetGameState<ASiltCountyGameState>())
	{
		GS->Phase = ESiltMatchPhase::Intro;
	}
}

APawn* ASiltCountyGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	if (!bPlayStarted)
	{
		return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
	}
	return nullptr;
}

void ASiltCountyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (ASiltCountyPlayerController* PC = Cast<ASiltCountyPlayerController>(NewPlayer))
	{
		PC->BindIntro(Intro);
	}
	if (bPlayStarted)
	{
		AssignTruck(NewPlayer);
	}
}

void ASiltCountyGameMode::BeginCountyPlay()
{
	if (bPlayStarted || !CountyWorld)
	{
		return;
	}
	bPlayStarted = true;

	if (ASiltCountyGameState* GS = GetGameState<ASiltCountyGameState>())
	{
		GS->Phase = ESiltMatchPhase::Playing;
	}

	UWorld* World = GetWorld();
	APlayerController* P1 = World->GetFirstPlayerController();
	AssignTruck(P1);

	bool bWantLocalTwo = true;
	if (const USiltCountyGameInstance* GI = Cast<USiltCountyGameInstance>(GetGameInstance()))
	{
		bWantLocalTwo = GI->bLocalCoop;
	}

	if (bWantLocalTwo && World->GetNetMode() != NM_Client && !bSecondLocalSpawned)
	{
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->SetForceDisableSplitscreen(false);
		}
		int32 NumPCs = 0;
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			++NumPCs;
		}
		if (NumPCs < 2)
		{
			UGameplayStatics::CreatePlayer(World, 1, true);
			bSecondLocalSpawned = true;
		}
	}

	// Second local player posts login after CreatePlayer.
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->Get() != P1)
		{
			AssignTruck(It->Get());
		}
	}
}

void ASiltCountyGameMode::AssignTruck(APlayerController* PC)
{
	if (!PC || !CountyWorld)
	{
		return;
	}

	ASiltCountyPlayerController* SiltPC = Cast<ASiltCountyPlayerController>(PC);
	ASiltTruck* Truck = nullptr;
	int32 Index = 0;
	if (SiltPC)
	{
		Index = SiltPC->GetLocalIndex();
	}
	else
	{
		// Remote listen-server client is always Gooch if Chief is taken.
		Index = (PC == GetWorld()->GetFirstPlayerController()) ? 0 : 1;
	}

	Truck = (Index == 0) ? CountyWorld->GetChief() : CountyWorld->GetGooch();
	if (!Truck)
	{
		return;
	}

	if (SiltPC)
	{
		SiltPC->PossessTruck(Truck);
	}
	else
	{
		PC->Possess(Truck);
	}
}

void ASiltCountyGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	WatchContract();
}

void ASiltCountyGameMode::WatchContract()
{
	if (!CountyWorld)
	{
		return;
	}
	ASiltCountyGameState* GS = GetGameState<ASiltCountyGameState>();
	ASiltCrate* Crate = CountyWorld->GetCrate();
	ASiltDropZone* Drop = CountyWorld->GetDropZone();
	if (!GS || !Crate || !Drop)
	{
		return;
	}

	const bool bOnChief = CountyWorld->GetChief() && CountyWorld->GetChief()->GetCarriedCrate() == Crate;
	const bool bOnGooch = CountyWorld->GetGooch() && CountyWorld->GetGooch()->GetCarriedCrate() == Crate;
	GS->bCrateLoaded = bOnChief || bOnGooch;

	if (Drop->Contains(Crate->GetActorLocation()))
	{
		GS->bContractDone = true;
		GS->Phase = ESiltMatchPhase::Complete;
	}
}
