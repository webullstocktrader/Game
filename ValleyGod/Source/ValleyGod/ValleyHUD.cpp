#include "ValleyHUD.h"
#include "ValleyWorld.h"
#include "Sim/ValleySim.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"

void AValleyHUD::DrawBar(float X, float Y, float W, float H, const FLinearColor& Color)
{
	DrawRect(Color, X, Y, W, H);
}

void AValleyHUD::ShadowText(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale)
{
	UFont* Font = GEngine ? GEngine->GetMediumFont() : nullptr;
	DrawText(Text, FLinearColor(0.f, 0.f, 0.f, 0.75f), X + 2.f, Y + 2.f, Font, Scale, false);
	DrawText(Text, Color, X, Y, Font, Scale, false);
}

FString AValleyHUD::Clock(float Hours) const
{
	const int32 H = FMath::Clamp(FMath::FloorToInt(Hours), 0, 23);
	const int32 M = FMath::Clamp(FMath::FloorToInt(FMath::Frac(Hours) * 60.f), 0, 59);
	return FString::Printf(TEXT("%02d:%02d"), H, M);
}

FString AValleyHUD::Countdown(float Seconds) const
{
	const int32 S = FMath::Max(0, FMath::RoundToInt(Seconds));
	return FString::Printf(TEXT("%d:%02d"), S / 60, S % 60);
}

void AValleyHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas)
	{
		return;
	}

	AValleyWorld* WorldActor = AValleyWorld::Get(GetWorld());
	if (!WorldActor)
	{
		return;
	}
	const vg::World& Sim = WorldActor->Sim();
	const float W = Canvas->SizeX;
	const float H = Canvas->SizeY;

	DrawBar(0.f, 0.f, W, 52.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.55f));
	ShadowText(22.f, 12.f, TEXT("VALLEY"), FLinearColor(0.82f, 0.74f, 0.42f), 1.15f);
	ShadowText(160.f, 16.f, Clock(Sim.TimeOfDayHours) + (Sim.Paused ? TEXT("  PAUSED") : TEXT("")), FLinearColor(0.9f, 0.88f, 0.78f), 1.0f);
	ShadowText(W * 0.5f - 180.f, 16.f, FString::Printf(TEXT("Weather  %s"), UTF8_TO_TCHAR(vg::WeatherName(Sim.Sky))), FLinearColor(0.75f, 0.85f, 0.9f), 0.95f);

	const float PanelX = W - 360.f;
	DrawBar(PanelX, 64.f, 340.f, 168.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.62f));
	ShadowText(PanelX + 16.f, 74.f, TEXT("EVENT CLOCK"), FLinearColor(0.82f, 0.74f, 0.42f), 0.85f);
	ShadowText(PanelX + 16.f, 102.f, FString::Printf(TEXT("Meal          %s"), *Countdown(Sim.MealCountdown)), FLinearColor(0.9f, 0.88f, 0.78f), 0.9f);
	ShadowText(PanelX + 16.f, 128.f, FString::Printf(TEXT("Sleep         %s"), *Countdown(Sim.SleepCountdown)), FLinearColor(0.9f, 0.88f, 0.78f), 0.9f);
	ShadowText(PanelX + 16.f, 154.f, FString::Printf(TEXT("Dawn          %s"), *Countdown(Sim.DawnCountdown)), FLinearColor(0.9f, 0.88f, 0.78f), 0.9f);
	ShadowText(PanelX + 16.f, 180.f, FString::Printf(TEXT("Day length    %.0fs"), Sim.DayLengthSeconds), FLinearColor(0.7f, 0.72f, 0.65f), 0.85f);
	if (Sim.Sky != vg::Weather::Clear)
	{
		ShadowText(PanelX + 16.f, 204.f, FString::Printf(TEXT("Storm ends    %s"), *Countdown(Sim.SkySecondsLeft)), FLinearColor(0.75f, 0.85f, 0.9f), 0.85f);
	}

	DrawBar(18.f, 64.f, 280.f, 132.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.55f));
	ShadowText(32.f, 74.f, TEXT("1  Rain"), FLinearColor(0.7f, 0.8f, 0.9f), 0.85f);
	ShadowText(32.f, 98.f, TEXT("2  Tornado"), FLinearColor(0.7f, 0.8f, 0.9f), 0.85f);
	ShadowText(32.f, 122.f, TEXT("3  Hurricane"), FLinearColor(0.7f, 0.8f, 0.9f), 0.85f);
	ShadowText(32.f, 146.f, TEXT("4  Flood     0 clear"), FLinearColor(0.7f, 0.8f, 0.9f), 0.85f);

	const float Mid = W * 0.5f;
	DrawRect(FLinearColor(0.95f, 0.9f, 0.7f, 0.8f), Mid - 7.f, H * 0.5f - 1.f, 14.f, 2.f);
	DrawRect(FLinearColor(0.95f, 0.9f, 0.7f, 0.8f), Mid - 1.f, H * 0.5f - 7.f, 2.f, 14.f);

	float LogY = H - 210.f;
	DrawBar(18.f, LogY - 10.f, 640.f, 150.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.5f));
	ShadowText(32.f, LogY - 6.f, TEXT("VOICES"), FLinearColor(0.82f, 0.74f, 0.42f), 0.8f);
	int32 Shown = 0;
	for (int32 I = 0; I < Sim.VillagerCount && Shown < 5; ++I)
	{
		if (Sim.Villagers[I].Speech && Sim.Villagers[I].Speech[0])
		{
			ShadowText(32.f, LogY + 22.f + Shown * 22.f,
				FString::Printf(TEXT("%s  —  %s"), UTF8_TO_TCHAR(Sim.Villagers[I].Name), UTF8_TO_TCHAR(Sim.Villagers[I].Speech)),
				FLinearColor(0.92f, 0.9f, 0.82f), 0.85f);
			++Shown;
		}
	}
	if (Shown == 0)
	{
		ShadowText(32.f, LogY + 28.f, TEXT("The valley is quiet."), FLinearColor(0.6f, 0.62f, 0.55f), 0.85f);
	}

	const int32 Pin = WorldActor->GetPinnedId();
	if (Pin >= 0 && Pin < Sim.VillagerCount)
	{
		const vg::Villager& V = Sim.Villagers[Pin];
		DrawBar(W - 360.f, H - 168.f, 340.f, 150.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.62f));
		ShadowText(W - 344.f, H - 140.f,
			FString::Printf(TEXT("%s  ·  %s  ·  %d"), UTF8_TO_TCHAR(V.Name), UTF8_TO_TCHAR(vg::SexName(V.Body)), V.AgeYears),
			FLinearColor(0.82f, 0.74f, 0.42f), 1.0f);
		ShadowText(W - 344.f, H - 114.f, UTF8_TO_TCHAR(V.Trait), FLinearColor(0.85f, 0.82f, 0.72f), 0.8f);
		ShadowText(W - 344.f, H - 92.f, FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(vg::ActivityName(V.Current))), FLinearColor(0.9f, 0.88f, 0.78f), 0.85f);
		ShadowText(W - 344.f, H - 70.f, FString::Printf(TEXT("Hunger %d    Energy %d"), FMath::RoundToInt(V.Hunger), FMath::RoundToInt(V.Energy)), FLinearColor(0.85f, 0.8f, 0.7f), 0.85f);
		DrawBar(W - 344.f, H - 48.f, 300.f * (V.Hunger / 100.f), 8.f, FLinearColor(0.72f, 0.45f, 0.18f, 0.9f));
		DrawBar(W - 344.f, H - 34.f, 300.f * (V.Energy / 100.f), 8.f, FLinearColor(0.3f, 0.55f, 0.75f, 0.9f));
	}

	ShadowText(22.f, H - 36.f, TEXT("WASD fly   Q/E up-down   mouse look   Shift fast   P pause   [ ] day   click pin"), FLinearColor(0.55f, 0.58f, 0.5f), 0.75f);
}
