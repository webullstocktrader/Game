#pragma once

// Engine-free stone-age valley simulation on prehistoric Earth
// (our solar system — this slice never leaves the valley).

namespace vg
{
	// Entire human population on Earth for this first slice. Do not raise this
	// to spawn other tribes. Later slices grow people through children.
	constexpr int kEarthHumans = 8;

	enum class Activity
	{
		Idle,
		Walk,
		Talk,
		Hunt,
		Eat,
		Sleep,
		Shelter,
		Panic,
		HighGround
	};

	enum class Sex
	{
		Female,
		Male
	};

	enum class Habit
	{
		Hunter,
		Tender,
		Knaps,
		Wander
	};

	enum class Weather
	{
		Clear,
		Rain,
		Tornado,
		Hurricane,
		Flood
	};

	struct Villager
	{
		int Id = 0;
		const char* Name = "";
		Sex Body = Sex::Female;
		Habit Role = Habit::Wander;
		int AgeYears = 21;
		const char* Trait = "";
		const char* PersonalLine = "";
		float X = 0.f;
		float Y = 0.f;
		float Z = 0.f;
		float Heading = 0.f;
		float Hunger = 70.f; // 0 starving, 100 full
		float Energy = 80.f; // 0 exhausted, 100 rested
		Activity Current = Activity::Idle;
		int ShelterIndex = 0;
		int HuntTarget = -1;
		float TalkTimer = 0.f;
		float StateTimer = 0.f;
		const char* Speech = "";
		float TargetX = 0.f;
		float TargetY = 0.f;
		bool CarryingKill = false;
	};

	struct Animal
	{
		int Id = 0;
		float X = 0.f;
		float Y = 0.f;
		float Heading = 0.f;
		bool Alive = true;
		float RespawnIn = 0.f;
	};

	struct World
	{
		float DayLengthSeconds = 75.f;
		float TimeOfDayHours = 10.f;
		bool Paused = false;
		Weather Sky = Weather::Clear;
		float SkySecondsLeft = 0.f;
		float FloodHeight = 0.f;
		float WindX = 0.f;
		float WindY = 0.f;
		float TornadoX = 0.f;
		float TornadoY = 1800.f;
		float TornadoAngle = 0.f;

		Villager Villagers[kEarthHumans];
		int VillagerCount = 0;
		Animal Animals[6];
		int AnimalCount = 0;

		float CampX = 0.f;
		float CampY = 700.f;
		float FireX = 0.f;
		float FireY = 640.f;
		float HuntX = 200.f;
		float HuntY = 2600.f;
		float HighX = 0.f;
		float HighY = -2800.f;
		float ShelterX[6]{};
		float ShelterY[6]{};
		int ShelterCount = 0;

		float MealCountdown = 0.f;
		float SleepCountdown = 0.f;
		float DawnCountdown = 0.f;

		unsigned Rng = 1u;
	};

	void InitWorld(World& W);
	void TickWorld(World& W, float Dt);
	void SetWeather(World& W, Weather Wx);
	void SetDayLength(World& W, float Seconds);
	void SetPaused(World& W, bool Paused);

	const char* ActivityName(Activity A);
	const char* WeatherName(Weather Wx);
	const char* SexName(Sex Body);
	int LineCount();
	const char* LineAt(int Index);
	bool IsNight(float Hours);
	float HoursUntil(float Now, float TargetHour);
	float Rand01(World& W);
} // namespace vg
