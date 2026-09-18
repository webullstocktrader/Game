#pragma once

// Engine-free look recipes for the graphics polish pass.
// Unreal code and host tests share this table so Content bake and -game
// fallback cannot drift apart.

namespace vg
{
	enum class SurfaceKind
	{
		Opaque = 0,
		Translucent = 1,
		Subsurface = 2,
		Emissive = 3
	};

	struct MaterialRecipe
	{
		const char* Name = "";
		float R = 0.f;
		float G = 0.f;
		float B = 0.f;
		float A = 1.f;
		float Metallic = 0.f;
		float Roughness = 0.5f;
		float Specular = 0.4f;
		float Emissive = 0.f;
		float Opacity = 1.f;
		float SubR = 0.f;
		float SubG = 0.f;
		float SubB = 0.f;
		SurfaceKind Kind = SurfaceKind::Opaque;
		bool TwoSided = false;
	};

	struct PersonLook
	{
		const char* Name = "";
		float Height = 1.f;
		float Shoulder = 0.4f;
		float Hip = 0.4f;
		float Torso = 0.35f;
		float Head = 0.32f;
		float SkinR = 0.4f;
		float SkinG = 0.28f;
		float SkinB = 0.18f;
		float ClothR = 0.2f;
		float ClothG = 0.12f;
		float ClothB = 0.06f;
		float HairR = 0.05f;
		float HairG = 0.03f;
		float HairB = 0.02f;
		int HairStyle = 0; // 0 crop, 1 bun, 2 long, 3 tied, 4 thick, 5 streak-bun
		bool Beard = false;
		bool Woman = true;
	};

	struct AnimalLook
	{
		float BodyLen = 1.f;
		float BodyRad = 0.28f;
		float LegLen = 0.55f;
		float NeckLen = 0.32f;
		float FurR = 0.22f;
		float FurG = 0.14f;
		float FurB = 0.08f;
	};

	int MaterialRecipeCount();
	const MaterialRecipe& MaterialRecipeAt(int Index);
	const MaterialRecipe* FindMaterialRecipe(const char* Name);

	int PersonLookCount();
	const PersonLook& PersonLookAt(int Id);

	int AnimalLookCount();
	const AnimalLook& AnimalLookAt(int Id);

	float ColorDistance(float R0, float G0, float B0, float R1, float G1, float B1);
}
