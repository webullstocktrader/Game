#include "SiltCounty.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "SiltCountyPlayerController.h"

DEFINE_LOG_CATEGORY(LogSiltCounty);

IMPLEMENT_PRIMARY_GAME_MODULE(FDefaultGameModuleImpl, SiltCounty, "SiltCounty");

namespace
{
	void ForEachLocalSiltController(TFunctionRef<void(ASiltCountyPlayerController&)> Fn)
	{
		if (!GEngine)
		{
			return;
		}

		for (const FWorldContext& Context : GEngine->GetWorldContexts())
		{
			UWorld* World = Context.World();
			if (!World)
			{
				continue;
			}

			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
			{
				if (ASiltCountyPlayerController* PC = Cast<ASiltCountyPlayerController>(It->Get()))
				{
					if (PC->IsLocalController())
					{
						Fn(*PC);
					}
				}
			}
		}
	}

	void SiltSkipIntroCmd()
	{
		ForEachLocalSiltController([](ASiltCountyPlayerController& PC) { PC.RequestSkipIntro(); });
	}

	void SiltAddLocalPlayerCmd()
	{
		ForEachLocalSiltController([](ASiltCountyPlayerController& PC) { PC.RequestSplitScreen(); });
	}

	FAutoConsoleCommand CmdSkipIntro(
		TEXT("silt.SkipIntro"),
		TEXT("Skip the Silt County intro and enable the sample contract."),
		FConsoleCommandDelegate::CreateStatic(&SiltSkipIntroCmd));

	FAutoConsoleCommand CmdAddLocal(
		TEXT("silt.AddLocalPlayer"),
		TEXT("Spawn the second local player (Gooch) on the host. Same as F9."),
		FConsoleCommandDelegate::CreateStatic(&SiltAddLocalPlayerCmd));
}
