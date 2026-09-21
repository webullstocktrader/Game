#include "SiltCountyPlayerController.h"

#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "SiltCountyGameState.h"
#include "SiltTerrain.h"

ASiltCountyPlayerController::ASiltCountyPlayerController()
{
	PrimaryActorTick.bCanEverTick = true;
	bShowMouseCursor = false;
}

void ASiltCountyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	UpdateIntroCamera();
}

void ASiltCountyPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	UpdateIntroCamera();
}

void ASiltCountyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!InputComponent)
	{
		return;
	}
	InputComponent->BindAction(TEXT("SiltSkipIntro"), IE_Pressed, this, &ASiltCountyPlayerController::RequestSkipIntro);
	InputComponent->BindAction(TEXT("SiltSplitScreen"), IE_Pressed, this, &ASiltCountyPlayerController::RequestSplitScreen);
}

void ASiltCountyPlayerController::RequestSkipIntro()
{
	if (HasAuthority())
	{
		ServerSkipIntro_Implementation();
	}
	else
	{
		ServerSkipIntro();
	}
}

void ASiltCountyPlayerController::ServerSkipIntro_Implementation()
{
	if (ASiltCountyGameState* GameState = GetWorld() ? GetWorld()->GetGameState<ASiltCountyGameState>() : nullptr)
	{
		GameState->AuthoritySkipIntro();
	}
}

void ASiltCountyPlayerController::RequestSplitScreen()
{
	UWorld* World = GetWorld();
	if (!World || World->GetNetMode() == NM_Client)
	{
		return;
	}
	if (World->GetNumPlayerControllers() >= 2)
	{
		return;
	}
	UGameplayStatics::CreatePlayer(World, -1, true);
}

void ASiltCountyPlayerController::UpdateIntroCamera()
{
	UWorld* World = GetWorld();
	const ASiltCountyGameState* GameState = World ? World->GetGameState<ASiltCountyGameState>() : nullptr;
	if (!GameState || GameState->IsGameplay())
	{
		if (bIntroView)
		{
			if (APawn* DrivenPawn = GetPawn())
			{
				SetViewTargetWithBlend(DrivenPawn, 0.8f);
			}
			bIntroView = false;
		}
		return;
	}

	auto Look = [](const FVector& From, const FVector& At)
	{
		return (At - From).Rotation();
	};

	FVector Location;
	FVector Target;
	switch (GameState->GetIntroPhase())
	{
	case ESiltIntroPhase::Garage:
		Location = FVector(5200.f, 74500.f, SiltTerrain::SampleHeight(5200.f, 74500.f) + 420.f);
		Target = FVector(0.f, 80000.f, SiltTerrain::SampleHeight(0.f, 80000.f) + 160.f);
		break;
	case ESiltIntroPhase::Dialogue:
		Location = FVector(2400.f, 75200.f, SiltTerrain::SampleHeight(2400.f, 75200.f) + 260.f);
		Target = FVector(0.f, 78600.f, SiltTerrain::SampleHeight(0.f, 78600.f) + 140.f);
		break;
	case ESiltIntroPhase::FloodOverlook:
	default:
		Location = FVector(-65000.f, 112000.f, SiltTerrain::SampleHeight(-65000.f, 112000.f) + 2800.f);
		Target = FVector(0.f, 52000.f, SiltTerrain::GetWaterLevel() + 200.f);
		break;
	}

	if (!IntroCamera)
	{
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		IntroCamera = World->SpawnActor<ACameraActor>(Location, Look(Location, Target), Params);
	}
	if (!IntroCamera)
	{
		return;
	}

	IntroCamera->SetActorLocationAndRotation(Location, Look(Location, Target));
	if (!bIntroView)
	{
		SetViewTarget(IntroCamera);
		bIntroView = true;
	}
}
