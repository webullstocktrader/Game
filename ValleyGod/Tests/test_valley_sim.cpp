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
		CHECK(std::strcmp(PersonLookAt(kMetaHumanMilestoneSlot).Name, "Mara") == 0, "first MetaHuman slot is Mara");
		CHECK(UsesMetaHumanSlot(0), "Mara prefers a MetaHuman Blueprint when one exists");
		int MetaSlots = 0;
		const char* Expected[8] = {"Mara", "Nima", "Lira", "Sable", "Flint", "Oak", "Reed", "Bram"};
		for (int I = 0; I < kEarthHumans; ++I)
		{
			CHECK(UsesMetaHumanSlot(I), "every starting adult may use a MetaHuman Blueprint");
			CHECK(std::strcmp(PersonLookAt(I).Name, Expected[I]) == 0, "slot order matches named adults");
			CHECK(VillagerMetaHumanPathCount(I) >= 4, "each adult has several Blueprint candidates");
			bool bHasEditable = false;
			bool bHasMetaHumans = false;
			bool bHasAlias = false;
			bool bHasNamedBP = false;
			for (int P = 0; P < VillagerMetaHumanPathCount(I); ++P)
			{
				const char* Path = VillagerMetaHumanPathAt(I, P);
				CHECK(Path && Path[0] == '/', "villager class path is an Unreal long package name");
				if (std::strstr(Path, "/Game/EditableMetahumans/"))
				{
					bHasEditable = true;
				}
				if (std::strstr(Path, "/Game/MetaHumans/"))
				{
					bHasMetaHumans = true;
				}
				if (std::strstr(Path, "/Game/ValleySlice/"))
				{
					bHasAlias = true;
				}
				if (std::strstr(Path, Expected[I]))
				{
					bHasNamedBP = true;
				}
			}
			CHECK(bHasEditable, "candidates include Content/EditableMetahumans");
			CHECK(bHasMetaHumans, "candidates include Content/MetaHumans");
			CHECK(bHasAlias, "candidates include Content/ValleySlice alias");
			CHECK(bHasNamedBP, "candidates include the villager name");
			++MetaSlots;
		}
		CHECK(MetaSlots == 8, "all eight starting adults are MetaHuman-eligible");
		CHECK(UsesMetaHumanSlot(8), "later adults may use a discovered Blueprint if population grows");
		CHECK(!UsesMetaHumanSlot(-1), "negative slots stay procedural");

		CHECK(PackageMatchesVillager("/Game/EditableMetahumans/MHC_Hannah/Mara/BP_Mara", "Mara"),
			"assembled Mara under MHC_Hannah matches Mara");
		CHECK(!PackageMatchesVillager("/Game/EditableMetahumans/MHC_Hannah/Mara/BP_Mara", "Nima"),
			"Mara Blueprint does not match Nima");
		CHECK(PackageMatchesVillager("/Game/MetaHumans/Nima/BP_Nima", "Nima"), "BP_Nima matches Nima");
		CHECK(PackageMatchesVillager("/Game/EditableMetahumans/Flint/BP_Flint", "Flint"), "BP_Flint matches Flint");
		CHECK(!PackageMatchesVillager("/Game/EditableMetahumans/Mara/SK_Mara_Body", "Mara"),
			"body skeletal mesh is not a presentation Blueprint");
		CHECK(LooksLikeGenericMetaHumanBlueprint("/Game/EditableMetahumans/Shared/BP_MetaHuman"),
			"generic BP_MetaHuman can fill an unmatched adult");
		CHECK(!LooksLikeGenericMetaHumanBlueprint("/Game/EditableMetahumans/Mara/BP_Mara"),
			"named villager Blueprint is not generic");
		CHECK(PackageMatchesVillager("/Game/MetaHumans/Mara/BP_MetaHuman", "Mara"),
			"BP_MetaHuman in Mara folder still matches Mara");
		CHECK(!PackageMatchesVillager("/Game/EditableMetahumans/Mara/BP_Body", "Mara"),
			"body Blueprint in Mara folder is not the villager presentation");
		CHECK(!PackageMatchesVillager("/Game/EditableMetahumans/Mara/BP_Face", "Mara"),
			"face Blueprint in Mara folder is not the villager presentation");
		CHECK(!LooksLikeGenericMetaHumanBlueprint("/Game/EditableMetahumans/MHC_Hannah/BP_MHC_Hannah"),
			"MHC preview Blueprint is not a generic adult fill");
		CHECK(!LooksLikeGenericMetaHumanBlueprint("/Game/EditableMetahumans/Shared/BP_MHC_Taro"),
			"MHC character product is not a generic adult fill");

		CHECK(std::strcmp(EditableMetahumansContentFolder(), "/Game/EditableMetahumans") == 0,
			"MetaHuman Creator assemble root");
		CHECK(std::strcmp(MetaHumanContentFolder(), "/Game/MetaHumans") == 0, "legacy MetaHumans assemble root");
		CHECK(std::strcmp(PNGrassLibraryContentFolder(), "/Game/PN_GrassLibrary") == 0, "PN grass pack folder");
		CHECK(std::strcmp(MegascansContentFolder(), "/Game/Megascans") == 0, "Fab/Quixel default folder");
		CHECK(std::strcmp(ValleySliceContentFolder(), "/Game/ValleySlice") == 0, "optional alias folder");
		CHECK(std::strcmp(ValleySliceMapPath(), "/Game/Maps/ValleySlice") == 0, "playable slice map");

		CHECK(MetaHumanClassPathCount() >= 4, "Mara still has several Blueprint candidates");
		bool bHasMaraFolder = false;
		bool bHasMaraEditable = false;
		bool bHasAlias = false;
		for (int I = 0; I < MetaHumanClassPathCount(); ++I)
		{
			const char* P = MetaHumanClassPathAt(I);
			CHECK(P && P[0] == '/', "class path is an Unreal long package name");
			if (std::strstr(P, "/Game/MetaHumans/Mara") || std::strstr(P, "/Game/MetaHumans/Mara/"))
			{
				bHasMaraFolder = true;
			}
			if (std::strstr(P, "/Game/EditableMetahumans/"))
			{
				bHasMaraEditable = true;
			}
			if (std::strstr(P, "/Game/ValleySlice/"))
			{
				bHasAlias = true;
			}
		}
		CHECK(bHasMaraFolder, "candidates include Content/MetaHumans/Mara");
		CHECK(bHasMaraEditable, "candidates include Content/EditableMetahumans");
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

		CHECK(PreferredTreeScatterCount() >= 56, "Quixel trees scatter denser than a thin ring");
		CHECK(PreferredGrassScatterCount() >= 140, "grass clumps are dense enough to read as a meadow");
		CHECK(PreferredRockScatterCount() >= 24, "rocks fill the valley floor");
		CHECK(ProceduralGrassTuftCount() >= 70, "empty-folder valley still gets cheap grass tufts");

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
