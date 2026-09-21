#include "ValleySim.h"
#include "ValleyPalette.h"
#include "ValleyLookPaths.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

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
		|| ContainsFold(Line, "solar") || ContainsFold(Line, "alien") || ContainsFold(Line, "tribe")
		|| ContainsFold(Line, "village") || ContainsFold(Line, "stranger") || ContainsFold(Line, "clan");
}

int main()
{
	using namespace vg;

	{
		World W;
		InitWorld(W);
		CHECK(W.TribeCount == kTribeCount, "eight tribes");
		CHECK(W.ContinentCount == kContinentCount, "eight continents");
		CHECK(W.VillagerCount == kStarterHumans, "starter camp is teacher plus learners");
		CHECK(static_cast<int>(sizeof(W.Villagers) / sizeof(W.Villagers[0])) == kMaxHumans, "spare slots for births");
		CHECK(kMaxHumans > W.VillagerCount, "population can grow");
		CHECK(W.AnimalCount >= 3, "a few hunt animals");
		CHECK(W.ShelterCount >= 4, "a few shelters");
		int Adults = 0;
		int Women = 0;
		int Men = 0;
		int Teachers = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			CHECK(W.Villagers[I].AgeYears >= kMinAdultAge, "adult only");
			CHECK(IsAdult(W.Villagers[I]), "starter cast is adult");
			CHECK(W.Villagers[I].Name && W.Villagers[I].Name[0], "named");
			CHECK(W.Villagers[I].Trait && W.Villagers[I].Trait[0], "personality");
			CHECK(!SpeechForbidden(W.Villagers[I].Trait), "trait has no cosmos/god");
			CHECK(!SpeechForbidden(W.Villagers[I].PersonalLine), "personal line has no cosmos/god");
			CHECK(W.Villagers[I].TribeId >= 0 && W.Villagers[I].TribeId < W.TribeCount, "member of a tribe");
			if (W.Villagers[I].AgeYears >= 21)
			{
				++Adults;
			}
			if (W.Villagers[I].Body == Sex::Female)
			{
				++Women;
			}
			else
			{
				CHECK(W.Villagers[I].Body == Sex::Male, "male or female");
				++Men;
			}
			if (W.Villagers[I].bTeacher)
			{
				++Teachers;
			}
			for (int J = 0; J < I; ++J)
			{
				CHECK(std::strcmp(W.Villagers[I].Name, W.Villagers[J].Name) != 0, "distinct names");
			}
		}
		CHECK(Adults == W.VillagerCount, "every starter is an adult");
		CHECK(Women > 0 && Men > 0, "both sexes are present");
		CHECK(Teachers == kTribeCount, "one teacher per tribe");
		CHECK(W.DayLengthSeconds > 30.f && W.DayLengthSeconds < 240.f, "compressed day");
		for (int Step = 0; Step < 80; ++Step)
		{
			TickWorld(W, 0.25f);
		}
		CHECK(W.VillagerCount == kStarterHumans, "a short watch does not birth anyone");
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

		const SpeechContext Pools[] = {
			SpeechContext::Hunt, SpeechContext::Eat, SpeechContext::Sleep, SpeechContext::Talk,
			SpeechContext::Rain, SpeechContext::Tornado, SpeechContext::Hurricane, SpeechContext::Flood,
			SpeechContext::Clear, SpeechContext::Teach, SpeechContext::Build
		};
		for (SpeechContext Context : Pools)
		{
			const int N = SpeechPoolSize(Context);
			CHECK(N >= 6, "each context has a real pool, not a two-line loop");
			for (int I = 0; I < N; ++I)
			{
				const char* Line = SpeechPoolLine(Context, I);
				CHECK(!SpeechForbidden(Line), "pool line has no cosmos/god");
				for (int J = 0; J < I; ++J)
				{
					CHECK(std::strcmp(Line, SpeechPoolLine(Context, J)) != 0, "pool lines are not copies");
				}
			}
		}
	}

	{
		World W;
		InitWorld(W);
		Villager& Speaker = W.Villagers[0];
		const char* Prev = nullptr;
		for (int N = 0; N < 24; ++N)
		{
			const char* Line = PickSpeech(W, Speaker, SpeechContext::Talk);
			CHECK(Line && Line[0], "picked a line");
			if (Prev)
			{
				CHECK(std::strcmp(Prev, Line) != 0, "talk lines do not repeat back to back");
			}
			Prev = Line;
		}
	}

	{
		World W;
		InitWorld(W);
		const char* Teachers[] = {"Mara", "Nima", "Lira", "Sable", "Flint", "Oak", "Reed", "Bram"};
		for (int T = 0; T < W.TribeCount; ++T)
		{
			const Tribe& TribeSlot = W.Tribes[T];
			CHECK(TribeSlot.TeacherId >= 0 && TribeSlot.TeacherId < W.VillagerCount, "teacher id is a person");
			const Villager& Teacher = W.Villagers[TribeSlot.TeacherId];
			CHECK(Teacher.bTeacher, "teacher flag");
			CHECK(std::strcmp(Teacher.Name, Teachers[T]) == 0, "named teacher per tribe");
			CHECK(Teacher.Knowledge >= 70.f, "teacher starts smarter");
			CHECK(W.Continents[T].TribeId == T, "one starter tribe per continent");
			int Learners = 0;
			int Women = 0;
			int Men = 0;
			float LearnerKnow = 1000.f;
			for (int I = 0; I < W.VillagerCount; ++I)
			{
				if (W.Villagers[I].TribeId != T)
				{
					continue;
				}
				if (W.Villagers[I].Body == Sex::Female)
				{
					++Women;
				}
				else
				{
					++Men;
				}
				if (!W.Villagers[I].bTeacher)
				{
					++Learners;
					if (W.Villagers[I].Knowledge < LearnerKnow)
					{
						LearnerKnow = W.Villagers[I].Knowledge;
					}
				}
			}
			CHECK(Learners >= 1, "someone to teach");
			CHECK(Women >= 1 && Men >= 1, "each tribe can pair");
			CHECK(LearnerKnow + 20.f < Teacher.Knowledge, "learners start behind the teacher");
			CHECK(TribeSlot.ClaimRadius < W.Continents[T].Radius * 0.9f, "leftover land stays unclaimed");
		}

		for (int A = 0; A < W.ContinentCount; ++A)
		{
			for (int B = A + 1; B < W.ContinentCount; ++B)
			{
				const float DX = W.Continents[A].X - W.Continents[B].X;
				const float DY = W.Continents[A].Y - W.Continents[B].Y;
				const float D = std::sqrt(DX * DX + DY * DY);
				CHECK(D > W.Continents[A].Radius + W.Continents[B].Radius, "ocean gap between continents");
			}
		}
		const Continent& Home = W.Continents[0];
		const Continent& Next = W.Continents[1];
		const float DX = Next.X - Home.X;
		const float DY = Next.Y - Home.Y;
		const float D = std::sqrt(DX * DX + DY * DY);
		const float OceanX = Home.X + DX / D * (Home.Radius + 1200.f);
		const float OceanY = Home.Y + DY / D * (Home.Radius + 1200.f);
		CHECK(ContinentAt(W, OceanX, OceanY) < 0, "water between the valley and the next land");
		int Claimed = -1;
		CHECK(IsClaimedLand(W, W.Tribes[0].CampX, W.Tribes[0].CampY, Claimed), "camp is claimed");
		CHECK(Claimed == 0, "willow camp belongs to willow");
		const float EdgeX = Home.X;
		const float EdgeY = Home.Y + Home.Radius * 0.9f;
		CHECK(ContinentAt(W, EdgeX, EdgeY) == 0, "outer basin is still land");
		CHECK(!IsClaimedLand(W, EdgeX, EdgeY, Claimed), "outer basin is unclaimed");
	}

	{
		World W;
		InitWorld(W);
		const int TeacherId = W.Tribes[0].TeacherId;
		int StudentId = -1;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (W.Villagers[I].TribeId == 0 && !W.Villagers[I].bTeacher)
			{
				StudentId = I;
				break;
			}
		}
		CHECK(StudentId >= 0, "willow has a learner");
		Villager& Teacher = W.Villagers[TeacherId];
		Villager& Student = W.Villagers[StudentId];
		Teacher.X = Teacher.Y = Student.X = Student.Y = W.Tribes[0].CampX;
		Teacher.Current = Activity::Talk;
		Student.Current = Activity::Talk;
		Teacher.StateTimer = 30.f;
		Student.StateTimer = 30.f;
		Teacher.Hunger = Student.Hunger = 80.f;
		Teacher.Energy = Student.Energy = 80.f;
		const float Before = Student.Knowledge;
		TickWorld(W, 2.f);
		CHECK(W.Villagers[StudentId].Knowledge > Before + 2.f, "teacher raises a nearby learner");
		CHECK(W.Tribes[0].SurvivalSeconds > 0.f, "fed camp counts as surviving");
	}

	{
		World W;
		InitWorld(W);
		Villager& Builder = W.Villagers[W.Tribes[2].TeacherId + 1];
		const int Before = W.Tribes[2].StructureCount;
		const float Claim = W.Tribes[2].ClaimRadius;
		Builder.Current = Activity::Build;
		Builder.bWorked = false;
		Builder.TargetX = Builder.X;
		Builder.TargetY = Builder.Y;
		Builder.Hunger = 80.f;
		Builder.Energy = 80.f;
		TickWorld(W, 0.3f);
		CHECK(W.Tribes[2].StructureCount == Before + 1, "building adds a structure");
		CHECK(W.Tribes[2].ClaimRadius >= Claim, "claim grows or holds at the cap");
	}

	{
		World W;
		InitWorld(W);
		int Woman = -1;
		int Man = -1;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			if (W.Villagers[I].TribeId != 0)
			{
				continue;
			}
			if (Woman < 0 && W.Villagers[I].Body == Sex::Female)
			{
				Woman = I;
			}
			else if (Man < 0 && W.Villagers[I].Body == Sex::Male)
			{
				Man = I;
			}
		}
		CHECK(Woman >= 0 && Man >= 0, "willow has a pair");
		Villager& She = W.Villagers[Woman];
		Villager& He = W.Villagers[Man];
		She.X = He.X = W.Tribes[0].FireX;
		She.Y = He.Y = W.Tribes[0].FireY;
		She.Current = He.Current = Activity::Talk;
		She.StateTimer = He.StateTimer = 40.f;
		She.Hunger = He.Hunger = 90.f;
		She.Energy = He.Energy = 90.f;
		She.AgeYears = 20;
		She.Bond = 99.f;
		const int Frozen = W.VillagerCount;
		TickWorld(W, 2.f);
		CHECK(W.VillagerCount == Frozen, "under 21 does not reproduce");
		CHECK(W.Births == 0, "no birth when a partner is under 21");
		CHECK(W.Villagers[Woman].Bond == 0.f, "under-21 bond is cleared");

		W.Villagers[Woman].AgeYears = 27;
		W.Villagers[Woman].Bond = 99.f;
		W.Villagers[Woman].Current = W.Villagers[Man].Current = Activity::Talk;
		W.Villagers[Woman].StateTimer = W.Villagers[Man].StateTimer = 40.f;
		W.Villagers[Woman].Hunger = W.Villagers[Man].Hunger = 90.f;
		W.Villagers[Woman].Energy = W.Villagers[Man].Energy = 90.f;
		W.Villagers[Woman].X = W.Villagers[Man].X = W.Tribes[0].FireX;
		W.Villagers[Woman].Y = W.Villagers[Man].Y = W.Tribes[0].FireY;
		TickWorld(W, 1.f);
		CHECK(W.Births == 1, "two adults can add kin");
		CHECK(W.VillagerCount == Frozen + 1, "kin takes a spare slot");
		const Villager& Kin = W.Villagers[W.VillagerCount - 1];
		CHECK(Kin.AgeYears >= kMinAdultAge, "new kin is an adult");
		CHECK(!Kin.bTeacher, "new kin is not a second teacher");
		CHECK(Kin.TribeId == 0, "kin stays with the camp");
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
		W.Villagers[0].Hunger = 40.f;
		W.Villagers[0].Energy = 80.f;
		W.Sky = Weather::Clear;
		W.TimeOfDayHours = 10.f;
		SetDayLength(W, 240.f);
		W.Villagers[0].Current = Activity::Hunt;
		for (int Step = 0; Step < 160; ++Step)
		{
			TickWorld(W, 0.25f);
		}
		int AliveAfter = 0;
		for (int I = 0; I < W.AnimalCount; ++I)
		{
			AliveAfter += W.Animals[I].Alive ? 1 : 0;
		}
		CHECK(AliveAfter < Alive || W.Villagers[0].Hunger > 50.f, "hunt eventually kills or feeds");
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

	{
		CHECK(MaterialRecipeCount() >= 13, "wet-look recipe table is populated");
		CHECK(FindMaterialRecipe("M_Dirt") != nullptr, "dirt recipe");
		CHECK(FindMaterialRecipe("M_DirtWet") != nullptr, "wet dirt recipe");
		CHECK(FindMaterialRecipe("M_Grass") != nullptr, "grass recipe");
		CHECK(FindMaterialRecipe("M_Water") != nullptr, "water recipe");
		CHECK(FindMaterialRecipe("M_Bark") != nullptr, "bark recipe");
		CHECK(FindMaterialRecipe("M_Foliage") != nullptr, "foliage recipe");
		CHECK(FindMaterialRecipe("M_Wood") != nullptr, "wood recipe");
		CHECK(FindMaterialRecipe("M_Hide") != nullptr, "hide recipe");
		CHECK(FindMaterialRecipe("M_SkinWarm") != nullptr, "skin recipe");
		CHECK(FindMaterialRecipe("M_ClothOchre") != nullptr, "cloth recipe");
		CHECK(FindMaterialRecipe("M_Hair") != nullptr, "hair recipe");
		CHECK(FindMaterialRecipe("M_Stone") != nullptr, "stone recipe");
		CHECK(FindMaterialRecipe("M_Fire") != nullptr, "fire recipe");
		CHECK(FindMaterialRecipe("M_Fur") != nullptr, "fur recipe");
		CHECK(FindMaterialRecipe("WorldGrid") == nullptr, "no engine placeholder in the palette");

		const MaterialRecipe* Dirt = FindMaterialRecipe("M_Dirt");
		const MaterialRecipe* Wet = FindMaterialRecipe("M_DirtWet");
		const MaterialRecipe* Water = FindMaterialRecipe("M_Water");
		const MaterialRecipe* Skin = FindMaterialRecipe("M_SkinWarm");
		if (Dirt && Wet)
		{
			CHECK(Wet->Roughness < Dirt->Roughness, "wet dirt is glossier than dry dirt");
			CHECK(Wet->R + Wet->G + Wet->B < Dirt->R + Dirt->G + Dirt->B, "wet dirt is darker");
		}
		if (Water)
		{
			CHECK(Water->Kind == SurfaceKind::Translucent, "water is translucent");
			CHECK(Water->Roughness <= 0.08f, "water is reflective");
			CHECK(Water->R < 0.05f && Water->G < 0.08f, "water is dark");
		}
		if (Skin)
		{
			CHECK(Skin->Kind == SurfaceKind::Subsurface, "skin uses soft subsurface, not plastic");
			CHECK(Skin->Roughness >= 0.62f, "skin is matte flesh, not glossy plastic");
			CHECK(Skin->Specular <= 0.35f, "skin specular stays below plastic default");
		}
		for (int I = 0; I < MaterialRecipeCount(); ++I)
		{
			const MaterialRecipe& R = MaterialRecipeAt(I);
			CHECK(R.Name && R.Name[0] == 'M' && R.Name[1] == '_', "baked names are M_*");
			for (int J = 0; J < I; ++J)
			{
				CHECK(std::strcmp(R.Name, MaterialRecipeAt(J).Name) != 0, "recipe names unique");
			}
		}
	}

	{
		CHECK(PersonLookCount() == kEarthHumans, "one look per adult");
		const char* Expected[8] = {"Mara", "Nima", "Lira", "Sable", "Flint", "Oak", "Reed", "Bram"};
		int Women = 0;
		int Men = 0;
		int Beards = 0;
		for (int I = 0; I < PersonLookCount(); ++I)
		{
			const PersonLook& L = PersonLookAt(I);
			CHECK(std::strcmp(L.Name, Expected[I]) == 0, "cast order matches sim names");
			CHECK(L.Height > 0.9f && L.Height < 1.2f, "adult height scale");
			CHECK(L.Head > 0.2f && L.Head < L.Torso, "head reads smaller than torso");
			CHECK(L.Shoulder > 0.25f && L.Hip > 0.25f, "shoulders and hips are built");
			CHECK(L.HairStyle >= 0 && L.HairStyle <= 5, "known hair style");
			if (L.Woman)
			{
				++Women;
				CHECK(!L.Beard, "women have no beards this pass");
			}
			else
			{
				++Men;
			}
			if (L.Beard)
			{
				++Beards;
			}
			for (int J = 0; J < I; ++J)
			{
				const PersonLook& O = PersonLookAt(J);
				CHECK(ColorDistance(L.SkinR, L.SkinG, L.SkinB, O.SkinR, O.SkinG, O.SkinB) > 0.04f,
					"each adult has distinct skin");
				const float HairD = ColorDistance(L.HairR, L.HairG, L.HairB, O.HairR, O.HairG, O.HairB);
				const bool bHairOrStyle = HairD > 0.02f || L.HairStyle != O.HairStyle || L.Beard != O.Beard;
				CHECK(bHairOrStyle, "hair reads different between adults");
			}
		}
		CHECK(Women == 4 && Men == 4, "four women, four men looks");
		CHECK(Beards >= 2, "Flint and Oak keep beards");
	}

	{
		CHECK(AnimalLookCount() >= 4, "each hunt animal has a fur look");
		for (int I = 0; I < AnimalLookCount(); ++I)
		{
			const AnimalLook& A = AnimalLookAt(I);
			CHECK(A.BodyLen > A.BodyRad * 2.f, "animals are long quadrupeds, not lumps");
			CHECK(A.LegLen > 0.35f, "legs long enough to read as deer/boar");
			CHECK(A.NeckLen > 0.15f, "neck/snout present");
		}
		CHECK(ColorDistance(AnimalLookAt(0).FurR, AnimalLookAt(0).FurG, AnimalLookAt(0).FurB,
				  AnimalLookAt(1).FurR, AnimalLookAt(1).FurG, AnimalLookAt(1).FurB)
				> 0.03f,
			"animals do not share one hide color");
	}

	{
		CHECK(kMetaHumanMilestoneSlot == 0, "Mara is sim slot 0");
		CHECK(std::strcmp(PersonLookAt(kMetaHumanMilestoneSlot).Name, "Mara") == 0, "milestone MetaHuman is Mara");
		CHECK(UsesMetaHumanSlot(0), "Mara prefers a MetaHuman Blueprint when one exists");
		int MetaSlots = 0;
		for (int I = 0; I < kEarthHumans; ++I)
		{
			if (UsesMetaHumanSlot(I))
			{
				++MetaSlots;
			}
			else
			{
				CHECK(I != 0, "only Mara is MetaHuman this milestone");
			}
		}
		CHECK(MetaSlots == 1, "exactly one adult uses MetaHuman this milestone");
		CHECK(!UsesMetaHumanSlot(-1) && !UsesMetaHumanSlot(8), "out of range slots stay procedural");

		CHECK(std::strcmp(MetaHumanContentFolder(), "/Game/MetaHumans/Mara") == 0, "Mara assemble folder");
		CHECK(std::strcmp(PNGrassLibraryContentFolder(), "/Game/PN_GrassLibrary") == 0, "PN grass pack folder");
		CHECK(std::strcmp(MegascansContentFolder(), "/Game/Megascans") == 0, "Fab/Quixel default folder");
		CHECK(std::strcmp(ValleySliceContentFolder(), "/Game/ValleySlice") == 0, "optional alias folder");
		CHECK(std::strcmp(ValleySliceMapPath(), "/Game/Maps/ValleySlice") == 0, "playable slice map");

		CHECK(MetaHumanClassPathCount() >= 3, "several Mara Blueprint candidates");
		bool bHasMaraFolder = false;
		bool bHasAlias = false;
		for (int I = 0; I < MetaHumanClassPathCount(); ++I)
		{
			const char* P = MetaHumanClassPathAt(I);
			CHECK(P && P[0] == '/', "class path is an Unreal long package name");
			if (std::strstr(P, "/Game/MetaHumans/Mara/"))
			{
				bHasMaraFolder = true;
			}
			if (std::strstr(P, "/Game/ValleySlice/"))
			{
				bHasAlias = true;
			}
		}
		CHECK(bHasMaraFolder, "candidates include Content/MetaHumans/Mara");
		CHECK(bHasAlias, "candidates include Content/ValleySlice alias");

		CHECK(DirtMaterialPathCount() >= 2 && GrassMaterialPathCount() >= 2, "ground material aliases exist");
		CHECK(TreeMeshPathCount() >= 2 && GrassMeshPathCount() >= 2 && RockMeshPathCount() >= 1,
			"foliage mesh aliases exist");
		CHECK(std::strstr(DirtMaterialPathAt(0), "/Game/") == DirtMaterialPathAt(0), "dirt alias is /Game");
		CHECK(std::strstr(TreeMeshPathAt(0), "/Game/") == TreeMeshPathAt(0), "tree alias is /Game");

		bool bHasPNGrass = false;
		for (int I = 0; I < GrassMeshPathCount(); ++I)
		{
			if (std::strstr(GrassMeshPathAt(I), "/Game/PN_GrassLibrary/"))
			{
				bHasPNGrass = true;
			}
		}
		CHECK(bHasPNGrass, "grass aliases include PN_GrassLibrary");

		CHECK(PreferredTreeScatterCount() >= 110, "Quixel trees fill a forest, not a thin ring");
		CHECK(PreferredGrassScatterCount() >= 900, "grass instances are dense enough to read as a meadow");
		CHECK(PreferredRockScatterCount() >= 60, "rocks fill the valley floor");
		CHECK(ProceduralGrassTuftCount() >= 160, "empty-folder valley still gets cheap grass tufts");

		CHECK(ClassifyContentPath("/Game/Megascans/Surfaces/Forest_Dirt_01/MI_Forest_Dirt_01", ScanKind::DirtMaterial),
			"Quixel dirt surface classifies as dirt");
		CHECK(!ClassifyContentPath("/Game/Megascans/3D_Plants/European_Beech/SM_European_Beech_Var1", ScanKind::DirtMaterial),
			"a beech mesh is not dirt");
		CHECK(ClassifyContentPath("/Game/Megascans/Surfaces/Wild_Grass/MI_Wild_Grass", ScanKind::GrassMaterial),
			"Quixel grass surface classifies as grass");
		CHECK(ClassifyContentPath("/Game/PN_GrassLibrary/Materials/MI_Grass_01", ScanKind::GrassMaterial),
			"PN grass library material classifies as grass");
		CHECK(ClassifyContentPath("/Game/PN_GrassLibrary/Meshes/SM_Grass_01", ScanKind::GrassMesh),
			"PN grass library mesh classifies as grass");
		CHECK(ClassifyContentPath("/Game/Megascans/3D_Plants/European_Beech/SM_European_Beech_Var1", ScanKind::TreeMesh),
			"beech plant classifies as a tree");
		CHECK(ClassifyContentPath("/Game/Fab/ForestPack/SM_European_Beech_Var1", ScanKind::TreeMesh),
			"Fab beech without 3D_Plants still classifies as a tree");
		CHECK(ClassifyContentPath("/Game/Fab/Meadow/SM_Wild_Grass_Clump", ScanKind::GrassMesh),
			"Fab grass clump without 3D_Plants classifies as grass mesh");
		CHECK(ClassifyContentPath("/Game/Megascans/3D_Plants/Wild_Grass_Clump/SM_Wild_Grass_Clump", ScanKind::GrassMesh),
			"grass clump plant classifies as grass mesh");
		CHECK(ClassifyContentPath("/Game/Megascans/3D_Assets/Cliff_Rock/SM_Cliff_Rock", ScanKind::RockMesh),
			"cliff rock classifies as rock");
		CHECK(ClassifyContentPath("/Game/Megascans/Surfaces/Wet_Mud/MI_Wet_Mud", ScanKind::WetDirtMaterial),
			"wet mud classifies as wet dirt");
		CHECK(!ClassifyContentPath("/Game/Materials/M_Dirt", ScanKind::TreeMesh), "baked M_Dirt is not a tree");
		CHECK(!ClassifyContentPath("/Game/Megascans/3D_Assets/Fire_Pit/SM_Fire_Pit", ScanKind::TreeMesh),
			"fire pit is not a tree");
		CHECK(!ClassifyContentPath("/Game/Megascans/3D_Assets/Rock_Clump/SM_Rock_Clump", ScanKind::GrassMesh),
			"rock clump is not grass");
		CHECK(ClassifyContentPath("/Game/Megascans/3D_Assets/Rock_Clump/SM_Rock_Clump", ScanKind::RockMesh),
			"rock clump is a rock");
		CHECK(!ClassifyContentPath("/Game/PN_GrassLibrary/Meshes/SM_GroundPlane", ScanKind::GrassMesh),
			"PN ground plane is not a grass clump");
		CHECK(!ClassifyContentPath(nullptr, ScanKind::DirtMaterial), "null path is not a match");
	}

	std::printf("%d passed, %d failed\n", GPasses, GFails);
	return GFails ? 1 : 0;
}
