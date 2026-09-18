#include "ValleySim.h"
#include "ValleyPalette.h"

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
		CHECK(W.VillagerCount == 8, "exactly 8 villagers");
		CHECK(W.VillagerCount == kEarthHumans, "8 is the whole Earth population");
		CHECK(static_cast<int>(sizeof(W.Villagers) / sizeof(W.Villagers[0])) == kEarthHumans, "no spare human slots");
		CHECK(W.AnimalCount >= 3, "a few hunt animals");
		CHECK(W.ShelterCount >= 4, "a few shelters");
		int Adults = 0;
		int Women = 0;
		int Men = 0;
		for (int I = 0; I < W.VillagerCount; ++I)
		{
			CHECK(W.Villagers[I].AgeYears >= 21, "adult only");
			CHECK(W.Villagers[I].Name && W.Villagers[I].Name[0], "named");
			CHECK(W.Villagers[I].Trait && W.Villagers[I].Trait[0], "personality");
			CHECK(!SpeechForbidden(W.Villagers[I].Trait), "trait has no cosmos/god");
			CHECK(!SpeechForbidden(W.Villagers[I].PersonalLine), "personal line has no cosmos/god");
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
			for (int J = 0; J < I; ++J)
			{
				CHECK(std::strcmp(W.Villagers[I].Name, W.Villagers[J].Name) != 0, "distinct names");
			}
		}
		CHECK(Adults == 8, "every villager is an adult");
		CHECK(Women == 4 && Men == 4, "four women and four men");
		CHECK(W.DayLengthSeconds > 30.f && W.DayLengthSeconds < 240.f, "compressed day");
		for (int Step = 0; Step < 80; ++Step)
		{
			TickWorld(W, 0.25f);
		}
		CHECK(W.VillagerCount == kEarthHumans, "time does not invent more humans");
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
		CHECK(MaterialRecipeCount() >= 24, "wet-look recipe table includes layered env materials");
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
		CHECK(FindMaterialRecipe("M_Mud") != nullptr, "river-bank mud recipe");
		CHECK(FindMaterialRecipe("M_GrassWet") != nullptr, "wet grass recipe");
		CHECK(FindMaterialRecipe("M_BarkDark") != nullptr, "dark bark recipe");
		CHECK(FindMaterialRecipe("M_FoliageSun") != nullptr, "sunlit foliage recipe");
		CHECK(FindMaterialRecipe("M_FoliageUnderside") != nullptr, "canopy underside recipe");
		CHECK(FindMaterialRecipe("M_WoodDark") != nullptr, "aged wood recipe");
		CHECK(FindMaterialRecipe("M_HideDark") != nullptr, "dark hide recipe");
		CHECK(FindMaterialRecipe("M_Moss") != nullptr, "moss recipe");
		CHECK(FindMaterialRecipe("M_Charcoal") != nullptr, "charcoal recipe");
		CHECK(FindMaterialRecipe("M_FurBelly") != nullptr, "belly fur recipe");
		CHECK(FindMaterialRecipe("M_FurDark") != nullptr, "dark fur recipe");
		CHECK(FindMaterialRecipe("WorldGrid") == nullptr, "no engine placeholder in the palette");

		const MaterialRecipe* Dirt = FindMaterialRecipe("M_Dirt");
		const MaterialRecipe* Wet = FindMaterialRecipe("M_DirtWet");
		const MaterialRecipe* Mud = FindMaterialRecipe("M_Mud");
		const MaterialRecipe* Grass = FindMaterialRecipe("M_Grass");
		const MaterialRecipe* GrassWet = FindMaterialRecipe("M_GrassWet");
		const MaterialRecipe* Water = FindMaterialRecipe("M_Water");
		const MaterialRecipe* Bark = FindMaterialRecipe("M_Bark");
		const MaterialRecipe* Foliage = FindMaterialRecipe("M_Foliage");
		const MaterialRecipe* Fur = FindMaterialRecipe("M_Fur");
		const MaterialRecipe* FurBelly = FindMaterialRecipe("M_FurBelly");
		const MaterialRecipe* FurDark = FindMaterialRecipe("M_FurDark");
		const MaterialRecipe* Skin = FindMaterialRecipe("M_SkinWarm");
		if (Dirt && Wet)
		{
			CHECK(Wet->Roughness < Dirt->Roughness, "wet dirt is glossier than dry dirt");
			CHECK(Wet->R + Wet->G + Wet->B < Dirt->R + Dirt->G + Dirt->B, "wet dirt is darker");
			CHECK(Dirt->UseVertexColor && Wet->UseVertexColor, "dirt shaders accept ground vertex shade");
		}
		if (Dirt && Mud)
		{
			CHECK(Mud->R + Mud->G + Mud->B < Dirt->R + Dirt->G + Dirt->B, "mud is darker than dry dirt");
			CHECK(Mud->Roughness < Dirt->Roughness, "mud is wetter than dry dirt");
		}
		if (Grass && GrassWet)
		{
			CHECK(GrassWet->Roughness < Grass->Roughness, "wet grass is glossier than dry grass");
			CHECK(GrassWet->R + GrassWet->G + GrassWet->B < Grass->R + Grass->G + Grass->B, "wet grass is darker");
		}
		if (Dirt && Grass)
		{
			CHECK(ColorDistance(Dirt->R, Dirt->G, Dirt->B, Grass->R, Grass->G, Grass->B) > 0.06f,
				"dirt and grass are distinct hues");
			CHECK(Grass->G > Dirt->G, "grass reads greener than dirt");
		}
		if (Bark && Foliage)
		{
			CHECK(ColorDistance(Bark->R, Bark->G, Bark->B, Foliage->R, Foliage->G, Foliage->B) > 0.06f,
				"bark and foliage are clearly distinct");
			CHECK(Foliage->G > Bark->G * 1.4f, "foliage is greener than bark");
			CHECK(Bark->Roughness > Foliage->Roughness, "bark is rougher than waxy leaves");
			CHECK(!Bark->UseVertexColor, "bark does not depend on mesh vertex colors");
		}
		if (Fur && FurBelly && FurDark)
		{
			CHECK(FurBelly->R + FurBelly->G + FurBelly->B > Fur->R + Fur->G + Fur->B, "belly fur is lighter");
			CHECK(FurDark->R + FurDark->G + FurDark->B < Fur->R + Fur->G + Fur->B, "point fur is darker");
			CHECK(Fur->Roughness >= 0.74f, "fur stays matte, not hide-shiny");
		}
		if (Water)
		{
			CHECK(Water->Kind == SurfaceKind::Translucent, "water is translucent");
			CHECK(Water->Roughness <= 0.04f, "water is highly reflective");
			CHECK(Water->R < 0.02f && Water->G < 0.05f, "water is dark");
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
		int Antlered = 0;
		int Tusked = 0;
		for (int I = 0; I < AnimalLookCount(); ++I)
		{
			const AnimalLook& A = AnimalLookAt(I);
			CHECK(A.BodyLen > A.BodyRad * 2.f, "animals are long quadrupeds, not lumps");
			CHECK(A.LegLen > 0.35f, "legs long enough to read as deer/boar");
			CHECK(A.NeckLen > 0.15f, "neck/snout present");
			CHECK(A.HeadScale > 0.14f && A.HeadScale < 0.32f, "head scale reads as an animal skull, not a blob");
			CHECK(ColorDistance(A.FurR, A.FurG, A.FurB, A.BellyR, A.BellyG, A.BellyB) > 0.04f,
				"belly fur is a different tone than the back");
			CHECK(ColorDistance(A.FurR, A.FurG, A.FurB, A.DarkR, A.DarkG, A.DarkB) > 0.04f,
				"legs/points are darker than the back");
			if (A.Antlers)
			{
				++Antlered;
				CHECK(A.LegLen > 0.58f, "deer keep long legs");
				CHECK(A.NeckLen > 0.32f, "deer keep a readable neck");
				CHECK(!A.Tusks, "antlered deer are not boars");
			}
			if (A.Tusks)
			{
				++Tusked;
				CHECK(A.BodyRad > 0.28f, "boar body is thicker");
				CHECK(A.LegLen < 0.55f, "boar legs are shorter than deer");
				CHECK(A.NeckLen < 0.28f, "boar snout sits on a short neck");
			}
		}
		CHECK(Antlered >= 1, "at least one deer has antlers");
		CHECK(Tusked >= 1, "at least one animal is a boar");
		CHECK(ColorDistance(AnimalLookAt(0).FurR, AnimalLookAt(0).FurG, AnimalLookAt(0).FurB,
				  AnimalLookAt(1).FurR, AnimalLookAt(1).FurG, AnimalLookAt(1).FurB)
				> 0.03f,
			"animals do not share one hide color");
	}

	std::printf("%d passed, %d failed\n", GPasses, GFails);
	return GFails ? 1 : 0;
}
