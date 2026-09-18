#include "ValleyPalette.h"

#include <cmath>
#include <cstring>

namespace vg
{
	namespace
	{
		const MaterialRecipe kMaterials[] = {
			// name, R G B A, metallic, roughness, specular, emissive, opacity, subRGB, kind, twoSided, useVertexColor
			{"M_Dirt", 0.18f, 0.095f, 0.042f, 1.f, 0.04f, 0.52f, 0.36f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false, true},
			{"M_DirtWet", 0.048f, 0.028f, 0.014f, 1.f, 0.08f, 0.11f, 0.78f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false, true},
			{"M_Mud", 0.07f, 0.045f, 0.025f, 1.f, 0.06f, 0.22f, 0.55f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false, true},
			{"M_Grass", 0.048f, 0.125f, 0.028f, 1.f, 0.f, 0.62f, 0.28f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false, true},
			{"M_GrassWet", 0.022f, 0.07f, 0.018f, 1.f, 0.f, 0.28f, 0.42f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false, true},
			{"M_Water", 0.008f, 0.022f, 0.028f, 0.62f, 0.12f, 0.028f, 0.92f, 0.f, 0.62f, 0.f, 0.f, 0.f, SurfaceKind::Translucent, false},
			{"M_Bark", 0.09f, 0.048f, 0.028f, 1.f, 0.f, 0.88f, 0.22f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_BarkDark", 0.045f, 0.028f, 0.018f, 1.f, 0.f, 0.90f, 0.20f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_Foliage", 0.032f, 0.11f, 0.024f, 1.f, 0.f, 0.42f, 0.32f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, true},
			{"M_FoliageDark", 0.018f, 0.055f, 0.014f, 1.f, 0.f, 0.50f, 0.26f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, true},
			{"M_FoliageSun", 0.07f, 0.16f, 0.035f, 1.f, 0.f, 0.38f, 0.34f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, true},
			{"M_FoliageUnderside", 0.03f, 0.06f, 0.028f, 1.f, 0.f, 0.55f, 0.24f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, true},
			{"M_Wood", 0.16f, 0.085f, 0.035f, 1.f, 0.f, 0.58f, 0.32f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_WoodDark", 0.07f, 0.04f, 0.02f, 1.f, 0.f, 0.70f, 0.26f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_Hide", 0.22f, 0.13f, 0.06f, 1.f, 0.f, 0.66f, 0.28f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_HideDark", 0.10f, 0.06f, 0.03f, 1.f, 0.f, 0.72f, 0.24f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_Fur", 0.26f, 0.16f, 0.08f, 1.f, 0.f, 0.86f, 0.18f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_FurBelly", 0.38f, 0.28f, 0.16f, 1.f, 0.f, 0.84f, 0.18f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_FurDark", 0.10f, 0.06f, 0.035f, 1.f, 0.f, 0.88f, 0.16f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_Moss", 0.04f, 0.09f, 0.03f, 1.f, 0.f, 0.78f, 0.22f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_Charcoal", 0.04f, 0.035f, 0.03f, 1.f, 0.02f, 0.92f, 0.18f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_SkinWarm", 0.46f, 0.3f, 0.2f, 1.f, 0.f, 0.74f, 0.26f, 0.f, 0.42f, 0.42f, 0.12f, 0.08f, SurfaceKind::Subsurface, false},
			{"M_ClothOchre", 0.26f, 0.14f, 0.055f, 1.f, 0.f, 0.7f, 0.28f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_Hair", 0.045f, 0.028f, 0.016f, 1.f, 0.04f, 0.52f, 0.36f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_Stone", 0.20f, 0.18f, 0.15f, 1.f, 0.06f, 0.64f, 0.38f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
			{"M_StoneGround", 0.20f, 0.18f, 0.15f, 1.f, 0.06f, 0.64f, 0.38f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false, true},
			{"M_Fire", 1.f, 0.42f, 0.08f, 1.f, 0.f, 1.f, 0.f, 18.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Emissive, false},
			{"M_Eye", 0.04f, 0.03f, 0.025f, 1.f, 0.05f, 0.28f, 0.45f, 0.f, 1.f, 0.f, 0.f, 0.f, SurfaceKind::Opaque, false},
		};

		const PersonLook kPeople[] = {
			{"Mara", 0.96f, 0.36f, 0.42f, 0.32f, 0.29f, 0.50f, 0.33f, 0.21f, 0.44f, 0.24f, 0.08f, 0.07f, 0.04f, 0.02f, 0, false, true},
			{"Nima", 1.00f, 0.38f, 0.48f, 0.36f, 0.30f, 0.64f, 0.44f, 0.30f, 0.40f, 0.12f, 0.08f, 0.22f, 0.10f, 0.04f, 2, false, true},
			{"Lira", 0.93f, 0.33f, 0.40f, 0.28f, 0.27f, 0.26f, 0.15f, 0.10f, 0.16f, 0.14f, 0.12f, 0.28f, 0.26f, 0.24f, 5, false, true},
			{"Sable", 1.04f, 0.37f, 0.46f, 0.34f, 0.31f, 0.18f, 0.10f, 0.07f, 0.18f, 0.10f, 0.05f, 0.025f, 0.016f, 0.012f, 4, false, true},
			{"Flint", 1.08f, 0.52f, 0.36f, 0.44f, 0.33f, 0.42f, 0.27f, 0.16f, 0.14f, 0.09f, 0.05f, 0.05f, 0.03f, 0.02f, 0, true, false},
			{"Oak", 1.12f, 0.58f, 0.42f, 0.52f, 0.34f, 0.58f, 0.41f, 0.29f, 0.10f, 0.08f, 0.06f, 0.14f, 0.10f, 0.06f, 4, true, false},
			{"Reed", 1.02f, 0.44f, 0.34f, 0.34f, 0.31f, 0.34f, 0.25f, 0.16f, 0.16f, 0.18f, 0.10f, 0.08f, 0.05f, 0.03f, 3, false, false},
			{"Bram", 1.05f, 0.50f, 0.40f, 0.46f, 0.32f, 0.56f, 0.26f, 0.16f, 0.30f, 0.10f, 0.05f, 0.10f, 0.04f, 0.025f, 0, false, false},
		};

		const AnimalLook kAnimals[] = {
			// len rad leg neck  fur            belly           dark            head antler tusk
			{1.18f, 0.22f, 0.72f, 0.42f, 0.34f, 0.22f, 0.10f, 0.48f, 0.38f, 0.22f, 0.16f, 0.09f, 0.04f, 0.20f, true, false},  // tan stag
			{1.08f, 0.24f, 0.66f, 0.38f, 0.16f, 0.10f, 0.06f, 0.28f, 0.20f, 0.12f, 0.08f, 0.05f, 0.03f, 0.19f, false, false}, // dark doe
			{0.92f, 0.34f, 0.44f, 0.20f, 0.28f, 0.20f, 0.14f, 0.40f, 0.32f, 0.22f, 0.12f, 0.08f, 0.05f, 0.24f, false, true},  // boar
			{1.22f, 0.20f, 0.70f, 0.44f, 0.44f, 0.30f, 0.14f, 0.56f, 0.46f, 0.28f, 0.22f, 0.12f, 0.06f, 0.21f, true, false},  // pale stag
		};
	}

	int MaterialRecipeCount()
	{
		return static_cast<int>(sizeof(kMaterials) / sizeof(kMaterials[0]));
	}

	const MaterialRecipe& MaterialRecipeAt(int Index)
	{
		if (Index < 0 || Index >= MaterialRecipeCount())
		{
			return kMaterials[0];
		}
		return kMaterials[Index];
	}

	const MaterialRecipe* FindMaterialRecipe(const char* Name)
	{
		if (!Name)
		{
			return nullptr;
		}
		for (int I = 0; I < MaterialRecipeCount(); ++I)
		{
			if (std::strcmp(kMaterials[I].Name, Name) == 0)
			{
				return &kMaterials[I];
			}
		}
		return nullptr;
	}

	int PersonLookCount()
	{
		return static_cast<int>(sizeof(kPeople) / sizeof(kPeople[0]));
	}

	const PersonLook& PersonLookAt(int Id)
	{
		if (Id < 0 || Id >= PersonLookCount())
		{
			return kPeople[0];
		}
		return kPeople[Id];
	}

	int AnimalLookCount()
	{
		return static_cast<int>(sizeof(kAnimals) / sizeof(kAnimals[0]));
	}

	const AnimalLook& AnimalLookAt(int Id)
	{
		const int Count = AnimalLookCount();
		if (Count <= 0)
		{
			return kAnimals[0];
		}
		int Index = Id % Count;
		if (Index < 0)
		{
			Index += Count;
		}
		return kAnimals[Index];
	}

	float ColorDistance(float R0, float G0, float B0, float R1, float G1, float B1)
	{
		const float DR = R0 - R1;
		const float DG = G0 - G1;
		const float DB = B0 - B1;
		return std::sqrt(DR * DR + DG * DG + DB * DB);
	}
}
