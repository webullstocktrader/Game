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
			"/Game/Megascans/Surfaces/Dirt/MI_Dirt"
		};

		const char* kGrassMaterials[] = {
			"/Game/ValleySlice/MI_Grass",
			"/Game/Megascans/Surfaces/Grass/MI_Grass"
		};

		const char* kWetDirtMaterials[] = {
			"/Game/ValleySlice/MI_DirtWet",
			"/Game/Megascans/Surfaces/DirtWet/MI_DirtWet"
		};

		const char* kTreeMeshes[] = {
			"/Game/ValleySlice/SM_Tree",
			"/Game/Megascans/3D_Plants/Tree/SM_Tree"
		};

		const char* kGrassMeshes[] = {
			"/Game/ValleySlice/SM_Grass",
			"/Game/Megascans/3D_Plants/Grass/SM_Grass"
		};

		const char* kRockMeshes[] = {
			"/Game/ValleySlice/SM_Rock",
			"/Game/Megascans/3D_Assets/Rock/SM_Rock"
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
	const char* MegascansContentFolder() { return "/Game/Megascans"; }
	const char* FabContentFolder() { return "/Game/Fab"; }
	const char* ValleySliceContentFolder() { return "/Game/ValleySlice"; }
	const char* ValleySliceMapPath() { return "/Game/Maps/ValleySlice"; }

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
		const bool bDirtWord = ContainsFold(Path, "dirt") || ContainsFold(Path, "soil") || ContainsFold(Path, "mud")
			|| ContainsFold(Path, "ground");
		const bool bGrassWord = ContainsFold(Path, "grass") || ContainsFold(Path, "meadow") || ContainsFold(Path, "lawn");
		const bool bWetWord = ContainsFold(Path, "wet") || ContainsFold(Path, "damp") || ContainsFold(Path, "moist");
		const bool bTreeWord = ContainsFold(Path, "tree") || ContainsFold(Path, "pine") || ContainsFold(Path, "oak")
			|| ContainsFold(Path, "beech") || ContainsFold(Path, "birch") || ContainsFold(Path, "spruce")
			|| ContainsFold(Path, "fir") || ContainsFold(Path, "willow") || ContainsFold(Path, "cedar");
		const bool bRockWord = ContainsFold(Path, "rock") || ContainsFold(Path, "stone") || ContainsFold(Path, "boulder")
			|| ContainsFold(Path, "cliff");

		switch (Kind)
		{
		case ScanKind::DirtMaterial:
			return bDirtWord && !bGrassWord && !bPlant;
		case ScanKind::GrassMaterial:
			return bGrassWord && !bPlant;
		case ScanKind::WetDirtMaterial:
			return (bDirtWord || ContainsFold(Path, "mud")) && bWetWord && !bPlant;
		case ScanKind::TreeMesh:
			return bTreeWord && !ContainsFold(Path, "/Surfaces") && !ContainsFold(Path, "MI_");
		case ScanKind::GrassMesh:
			return bGrassWord && !ContainsFold(Path, "/Surfaces") && !ContainsFold(Path, "MI_");
		case ScanKind::RockMesh:
			return bRockWord && !bTreeWord && !bGrassWord && !ContainsFold(Path, "/Surfaces") && !ContainsFold(Path, "MI_");
		default:
			return false;
		}
	}
} // namespace vg
