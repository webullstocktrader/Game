#include "ValleySim.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace vg
{
	namespace
	{
		constexpr float kPi = 3.14159265f;

		const char* kLines[] = {
			"The dirt is still wet from last night.",
			"We should check the snares.",
			"That ridge keeps the north wind off the huts.",
			"I saw tracks by the willows.",
			"Stone this sharp will cut hide.",
			"Leave some berries for later.",
			"Come sit by the fire when you are done.",
			"Your spear is crooked. I can straighten it.",
			"The deer went west at first light.",
			"Drink from the fast water, not the still pool.",
			"Help me lift this hide.",
			"Quiet. It will hear us.",
			"The animal is down. We eat tonight.",
			"Hot fat. Good.",
			"Eat. Walk later.",
			"The dark is here. I am lying down.",
			"Rain. Under the hides.",
			"Keep the fire covered.",
			"The wind is breaking trees. Run!",
			"Get off the flat ground!",
			"Hold the posts. Stay low.",
			"This wind will last. Stay inside.",
			"The river is climbing. Go up.",
			"Leave the bank. Now.",
			"It passed. We can work.",
			"Sky is quieter.",
			"The valley smells like clay and smoke.",
			"Walk with me to the river stones.",
			"The night is thick with lights.",
		};

		const char* kNames[] = {
			"Mara", "Nima", "Lira", "Sable", "Flint", "Oak", "Reed", "Bram"
		};

		const Sex kBodies[] = {
			Sex::Female, Sex::Female, Sex::Female, Sex::Female,
			Sex::Male, Sex::Male, Sex::Male, Sex::Male
		};

		const int kAges[] = { 27, 32, 41, 24, 38, 44, 29, 34 };

		const Habit kRoles[] = {
			Habit::Hunter, Habit::Tender, Habit::Knaps, Habit::Wander,
			Habit::Hunter, Habit::Tender, Habit::Wander, Habit::Tender
		};

		const char* kTraits[] = {
			"Watches the tree line. Speaks little.",
			"Keeps the fire and the talk going.",
			"Knaps stone by the river. Patient.",
			"Restless. Walks the ridge for berries.",
			"First to the hunt. Does not boast.",
			"Slow. Carries the heavy wood.",
			"Quiet. Follows the river fish.",
			"Laughs at the fire. Fixes hides."
		};

		const char* kPersonal[] = {
			"Tracks go west.",
			"Sit. The fat is still hot.",
			"This edge will cut.",
			"Berries on the south bank.",
			"Quiet. Wind is wrong.",
			"Wood first. Then we eat.",
			"The fast water has fish.",
			"Leave the hide to dry."
		};

		float Dist2(float AX, float AY, float BX, float BY)
		{
			const float DX = AX - BX;
			const float DY = AY - BY;
			return std::sqrt(DX * DX + DY * DY);
		}

		void MoveToward(Villager& V, float TX, float TY, float Speed, float Dt)
		{
			const float DX = TX - V.X;
			const float DY = TY - V.Y;
			const float D = std::sqrt(DX * DX + DY * DY);
			if (D < 8.f)
			{
				V.X = TX;
				V.Y = TY;
				return;
			}
			V.X += DX / D * Speed * Dt;
			V.Y += DY / D * Speed * Dt;
			V.Heading = std::atan2(DY, DX);
		}

		void Speak(Villager& V, const char* Line, float Seconds)
		{
			V.Speech = Line;
			V.TalkTimer = Seconds;
		}

		const char* PickLine(World& W, int Begin, int Count)
		{
			const int I = Begin + static_cast<int>(Rand01(W) * static_cast<float>(Count)) % Count;
			return kLines[I];
		}

		void ChooseNeed(World& W, Villager& V)
		{
			if (IsNight(W.TimeOfDayHours) && V.Energy < 85.f)
			{
				V.Current = Activity::Sleep;
				V.TargetX = W.ShelterX[V.ShelterIndex];
				V.TargetY = W.ShelterY[V.ShelterIndex];
				Speak(V, kLines[15], 3.f);
				return;
			}
			if (V.Hunger < 32.f)
			{
				V.Current = Activity::Eat;
				V.TargetX = W.FireX;
				V.TargetY = W.FireY;
				return;
			}
			if (V.Energy < 22.f)
			{
				V.Current = Activity::Sleep;
				V.TargetX = W.ShelterX[V.ShelterIndex];
				V.TargetY = W.ShelterY[V.ShelterIndex];
				return;
			}
			if (V.Hunger < 62.f && !V.CarryingKill && (V.Role == Habit::Hunter || Rand01(W) < 0.35f))
			{
				V.Current = Activity::Hunt;
				int Best = -1;
				float BestD = 1.0e9f;
				for (int I = 0; I < W.AnimalCount; ++I)
				{
					if (!W.Animals[I].Alive)
					{
						continue;
					}
					const float D = Dist2(V.X, V.Y, W.Animals[I].X, W.Animals[I].Y);
					if (D < BestD)
					{
						BestD = D;
						Best = I;
					}
				}
				V.HuntTarget = Best;
				if (Best >= 0)
				{
					V.TargetX = W.Animals[Best].X;
					V.TargetY = W.Animals[Best].Y;
				}
				else
				{
					V.TargetX = W.HuntX;
					V.TargetY = W.HuntY;
				}
				return;
			}
			if (V.Role == Habit::Tender && Rand01(W) < 0.55f)
			{
				V.Current = Activity::Talk;
				V.StateTimer = 2.8f + Rand01(W) * 2.f;
				Speak(V, V.PersonalLine && V.PersonalLine[0] ? V.PersonalLine : PickLine(W, 6, 5), V.StateTimer);
				return;
			}
			if (Rand01(W) < 0.35f)
			{
				V.Current = Activity::Talk;
				V.StateTimer = 2.5f + Rand01(W) * 2.f;
				Speak(V, (V.PersonalLine && Rand01(W) < 0.6f) ? V.PersonalLine : PickLine(W, 6, 5), V.StateTimer);
				return;
			}
			V.Current = Activity::Walk;
			if (V.Role == Habit::Knaps)
			{
				V.TargetX = (Rand01(W) * 2.f - 1.f) * 400.f;
				V.TargetY = (Rand01(W) * 2.f - 1.f) * 200.f;
			}
			else if (V.Role == Habit::Wander)
			{
				V.TargetX = W.CampX + (Rand01(W) * 2.f - 1.f) * 1400.f;
				V.TargetY = W.CampY + (Rand01(W) * 2.f - 1.f) * 1200.f;
			}
			else
			{
				V.TargetX = W.CampX + (Rand01(W) * 2.f - 1.f) * 900.f;
				V.TargetY = W.CampY + (Rand01(W) * 2.f - 1.f) * 700.f;
			}
			if (Rand01(W) < 0.3f)
			{
				Speak(V, V.PersonalLine && V.PersonalLine[0] ? V.PersonalLine : PickLine(W, 0, 6), 3.2f);
			}
		}

		void ApplyWeatherIntent(World& W, Villager& V)
		{
			switch (W.Sky)
			{
			case Weather::Rain:
				V.Current = Activity::Shelter;
				V.TargetX = W.ShelterX[V.ShelterIndex];
				V.TargetY = W.ShelterY[V.ShelterIndex];
				Speak(V, Rand01(W) < 0.5f ? kLines[16] : kLines[17], 4.f);
				break;
			case Weather::Tornado:
				V.Current = Activity::Panic;
				{
					const float DX = V.X - W.TornadoX;
					const float DY = V.Y - W.TornadoY;
					const float D = std::sqrt(DX * DX + DY * DY) + 1.f;
					V.TargetX = V.X + DX / D * 1400.f;
					V.TargetY = V.Y + DY / D * 1400.f;
				}
				Speak(V, Rand01(W) < 0.5f ? kLines[18] : kLines[19], 3.5f);
				break;
			case Weather::Hurricane:
				V.Current = Activity::Shelter;
				V.TargetX = W.ShelterX[V.ShelterIndex];
				V.TargetY = W.ShelterY[V.ShelterIndex];
				Speak(V, Rand01(W) < 0.5f ? kLines[20] : kLines[21], 4.f);
				break;
			case Weather::Flood:
				V.Current = Activity::HighGround;
				V.TargetX = W.HighX + (Rand01(W) * 2.f - 1.f) * 400.f;
				V.TargetY = W.HighY;
				Speak(V, Rand01(W) < 0.5f ? kLines[22] : kLines[23], 4.f);
				break;
			case Weather::Clear:
			default:
				break;
			}
		}

		void TickVillager(World& W, Villager& V, float Dt, float HoursPerSec)
		{
			if (V.TalkTimer > 0.f)
			{
				V.TalkTimer -= Dt;
				if (V.TalkTimer <= 0.f)
				{
					V.Speech = "";
					V.TalkTimer = 0.f;
				}
			}

			const bool Storm = W.Sky != Weather::Clear;
			if (Storm)
			{
				ApplyWeatherIntent(W, V);
			}

			float Drain = HoursPerSec * Dt;
			if (V.Current == Activity::Sleep)
			{
				V.Energy = std::min(100.f, V.Energy + Drain * 55.f);
				V.Hunger = std::max(0.f, V.Hunger - Drain * 4.f);
			}
			else if (V.Current == Activity::Eat)
			{
				V.Hunger = std::min(100.f, V.Hunger + Drain * 70.f);
				V.Energy = std::min(100.f, V.Energy + Drain * 8.f);
			}
			else
			{
				V.Hunger = std::max(0.f, V.Hunger - Drain * 18.f);
				V.Energy = std::max(0.f, V.Energy - Drain * 12.f);
			}

			const bool NeedsFood = V.Hunger < 32.f;
			const bool NeedsSleep = V.Energy < 22.f || (IsNight(W.TimeOfDayHours) && V.Energy < 85.f);
			if (!Storm
				&& (V.Current == Activity::Idle || V.Current == Activity::Walk || V.Current == Activity::Talk)
				&& (V.StateTimer <= 0.f || NeedsFood || NeedsSleep))
			{
				ChooseNeed(W, V);
			}

			if (!Storm && V.Current == Activity::Talk)
			{
				V.StateTimer -= Dt;
				if (V.StateTimer <= 0.f)
				{
					V.Current = Activity::Idle;
				}
				return;
			}

			float Speed = 160.f;
			if (V.Current == Activity::Panic)
			{
				Speed = 420.f;
			}
			else if (V.Current == Activity::Hunt || V.Current == Activity::HighGround)
			{
				Speed = 360.f;
			}
			else if (V.Current == Activity::Sleep && Dist2(V.X, V.Y, V.TargetX, V.TargetY) < 90.f)
			{
				Speed = 0.f;
			}

			if (V.Current == Activity::Hunt)
			{
				if (V.HuntTarget < 0 || V.HuntTarget >= W.AnimalCount || !W.Animals[V.HuntTarget].Alive)
				{
					int Best = -1;
					float BestD = 1.0e9f;
					for (int I = 0; I < W.AnimalCount; ++I)
					{
						if (!W.Animals[I].Alive)
						{
							continue;
						}
						const float D = Dist2(V.X, V.Y, W.Animals[I].X, W.Animals[I].Y);
						if (D < BestD)
						{
							BestD = D;
							Best = I;
						}
					}
					V.HuntTarget = Best;
				}
			}

			if (V.Current == Activity::Hunt && V.HuntTarget >= 0 && V.HuntTarget < W.AnimalCount)
			{
				Animal& Prey = W.Animals[V.HuntTarget];
				if (Prey.Alive)
				{
					V.TargetX = Prey.X;
					V.TargetY = Prey.Y;
					if (Dist2(V.X, V.Y, Prey.X, Prey.Y) < 130.f)
					{
						Prey.Alive = false;
						Prey.RespawnIn = 18.f;
						V.CarryingKill = true;
						V.Current = Activity::Eat;
						V.TargetX = W.FireX;
						V.TargetY = W.FireY;
						Speak(V, kLines[12], 4.f);
					}
					else if (V.TalkTimer <= 0.f && Rand01(W) < 0.02f)
					{
						Speak(V, kLines[11], 2.5f);
					}
				}
				else
				{
					V.HuntTarget = -1;
					V.Current = Activity::Idle;
				}
			}

			if (V.Current == Activity::Eat && Dist2(V.X, V.Y, W.FireX, W.FireY) < 80.f)
			{
				if (V.CarryingKill)
				{
					V.CarryingKill = false;
				}
				if (V.TalkTimer <= 0.f)
				{
					Speak(V, Rand01(W) < 0.5f ? kLines[13] : kLines[14], 3.f);
				}
				if (V.Hunger > 88.f && !Storm)
				{
					V.Current = Activity::Idle;
					V.StateTimer = 0.f;
				}
			}

			MoveToward(V, V.TargetX, V.TargetY, Speed, Dt);
		}

		void TickAnimals(World& W, float Dt)
		{
			for (int I = 0; I < W.AnimalCount; ++I)
			{
				Animal& A = W.Animals[I];
				if (!A.Alive)
				{
					A.RespawnIn -= Dt;
					if (A.RespawnIn <= 0.f)
					{
						A.Alive = true;
						A.X = W.HuntX + (Rand01(W) * 2.f - 1.f) * 900.f;
						A.Y = W.HuntY + (Rand01(W) * 2.f - 1.f) * 700.f;
					}
					continue;
				}

				float FleeX = 0.f;
				float FleeY = 0.f;
				bool Flee = false;
				for (int V = 0; V < W.VillagerCount; ++V)
				{
					if (W.Villagers[V].Current != Activity::Hunt)
					{
						continue;
					}
					if (Dist2(A.X, A.Y, W.Villagers[V].X, W.Villagers[V].Y) < 700.f)
					{
						Flee = true;
						FleeX += A.X - W.Villagers[V].X;
						FleeY += A.Y - W.Villagers[V].Y;
					}
				}
				float TX = A.X;
				float TY = A.Y;
				float Speed = 90.f;
				if (Flee)
				{
					Speed = 210.f;
					TX = A.X + FleeX;
					TY = A.Y + FleeY;
				}
				else
				{
					TX = A.X + (Rand01(W) * 2.f - 1.f) * 200.f;
					TY = A.Y + (Rand01(W) * 2.f - 1.f) * 200.f;
				}
				const float DX = TX - A.X;
				const float DY = TY - A.Y;
				const float D = std::sqrt(DX * DX + DY * DY) + 0.001f;
				A.X += DX / D * Speed * Dt;
				A.Y += DY / D * Speed * Dt;
				A.X = W.HuntX + std::max(-1100.f, std::min(1100.f, A.X - W.HuntX));
				A.Y = W.HuntY + std::max(-900.f, std::min(900.f, A.Y - W.HuntY));
				A.Heading = std::atan2(DY, DX);
			}
		}

		void RefreshCountdowns(World& W)
		{
			W.DawnCountdown = HoursUntil(W.TimeOfDayHours, 6.f) / 24.f * W.DayLengthSeconds;
			W.SleepCountdown = HoursUntil(W.TimeOfDayHours, 20.f) / 24.f * W.DayLengthSeconds;
			const float Noon = HoursUntil(W.TimeOfDayHours, 12.f);
			const float DuskMeal = HoursUntil(W.TimeOfDayHours, 18.f);
			const float NextMealHours = Noon < DuskMeal ? Noon : DuskMeal;
			W.MealCountdown = NextMealHours / 24.f * W.DayLengthSeconds;
		}
	} // namespace

	float Rand01(World& W)
	{
		W.Rng = W.Rng * 1664525u + 1013904223u;
		return static_cast<float>((W.Rng >> 8) & 0x00FFFFFF) / static_cast<float>(0x01000000);
	}

	bool IsNight(float Hours)
	{
		return Hours < 6.f || Hours >= 20.f;
	}

	float HoursUntil(float Now, float TargetHour)
	{
		float D = TargetHour - Now;
		if (D <= 0.f)
		{
			D += 24.f;
		}
		return D;
	}

	void InitWorld(World& W)
	{
		W = World{};
		W.DayLengthSeconds = 75.f;
		W.TimeOfDayHours = 10.f;
		W.Rng = 1u;
		W.ShelterCount = 5;
		W.ShelterX[0] = -240.f;
		W.ShelterY[0] = 820.f;
		W.ShelterX[1] = 260.f;
		W.ShelterY[1] = 840.f;
		W.ShelterX[2] = -420.f;
		W.ShelterY[2] = 520.f;
		W.ShelterX[3] = 430.f;
		W.ShelterY[3] = 500.f;
		W.ShelterX[4] = 40.f;
		W.ShelterY[4] = 980.f;

		W.VillagerCount = 8;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			Villager& V = W.Villagers[I];
			V.Id = I;
			V.Name = kNames[I];
			V.Body = kBodies[I];
			V.Role = kRoles[I];
			V.AgeYears = kAges[I];
			V.Trait = kTraits[I];
			V.PersonalLine = kPersonal[I];
			const float Ang = static_cast<float>(I) / static_cast<float>(W.VillagerCount) * 2.f * kPi;
			V.X = W.CampX + std::cos(Ang) * 280.f;
			V.Y = W.CampY + std::sin(Ang) * 220.f;
			V.Z = 0.f;
			V.Hunger = 48.f + Rand01(W) * 40.f;
			V.Energy = 55.f + Rand01(W) * 35.f;
			V.ShelterIndex = I % W.ShelterCount;
			V.TargetX = V.X;
			V.TargetY = V.Y;
			V.Current = Activity::Idle;
			if (I == 1 || I == 7)
			{
				V.Current = Activity::Talk;
				V.StateTimer = 4.f;
				V.TalkTimer = 4.f;
				V.Speech = kPersonal[I];
			}
		}

		W.AnimalCount = 4;
		for (int I = 0; I < W.AnimalCount; ++I)
		{
			W.Animals[I].Id = I;
			W.Animals[I].Alive = true;
			W.Animals[I].X = W.HuntX + (Rand01(W) * 2.f - 1.f) * 600.f;
			W.Animals[I].Y = W.HuntY + (Rand01(W) * 2.f - 1.f) * 500.f;
		}

		RefreshCountdowns(W);
	}

	void SetWeather(World& W, Weather Wx)
	{
		W.Sky = Wx;
		W.SkySecondsLeft = (Wx == Weather::Clear) ? 0.f : 28.f;
		W.FloodHeight = (Wx == Weather::Flood) ? 110.f : 0.f;
		if (Wx == Weather::Hurricane)
		{
			W.WindX = 1.f;
			W.WindY = 0.35f;
		}
		else if (Wx == Weather::Tornado)
		{
			W.WindX = 0.4f;
			W.WindY = 0.2f;
		}
		else if (Wx == Weather::Rain)
		{
			W.WindX = 0.1f;
			W.WindY = 0.f;
		}
		else
		{
			W.WindX = 0.f;
			W.WindY = 0.f;
			W.FloodHeight = 0.f;
		}

		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (Wx == Weather::Clear)
			{
				W.Villagers[I].Current = Activity::Idle;
				W.Villagers[I].StateTimer = 0.f;
				Speak(W.Villagers[I], Rand01(W) < 0.5f ? kLines[24] : kLines[25], 3.f);
			}
			else
			{
				ApplyWeatherIntent(W, W.Villagers[I]);
			}
		}
	}

	void SetDayLength(World& W, float Seconds)
	{
		if (Seconds < 30.f)
		{
			Seconds = 30.f;
		}
		if (Seconds > 240.f)
		{
			Seconds = 240.f;
		}
		W.DayLengthSeconds = Seconds;
	}

	void SetPaused(World& W, bool Paused)
	{
		W.Paused = Paused;
	}

	void TickWorld(World& W, float Dt)
	{
		if (W.Paused || Dt <= 0.f)
		{
			return;
		}

		const float HoursPerSec = 24.f / W.DayLengthSeconds;
		W.TimeOfDayHours = W.TimeOfDayHours + Dt * HoursPerSec;
		while (W.TimeOfDayHours >= 24.f)
		{
			W.TimeOfDayHours -= 24.f;
		}

		if (W.Sky != Weather::Clear)
		{
			W.SkySecondsLeft -= Dt;
			if (W.SkySecondsLeft <= 0.f)
			{
				SetWeather(W, Weather::Clear);
			}
		}

		if (W.Sky == Weather::Tornado)
		{
			W.TornadoAngle += Dt * 0.7f;
			W.TornadoX = std::cos(W.TornadoAngle) * 1400.f;
			W.TornadoY = 900.f + std::sin(W.TornadoAngle * 0.85f) * 1100.f;
		}

		if (W.Sky == Weather::Flood)
		{
			W.FloodHeight = 90.f + 30.f * std::sin(W.TimeOfDayHours);
		}

		for (int I = 0; I < W.VillagerCount; ++I)
		{
			TickVillager(W, W.Villagers[I], Dt, HoursPerSec);
		}
		TickAnimals(W, Dt);
		RefreshCountdowns(W);
	}

	const char* ActivityName(Activity A)
	{
		switch (A)
		{
		case Activity::Walk: return "Walk";
		case Activity::Talk: return "Talk";
		case Activity::Hunt: return "Hunt";
		case Activity::Eat: return "Eat";
		case Activity::Sleep: return "Sleep";
		case Activity::Shelter: return "Shelter";
		case Activity::Panic: return "Panic";
		case Activity::HighGround: return "High ground";
		case Activity::Idle:
		default: return "Idle";
		}
	}

	const char* SexName(Sex Body)
	{
		return Body == Sex::Female ? "Woman" : "Man";
	}

	const char* WeatherName(Weather Wx)
	{
		switch (Wx)
		{
		case Weather::Rain: return "Rain";
		case Weather::Tornado: return "Tornado";
		case Weather::Hurricane: return "Hurricane";
		case Weather::Flood: return "Flood";
		case Weather::Clear:
		default: return "Clear";
		}
	}

	int LineCount()
	{
		return static_cast<int>(sizeof(kLines) / sizeof(kLines[0]));
	}

	const char* LineAt(int Index)
	{
		if (Index < 0 || Index >= LineCount())
		{
			return "";
		}
		return kLines[Index];
	}
} // namespace vg
