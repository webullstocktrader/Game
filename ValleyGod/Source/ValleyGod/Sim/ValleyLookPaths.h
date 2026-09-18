#pragma once

// Engine-free path table for MetaHuman + Quixel/PN foliage.
// Unreal loaders and host tests share this so documented folders cannot drift.

namespace vg
{
	constexpr int kMetaHumanMilestoneSlot = 0; // Mara

	enum class ScanKind
	{
		DirtMaterial = 0,
		GrassMaterial = 1,
		WetDirtMaterial = 2,
		TreeMesh = 3,
		GrassMesh = 4,
		RockMesh = 5
	};

	bool UsesMetaHumanSlot(int Slot);

	const char* MetaHumanContentFolder();
	const char* PNGrassLibraryContentFolder();
	const char* MegascansContentFolder();
	const char* FabContentFolder();
	const char* ValleySliceContentFolder();
	const char* ValleySliceMapPath();

	int PreferredTreeScatterCount();
	int PreferredGrassScatterCount();
	int PreferredRockScatterCount();
	int ProceduralGrassTuftCount();

	int MetaHumanClassPathCount();
	const char* MetaHumanClassPathAt(int Index);

	int DirtMaterialPathCount();
	const char* DirtMaterialPathAt(int Index);

	int GrassMaterialPathCount();
	const char* GrassMaterialPathAt(int Index);

	int WetDirtMaterialPathCount();
	const char* WetDirtMaterialPathAt(int Index);

	int TreeMeshPathCount();
	const char* TreeMeshPathAt(int Index);

	int GrassMeshPathCount();
	const char* GrassMeshPathAt(int Index);

	int RockMeshPathCount();
	const char* RockMeshPathAt(int Index);

	bool ClassifyContentPath(const char* Path, ScanKind Kind);
} // namespace vg
