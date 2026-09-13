#include "SiltIntroDirector.h"
#include "SiltWorldBuilder.h"
#include "SiltCountyGameMode.h"
#include "SiltTruck.h"
#include "Camera/CameraActor.h"
#include "Kismet/GameplayStatics.h"

ASiltIntroDirector::ASiltIntroDirector()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ASiltIntroDirector::StartIntro(ASiltWorldBuilder* InWorld, ASiltCountyGameMode* InGameMode)
{
	CountyWorld = InWorld;
	GameMode = InGameMode;
	bRunning = true;
	Time = 0.f;
	Rig = GetWorld()->SpawnActor<ACameraActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->SetViewTarget(Rig);
	}
	Subtitle.Reset();
}

void ASiltIntroDirector::Skip()
{
	if (bRunning)
	{
		Time = 12.f;
	}
}

void ASiltIntroDirector::SetCam(const FVector& Loc, const FVector& LookAt)
{
	if (!Rig)
	{
		return;
	}
	const FRotator Rot = (LookAt - Loc).Rotation();
	Rig->SetActorLocationAndRotation(Loc, Rot);
}

void ASiltIntroDirector::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bRunning || !CountyWorld)
	{
		return;
	}

	Time += DeltaSeconds;
	const FVector Keys = CountyWorld->GetKeyRackLocation();
	const FVector Trucks = CountyWorld->GetGarageLookAt();

	if (Time < 2.4f)
	{
		Subtitle = TEXT("");
		SetCam(Keys + FVector(80.f, -40.f, 20.f), Keys + FVector(0.f, 20.f, 0.f));
	}
	else if (Time < 5.0f)
	{
		Subtitle = TEXT("");
		const float T = (Time - 2.4f) / 2.6f;
		const FVector Loc = FMath::Lerp(Keys + FVector(120.f, 80.f, 60.f), Trucks + FVector(-420.f, -80.f, 180.f), T);
		SetCam(Loc, Trucks + FVector(40.f, 0.f, 40.f));
	}
	else if (Time < 8.2f)
	{
		Subtitle = TEXT("GOOCH:  Chief, you ready for this adventure?");
		SetCam(Trucks + FVector(-380.f, 220.f, 160.f), Trucks + FVector(40.f, 80.f, 40.f));
	}
	else if (Time < 11.4f)
	{
		Subtitle = TEXT("CHIEF:  Hell yeah, brother, let's get it.");
		SetCam(Trucks + FVector(-400.f, -240.f, 170.f), Trucks + FVector(60.f, -80.f, 40.f));
	}
	else if (!bFinished)
	{
		bFinished = true;
		bRunning = false;
		Subtitle.Reset();
		if (GameMode)
		{
			GameMode->BeginCountyPlay();
		}
	}
}
