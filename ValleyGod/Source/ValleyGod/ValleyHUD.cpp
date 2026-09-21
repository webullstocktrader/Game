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
	int32 EnemyPairs = 0;
	int32 AllyPairs = 0;
	for (int32 A = 0; A < Sim.TribeCount; ++A)
	{
		for (int32 B = A + 1; B < Sim.TribeCount; ++B)
		{
			if (vg::TribesAtWar(Sim, A, B))
			{
				++EnemyPairs;
			}
			else if (vg::TribesAllied(Sim, A, B))
			{
				++AllyPairs;
			}
		}
	}
	const TCHAR* StanceWord = EnemyPairs > 0 ? TEXT("enemies") : (AllyPairs > 0 ? TEXT("allies") : TEXT("neutral"));
	ShadowText(W * 0.5f + 40.f, 16.f,
		FString::Printf(TEXT("%d lands  ·  ocean  ·  %s  ·  kin %d"), Sim.TribeCount, StanceWord, Sim.Births),
		FLinearColor(0.78f, 0.74f, 0.55f), 0.9f);

	const float PanelX = W - 360.f;
	DrawBar(PanelX, 64.f, 340.f, 200.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.62f));
	ShadowText(PanelX + 16.f, 74.f, TEXT("EVENT CLOCK"), FLinearColor(0.82f, 0.74f, 0.42f), 0.85f);
	ShadowText(PanelX + 16.f, 102.f, FString::Printf(TEXT("Meal          %s"), *Countdown(Sim.MealCountdown)), FLinearColor(0.9f, 0.88f, 0.78f), 0.9f);
	ShadowText(PanelX + 16.f, 128.f, FString::Printf(TEXT("Sleep         %s"), *Countdown(Sim.SleepCountdown)), FLinearColor(0.9f, 0.88f, 0.78f), 0.9f);
	ShadowText(PanelX + 16.f, 154.f, FString::Printf(TEXT("Dawn          %s"), *Countdown(Sim.DawnCountdown)), FLinearColor(0.9f, 0.88f, 0.78f), 0.9f);
	ShadowText(PanelX + 16.f, 180.f, FString::Printf(TEXT("Day length    %.0fs"), Sim.DayLengthSeconds), FLinearColor(0.7f, 0.72f, 0.65f), 0.85f);
	if (Sim.TribeCount > 0)
	{
		const vg::Tribe& Home = Sim.Tribes[0];
		const char* Stage = "";
		if (Home.ActiveSite >= 0 && Home.ActiveSite < Sim.SiteCount)
		{
			Stage = vg::BuildStageName(Sim.Sites[Home.ActiveSite].Stage);
		}
		ShadowText(PanelX + 16.f, 204.f,
			FString::Printf(TEXT("%s  wood %d  %s"), UTF8_TO_TCHAR(vg::TechName(Home.TechTier)), Home.Wood, UTF8_TO_TCHAR(Stage)),
			FLinearColor(0.7f, 0.72f, 0.65f), 0.75f);
	}
	if (Sim.Sky != vg::Weather::Clear)
	{
		ShadowText(PanelX + 16.f, 228.f, FString::Printf(TEXT("Storm ends    %s"), *Countdown(Sim.SkySecondsLeft)), FLinearColor(0.75f, 0.85f, 0.9f), 0.85f);
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
		const vg::Tribe* Tribe = (V.TribeId >= 0 && V.TribeId < Sim.TribeCount) ? &Sim.Tribes[V.TribeId] : nullptr;
		DrawBar(W - 360.f, H - 236.f, 340.f, 218.f, FLinearColor(0.02f, 0.03f, 0.02f, 0.62f));
		const TCHAR* AgeWord = V.AgeYears < vg::kMinAdultAge ? TEXT("Baby") : UTF8_TO_TCHAR(vg::SexName(V.Body));
		ShadowText(W - 344.f, H - 222.f,
			FString::Printf(TEXT("%s  ·  %s  ·  %d"), UTF8_TO_TCHAR(V.Name), AgeWord, V.AgeYears),
			FLinearColor(0.82f, 0.74f, 0.42f), 1.0f);
		bool bAtWar = false;
		bool bAlly = false;
		for (int32 Other = 0; Other < Sim.TribeCount; ++Other)
		{
			if (vg::TribesAtWar(Sim, V.TribeId, Other))
			{
				bAtWar = true;
			}
			if (vg::TribesAllied(Sim, V.TribeId, Other))
			{
				bAlly = true;
			}
		}
		const TCHAR* StanceWord = bAtWar ? TEXT("Enemy") : (bAlly ? TEXT("Ally") : TEXT("Neutral"));
		ShadowText(W - 344.f, H - 198.f,
			FString::Printf(TEXT("%s  ·  %s  ·  %s  ·  know %d"),
				UTF8_TO_TCHAR(vg::TribeLabel(Sim, V.TribeId)),
				V.bTeacher ? TEXT("Teacher") : (V.AgeYears < vg::kMinAdultAge ? TEXT("Child") : TEXT("Learning")),
				StanceWord,
				FMath::RoundToInt(V.Knowledge)),
			FLinearColor(0.78f, 0.74f, 0.55f), 0.8f);
		ShadowText(W - 344.f, H - 176.f, UTF8_TO_TCHAR(V.Trait), FLinearColor(0.85f, 0.82f, 0.72f), 0.75f);
		ShadowText(W - 344.f, H - 154.f, FString::Printf(TEXT("%s"), UTF8_TO_TCHAR(vg::ActivityName(V.Current))), FLinearColor(0.9f, 0.88f, 0.78f), 0.85f);
		if (Tribe)
		{
			const char* Stage = "";
			if (Tribe->ActiveSite >= 0 && Tribe->ActiveSite < Sim.SiteCount)
			{
				Stage = vg::BuildStageName(Sim.Sites[Tribe->ActiveSite].Stage);
			}
			ShadowText(W - 344.f, H - 132.f,
				FString::Printf(TEXT("%s  wood %d  %s"), UTF8_TO_TCHAR(vg::TechName(Tribe->TechTier)), Tribe->Wood, UTF8_TO_TCHAR(Stage)),
				FLinearColor(0.7f, 0.72f, 0.65f), 0.75f);
			ShadowText(W - 344.f, H - 110.f,
				FString::Printf(TEXT("Shelters %d   claim %dm"), Tribe->ShelterCount, FMath::RoundToInt(Tribe->ClaimRadius / 100.f)),
				FLinearColor(0.7f, 0.72f, 0.65f), 0.8f);
		}
		ShadowText(W - 344.f, H - 84.f, FString::Printf(TEXT("Hunger %d    Energy %d"), FMath::RoundToInt(V.Hunger), FMath::RoundToInt(V.Energy)), FLinearColor(0.85f, 0.8f, 0.7f), 0.85f);
		DrawBar(W - 344.f, H - 58.f, 300.f * (V.Hunger / 100.f), 8.f, FLinearColor(0.72f, 0.45f, 0.18f, 0.9f));
		DrawBar(W - 344.f, H - 44.f, 300.f * (V.Energy / 100.f), 8.f, FLinearColor(0.3f, 0.55f, 0.75f, 0.9f));
	}

	ShadowText(22.f, H - 56.f, WorldActor->GraphicsStatusLine(), FLinearColor(0.62f, 0.68f, 0.55f), 0.7f);
	ShadowText(22.f, H - 36.f, TEXT("WASD fly   Q/E up-down   Shift fast   G next land   P pause   [ ] day   click pin"), FLinearColor(0.55f, 0.58f, 0.5f), 0.75f);
}
