#include "ValleySim.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace vg
{
	namespace
	{
		constexpr float kPi = 3.14159265f;
		constexpr float kValleyRadius = 10000.f;
		constexpr float kOuterRadius = 3600.f;
		constexpr float kOuterOrbit = 16800.f;
		constexpr float kBondNeed = 100.f;
		constexpr float kBondPerSecond = 3.2f;
		constexpr float kPairRange = 780.f;
		constexpr float kTeachRange = 420.f;
		constexpr float kChopPerSecond = 50.f;
		constexpr float kBuildPerSecond = 36.f;
		constexpr float kCraftPerSecond = 45.f;
		constexpr float kResearchPerSecond = 3.f;

		const char* kHuntLines[] = {
			"Tracks go into the brush.",
			"Quiet. It will hear us.",
			"Stay low. The wind is wrong.",
			"I see it. Do not run yet.",
			"The herd moved past the stones.",
			"Hold the spear up. Feet soft.",
			"It stopped. Wait.",
			"Blood on the grass. Close now.",
		};

		const char* kEatLines[] = {
			"Hot fat. Sit and eat.",
			"Eat. Walk later.",
			"Leave some for the fire.",
			"The meat is ready. Come in.",
			"Burnt edge, sweet middle.",
			"Pass the bone. I am not done.",
			"This will hold us till dark.",
			"Drink, then take a share.",
		};

		const char* kSleepLines[] = {
			"The dark is here. I am lying down.",
			"Keep a hand on the hide.",
			"I will wake if the wind turns.",
			"Eyes shut. The fire can watch.",
			"My legs are done. Move over.",
			"Sleep. The snares can wait.",
			"The night is thick with lights.",
			"Do not kick. I am sleeping.",
		};

		const char* kTalkLines[] = {
			"The dirt is still wet from last night.",
			"Come sit by the fire when you are done.",
			"Walk with me to the river stones.",
			"Your spear is crooked. I can straighten it.",
			"Leave some berries for later.",
			"That ridge keeps the north wind off the huts.",
			"I dreamed of rain and woke dry.",
			"Tell it again. I missed the end.",
			"The smoke goes straight up. A good sign.",
			"Sit. I saved you the soft bit.",
		};

		const char* kCampLines[] = {
			"We should check the snares.",
			"Stone this sharp will cut hide.",
			"I saw tracks by the willows.",
			"Help me lift this hide.",
			"The fast water is the one to drink.",
			"Berries on the south bank.",
			"Wood first. Then we eat.",
			"The ground smells like clay and smoke.",
		};

		const char* kRainLines[] = {
			"Rain. Under the hides.",
			"Keep the fire covered.",
			"The sky is letting go. Come in.",
			"Wet hides. Move.",
			"Drips through the roof. Shift over.",
			"It will pass. Stay dry.",
		};

		const char* kTornadoLines[] = {
			"The wind is breaking trees. Run!",
			"Get off the flat ground!",
			"Down! Cover your head!",
			"That spin is coming. Move!",
			"Leave the pots. Run wide!",
			"Do not stand in the open!",
		};

		const char* kHurricaneLines[] = {
			"Hold the posts. Stay low.",
			"This wind will last. Stay inside.",
			"The whole sky is pushing. Down.",
			"Tie the hides. Then sit.",
			"If the post goes, roll clear.",
			"Listen. It is not done yet.",
		};

		const char* kFloodLines[] = {
			"The river is climbing. Go up.",
			"Leave the bank. Now.",
			"Water over the stones. Higher.",
			"Do not wade. Climb.",
			"The low huts will soak. Up the rise.",
			"Grab the dry hides. Up.",
		};

		const char* kClearLines[] = {
			"It passed. We can work.",
			"Sky is quieter.",
			"Come out. The wind dropped.",
			"Count us. Then the fire.",
			"Branches down. We can use them.",
			"The ground drinks. Then we walk.",
		};

		const char* kTeachLines[] = {
			"Watch my hands. Then you try.",
			"Hold the stone like this.",
			"The edge faces out. See?",
			"Again. Slow.",
			"You have it. Do not rush.",
			"Fire needs dry grass first.",
			"Spear high. Feet quiet.",
			"This knot holds the hide.",
		};

		const char* kBuildLines[] = {
			"We need another roof.",
			"Set the post here.",
			"More hides for the wall.",
			"This ground will hold a hut.",
			"Lift. I have the other side.",
			"The new shelter faces the wind.",
			"Stack the wood before dark.",
			"Our camp can take one more.",
		};

		const char* kCraftLines[] = {
			"This stone will bite.",
			"Bind it tight. The edge is sharp.",
			"A spear for the brush.",
			"Hold the shaft. I have the point.",
			"The point sits on the wood.",
			"One spear by the fire.",
		};

		const char* kBirthLines[] = {
			"Another pair of hands by the fire.",
			"They are grown. They eat with us.",
			"Make room. One more sleeps here.",
			"The camp is louder tonight.",
		};

		struct Pool
		{
			SpeechContext Context;
			const char* const* Lines;
			int Count;
		};

		const Pool kPools[] = {
			{SpeechContext::Hunt, kHuntLines, static_cast<int>(sizeof(kHuntLines) / sizeof(kHuntLines[0]))},
			{SpeechContext::Eat, kEatLines, static_cast<int>(sizeof(kEatLines) / sizeof(kEatLines[0]))},
			{SpeechContext::Sleep, kSleepLines, static_cast<int>(sizeof(kSleepLines) / sizeof(kSleepLines[0]))},
			{SpeechContext::Talk, kTalkLines, static_cast<int>(sizeof(kTalkLines) / sizeof(kTalkLines[0]))},
			{SpeechContext::Camp, kCampLines, static_cast<int>(sizeof(kCampLines) / sizeof(kCampLines[0]))},
			{SpeechContext::Rain, kRainLines, static_cast<int>(sizeof(kRainLines) / sizeof(kRainLines[0]))},
			{SpeechContext::Tornado, kTornadoLines, static_cast<int>(sizeof(kTornadoLines) / sizeof(kTornadoLines[0]))},
			{SpeechContext::Hurricane, kHurricaneLines, static_cast<int>(sizeof(kHurricaneLines) / sizeof(kHurricaneLines[0]))},
			{SpeechContext::Flood, kFloodLines, static_cast<int>(sizeof(kFloodLines) / sizeof(kFloodLines[0]))},
			{SpeechContext::Clear, kClearLines, static_cast<int>(sizeof(kClearLines) / sizeof(kClearLines[0]))},
			{SpeechContext::Teach, kTeachLines, static_cast<int>(sizeof(kTeachLines) / sizeof(kTeachLines[0]))},
			{SpeechContext::Build, kBuildLines, static_cast<int>(sizeof(kBuildLines) / sizeof(kBuildLines[0]))},
			{SpeechContext::Birth, kBirthLines, static_cast<int>(sizeof(kBirthLines) / sizeof(kBirthLines[0]))},
			{SpeechContext::Craft, kCraftLines, static_cast<int>(sizeof(kCraftLines) / sizeof(kCraftLines[0]))},
		};

		const char* kTeacherNames[] = {
			"Mara", "Nima", "Lira", "Sable", "Flint", "Oak", "Reed", "Bram"
		};

		const Sex kTeacherBodies[] = {
			Sex::Female, Sex::Female, Sex::Female, Sex::Female,
			Sex::Male, Sex::Male, Sex::Male, Sex::Male
		};

		const int kTeacherAges[] = {27, 32, 41, 24, 38, 44, 29, 34};

		const Habit kTeacherRoles[] = {
			Habit::Hunter, Habit::Tender, Habit::Knaps, Habit::Wander,
			Habit::Hunter, Habit::Tender, Habit::Wander, Habit::Tender
		};

		const char* kTeacherTraits[] = {
			"Watches the tree line. Speaks little. Teaches the hunt.",
			"Keeps the fire and the talk going. Teaches the hearth.",
			"Knaps stone by the river. Patient teacher.",
			"Restless. Walks the ridge and shows the paths.",
			"First to the hunt. Teaches without boasting.",
			"Slow. Carries the heavy wood and shows how.",
			"Quiet. Follows the river and teaches the fish.",
			"Laughs at the fire. Teaches the hides."
		};

		const char* kTeacherLines[] = {
			"Tracks go west.",
			"Sit. The fat is still hot.",
			"This edge will cut.",
			"Berries on the south bank.",
			"Quiet. Wind is wrong.",
			"Wood first. Then we eat.",
			"The fast water has fish.",
			"Leave the hide to dry."
		};

		const char* kLearnerNames[] = {
			"Hana", "Vela", "Tor",
			"Pela", "Ivo", "Sura",
			"Ness", "Kerr", "Wyn",
			"Aru", "Moss", "Keel",
			"Ryn", "Holt", "Tavi",
			"Orla", "Joss", "Ede",
			"Quin", "Faye", "Colm",
			"Nix", "Bree", "Ash"
		};

		const char* kLearnerTraits[] = {
			"Learning the hunt. Still loud on the grass.",
			"Learning the fire. Hands careful.",
			"Learning the stone. Asks twice.",
			"Learning the hearth. Talks while they work.",
			"Learning the wood. Strong arms.",
			"Learning the paths. Easy to lose.",
			"Learning the river stones.",
			"Learning the hides. Slow knots.",
			"Learning the ridge berries.",
			"Learning the spear. Feet too fast.",
			"Learning the smoke and the pots.",
			"Learning which stone splits.",
			"Learning the night watch.",
			"Learning to carry without dropping.",
			"Learning the fish pools.",
			"Learning the dry racks.",
			"Learning to sit still.",
			"Learning the wind on the rise.",
			"Learning the cut of a hide.",
			"Learning when to speak.",
			"Learning the heavy end of a log.",
			"Learning the snares.",
			"Learning the low paths.",
			"Learning to share the fat."
		};

		const char* kLearnerLines[] = {
			"Show me the tracks again.",
			"Is the fat hot yet?",
			"Does this edge cut?",
			"I can take the south bank.",
			"The wind feels wrong.",
			"Wood is heavy. I have it.",
			"Fish jumped. I saw it.",
			"The hide is still damp.",
			"Berries here, if I remember.",
			"My spear dips. Watch.",
			"I fed the fire.",
			"This stone looks right.",
			"I can watch till you sleep.",
			"Where do I set this?",
			"The pool is fast today.",
			"Racks need more sun.",
			"I will sit. Tell me.",
			"The rise is cold.",
			"Knots slip on me.",
			"I am listening.",
			"Log goes here?",
			"Snares checked. I think.",
			"Which path is low?",
			"I left you the soft bit."
		};

		const char* kKinNames[] = {
			"Una", "Pell", "Soren", "Dara", "Kest", "Mio", "Voss", "Leni",
			"Harp", "Oda", "Cress", "Jori", "Nils", "Tess", "Ida", "Roan",
			"Cal", "Petra", "Hugo", "Nia", "Mira", "Otto", "Sera", "Gil",
			"Posy", "Wren", "Hal", "Ione", "Beck", "Noor", "Leif", "Esme"
		};

		const char* kTribeNames[] = {
			"Willow", "Red bluff", "Salt", "Dark wood",
			"High stone", "Reed water", "Cold ridge", "Ash shore"
		};

		// Humans, not monsters. Greed and low honesty are the capacity to cheat.
		// Ambition stays too low to open a fight.
		const float kHonesty[] = {0.72f, 0.42f, 0.84f, 0.55f, 0.48f, 0.78f, 0.66f, 0.34f};
		const float kGreed[] = {0.28f, 0.58f, 0.18f, 0.46f, 0.40f, 0.22f, 0.33f, 0.70f};
		const float kAmbition[] = {0.18f, 0.32f, 0.12f, 0.36f, 0.28f, 0.16f, 0.22f, 0.40f};
		const char* kTempers[] = {
			"Quiet. May bend a tale to keep the meat.",
			"Warm talk. Will short a count of hides.",
			"Patient. Trades straight unless the pot is empty.",
			"Restless. Keeps a path they do not share.",
			"Steady hunter. Will claim a kill was smaller.",
			"Slow and fair. Hides a good log now and then.",
			"Soft voice. May lie about where the fish went.",
			"Laughs. Will trade, and will pocket a spare stone."
		};

		const char* kContinentNames[] = {
			"Willow basin", "Red bluff", "Salt flats", "Dark wood",
			"High stones", "Reed water", "Cold ridge", "Ash shore"
		};

		const Pool* FindPool(SpeechContext Context)
		{
			const int Count = static_cast<int>(sizeof(kPools) / sizeof(kPools[0]));
			for (int I = 0; I < Count; ++I)
			{
				if (kPools[I].Context == Context)
				{
					return &kPools[I];
				}
			}
			return &kPools[0];
		}

		float Dist(float AX, float AY, float BX, float BY)
		{
			const float DX = AX - BX;
			const float DY = AY - BY;
			return std::sqrt(DX * DX + DY * DY);
		}

		int SkillFor(Habit Role)
		{
			switch (Role)
			{
			case Habit::Hunter: return 0;
			case Habit::Tender: return 1;
			case Habit::Knaps: return 2;
			case Habit::Wander: return 5;
			default: return 0;
			}
		}

		Tribe* Home(World& W, const Villager& V)
		{
			if (V.TribeId < 0 || V.TribeId >= W.TribeCount)
			{
				return nullptr;
			}
			return &W.Tribes[V.TribeId];
		}

		const Tribe* Home(const World& W, const Villager& V)
		{
			if (V.TribeId < 0 || V.TribeId >= W.TribeCount)
			{
				return nullptr;
			}
			return &W.Tribes[V.TribeId];
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
			if (!Line)
			{
				Line = "";
			}
			V.LastSpeech = Line;
			V.Speech = Line;
			V.TalkTimer = Seconds;
		}

		const char* PickFresh(World& W, Villager& V, SpeechContext Context)
		{
			const Pool* P = FindPool(Context);
			if (!P || P->Count <= 0 || !P->Lines)
			{
				return "";
			}
			int Start = static_cast<int>(Rand01(W) * static_cast<float>(P->Count));
			if (Start < 0)
			{
				Start = 0;
			}
			Start %= P->Count;
			for (int N = 0; N < P->Count; ++N)
			{
				const char* Line = P->Lines[(Start + N) % P->Count];
				if (Line != V.Speech && Line != V.LastSpeech)
				{
					return Line;
				}
			}
			return P->Lines[Start];
		}

		void ClampTargetToLand(const World& W, Villager& V)
		{
			if (V.TribeId < 0 || V.TribeId >= W.ContinentCount)
			{
				return;
			}
			const Continent& C = W.Continents[V.TribeId];
			const float DX = V.TargetX - C.X;
			const float DY = V.TargetY - C.Y;
			const float D = std::sqrt(DX * DX + DY * DY);
			const float Max = C.Radius * 0.72f;
			if (D > Max && D > 1.f)
			{
				V.TargetX = C.X + DX / D * Max;
				V.TargetY = C.Y + DY / D * Max;
			}
		}

		int ShelterFor(const World& W, const Villager& V)
		{
			if (V.ShelterIndex >= 0 && V.ShelterIndex < W.ShelterCount)
			{
				return V.ShelterIndex;
			}
			const Tribe* T = Home(W, V);
			if (T && T->FirstShelter >= 0 && T->FirstShelter < W.ShelterCount)
			{
				return T->FirstShelter;
			}
			return 0;
		}

		int FindPrey(const World& W, const Villager& V)
		{
			int Best = -1;
			float BestD = 1.0e9f;
			for (int I = 0; I < W.AnimalCount; ++I)
			{
				if (!W.Animals[I].Alive || W.Animals[I].TribeId != V.TribeId)
				{
					continue;
				}
				const float D = Dist(V.X, V.Y, W.Animals[I].X, W.Animals[I].Y);
				if (D < BestD)
				{
					BestD = D;
					Best = I;
				}
			}
			return Best;
		}

		int NearestStudent(const World& W, const Villager& Teacher)
		{
			int Best = -1;
			float BestD = kTeachRange;
			for (int I = 0; I < W.VillagerCount; ++I)
			{
				const Villager& Other = W.Villagers[I];
				if (Other.Id == Teacher.Id || Other.TribeId != Teacher.TribeId || Other.bTeacher)
				{
					continue;
				}
				if (Other.Knowledge + 6.f >= Teacher.Knowledge)
				{
					continue;
				}
				const float D = Dist(Teacher.X, Teacher.Y, Other.X, Other.Y);
				if (D < BestD)
				{
					BestD = D;
					Best = I;
				}
			}
			return Best;
		}

		void RefreshClaim(World& W, Tribe& T)
		{
			if (T.ContinentId < 0 || T.ContinentId >= W.ContinentCount)
			{
				return;
			}
			const float Grown = 1100.f + static_cast<float>(T.StructureCount) * 260.f;
			const float Cap = W.Continents[T.ContinentId].Radius * 0.62f;
			T.ClaimRadius = Grown < Cap ? Grown : Cap;
		}

		void AddStructure(World& W, Tribe& T)
		{
			if (T.StructureCount >= kMaxStructuresPerTribe)
			{
				return;
			}
			T.StructureCount += 1;
			if (T.ShelterCount < kMaxSheltersPerTribe && W.ShelterCount < kMaxShelters)
			{
				const int S = W.ShelterCount;
				const float Ang = static_cast<float>(T.ShelterCount) * 1.15f + 0.4f;
				const float Rad = 240.f + static_cast<float>(T.ShelterCount) * 80.f;
				W.ShelterX[S] = T.CampX + std::cos(Ang) * Rad;
				W.ShelterY[S] = T.CampY + std::sin(Ang) * Rad;
				W.ShelterCount += 1;
				T.ShelterCount += 1;
			}
			RefreshClaim(W, T);
		}

		void RefreshTribeKnowledge(World& W, Tribe& T)
		{
			float Sum = 0.f;
			int N = 0;
			for (int I = 0; I < W.VillagerCount; ++I)
			{
				if (W.Villagers[I].TribeId == T.Id)
				{
					Sum += W.Villagers[I].Knowledge;
					N += 1;
				}
			}
			T.Knowledge = N > 0 ? Sum / static_cast<float>(N) : 0.f;
		}

		bool TryBirth(World& W, int TribeId, Villager& Speaker)
		{
			if (TribeId < 0 || TribeId >= W.TribeCount)
			{
				return false;
			}
			if (W.VillagerCount >= kMaxHumans)
			{
				return false;
			}
			if (W.KinCursor < 0 || W.KinCursor >= static_cast<int>(sizeof(kKinNames) / sizeof(kKinNames[0])))
			{
				return false;
			}
			Tribe& T = W.Tribes[TribeId];
			const int Id = W.VillagerCount;
			Villager& V = W.Villagers[Id];
			V = Villager{};
			V.Id = Id;
			V.Name = kKinNames[W.KinCursor];
			W.KinCursor += 1;
			V.Body = (W.Births % 2 == 0) ? Sex::Female : Sex::Male;
			V.Role = Habit::Wander;
			V.AgeYears = 0;
			V.AgeCarry = 0.f;
			V.Trait = "New by the fire. Still small.";
			V.PersonalLine = "I am small. I stay near the fire.";
			V.TribeId = TribeId;
			V.bTeacher = false;
			V.LookId = 1 + (Id % 7);
			V.Knowledge = 2.f;
			for (int S = 0; S < kSkillCount; ++S)
			{
				V.Skill[S] = 1.f;
			}
			V.X = T.CampX + 80.f;
			V.Y = T.CampY + 40.f;
			V.TargetX = V.X;
			V.TargetY = V.Y;
			V.Hunger = 75.f;
			V.Energy = 75.f;
			V.ShelterIndex = T.FirstShelter;
			V.Current = Activity::Idle;
			W.VillagerCount += 1;
			W.Births += 1;
			T.Births += 1;
			Speak(Speaker, PickFresh(W, Speaker, SpeechContext::Birth), 4.f);
			return true;
		}

		void ChooseNeed(World& W, Villager& V)
		{
			Tribe* T = Home(W, V);
			if (!T)
			{
				return;
			}
			const int Shelter = ShelterFor(W, V);
			if (IsNight(W.TimeOfDayHours) && V.Energy < 85.f)
			{
				V.Current = Activity::Sleep;
				V.TargetX = W.ShelterX[Shelter];
				V.TargetY = W.ShelterY[Shelter];
				Speak(V, PickFresh(W, V, SpeechContext::Sleep), 3.2f);
				return;
			}
			if (V.Hunger < 32.f)
			{
				V.Current = Activity::Eat;
				V.TargetX = T->FireX;
				V.TargetY = T->FireY;
				return;
			}
			if (V.Energy < 22.f)
			{
				V.Current = Activity::Sleep;
				V.TargetX = W.ShelterX[Shelter];
				V.TargetY = W.ShelterY[Shelter];
				Speak(V, PickFresh(W, V, SpeechContext::Sleep), 3.f);
				return;
			}
			if (!IsAdult(V))
			{
				V.Current = Activity::Walk;
				V.TargetX = T->CampX + 50.f;
				V.TargetY = T->CampY + 30.f;
				return;
			}
			if (V.Hunger < 62.f && !V.CarryingKill && (V.Role == Habit::Hunter || Rand01(W) < 0.35f))
			{
				V.Current = Activity::Hunt;
				V.bWorked = false;
				const int Best = FindPrey(W, V);
				V.HuntTarget = Best;
				if (Best >= 0)
				{
					V.TargetX = W.Animals[Best].X;
					V.TargetY = W.Animals[Best].Y;
				}
				else
				{
					V.TargetX = T->HuntX;
					V.TargetY = T->HuntY;
				}
				if (Rand01(W) < 0.45f)
				{
					Speak(V, PickFresh(W, V, SpeechContext::Hunt), 2.8f);
				}
				return;
			}
			if (V.bTeacher)
			{
				const int Student = NearestStudent(W, V);
				if (Student >= 0 && Rand01(W) < 0.72f)
				{
					V.Current = Activity::Teach;
					V.StateTimer = 3.4f + Rand01(W) * 2.f;
					V.TargetX = W.Villagers[Student].X;
					V.TargetY = W.Villagers[Student].Y;
					Speak(V, PickFresh(W, V, SpeechContext::Teach), V.StateTimer);
					return;
				}
			}
			if (V.Role == Habit::Tender && Rand01(W) < 0.55f)
			{
				V.Current = Activity::Talk;
				V.StateTimer = 2.8f + Rand01(W) * 2.f;
				const bool bPersonal = V.PersonalLine && V.PersonalLine[0] && Rand01(W) < 0.22f;
				Speak(V, bPersonal ? V.PersonalLine : PickFresh(W, V, SpeechContext::Talk), V.StateTimer);
				return;
			}
			if (Rand01(W) < 0.28f)
			{
				V.Current = Activity::Talk;
				V.StateTimer = 2.5f + Rand01(W) * 2.f;
				const bool bPersonal = V.PersonalLine && V.PersonalLine[0] && Rand01(W) < 0.18f;
				Speak(V, bPersonal ? V.PersonalLine : PickFresh(W, V, SpeechContext::Talk), V.StateTimer);
				return;
			}
			V.Current = Activity::Walk;
			if (V.Role == Habit::Knaps)
			{
				V.TargetX = T->CampX + (Rand01(W) * 2.f - 1.f) * 280.f;
				V.TargetY = T->CampY + (Rand01(W) * 2.f - 1.f) * 180.f;
			}
			else if (V.Role == Habit::Wander)
			{
				V.TargetX = T->CampX + (Rand01(W) * 2.f - 1.f) * 900.f;
				V.TargetY = T->CampY + (Rand01(W) * 2.f - 1.f) * 700.f;
			}
			else
			{
				V.TargetX = T->CampX + (Rand01(W) * 2.f - 1.f) * 520.f;
				V.TargetY = T->CampY + (Rand01(W) * 2.f - 1.f) * 420.f;
			}
			ClampTargetToLand(W, V);
			if (Rand01(W) < 0.35f)
			{
				const bool bPersonal = V.PersonalLine && V.PersonalLine[0] && Rand01(W) < 0.2f;
				Speak(V, bPersonal ? V.PersonalLine : PickFresh(W, V, SpeechContext::Camp), 3.2f);
			}
		}

		void ApplyWeatherIntent(World& W, Villager& V)
		{
			Tribe* T = Home(W, V);
			if (!T)
			{
				return;
			}
			const Activity Was = V.Current;
			SpeechContext Context = SpeechContext::Rain;
			switch (W.Sky)
			{
			case Weather::Rain:
				V.Current = Activity::Shelter;
				Context = SpeechContext::Rain;
				{
					const int Shelter = ShelterFor(W, V);
					V.TargetX = W.ShelterX[Shelter];
					V.TargetY = W.ShelterY[Shelter];
				}
				break;
			case Weather::Tornado:
				V.Current = Activity::Panic;
				Context = SpeechContext::Tornado;
				{
					const float DX = V.X - W.TornadoX;
					const float DY = V.Y - W.TornadoY;
					const float D = std::sqrt(DX * DX + DY * DY) + 1.f;
					V.TargetX = V.X + DX / D * 1400.f;
					V.TargetY = V.Y + DY / D * 1400.f;
					ClampTargetToLand(W, V);
				}
				break;
			case Weather::Hurricane:
				V.Current = Activity::Shelter;
				Context = SpeechContext::Hurricane;
				{
					const int Shelter = ShelterFor(W, V);
					V.TargetX = W.ShelterX[Shelter];
					V.TargetY = W.ShelterY[Shelter];
				}
				break;
			case Weather::Flood:
				V.Current = Activity::HighGround;
				Context = SpeechContext::Flood;
				V.TargetX = T->HighX + (Rand01(W) * 2.f - 1.f) * 180.f;
				V.TargetY = T->HighY;
				ClampTargetToLand(W, V);
				break;
			case Weather::Clear:
			default:
				return;
			}
			if (Was != V.Current || V.TalkTimer <= 0.f)
			{
				Speak(V, PickFresh(W, V, Context), 4.f);
			}
		}

		int NearestTimber(const World& W, const Villager& V)
		{
			int Best = -1;
			float BestD = 1.0e9f;
			for (int I = 0; I < W.TreeCount; ++I)
			{
				const Timber& Tree = W.Trees[I];
				if (!Tree.Standing || Tree.TribeId != V.TribeId)
				{
					continue;
				}
				const float D = Dist(V.X, V.Y, Tree.X, Tree.Y);
				if (D < BestD)
				{
					BestD = D;
					Best = I;
				}
			}
			return Best;
		}

		bool AssignChop(World& W, Villager& V)
		{
			if (!IsAdult(V))
			{
				return false;
			}
			const int Idx = NearestTimber(W, V);
			if (Idx < 0)
			{
				return false;
			}
			V.Current = Activity::Chop;
			V.WorkTarget = Idx;
			V.TargetX = W.Trees[Idx].X;
			V.TargetY = W.Trees[Idx].Y;
			V.bWorked = false;
			return true;
		}

		void ClampPointToContinent(const World& W, int TribeId, float& X, float& Y)
		{
			if (TribeId < 0 || TribeId >= W.ContinentCount)
			{
				return;
			}
			const Continent& C = W.Continents[TribeId];
			const float DX = X - C.X;
			const float DY = Y - C.Y;
			const float D = std::sqrt(DX * DX + DY * DY);
			const float Max = C.Radius * 0.72f;
			if (D > Max && D > 1.f)
			{
				X = C.X + DX / D * Max;
				Y = C.Y + DY / D * Max;
			}
		}

		void PlantExpansionGrove(World& W, Tribe& T)
		{
			// A finished roof opens timber farther out. The trees that were chopped stay down.
			for (int N = 0; N < 2; ++N)
			{
				if (W.TreeCount >= kMaxTrees)
				{
					return;
				}
				const float Ang = 0.85f + static_cast<float>(T.StructureCount) * 0.65f + static_cast<float>(N) * 0.7f;
				const float Rad = 980.f + static_cast<float>(T.StructureCount) * 140.f;
				float X = T.CampX + std::cos(Ang) * Rad;
				float Y = T.CampY + std::sin(Ang) * Rad;
				ClampPointToContinent(W, T.Id, X, Y);
				Timber& Tree = W.Trees[W.TreeCount];
				Tree = Timber{};
				Tree.Id = W.TreeCount;
				Tree.TribeId = T.Id;
				Tree.X = X;
				Tree.Y = Y;
				Tree.Standing = true;
				Tree.Chop = 0.f;
				W.TreeCount += 1;
			}
		}

		int OpenSite(World& W, Tribe& T, float X, float Y)
		{
			if (T.ActiveSite >= 0 && T.ActiveSite < W.SiteCount && W.Sites[T.ActiveSite].Live)
			{
				return T.ActiveSite;
			}
			if (T.StructureCount >= kMaxStructuresPerTribe || W.SiteCount >= kMaxSites)
			{
				return -1;
			}
			const int Id = W.SiteCount;
			WorkSite& S = W.Sites[Id];
			S = WorkSite{};
			S.TribeId = T.Id;
			S.X = X;
			S.Y = Y;
			S.Stage = 1;
			S.Progress = 0.f;
			S.Live = true;
			W.SiteCount += 1;
			T.ActiveSite = Id;
			return Id;
		}

		void AdvanceSite(World& W, Tribe& T, Villager& V, float Dt)
		{
			if (T.ActiveSite < 0 || T.ActiveSite >= W.SiteCount)
			{
				return;
			}
			WorkSite& S = W.Sites[T.ActiveSite];
			if (!S.Live)
			{
				return;
			}
			float Work = Dt * kBuildPerSecond;
			while (Work > 0.f && S.Live)
			{
				const float Room = 100.f - S.Progress;
				if (Room <= 0.01f)
				{
					if (S.Stage >= 4)
					{
						S.Live = false;
						S.Progress = 100.f;
						T.ActiveSite = -1;
						AddStructure(W, T);
						PlantExpansionGrove(W, T);
						V.Current = Activity::Idle;
						V.bWorked = true;
						Speak(V, PickFresh(W, V, SpeechContext::Build), 3.4f);
						return;
					}
					if (T.Wood <= 0)
					{
						S.Progress = 100.f;
						AssignChop(W, V);
						return;
					}
					T.Wood -= 1;
					S.Stage += 1;
					S.Progress = 0.f;
					continue;
				}
				const float Step = Work < Room ? Work : Room;
				S.Progress += Step;
				Work -= Step;
			}
		}

		void TickVillager(World& W, Villager& V, float Dt, float HoursPerSec)
		{
			if (V.AgeYears < kMinAdultAge)
			{
				V.AgeCarry += Dt;
				while (V.AgeCarry >= kBabyYearSeconds && V.AgeYears < kMinAdultAge)
				{
					V.AgeCarry -= kBabyYearSeconds;
					V.AgeYears += 1;
				}
			}

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

			// Night and exhaustion send people to sleep. Dawn, or a finished nap, lets them work again.
			if (!Storm && V.Current == Activity::Sleep && !IsNight(W.TimeOfDayHours) && V.Energy >= 60.f)
			{
				V.Current = Activity::Idle;
				V.StateTimer = 0.f;
			}

			const bool NeedsFood = V.Hunger < 32.f;
			const bool NeedsSleep = V.Energy < 22.f || (IsNight(W.TimeOfDayHours) && V.Energy < 85.f);
			if (!Storm
				&& (V.Current == Activity::Idle || V.Current == Activity::Walk || V.Current == Activity::Talk
					|| V.Current == Activity::Teach)
				&& (V.StateTimer <= 0.f || NeedsFood || NeedsSleep))
			{
				ChooseNeed(W, V);
			}
			if (!Storm && (V.Current == Activity::Build || V.Current == Activity::Chop || V.Current == Activity::Craft)
				&& (NeedsFood || NeedsSleep))
			{
				const bool bFinishStage = V.Current == Activity::Build && V.Hunger >= 16.f && !IsNight(W.TimeOfDayHours);
				if (!bFinishStage)
				{
					ChooseNeed(W, V);
				}
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
			if (!Storm && V.Current == Activity::Teach)
			{
				V.StateTimer -= Dt;
				if (V.StateTimer <= 0.f)
				{
					V.Current = Activity::Idle;
				}
			}

			Tribe* T = Home(W, V);
			float Speed = 160.f;
			if (V.Current == Activity::Panic)
			{
				Speed = 420.f;
			}
			else if (V.Current == Activity::Hunt || V.Current == Activity::HighGround)
			{
				Speed = 360.f;
			}
			else if (V.Current == Activity::Build || V.Current == Activity::Chop || V.Current == Activity::Craft)
			{
				Speed = 140.f;
			}
			else if (V.Current == Activity::Sleep && Dist(V.X, V.Y, V.TargetX, V.TargetY) < 90.f)
			{
				Speed = 0.f;
			}

			if (V.Current == Activity::Hunt)
			{
				if (V.HuntTarget < 0 || V.HuntTarget >= W.AnimalCount || !W.Animals[V.HuntTarget].Alive
					|| W.Animals[V.HuntTarget].TribeId != V.TribeId)
				{
					V.HuntTarget = FindPrey(W, V);
				}
			}

			if (V.Current == Activity::Hunt && V.HuntTarget >= 0 && V.HuntTarget < W.AnimalCount && T)
			{
				Animal& Prey = W.Animals[V.HuntTarget];
				if (Prey.Alive)
				{
					V.TargetX = Prey.X;
					V.TargetY = Prey.Y;
					if (Dist(V.X, V.Y, Prey.X, Prey.Y) < 130.f)
					{
						Prey.Alive = false;
						Prey.RespawnIn = 18.f;
						V.CarryingKill = true;
						V.Current = Activity::Eat;
						V.TargetX = T->FireX;
						V.TargetY = T->FireY;
						Speak(V, PickFresh(W, V, SpeechContext::Hunt), 4.f);
					}
					else if (V.TalkTimer <= 0.f && Rand01(W) < 0.04f)
					{
						Speak(V, PickFresh(W, V, SpeechContext::Hunt), 2.5f);
					}
				}
				else
				{
					V.HuntTarget = -1;
					V.Current = Activity::Idle;
				}
			}

			if (V.Current == Activity::Eat && T && Dist(V.X, V.Y, T->FireX, T->FireY) < 80.f)
			{
				if (V.CarryingKill)
				{
					V.CarryingKill = false;
				}
				if (V.TalkTimer <= 0.f)
				{
					Speak(V, PickFresh(W, V, SpeechContext::Eat), 3.f);
				}
				if (V.Hunger > 88.f && !Storm)
				{
					V.Current = Activity::Idle;
					V.StateTimer = 0.f;
				}
			}

			if (V.Current == Activity::Chop && T && IsAdult(V))
			{
				int Idx = V.WorkTarget;
				if (Idx < 0 || Idx >= W.TreeCount || !W.Trees[Idx].Standing || W.Trees[Idx].TribeId != V.TribeId)
				{
					Idx = NearestTimber(W, V);
					V.WorkTarget = Idx;
				}
				if (Idx < 0)
				{
					V.Current = Activity::Idle;
				}
				else
				{
					Timber& Tree = W.Trees[Idx];
					V.TargetX = Tree.X;
					V.TargetY = Tree.Y;
					if (Dist(V.X, V.Y, Tree.X, Tree.Y) < 130.f)
					{
						Speed = 0.f;
						Tree.Chop += Dt * kChopPerSecond;
						if (Tree.Chop >= 100.f)
						{
							Tree.Standing = false;
							Tree.Chop = 100.f;
							T->Wood += 1;
							if (T->ActiveSite >= 0 && T->ActiveSite < W.SiteCount && W.Sites[T->ActiveSite].Live)
							{
								V.Current = Activity::Build;
								V.WorkTarget = T->ActiveSite;
								V.TargetX = W.Sites[T->ActiveSite].X;
								V.TargetY = W.Sites[T->ActiveSite].Y;
							}
							else
							{
								V.Current = Activity::Idle;
							}
						}
					}
				}
			}

			if (V.Current == Activity::Craft && T && IsAdult(V))
			{
				V.TargetX = T->FireX;
				V.TargetY = T->FireY;
				const bool bCanCraft = T->TechUnlocked[static_cast<int>(TechId::StoneTools)] && T->Wood > 0 && T->Spears < 4;
				if (!bCanCraft)
				{
					V.Current = Activity::Idle;
				}
				else if (Dist(V.X, V.Y, T->FireX, T->FireY) < 120.f)
				{
					Speed = 0.f;
					V.Craft += Dt * kCraftPerSecond;
					if (V.Craft >= 100.f)
					{
						V.Craft = 0.f;
						T->Wood -= 1;
						T->Spears += 1;
						V.bCarriesSpear = true;
						V.Current = Activity::Idle;
						Speak(V, PickFresh(W, V, SpeechContext::Craft), 3.f);
					}
				}
			}

			if (V.Current == Activity::Build && T && IsAdult(V))
			{
				if (T->ActiveSite < 0 || T->ActiveSite >= W.SiteCount || !W.Sites[T->ActiveSite].Live)
				{
					V.WorkTarget = OpenSite(W, *T, V.TargetX, V.TargetY);
				}
				if (T->ActiveSite >= 0 && T->ActiveSite < W.SiteCount && W.Sites[T->ActiveSite].Live)
				{
					WorkSite& S = W.Sites[T->ActiveSite];
					V.TargetX = S.X;
					V.TargetY = S.Y;
					V.WorkTarget = T->ActiveSite;
					if (Dist(V.X, V.Y, S.X, S.Y) < 140.f)
					{
						Speed = 0.f;
						AdvanceSite(W, *T, V, Dt);
					}
				}
			}

			MoveToward(V, V.TargetX, V.TargetY, Speed, Dt);
		}

		void TickAnimals(World& W, float Dt)
		{
			for (int I = 0; I < W.AnimalCount; ++I)
			{
				Animal& A = W.Animals[I];
				if (A.TribeId < 0 || A.TribeId >= W.TribeCount)
				{
					continue;
				}
				const Tribe& T = W.Tribes[A.TribeId];
				if (!A.Alive)
				{
					A.RespawnIn -= Dt;
					if (A.RespawnIn <= 0.f)
					{
						A.Alive = true;
						A.X = T.HuntX + (Rand01(W) * 2.f - 1.f) * 500.f;
						A.Y = T.HuntY + (Rand01(W) * 2.f - 1.f) * 380.f;
					}
					continue;
				}

				float FleeX = 0.f;
				float FleeY = 0.f;
				bool Flee = false;
				for (int V = 0; V < W.VillagerCount; ++V)
				{
					if (W.Villagers[V].Current != Activity::Hunt || W.Villagers[V].TribeId != A.TribeId)
					{
						continue;
					}
					if (Dist(A.X, A.Y, W.Villagers[V].X, W.Villagers[V].Y) < 700.f)
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
				A.X = T.HuntX + std::max(-700.f, std::min(700.f, A.X - T.HuntX));
				A.Y = T.HuntY + std::max(-520.f, std::min(520.f, A.Y - T.HuntY));
				A.Heading = std::atan2(DY, DX);
			}
		}

		void TickResearch(World& W, Tribe& T, float Dt)
		{
			if (T.TeacherId < 0 || T.TeacherId >= W.VillagerCount)
			{
				return;
			}
			Villager& Teacher = W.Villagers[T.TeacherId];
			if (!Teacher.bTeacher || !IsAdult(Teacher))
			{
				return;
			}
			if (T.TechCooldown > 0.f)
			{
				T.TechCooldown -= Dt;
				if (T.TechCooldown > 0.f)
				{
					return;
				}
				T.TechCooldown = 0.f;
			}
			const int Next = T.TechTier + 1;
			if (Next < 0 || Next >= kTechCount)
			{
				return;
			}
			T.Research += Dt * kResearchPerSecond;
			if (T.Research < TechTierAt(Next).ResearchNeed)
			{
				return;
			}
			T.TechUnlocked[Next] = true;
			T.TechTier = Next;
			T.TechCooldown = 4.f;
			if (Next == static_cast<int>(TechId::AnimalHusbandry))
			{
				T.bRidingUnlocked = true;
			}
			const float Lift = 1.5f + static_cast<float>(Next) * 0.15f;
			for (int I = 0; I < W.VillagerCount; ++I)
			{
				Villager& Member = W.Villagers[I];
				if (Member.TribeId != T.Id || Member.bTeacher)
				{
					continue;
				}
				Member.Knowledge = std::min(Teacher.Knowledge, Member.Knowledge + Lift);
				const int Sk = SkillFor(Member.Role);
				if (Sk >= 0 && Sk < kSkillCount)
				{
					Member.Skill[Sk] = std::min(100.f, Member.Skill[Sk] + Lift);
				}
			}
		}

		void TickProgress(World& W, float Dt)
		{
			const bool Storm = W.Sky != Weather::Clear;
			for (int TribeIndex = 0; TribeIndex < W.TribeCount; ++TribeIndex)
			{
				Tribe& T = W.Tribes[TribeIndex];
				bool bScarce = false;
				int Members = 0;
				for (int I = 0; I < W.VillagerCount; ++I)
				{
					if (W.Villagers[I].TribeId != T.Id)
					{
						continue;
					}
					Members += 1;
					if (W.Villagers[I].Hunger < 12.f)
					{
						bScarce = true;
					}
				}
				if (Members == 0)
				{
					continue;
				}
				if (bScarce)
				{
					T.ScarceSeconds += Dt;
				}
				else
				{
					T.SurvivalSeconds += Dt;
				}

				if (!Storm && T.TeacherId >= 0 && T.TeacherId < W.VillagerCount)
				{
					Villager& Teacher = W.Villagers[T.TeacherId];
					if (Teacher.bTeacher && IsAdult(Teacher))
					{
						const float Scale = (bScarce ? 0.25f : 1.f) * (1.f + 0.15f * static_cast<float>(T.TechTier));
						const int Sk = SkillFor(Teacher.Role);
						for (int I = 0; I < W.VillagerCount; ++I)
						{
							Villager& Student = W.Villagers[I];
							if (Student.TribeId != T.Id || Student.bTeacher)
							{
								continue;
							}
							if (Dist(Teacher.X, Teacher.Y, Student.X, Student.Y) > kTeachRange)
							{
								continue;
							}
							if (Student.Knowledge + 1.f >= Teacher.Knowledge)
							{
								continue;
							}
							Student.Knowledge = std::min(Teacher.Knowledge, Student.Knowledge + Dt * 3.5f * Scale);
							if (Sk >= 0 && Sk < kSkillCount)
							{
								Student.Skill[Sk] = std::min(100.f, Student.Skill[Sk] + Dt * 4.f * Scale);
							}
						}
					}
				}
				RefreshTribeKnowledge(W, T);
				TickResearch(W, T, Dt);

				if (!Storm)
				{
					T.BuildCooldown -= Dt;
					const bool bHasLive = T.ActiveSite >= 0 && T.ActiveSite < W.SiteCount && W.Sites[T.ActiveSite].Live;
					bool bChopper = false;
					int Standing = 0;
					for (int I = 0; I < W.VillagerCount; ++I)
					{
						if (W.Villagers[I].TribeId == T.Id && W.Villagers[I].Current == Activity::Chop)
						{
							bChopper = true;
						}
					}
					for (int I = 0; I < W.TreeCount; ++I)
					{
						if (W.Trees[I].Standing && W.Trees[I].TribeId == T.Id)
						{
							Standing += 1;
						}
					}
					Villager* IdleAdult = nullptr;
					Villager* Talker = nullptr;
					for (int I = 0; I < W.VillagerCount; ++I)
					{
						Villager& V = W.Villagers[I];
						if (V.TribeId != T.Id || !IsAdult(V))
						{
							continue;
						}
						if (!IdleAdult && (V.Current == Activity::Idle || V.Current == Activity::Walk))
						{
							IdleAdult = &V;
						}
						else if (!Talker && (V.Current == Activity::Talk || V.Current == Activity::Teach))
						{
							Talker = &V;
						}
					}
					bool bCrafter = false;
					bool bBuilder = false;
					for (int I = 0; I < W.VillagerCount; ++I)
					{
						if (W.Villagers[I].TribeId != T.Id)
						{
							continue;
						}
						if (W.Villagers[I].Current == Activity::Craft)
						{
							bCrafter = true;
						}
						if (W.Villagers[I].Current == Activity::Build)
						{
							bBuilder = true;
						}
					}
					const bool bNeedWood = Standing > 0 && (T.Wood < 1 || (bHasLive && T.Wood <= 0));
					const bool bSiteWaiting = bHasLive && W.Sites[T.ActiveSite].Progress >= 100.f
						&& W.Sites[T.ActiveSite].Stage < 4 && T.Wood <= 0;
					bool bHasRoof = false;
					for (int I = 0; I < W.SiteCount; ++I)
					{
						if (W.Sites[I].TribeId == T.Id && W.Sites[I].Stage == 4 && !W.Sites[I].Live)
						{
							bHasRoof = true;
							break;
						}
					}
					const int SpearCap = bHasRoof ? 2 : 1;
					const bool bWantSpear = !bHasLive && T.TechUnlocked[static_cast<int>(TechId::StoneTools)]
						&& T.Spears < SpearCap && T.Wood >= 1;
					const bool bWantBuild = !bHasLive && T.BuildCooldown <= 0.f
						&& T.TechUnlocked[static_cast<int>(TechId::ShelterCraft)] && T.Wood >= 1
						&& T.StructureCount < kMaxStructuresPerTribe;
					Villager* Hands = IdleAdult ? IdleAdult : Talker;
					if ((bNeedWood || bSiteWaiting) && !bChopper && Hands)
					{
						AssignChop(W, *Hands);
					}
					else if (bHasLive && !bBuilder && !bSiteWaiting && Hands)
					{
						Hands->Current = Activity::Build;
						Hands->bWorked = false;
						Hands->WorkTarget = T.ActiveSite;
						Hands->TargetX = W.Sites[T.ActiveSite].X;
						Hands->TargetY = W.Sites[T.ActiveSite].Y;
						Hands->StateTimer = 6.f;
					}
					else if (bWantSpear && T.Spears < 1 && !bCrafter && IdleAdult)
					{
						IdleAdult->Current = Activity::Craft;
						IdleAdult->Craft = 0.f;
						IdleAdult->TargetX = T.FireX;
						IdleAdult->TargetY = T.FireY;
						IdleAdult->StateTimer = 4.f;
						Speak(*IdleAdult, PickFresh(W, *IdleAdult, SpeechContext::Craft), 3.f);
					}
					else if (bWantBuild && IdleAdult)
					{
						int Roofs = 0;
						for (int I = 0; I < W.SiteCount; ++I)
						{
							if (W.Sites[I].TribeId == T.Id && !W.Sites[I].Live && W.Sites[I].Stage >= 4)
							{
								++Roofs;
							}
						}
						const float Ang = 0.55f + static_cast<float>(Roofs) * 1.35f;
						const float Rad = 720.f + static_cast<float>(Roofs) * 420.f;
						float X = T.CampX + std::cos(Ang) * Rad;
						float Y = T.CampY + std::sin(Ang) * Rad;
						ClampPointToContinent(W, T.Id, X, Y);
						const int Id = OpenSite(W, T, X, Y);
						if (Id >= 0)
						{
							IdleAdult->Current = Activity::Build;
							IdleAdult->bWorked = false;
							IdleAdult->WorkTarget = Id;
							IdleAdult->TargetX = W.Sites[Id].X;
							IdleAdult->TargetY = W.Sites[Id].Y;
							ClampTargetToLand(W, *IdleAdult);
							IdleAdult->StateTimer = 6.f;
							Speak(*IdleAdult, PickFresh(W, *IdleAdult, SpeechContext::Build), 3.2f);
							T.BuildCooldown = 6.f;
						}
					}
					else if (bWantSpear && !bCrafter && IdleAdult)
					{
						IdleAdult->Current = Activity::Craft;
						IdleAdult->Craft = 0.f;
						IdleAdult->TargetX = T.FireX;
						IdleAdult->TargetY = T.FireY;
						IdleAdult->StateTimer = 4.f;
						Speak(*IdleAdult, PickFresh(W, *IdleAdult, SpeechContext::Craft), 3.f);
					}
				}

				if (Storm)
				{
					continue;
				}
				int Woman = -1;
				int Man = -1;
				float BestScore = -1.0e9f;
				for (int I = 0; I < W.VillagerCount; ++I)
				{
					Villager& A = W.Villagers[I];
					if (A.TribeId != T.Id || !IsAdult(A) || A.Hunger < 48.f || A.Energy < 36.f)
					{
						if (A.TribeId == T.Id && !IsAdult(A))
						{
							A.Bond = 0.f;
						}
						continue;
					}
					for (int J = I + 1; J < W.VillagerCount; ++J)
					{
						Villager& B = W.Villagers[J];
						if (B.TribeId != T.Id || !IsAdult(B) || B.Body == A.Body)
						{
							continue;
						}
						if (B.Hunger < 48.f || B.Energy < 36.f)
						{
							continue;
						}
						const float D = Dist(A.X, A.Y, B.X, B.Y);
						const bool bAtCamp = Dist(A.X, A.Y, T.CampX, T.CampY) < 900.f
							&& Dist(B.X, B.Y, T.CampX, T.CampY) < 900.f;
						const float Limit = bAtCamp ? 1400.f : kPairRange;
						if (D >= Limit)
						{
							continue;
						}
						const int SheId = A.Body == Sex::Female ? I : J;
						const int HeId = A.Body == Sex::Male ? I : J;
						// Stay with the woman who already has a bond, so two pairs do not split one birth.
						const float Score = W.Villagers[SheId].Bond * 1000.f - D;
						if (Score > BestScore)
						{
							BestScore = Score;
							Woman = SheId;
							Man = HeId;
						}
					}
				}
				if (Woman < 0 || Man < 0)
				{
					continue;
				}
				Villager& She = W.Villagers[Woman];
				Villager& He = W.Villagers[Man];
				if (!IsAdult(She) || !IsAdult(He))
				{
					She.Bond = 0.f;
					continue;
				}
				She.Bond += Dt * kBondPerSecond;
				if (She.Bond >= kBondNeed)
				{
					She.Bond = 0.f;
					TryBirth(W, T.Id, She);
				}
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

		void PlaceShelter(World& W, Tribe& T, float X, float Y)
		{
			if (W.ShelterCount >= kMaxShelters || T.ShelterCount >= kMaxSheltersPerTribe)
			{
				return;
			}
			if (T.ShelterCount == 0)
			{
				T.FirstShelter = W.ShelterCount;
			}
			W.ShelterX[W.ShelterCount] = X;
			W.ShelterY[W.ShelterCount] = Y;
			W.ShelterCount += 1;
			T.ShelterCount += 1;
			T.StructureCount += 1;
		}

		Sex LearnerBody(Sex Teacher, int Member)
		{
			const bool WantFemale = (Teacher == Sex::Male) ? (Member % 2 == 0) : (Member % 2 == 1);
			return WantFemale ? Sex::Female : Sex::Male;
		}

		void FillCommon(Villager& V, int Id, int TribeId)
		{
			V = Villager{};
			V.Id = Id;
			V.TribeId = TribeId;
			V.Hunger = 48.f;
			V.Energy = 70.f;
			V.Current = Activity::Idle;
			V.HuntTarget = -1;
		}

		constexpr float kMeetRange = 800.f;

		void InitDiplomacy(World& W)
		{
			for (int A = 0; A < kTribeCount; ++A)
			{
				for (int B = 0; B < kTribeCount; ++B)
				{
					Relation& R = W.Relations[A * kTribeCount + B];
					R = Relation{};
					R.State = Stance::Neutral;
					if (A == B)
					{
						R.Trust = 100.f;
						R.Trade = 0.f;
						R.Betrayal = 0.f;
						continue;
					}
					const float Cheat = (W.Tribes[A].Greed + (1.f - W.Tribes[A].Honesty)
						+ W.Tribes[B].Greed + (1.f - W.Tribes[B].Honesty))
						* 0.25f;
					R.Trust = 58.f - Cheat * 10.f;
					if (R.Trust < 46.f)
					{
						R.Trust = 46.f;
					}
					if (R.Trust > 64.f)
					{
						R.Trust = 64.f;
					}
					R.Trade = 50.f;
					R.Betrayal = 8.f + Cheat * 28.f;
					if (R.Betrayal > 40.f)
					{
						R.Betrayal = 40.f;
					}
				}
			}
		}

		bool PeopleInContact(const World& W, int TribeA, int TribeB)
		{
			for (int I = 0; I < W.VillagerCount; ++I)
			{
				if (W.Villagers[I].TribeId != TribeA)
				{
					continue;
				}
				for (int J = 0; J < W.VillagerCount; ++J)
				{
					if (W.Villagers[J].TribeId != TribeB)
					{
						continue;
					}
					if (Dist(W.Villagers[I].X, W.Villagers[I].Y, W.Villagers[J].X, W.Villagers[J].Y) <= kMeetRange)
					{
						return true;
					}
				}
			}
			return false;
		}

		void TickDiplomacy(World& W, float Dt)
		{
			for (int A = 0; A < W.TribeCount; ++A)
			{
				for (int B = A + 1; B < W.TribeCount; ++B)
				{
					Relation& AB = W.Relations[A * kTribeCount + B];
					Relation& BA = W.Relations[B * kTribeCount + A];
					if (AB.MeetCooldown > 0.f)
					{
						AB.MeetCooldown -= Dt;
					}
					if (BA.MeetCooldown > 0.f)
					{
						BA.MeetCooldown -= Dt;
					}
					const bool bScarce = W.Tribes[A].ScarceSeconds > W.Tribes[A].SurvivalSeconds
						&& W.Tribes[B].ScarceSeconds > W.Tribes[B].SurvivalSeconds;
					const float Toward = bScarce ? 50.f : 58.f;
					const float Step = Dt * 0.08f;
					AB.Trust += (Toward - AB.Trust) * Step;
					BA.Trust = AB.Trust;
					if (!bScarce && AB.Trust > 48.f && AB.Trade < 62.f)
					{
						AB.Trade += Dt * 0.15f;
					}
					BA.Trade = AB.Trade;

					// Population and shelters do not open a war. Only a meeting does,
					// and each teacher may still pick neutral.
					if (!PeopleInContact(W, A, B) || AB.MeetCooldown > 0.f)
					{
						continue;
					}
					AB.State = TeacherStance(W, A, B);
					BA.State = TeacherStance(W, B, A);
					AB.Met = true;
					BA.Met = true;
					AB.MeetCooldown = 24.f;
					BA.MeetCooldown = 24.f;
				}
			}
		}

		void LayoutContinents(World& W)
		{
			W.ContinentCount = kContinentCount;
			W.Continents[0].Id = 0;
			W.Continents[0].Name = kContinentNames[0];
			W.Continents[0].X = 0.f;
			W.Continents[0].Y = 400.f;
			W.Continents[0].Radius = kValleyRadius;
			W.Continents[0].TribeId = 0;
			for (int I = 1; I < kContinentCount; ++I)
			{
				const float Ang = static_cast<float>(I - 1) * (2.f * kPi / 7.f) - kPi * 0.15f;
				Continent& C = W.Continents[I];
				C.Id = I;
				C.Name = kContinentNames[I];
				C.X = std::cos(Ang) * kOuterOrbit;
				C.Y = std::sin(Ang) * kOuterOrbit;
				C.Radius = kOuterRadius;
				C.TribeId = I;
			}
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

	bool IsAdult(const Villager& V)
	{
		return V.AgeYears >= kMinAdultAge;
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

	int ContinentAt(const World& W, float X, float Y)
	{
		int Best = -1;
		float BestD = 1.0e9f;
		for (int I = 0; I < W.ContinentCount; ++I)
		{
			const float D = Dist(X, Y, W.Continents[I].X, W.Continents[I].Y);
			if (D <= W.Continents[I].Radius && D < BestD)
			{
				BestD = D;
				Best = I;
			}
		}
		return Best;
	}

	bool IsClaimedLand(const World& W, float X, float Y, int& OutTribe)
	{
		OutTribe = -1;
		const int Land = ContinentAt(W, X, Y);
		if (Land < 0)
		{
			return false;
		}
		const int TribeId = W.Continents[Land].TribeId;
		if (TribeId < 0 || TribeId >= W.TribeCount)
		{
			return false;
		}
		const Tribe& T = W.Tribes[TribeId];
		if (Dist(X, Y, T.CampX, T.CampY) <= T.ClaimRadius)
		{
			OutTribe = TribeId;
			return true;
		}
		return false;
	}

	const Relation& RelationBetween(const World& W, int FromTribe, int ToTribe)
	{
		static const Relation Neutral{};
		if (FromTribe < 0 || ToTribe < 0 || FromTribe >= W.TribeCount || ToTribe >= W.TribeCount)
		{
			return Neutral;
		}
		return W.Relations[FromTribe * kTribeCount + ToTribe];
	}

	const char* StanceName(Stance State)
	{
		switch (State)
		{
		case Stance::Ally: return "Ally";
		case Stance::Enemy: return "Enemy";
		case Stance::Neutral:
		default: return "Neutral";
		}
	}

	bool TribesAtWar(const World& W, int TribeA, int TribeB)
	{
		if (TribeA == TribeB)
		{
			return false;
		}
		return RelationBetween(W, TribeA, TribeB).State == Stance::Enemy
			|| RelationBetween(W, TribeB, TribeA).State == Stance::Enemy;
	}

	bool TribesAllied(const World& W, int TribeA, int TribeB)
	{
		if (TribeA == TribeB)
		{
			return false;
		}
		return RelationBetween(W, TribeA, TribeB).State == Stance::Ally
			&& RelationBetween(W, TribeB, TribeA).State == Stance::Ally;
	}

	Stance TeacherStance(World& W, int FromTribe, int ToTribe)
	{
		if (FromTribe < 0 || ToTribe < 0 || FromTribe >= W.TribeCount || ToTribe >= W.TribeCount || FromTribe == ToTribe)
		{
			return Stance::Neutral;
		}
		const Tribe& Leader = W.Tribes[FromTribe];
		const Relation& Link = W.Relations[FromTribe * kTribeCount + ToTribe];
		float TrustNorm = Link.Trust / 100.f;
		if (TrustNorm < 0.f)
		{
			TrustNorm = 0.f;
		}
		if (TrustNorm > 1.f)
		{
			TrustNorm = 1.f;
		}
		// The teacher's own lean. Greed is how they cheat their own people, not this vote.
		// Neutral stays the heaviest weight, so neither peace nor war is mandatory.
		const float AllyWeight = Leader.Honesty * 0.45f + TrustNorm * 0.35f;
		const float EnemyWeight = Leader.Ambition * 0.45f;
		const float NeutralWeight = 0.7f;
		const float Sum = AllyWeight + EnemyWeight + NeutralWeight;
		float Pick = Rand01(W) * Sum;
		if (Pick < NeutralWeight)
		{
			return Stance::Neutral;
		}
		Pick -= NeutralWeight;
		if (Pick < AllyWeight)
		{
			return Stance::Ally;
		}
		return Stance::Enemy;
	}

	const char* TribeLabel(const World& W, int TribeId)
	{
		if (TribeId < 0 || TribeId >= W.TribeCount || !W.Tribes[TribeId].Name)
		{
			return "";
		}
		return W.Tribes[TribeId].Name;
	}

	void InitWorld(World& W)
	{
		W = World{};
		W.DayLengthSeconds = 75.f;
		W.TimeOfDayHours = 10.f;
		W.Rng = 1u;
		LayoutContinents(W);

		W.TribeCount = kTribeCount;
		for (int T = 0; T < kTribeCount; ++T)
		{
			Tribe& TribeSlot = W.Tribes[T];
			const Continent& Land = W.Continents[T];
			TribeSlot.Id = T;
			TribeSlot.Name = kTribeNames[T];
			TribeSlot.ContinentId = T;
			TribeSlot.TeacherId = -1;
			if (T == 0)
			{
				TribeSlot.CampX = 0.f;
				TribeSlot.CampY = 700.f;
				TribeSlot.FireX = 0.f;
				TribeSlot.FireY = 640.f;
				TribeSlot.HuntX = 200.f;
				TribeSlot.HuntY = 2600.f;
				TribeSlot.HighX = 0.f;
				TribeSlot.HighY = -2800.f;
				PlaceShelter(W, TribeSlot, -240.f, 820.f);
				PlaceShelter(W, TribeSlot, 260.f, 840.f);
				PlaceShelter(W, TribeSlot, -420.f, 520.f);
				PlaceShelter(W, TribeSlot, 430.f, 500.f);
				PlaceShelter(W, TribeSlot, 40.f, 980.f);
			}
			else
			{
				TribeSlot.CampX = Land.X;
				TribeSlot.CampY = Land.Y;
				TribeSlot.FireX = Land.X;
				TribeSlot.FireY = Land.Y - 90.f;
				TribeSlot.HuntX = Land.X + Land.Radius * 0.42f;
				TribeSlot.HuntY = Land.Y + Land.Radius * 0.12f;
				TribeSlot.HighX = Land.X;
				TribeSlot.HighY = Land.Y - Land.Radius * 0.48f;
				PlaceShelter(W, TribeSlot, Land.X - 180.f, Land.Y + 140.f);
				PlaceShelter(W, TribeSlot, Land.X + 200.f, Land.Y + 120.f);
			}
			TribeSlot.BuildCooldown = 2.f;
			TribeSlot.Honesty = kHonesty[T];
			TribeSlot.Greed = kGreed[T];
			TribeSlot.Ambition = kAmbition[T];
			TribeSlot.Temper = kTempers[T];
			TribeSlot.Wood = 0;
			TribeSlot.TechTier = 0;
			TribeSlot.TechCooldown = 0.f;
			TribeSlot.ActiveSite = -1;
			for (int Tech = 0; Tech < kTechCount; ++Tech)
			{
				TribeSlot.TechUnlocked[Tech] = Tech == static_cast<int>(TechId::Fire);
			}
			RefreshClaim(W, TribeSlot);
		}
		InitDiplomacy(W);

		W.CampX = W.Tribes[0].CampX;
		W.CampY = W.Tribes[0].CampY;
		W.FireX = W.Tribes[0].FireX;
		W.FireY = W.Tribes[0].FireY;
		W.HuntX = W.Tribes[0].HuntX;
		W.HuntY = W.Tribes[0].HuntY;
		W.HighX = W.Tribes[0].HighX;
		W.HighY = W.Tribes[0].HighY;

		W.VillagerCount = 0;
		for (int T = 0; T < kTribeCount; ++T)
		{
			const int TeacherId = W.VillagerCount;
			Villager& Teacher = W.Villagers[TeacherId];
			FillCommon(Teacher, TeacherId, T);
			Teacher.Name = kTeacherNames[T];
			Teacher.Body = kTeacherBodies[T];
			Teacher.Role = kTeacherRoles[T];
			Teacher.AgeYears = kTeacherAges[T];
			Teacher.Trait = kTeacherTraits[T];
			Teacher.PersonalLine = kTeacherLines[T];
			Teacher.bTeacher = true;
			Teacher.LookId = T;
			Teacher.Knowledge = 84.f;
			for (int S = 0; S < kSkillCount; ++S)
			{
				Teacher.Skill[S] = 58.f;
			}
			Teacher.Skill[SkillFor(Teacher.Role)] = 92.f;
			W.Tribes[T].TeacherId = TeacherId;
			W.VillagerCount += 1;

			for (int M = 0; M < kStarterMembers - 1; ++M)
			{
				const int Id = W.VillagerCount;
				const int NameIndex = T * 3 + M;
				Villager& V = W.Villagers[Id];
				FillCommon(V, Id, T);
				V.Name = kLearnerNames[NameIndex];
				V.Body = LearnerBody(Teacher.Body, M);
				V.Role = static_cast<Habit>((M + T) % 4);
				V.AgeYears = 21 + ((T * 3 + M * 5) % 16);
				V.Trait = kLearnerTraits[NameIndex];
				V.PersonalLine = kLearnerLines[NameIndex];
				V.bTeacher = false;
				V.LookId = 1 + ((T * 3 + M) % 7);
				V.Knowledge = 12.f + static_cast<float>(M) * 5.f;
				for (int S = 0; S < kSkillCount; ++S)
				{
					V.Skill[S] = 6.f + static_cast<float>(M);
				}
				V.Skill[SkillFor(V.Role)] = 18.f;
				W.VillagerCount += 1;
			}

			for (int M = 0; M < kStarterMembers; ++M)
			{
				Villager& V = W.Villagers[W.Tribes[T].TeacherId + M];
				const float Ang = static_cast<float>(M) / static_cast<float>(kStarterMembers) * 2.f * kPi;
				V.X = W.Tribes[T].CampX + std::cos(Ang) * 220.f;
				V.Y = W.Tribes[T].CampY + std::sin(Ang) * 180.f;
				V.TargetX = V.X;
				V.TargetY = V.Y;
				V.Hunger = 48.f + Rand01(W) * 40.f;
				V.Energy = 55.f + Rand01(W) * 35.f;
				V.ShelterIndex = W.Tribes[T].FirstShelter + (M % W.Tribes[T].ShelterCount);
			}
		}

		if (W.Tribes[1].TeacherId >= 0)
		{
			Villager& Nima = W.Villagers[W.Tribes[1].TeacherId];
			Nima.Current = Activity::Talk;
			Nima.StateTimer = 4.f;
			Nima.TalkTimer = 4.f;
			Nima.Speech = Nima.PersonalLine;
			Nima.LastSpeech = Nima.PersonalLine;
		}
		if (W.Tribes[7].TeacherId >= 0)
		{
			Villager& Bram = W.Villagers[W.Tribes[7].TeacherId];
			Bram.Current = Activity::Talk;
			Bram.StateTimer = 4.f;
			Bram.TalkTimer = 4.f;
			Bram.Speech = Bram.PersonalLine;
			Bram.LastSpeech = Bram.PersonalLine;
		}

		for (int T = 0; T < W.TribeCount; ++T)
		{
			RefreshTribeKnowledge(W, W.Tribes[T]);
		}

		W.AnimalCount = 0;
		for (int T = 0; T < kTribeCount; ++T)
		{
			for (int A = 0; A < kAnimalsPerTribe; ++A)
			{
				if (W.AnimalCount >= kMaxAnimals)
				{
					break;
				}
				Animal& AnimalSlot = W.Animals[W.AnimalCount];
				AnimalSlot = Animal{};
				AnimalSlot.Id = W.AnimalCount;
				AnimalSlot.TribeId = T;
				AnimalSlot.Alive = true;
				AnimalSlot.X = W.Tribes[T].HuntX + (A == 0 ? -220.f : 260.f);
				AnimalSlot.Y = W.Tribes[T].HuntY + (A == 0 ? 80.f : -140.f);
				W.AnimalCount += 1;
			}
		}

		W.TreeCount = 0;
		W.SiteCount = 0;
		for (int T = 0; T < W.TribeCount; ++T)
		{
			for (int N = 0; N < kTreesPerTribe; ++N)
			{
				if (W.TreeCount >= kMaxTrees)
				{
					break;
				}
				Timber& Tree = W.Trees[W.TreeCount];
				Tree = Timber{};
				Tree.Id = W.TreeCount;
				Tree.TribeId = T;
				const float Ang = 0.4f + static_cast<float>(N) * 2.05f;
				Tree.X = W.Tribes[T].CampX + std::cos(Ang) * 460.f;
				Tree.Y = W.Tribes[T].CampY + std::sin(Ang) * 380.f;
				Tree.Standing = true;
				W.TreeCount += 1;
			}
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
				Speak(W.Villagers[I], PickFresh(W, W.Villagers[I], SpeechContext::Clear), 3.f);
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
		if (W.VillagerCount < 0)
		{
			W.VillagerCount = 0;
		}
		if (W.VillagerCount > kMaxHumans)
		{
			W.VillagerCount = kMaxHumans;
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
		TickProgress(W, Dt);
		TickDiplomacy(W, Dt);
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
		case Activity::Teach: return "Teach";
		case Activity::Build: return "Build";
		case Activity::Chop: return "Chop";
		case Activity::Craft: return "Craft";
		case Activity::Idle:
		default: return "Idle";
		}
	}

	const char* TechName(int Index)
	{
		if (Index < 0 || Index >= kTechCount)
		{
			return "";
		}
		return TechTierAt(Index).Name;
	}

	const char* BuildStageName(int Stage)
	{
		switch (Stage)
		{
		case 1: return "Site";
		case 2: return "Frame";
		case 3: return "Walls";
		case 4: return "Roof";
		default: return "";
		}
	}

	int TechCount()
	{
		return kTechCount;
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

	int SpeechPoolSize(SpeechContext Context)
	{
		const Pool* P = FindPool(Context);
		return P ? P->Count : 0;
	}

	const char* SpeechPoolLine(SpeechContext Context, int Index)
	{
		const Pool* P = FindPool(Context);
		if (!P || Index < 0 || Index >= P->Count)
		{
			return "";
		}
		return P->Lines[Index];
	}

	const char* PickSpeech(World& W, Villager& V, SpeechContext Context)
	{
		const char* Line = PickFresh(W, V, Context);
		V.LastSpeech = Line;
		return Line;
	}

	int LineCount()
	{
		int N = 0;
		const int Pools = static_cast<int>(sizeof(kPools) / sizeof(kPools[0]));
		for (int I = 0; I < Pools; ++I)
		{
			N += kPools[I].Count;
		}
		return N;
	}

	const char* LineAt(int Index)
	{
		if (Index < 0)
		{
			return "";
		}
		const int Pools = static_cast<int>(sizeof(kPools) / sizeof(kPools[0]));
		for (int I = 0; I < Pools; ++I)
		{
			if (Index < kPools[I].Count)
			{
				return kPools[I].Lines[Index];
			}
			Index -= kPools[I].Count;
		}
		return "";
	}
} // namespace vg
