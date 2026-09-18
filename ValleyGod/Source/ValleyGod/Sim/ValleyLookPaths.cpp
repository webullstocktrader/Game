#include "ValleyLookPaths.h"

#include <cctype>
#include <cstring>

namespace vg
{
	namespace
	{
		const char* kMetaHumanClassPaths[] = {
			"/Game/MetaHumans/Mara/BP_Mara",
			"/Game/MetaHumans/Mara/Mara",
			"/Game/MetaHumans/Mara/BP_MetaHuman",
			"/Game/ValleySlice/BP_Mara"
		};

		const char* kDirtMaterials[] = {
			"/Game/ValleySlice/MI_Dirt",
			"/Game/Megascans/Surfaces/Dirt/MI_Dirt",
			"/Game/Megascans/Surfaces/Forest_Dirt/MI_Forest_Dirt",
			"/Game/Megascans/Surfaces/Forest_Floor/MI_Forest_Floor",
			"/Game/Megascans/Surfaces/Soil/MI_Soil",
			"/Game/Megascans/Surfaces/Ground/MI_Ground"
		};

		const char* kGrassMaterials[] = {
			"/Game/ValleySlice/MI_Grass",
			"/Game/Megascans/Surfaces/Grass/MI_Grass",
			"/Game/PN_GrassLibrary/Materials/MI_Grass",
			"/Game/PN_GrassLibrary/MI_Grass",
			"/Game/Megascans/Surfaces/Wild_Grass/MI_Wild_Grass",
			"/Game/Megascans/Surfaces/Meadow/MI_Meadow"
		};

		const char* kWetDirtMaterials[] = {
			"/Game/ValleySlice/MI_DirtWet",
			"/Game/Megascans/Surfaces/DirtWet/MI_DirtWet",
			"/Game/Megascans/Surfaces/Wet_Mud/MI_Wet_Mud",
			"/Game/Megascans/Surfaces/Wet_Ground/MI_Wet_Ground"
		};

		const char* kTreeMeshes[] = {
			"/Game/ValleySlice/SM_Tree",
			"/Game/Megascans/3D_Plants/Tree/SM_Tree",
			"/Game/Megascans/3D_Plants/European_Beech/SM_European_Beech",
			"/Game/Megascans/3D_Plants/Pine/SM_Pine",
			"/Game/Megascans/3D_Plants/Oak/SM_Oak",
			"/Game/Megascans/3D_Plants/Silver_Birch/SM_Silver_Birch",
			"/Game/Megascans/3D_Plants/Scots_Pine/SM_Scots_Pine",
			"/Game/Fab/ForestPack/SM_Tree"
		};

		const char* kGrassMeshes[] = {
			"/Game/ValleySlice/SM_Grass",
			"/Game/PN_GrassLibrary/Meshes/SM_Grass",
			"/Game/PN_GrassLibrary/Meshes/SM_Grass_01",
			"/Game/PN_GrassLibrary/Foliage/SM_Grass",
			"/Game/Megascans/3D_Plants/Grass/SM_Grass",
			"/Game/Megascans/3D_Plants/Wild_Grass_Clump/SM_Wild_Grass_Clump",
			"/Game/Megascans/3D_Plants/Meadow_Grass/SM_Meadow_Grass"
		};

		const char* kRockMeshes[] = {
			"/Game/ValleySlice/SM_Rock",
			"/Game/Megascans/3D_Assets/Rock/SM_Rock",
			"/Game/Megascans/3D_Assets/Cliff_Rock/SM_Cliff_Rock",
			"/Game/Megascans/3D_Assets/Forest_Rock/SM_Forest_Rock",
			"/Game/Megascans/3D_Assets/Mossy_Rock/SM_Mossy_Rock"
		};

		bool ContainsFold(const char* Hay, const char* Needle)
		{
			if (!Hay || !Needle || !Needle[0])
			{
				return false;
			}
			const size_t N = std::strlen(Needle);
			for (const char* P = Hay; *P; ++P)
			{
				size_t I = 0;
				while (I < N)
				{
					const unsigned char A = static_cast<unsigned char>(P[I]);
					const unsigned char B = static_cast<unsigned char>(Needle[I]);
					if (!P[I] || std::tolower(A) != std::tolower(B))
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

		bool ContainsWord(const char* Hay, const char* Needle)
		{
			if (!Hay || !Needle || !Needle[0])
			{
				return false;
			}
			const size_t N = std::strlen(Needle);
			for (const char* P = Hay; *P; ++P)
			{
				if (P != Hay)
				{
					const unsigned char Prev = static_cast<unsigned char>(*(P - 1));
					if (std::isalnum(Prev))
					{
						continue;
					}
				}
				size_t I = 0;
				while (I < N)
				{
					const unsigned char A = static_cast<unsigned char>(P[I]);
					const unsigned char B = static_cast<unsigned char>(Needle[I]);
					if (!P[I] || std::tolower(A) != std::tolower(B))
					{
						break;
					}
					++I;
				}
				if (I == N)
				{
					const unsigned char Next = static_cast<unsigned char>(P[N]);
					if (!Next || !std::isalnum(Next))
					{
						return true;
					}
				}
			}
			return false;
		}

		bool InPlantFolder(const char* Path)
		{
			return ContainsFold(Path, "/3D_Plants") || ContainsFold(Path, "/Foliage") || ContainsFold(Path, "/3D_Assets");
		}

		template <int N>
		const char* AtOrEmpty(const char* (&Table)[N], int Index)
		{
			if (Index < 0 || Index >= N)
			{
				return "";
			}
			return Table[Index];
		}
	} // namespace

	bool UsesMetaHumanSlot(int Slot)
	{
		return Slot == kMetaHumanMilestoneSlot;
	}

	const char* MetaHumanContentFolder() { return "/Game/MetaHumans/Mara"; }
	const char* PNGrassLibraryContentFolder() { return "/Game/PN_GrassLibrary"; }
	const char* MegascansContentFolder() { return "/Game/Megascans"; }
	const char* FabContentFolder() { return "/Game/Fab"; }
	const char* ValleySliceContentFolder() { return "/Game/ValleySlice"; }
	const char* ValleySliceMapPath() { return "/Game/Maps/ValleySlice"; }

	int PreferredTreeScatterCount() { return 140; }
	int PreferredGrassScatterCount() { return 1600; }
	int PreferredRockScatterCount() { return 90; }
	int ProceduralGrassTuftCount() { return 400; }

	int MetaHumanClassPathCount() { return static_cast<int>(sizeof(kMetaHumanClassPaths) / sizeof(kMetaHumanClassPaths[0])); }
	const char* MetaHumanClassPathAt(int Index) { return AtOrEmpty(kMetaHumanClassPaths, Index); }

	int DirtMaterialPathCount() { return static_cast<int>(sizeof(kDirtMaterials) / sizeof(kDirtMaterials[0])); }
	const char* DirtMaterialPathAt(int Index) { return AtOrEmpty(kDirtMaterials, Index); }

	int GrassMaterialPathCount() { return static_cast<int>(sizeof(kGrassMaterials) / sizeof(kGrassMaterials[0])); }
	const char* GrassMaterialPathAt(int Index) { return AtOrEmpty(kGrassMaterials, Index); }

	int WetDirtMaterialPathCount() { return static_cast<int>(sizeof(kWetDirtMaterials) / sizeof(kWetDirtMaterials[0])); }
	const char* WetDirtMaterialPathAt(int Index) { return AtOrEmpty(kWetDirtMaterials, Index); }

	int TreeMeshPathCount() { return static_cast<int>(sizeof(kTreeMeshes) / sizeof(kTreeMeshes[0])); }
	const char* TreeMeshPathAt(int Index) { return AtOrEmpty(kTreeMeshes, Index); }

	int GrassMeshPathCount() { return static_cast<int>(sizeof(kGrassMeshes) / sizeof(kGrassMeshes[0])); }
	const char* GrassMeshPathAt(int Index) { return AtOrEmpty(kGrassMeshes, Index); }

	int RockMeshPathCount() { return static_cast<int>(sizeof(kRockMeshes) / sizeof(kRockMeshes[0])); }
	const char* RockMeshPathAt(int Index) { return AtOrEmpty(kRockMeshes, Index); }

	bool ClassifyContentPath(const char* Path, ScanKind Kind)
	{
		if (!Path || !Path[0])
		{
			return false;
		}

		const bool bPlant = InPlantFolder(Path);
		const bool bPNGrass = ContainsFold(Path, "PN_GrassLibrary");
		const bool bDirtWord = ContainsWord(Path, "dirt") || ContainsWord(Path, "soil") || ContainsWord(Path, "mud")
			|| ContainsWord(Path, "ground") || ContainsWord(Path, "forest_floor");
		const bool bGrassWord = ContainsWord(Path, "grass") || ContainsWord(Path, "meadow") || ContainsWord(Path, "lawn")
			|| ContainsWord(Path, "tuft") || ContainsWord(Path, "weed") || ContainsWord(Path, "fern");
		const bool bWetWord = ContainsWord(Path, "wet") || ContainsWord(Path, "damp") || ContainsWord(Path, "moist");
		const bool bTreeWord = ContainsWord(Path, "tree") || ContainsWord(Path, "pine") || ContainsWord(Path, "oak")
			|| ContainsWord(Path, "beech") || ContainsWord(Path, "birch") || ContainsWord(Path, "spruce")
			|| ContainsWord(Path, "fir") || ContainsWord(Path, "willow") || ContainsWord(Path, "cedar")
			|| ContainsWord(Path, "aspen") || ContainsWord(Path, "maple") || ContainsWord(Path, "poplar")
			|| ContainsWord(Path, "larch") || ContainsWord(Path, "elm") || ContainsWord(Path, "hemlock")
			|| ContainsWord(Path, "sycamore") || ContainsWord(Path, "cypress");
		const bool bRockWord = ContainsWord(Path, "rock") || ContainsWord(Path, "stone") || ContainsWord(Path, "boulder")
			|| ContainsWord(Path, "cliff");
		const bool bMaterialToken = ContainsFold(Path, "MI_") || ContainsFold(Path, "/Materials");
		const bool bSurface = ContainsFold(Path, "/Surfaces");
		const bool bPNGrassNamed = bPNGrass
			&& (ContainsWord(Path, "grass") || ContainsWord(Path, "tuft") || ContainsWord(Path, "lawn")
				|| ContainsWord(Path, "meadow"));

		switch (Kind)
		{
		case ScanKind::DirtMaterial:
			return bDirtWord && !bGrassWord && !bPlant;
		case ScanKind::GrassMaterial:
			return (bGrassWord || (bPNGrassNamed && bMaterialToken)) && !bPlant;
		case ScanKind::WetDirtMaterial:
			return (bDirtWord || ContainsWord(Path, "mud")) && bWetWord && !bPlant;
		case ScanKind::TreeMesh:
			return bTreeWord && !bSurface && !bMaterialToken && !bGrassWord && !bRockWord;
		case ScanKind::GrassMesh:
			return (bGrassWord || (bPNGrassNamed && !bMaterialToken)) && !bSurface && !bMaterialToken && !bRockWord;
		case ScanKind::RockMesh:
			return bRockWord && !bTreeWord && !bGrassWord && !bSurface && !bMaterialToken;
		default:
			return false;
		}
	}
} // namespace vg
