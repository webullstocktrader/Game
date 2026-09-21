#pragma once

// Engine-free stone-age simulation on a miniature Earth.
// Eight continents, one starter tribe each, oceans between.
// Adults only (21+). Pairing never runs for anyone younger, and this slice
// does not place child bodies — a birth resolves as a new adult.

namespace vg
{
	// Named look slots. Mara is 0. Living population is kStarterHumans and can grow.
	constexpr int kNamedCast = 8;
	constexpr int kEarthHumans = kNamedCast;

	constexpr int kTribeCount = 8;
	constexpr int kContinentCount = 8;
	constexpr int kStarterMembers = 4; // one teacher + three learners
	constexpr int kStarterHumans = kTribeCount * kStarterMembers;
	constexpr int kMaxHumans = 64;
	constexpr int kMinAdultAge = 21;
	constexpr int kSkillCount = 8;
	constexpr int kMaxShelters = 48;
	constexpr int kMaxSheltersPerTribe = 6;
	constexpr int kMaxStructuresPerTribe = 8;
	constexpr int kMaxAnimals = 24;
	constexpr int kAnimalsPerTribe = 2;

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
		HighGround,
		Teach,
		Build
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

	enum class SpeechContext
	{
		Hunt,
		Eat,
		Sleep,
		Talk,
		Camp,
		Rain,
		Tornado,
		Hurricane,
		Flood,
		Clear,
		Teach,
		Build,
		Birth
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
		const char* LastSpeech = nullptr;
		float TargetX = 0.f;
		float TargetY = 0.f;
		bool CarryingKill = false;
		int TribeId = 0;
		int LookId = 0;
		bool bTeacher = false;
		float Knowledge = 10.f;
		float Skill[kSkillCount]{};
		float Bond = 0.f;
		bool bWorked = false;
	};

	struct Animal
	{
		int Id = 0;
		int TribeId = 0;
		float X = 0.f;
		float Y = 0.f;
		float Heading = 0.f;
		bool Alive = true;
		float RespawnIn = 0.f;
	};

	struct Tribe
	{
		int Id = 0;
		const char* Name = "";
		int ContinentId = 0;
		int TeacherId = -1;
		float CampX = 0.f;
		float CampY = 0.f;
		float FireX = 0.f;
		float FireY = 0.f;
		float HuntX = 0.f;
		float HuntY = 0.f;
		float HighX = 0.f;
		float HighY = 0.f;
		float ClaimRadius = 900.f;
		float Knowledge = 0.f;
		int StructureCount = 0;
		int ShelterCount = 0;
		int FirstShelter = 0;
		float BuildCooldown = 6.f;
		float SurvivalSeconds = 0.f;
		float ScarceSeconds = 0.f;
		int Births = 0;
		// Personality. They can lie, cheat, or steal later. None of this starts a war.
		float Honesty = 0.5f;
		float Greed = 0.3f;
		float Ambition = 0.15f;
		const char* Temper = "";
	};

	// One directed link. Hostile stays false until a later slice, when population,
	// skill, and ambition are all high. Starter camps are neighbors, not enemies.
	struct Relation
	{
		float Trust = 55.f;
		float Trade = 50.f;
		float Betrayal = 12.f;
		bool Hostile = false;
	};

	struct Continent
	{
		int Id = 0;
		const char* Name = "";
		float X = 0.f;
		float Y = 0.f;
		float Radius = 1000.f;
		int TribeId = -1;
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

		Villager Villagers[kMaxHumans];
		int VillagerCount = 0;
		Animal Animals[kMaxAnimals];
		int AnimalCount = 0;

		Tribe Tribes[kTribeCount];
		int TribeCount = 0;
		Continent Continents[kContinentCount];
		int ContinentCount = 0;

		// Tribe 0 camp, mirrored so the sculpted valley props stay put.
		float CampX = 0.f;
		float CampY = 700.f;
		float FireX = 0.f;
		float FireY = 640.f;
		float HuntX = 200.f;
		float HuntY = 2600.f;
		float HighX = 0.f;
		float HighY = -2800.f;
		float ShelterX[kMaxShelters]{};
		float ShelterY[kMaxShelters]{};
		int ShelterCount = 0;

		float MealCountdown = 0.f;
		float SleepCountdown = 0.f;
		float DawnCountdown = 0.f;
		int Births = 0;
		int KinCursor = 0;
		Relation Relations[kTribeCount * kTribeCount];

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
	const char* TribeLabel(const World& W, int TribeId);
	int LineCount();
	const char* LineAt(int Index);
	int SpeechPoolSize(SpeechContext Context);
	const char* SpeechPoolLine(SpeechContext Context, int Index);
	const char* PickSpeech(World& W, Villager& V, SpeechContext Context);
	bool IsAdult(const Villager& V);
	int ContinentAt(const World& W, float X, float Y);
	bool IsClaimedLand(const World& W, float X, float Y, int& OutTribe);
	const Relation& RelationBetween(const World& W, int FromTribe, int ToTribe);
	bool TribesAtWar(const World& W, int TribeA, int TribeB);
	bool ConflictReady(const World& W, int TribeId);
	bool IsNight(float Hours);
	float HoursUntil(float Now, float TargetHour);
	float Rand01(World& W);
} // namespace vg
