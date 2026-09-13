#include "SiltCountyHUD.h"
#include "SiltTruck.h"
#include "SiltCountyGameState.h"
#include "SiltIntroDirector.h"
#include "EngineUtils.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void ASiltCountyHUD::DrawBar(float X, float Y, float W, float H, const FLinearColor& Color)
{
	DrawRect(Color, X, Y, W, H);
}

void ASiltCountyHUD::ShadowText(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale)
{
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	DrawText(Text, FLinearColor(0.f, 0.f, 0.f, 0.7f), X + 2.f, Y + 2.f, Font, Scale, false);
	DrawText(Text, Color, X, Y, Font, Scale, false);
}

void ASiltCountyHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}

	ASiltCountyGameState* GS = GetWorld() ? GetWorld()->GetGameState<ASiltCountyGameState>() : nullptr;
	ASiltTruck* Truck = Cast<ASiltTruck>(GetOwningPawn());
	ASiltIntroDirector* Intro = nullptr;
	for (TActorIterator<ASiltIntroDirector> It(GetWorld()); It; ++It)
	{
		Intro = *It;
		break;
	}

	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	DrawBar(0.f, 0.f, W, 56.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.55f));
	ShadowText(24.f, 12.f, TEXT("SILT COUNTY"), FLinearColor(0.82f, 0.74f, 0.42f), 1.15f);

	if (Intro && Intro->IsRunning())
	{
		DrawBar(0.f, H * 0.72f, W, 80.f, FLinearColor(0.f, 0.f, 0.f, 0.62f));
		ShadowText(40.f, H * 0.74f, Intro->GetSubtitle(), FLinearColor(0.95f, 0.93f, 0.85f), 1.35f);
		ShadowText(40.f, H * 0.82f, TEXT("Enter / Start  —  skip"), FLinearColor(0.7f, 0.7f, 0.65f), 0.9f);
		return;
	}

	if (GS)
	{
		ShadowText(24.f, H - 70.f, GS->ContractLine(), FLinearColor(0.9f, 0.88f, 0.78f), 0.95f);
	}

	if (!Truck)
	{
		return;
	}

	const FString Gear = Truck->GetGear() == ESiltGear::Low ? TEXT("LOW")
		: Truck->GetGear() == ESiltGear::Reverse ? TEXT("R")
		: Truck->GetGear() == ESiltGear::Neutral ? TEXT("N")
		: TEXT("D");

	const FString Dash = FString::Printf(
		TEXT("%s   %s   %d PSI   %d mph   %s"),
		*Truck->GetHudName(),
		*Gear,
		FMath::RoundToInt(Truck->GetPSI()),
		FMath::RoundToInt(Truck->GetSpeedMPH()),
		*Truck->GetWinchLabel());

	DrawBar(0.f, H - 108.f, W, 40.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.55f));
	ShadowText(24.f, H - 102.f, Dash, FLinearColor(0.85f, 0.9f, 0.8f), 1.05f);

	ShadowText(W - 520.f, 14.f, TEXT("W/I throttle   A/D J/L steer   [ ] air   Q/U winch   E/O pull   F/' crate"), FLinearColor(0.65f, 0.7f, 0.6f), 0.75f);
}
