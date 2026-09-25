#include "SiltCountyGameMode.h"

#include "Engine/World.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "SiltContractActor.h"
#include "SiltCounty.h"
#include "SiltCountyGameState.h"
#include "SiltCountyHUD.h"
#include "SiltCountyPlayerController.h"
#include "SiltTerrain.h"
#include "SiltTruckPawn.h"
#include "SiltWorldSubsystem.h"

ASiltCountyGameMode::ASiltCountyGameMode()
{
	GameStateClass = ASiltCountyGameState::StaticClass();
	PlayerControllerClass = ASiltCountyPlayerController::StaticClass();
	HUDClass = ASiltCountyHUD::StaticClass();
	DefaultPawnClass = ASiltTruckPawn::StaticClass();
	bStartPlayersAsSpectators = false;
}

void ASiltCountyGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	if (USiltWorldSubsystem* Terrain = World->GetSubsystem<USiltWorldSubsystem>())
	{
		Terrain->EnsureBuilt();
	}

	const FRotator Yaw = SiltTerrain::GetTruckYaw();
	ChiefTruck = SpawnTruck(0, TEXT("Chief"), FLinearColor(0.45f, 0.075f, 0.05f), SiltTerrain::GetChiefSpawn(), Yaw);
	GoochTruck = SpawnTruck(1, TEXT("Gooch"), FLinearColor(0.55f, 0.42f, 0.08f), SiltTerrain::GetGoochSpawn(), Yaw);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Contract = World->SpawnActor<ASiltContractActor>(SiltTerrain::GetContractSpawn(), FRotator(0.f, 35.f, 0.f), Params);

	UE_LOG(LogSiltCounty, Display, TEXT("Silt County trucks parked. Chief and Gooch are ready."));
}

ASiltTruckPawn* ASiltCountyGameMode::SpawnTruck(int32 Index, const FString& Name, const FLinearColor& Paint, const FVector& Location, const FRotator& Rotation)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}

	const FTransform Xform(Rotation, Location);
	ASiltTruckPawn* Truck = World->SpawnActorDeferred<ASiltTruckPawn>(
		ASiltTruckPawn::StaticClass(),
		Xform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
	if (!Truck)
	{
		UE_LOG(LogSiltCounty, Error, TEXT("Failed to spawn %s"), *Name);
		return nullptr;
	}
	Truck->ConfigureIdentity(Index, Name, Paint);
	return Cast<ASiltTruckPawn>(UGameplayStatics::FinishSpawningActor(Truck, Xform));
}

void ASiltCountyGameMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer || NewPlayer->GetPawn())
	{
		return;
	}

	if (ASiltTruckPawn* Truck = ClaimTruck())
	{
		NewPlayer->Possess(Truck);
		return;
	}

	UE_LOG(LogSiltCounty, Warning, TEXT("No free Silt County truck for %s"), *GetNameSafe(NewPlayer));
}

ASiltTruckPawn* ASiltCountyGameMode::ClaimTruck()
{
	if (ChiefTruck && !ChiefTruck->GetController())
	{
		return ChiefTruck;
	}
	if (GoochTruck && !GoochTruck->GetController())
	{
		return GoochTruck;
	}
	return nullptr;
}

void ASiltCountyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	if (!NewPlayer)
	{
		return;
	}
	if (const ASiltTruckPawn* Truck = Cast<ASiltTruckPawn>(NewPlayer->GetPawn()))
	{
		if (APlayerState* State = NewPlayer->PlayerState)
		{
			State->SetPlayerName(Truck->GetDriverName());
		}
	}
}

APawn* ASiltCountyGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	(void)NewPlayer;
	(void)StartSpot;
	return nullptr;
}
