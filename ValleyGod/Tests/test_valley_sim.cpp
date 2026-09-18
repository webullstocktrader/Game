#include "ValleySim.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

static int GFails = 0;
static int GPasses = 0;

#define CHECK(cond, msg)                                                                                               \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(cond))                                                                                                   \
		{                                                                                                              \
			std::fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, msg);                                         \
			++GFails;                                                                                                  \
		}                                                                                                              \
		else                                                                                                           \
		{                                                                                                              \
			++GPasses;                                                                                                 \
		}                                                                                                              \
	} while (0)

static bool ContainsFold(const char* Hay, const char* Needle)
{
	if (!Hay || !Needle)
	{
		return false;
	}
	const size_t N = std::strlen(Needle);
	for (const char* P = Hay; *P; ++P)
	{
		size_t I = 0;
		while (I < N)
		{
			const char A = static_cast<char>(P[I] | 32);
			const char B = static_cast<char>(Needle[I] | 32);
			if (!P[I] || A != B)
			{
				break;
			}
			++I;
		}
		if (I == N)
		{
			return true;
		}
	}
	return false;
}

static bool SpeechForbidden(const char* Line)
{
	return ContainsFold(Line, "god") || ContainsFold(Line, "player") || ContainsFold(Line, "camera")
		|| ContainsFold(Line, "watcher") || ContainsFold(Line, "spectator") || ContainsFold(Line, "sky father")
		|| ContainsFold(Line, "planet") || ContainsFold(Line, "mars") || ContainsFold(Line, "venus")
		|| ContainsFold(Line, "jupiter") || ContainsFold(Line, "galaxy") || ContainsFold(Line, "milky")
		|| ContainsFold(Line, "orbit") || ContainsFold(Line, "spaceship") || ContainsFold(Line, "cosmos")
		|| ContainsFold(Line, "solar") || ContainsFold(Line, "alien");
}

int main()
{
	using namespace vg;

	{
		World W;
		InitWorld(W);
		CHECK(W.VillagerCount >= 12 && W.VillagerCount <= 16, "12-16 villagers");
		CHECK(W.AnimalCount >= 3, "a few hunt animals");
		CHECK(W.ShelterCount >= 4, "a few shelters");
		int Adults = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			CHECK(W.Villagers[I].AgeYears >= 21, "adult only");
			CHECK(W.Villagers[I].Name && W.Villagers[I].Name[0], "named");
			if (W.Villagers[I].AgeYears >= 21)
			{
				++Adults;
			}
		}
		CHECK(Adults == W.VillagerCount, "every villager is an adult");
		CHECK(W.DayLengthSeconds > 30.f && W.DayLengthSeconds < 240.f, "compressed day");
	}

	{
		for (int I = 0; I < LineCount(); ++I)
		{
			CHECK(!SpeechForbidden(LineAt(I)), "speech never names a god, watcher, or the cosmos");
			CHECK(LineAt(I) && LineAt(I)[0] >= 'A' && LineAt(I)[0] <= 'Z', "English sentence case");
		}
		bool bNightSky = false;
		for (int I = 0; I < LineCount(); ++I)
		{
			if (ContainsFold(LineAt(I), "night") && ContainsFold(LineAt(I), "lights"))
			{
				bNightSky = true;
			}
		}
		CHECK(bNightSky, "optional night-sky flavor exists, without naming planets");
	}

	{
		World W;
		InitWorld(W);
		W.Paused = true;
		const float Hour = W.TimeOfDayHours;
		const float Hunger = W.Villagers[0].Hunger;
		TickWorld(W, 5.f);
		CHECK(W.TimeOfDayHours == Hour, "pause freezes the clock");
		CHECK(W.Villagers[0].Hunger == Hunger, "pause freezes needs");
	}

	{
		World W;
		InitWorld(W);
		SetDayLength(W, 60.f);
		const float Start = W.TimeOfDayHours;
		TickWorld(W, 30.f);
		const float Advanced = W.TimeOfDayHours - Start;
		CHECK(Advanced > 11.f && Advanced < 13.f, "half a 60s day is ~12 hours");
	}

	{
		World W;
		InitWorld(W);
		W.Villagers[0].Hunger = 10.f;
		W.Villagers[0].Energy = 80.f;
		W.Sky = Weather::Clear;
		TickWorld(W, 0.25f);
		CHECK(W.Villagers[0].Current == Activity::Eat || W.Villagers[0].Current == Activity::Hunt,
			"empty stomach seeks food");
	}

	{
		World W;
		InitWorld(W);
		W.TimeOfDayHours = 22.f;
		W.Villagers[1].Energy = 20.f;
		W.Sky = Weather::Clear;
		TickWorld(W, 0.25f);
		CHECK(W.Villagers[1].Current == Activity::Sleep, "night + low energy sleeps");
	}

	{
		World W;
		InitWorld(W);
		SetWeather(W, Weather::Rain);
		TickWorld(W, 0.5f);
		int Sheltering = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (W.Villagers[I].Current == Activity::Shelter)
			{
				++Sheltering;
			}
			CHECK(!SpeechForbidden(W.Villagers[I].Speech), "rain line has no god");
		}
		CHECK(Sheltering >= W.VillagerCount - 2, "rain sends almost everyone to shelter");
	}

	{
		World W;
		InitWorld(W);
		SetWeather(W, Weather::Tornado);
		TickWorld(W, 0.5f);
		int Panic = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (W.Villagers[I].Current == Activity::Panic)
			{
				++Panic;
			}
		}
		CHECK(Panic >= W.VillagerCount - 2, "tornado causes panic");
	}

	{
		World W;
		InitWorld(W);
		SetWeather(W, Weather::Hurricane);
		TickWorld(W, 0.5f);
		int React = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (W.Villagers[I].Current == Activity::Shelter || W.Villagers[I].Current == Activity::Panic)
			{
				++React;
			}
		}
		CHECK(React >= W.VillagerCount - 2, "hurricane drives people inside");
	}

	{
		World W;
		InitWorld(W);
		SetWeather(W, Weather::Flood);
		TickWorld(W, 0.5f);
		int High = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (W.Villagers[I].Current == Activity::HighGround)
			{
				++High;
			}
		}
		CHECK(High >= W.VillagerCount - 2, "flood sends people uphill");
		CHECK(W.FloodHeight > 40.f, "flood raises the river");
	}

	{
		World W;
		InitWorld(W);
		SetWeather(W, Weather::Tornado);
		TickWorld(W, 0.2f);
		SetWeather(W, Weather::Clear);
		TickWorld(W, 0.5f);
		int Panic = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (W.Villagers[I].Current == Activity::Panic)
			{
				++Panic;
			}
		}
		CHECK(Panic == 0, "clear weather lets panic end");
	}

	{
		World W;
		InitWorld(W);
		int Alive = 0;
		for (int I = 0; I < W.AnimalCount; ++I)
		{
			Alive += W.Animals[I].Alive ? 1 : 0;
		}
		W.Villagers[2].Hunger = 40.f;
		W.Villagers[2].Energy = 80.f;
		W.Sky = Weather::Clear;
		W.TimeOfDayHours = 10.f;
		W.Villagers[2].Current = Activity::Hunt;
		for (int Step = 0; Step < 400; ++Step)
		{
			TickWorld(W, 0.25f);
		}
		int AliveAfter = 0;
		for (int I = 0; I < W.AnimalCount; ++I)
		{
			AliveAfter += W.Animals[I].Alive ? 1 : 0;
		}
		CHECK(AliveAfter < Alive || W.Villagers[2].Hunger > 50.f, "hunt eventually kills or feeds");
	}

	{
		World W;
		InitWorld(W);
		const float Meal = W.MealCountdown;
		TickWorld(W, 2.f);
		CHECK(W.MealCountdown < Meal, "meal countdown ticks");
		CHECK(W.SleepCountdown > 0.f, "sleep countdown present");
		CHECK(W.DawnCountdown > 0.f, "dawn countdown present");
	}

	{
		CHECK(std::strcmp(WeatherName(Weather::Rain), "Rain") == 0, "rain label");
		CHECK(std::strcmp(ActivityName(Activity::Talk), "Talk") == 0, "talk label");
	}

	std::printf("%d passed, %d failed\n", GPasses, GFails);
	return GFails ? 1 : 0;
}
