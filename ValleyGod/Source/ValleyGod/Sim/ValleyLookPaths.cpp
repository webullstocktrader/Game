#include "ValleyLookPaths.h"
#include "ValleyPalette.h"

#include <cctype>
#include <cstdio>
#include <cstring>

namespace vg
{
	namespace
	{
		const char* kVillagerPathFmts[] = {
			"/Game/EditableMetahumans/%s/BP_%s",
			"/Game/EditableMetahumans/%s/%s",
			"/Game/MetaHumans/%s/BP_%s",
			"/Game/MetaHumans/%s/%s",
			"/Game/MetaHumans/%s/BP_MetaHuman",
			"/Game/ValleySlice/BP_%s"
		};

		const char* kMaraExtraPaths[] = {
			"/Game/EditableMetahumans/MHC_Hannah/Mara/BP_Mara"
		};

		const char* kDirtMaterials[] = {
			"/Game/ValleySlice/MI_Dirt",
			"/Game/Megascans/Surfaces/Dirt/MI_Dirt",
			"/Game/Megascans/Surfaces/Forest_Dirt/MI_Forest_Dirt",
			"/Game/Megascans/Surfaces/Forest_Floor/MI_Forest_Floor"
		};

		const char* kGrassMaterials[] = {
			"/Game/ValleySlice/MI_Grass",
			"/Game/Megascans/Surfaces/Grass/MI_Grass",
			"/Game/PN_GrassLibrary/Materials/MI_Grass",
			"/Game/PN_GrassLibrary/MI_Grass",
			"/Game/Megascans/Surfaces/Wild_Grass/MI_Wild_Grass"
		};

		const char* kWetDirtMaterials[] = {
			"/Game/ValleySlice/MI_DirtWet",
			"/Game/Megascans/Surfaces/DirtWet/MI_DirtWet",
			"/Game/Megascans/Surfaces/Wet_Mud/MI_Wet_Mud"
		};

		const char* kTreeMeshes[] = {
			"/Game/ValleySlice/SM_Tree",
			"/Game/Megascans/3D_Plants/Tree/SM_Tree",
			"/Game/Megascans/3D_Plants/European_Beech/SM_European_Beech",
			"/Game/Megascans/3D_Plants/Pine/SM_Pine",
			"/Game/Fab/ForestPack/SM_Tree"
		};

		const char* kGrassMeshes[] = {
			"/Game/ValleySlice/SM_Grass",
			"/Game/PN_GrassLibrary/Meshes/SM_Grass",
			"/Game/PN_GrassLibrary/Foliage/SM_Grass",
			"/Game/Megascans/3D_Plants/Grass/SM_Grass",
			"/Game/Megascans/3D_Plants/Wild_Grass_Clump/SM_Wild_Grass_Clump"
		};

		const char* kRockMeshes[] = {
			"/Game/ValleySlice/SM_Rock",
			"/Game/Megascans/3D_Assets/Rock/SM_Rock",
			"/Game/Megascans/3D_Assets/Cliff_Rock/SM_Cliff_Rock"
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

		bool EqualsFold(const char* A, const char* B)
		{
			if (!A || !B)
			{
				return false;
			}
			while (*A && *B)
			{
				if (std::tolower(static_cast<unsigned char>(*A)) != std::tolower(static_cast<unsigned char>(*B)))
				{
					return false;
				}
				++A;
				++B;
			}
			return *A == *B;
		}

		bool StartsWithFold(const char* Hay, const char* Needle)
		{
			if (!Hay || !Needle || !Needle[0])
			{
				return false;
			}
			while (*Needle)
			{
				if (!*Hay
					|| std::tolower(static_cast<unsigned char>(*Hay)) != std::tolower(static_cast<unsigned char>(*Needle)))
				{
					return false;
				}
				++Hay;
				++Needle;
			}
			return true;
		}

		const char* LastComponent(const char* Path)
		{
			if (!Path)
			{
				return "";
			}
			const char* Last = Path;
			for (const char* P = Path; *P; ++P)
			{
				if (*P == '/')
				{
					Last = P + 1;
				}
			}
			return Last;
		}

		bool PathHasSegment(const char* Path, const char* Seg)
		{
			if (!Path || !Seg || !Seg[0])
			{
				return false;
			}
			const size_t N = std::strlen(Seg);
			for (const char* P = Path; *P; ++P)
			{
				if (P != Path && *(P - 1) != '/')
				{
					continue;
				}
				size_t I = 0;
				while (I < N)
				{
					if (!P[I]
						|| std::tolower(static_cast<unsigned char>(P[I]))
							!= std::tolower(static_cast<unsigned char>(Seg[I])))
					{
						break;
					}
					++I;
				}
				if (I == N && (P[N] == 0 || P[N] == '/'))
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

		bool RejectPresentationAsset(const char* Path, const char* Short)
		{
			if (StartsWithFold(Short, "SK_") || StartsWithFold(Short, "ABP_") || StartsWithFold(Short, "MI_")
				|| StartsWithFold(Short, "M_") || StartsWithFold(Short, "SM_") || StartsWithFold(Short, "T_"))
			{
				return true;
			}
			if (ContainsFold(Short, "Groom") || ContainsFold(Path, "/Groom") || ContainsFold(Path, "/Materials")
				|| ContainsFold(Path, "/Textures") || ContainsFold(Path, "/Maps/"))
			{
				return true;
			}
			if (ContainsFold(Short, "Hair") && !StartsWithFold(Short, "BP_"))
			{
				return true;
			}
			return false;
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

		constexpr int kFmtCount = static_cast<int>(sizeof(kVillagerPathFmts) / sizeof(kVillagerPathFmts[0]));
		constexpr int kMaraExtraCount = static_cast<int>(sizeof(kMaraExtraPaths) / sizeof(kMaraExtraPaths[0]));
	} // namespace

	bool UsesMetaHumanSlot(int Slot)
	{
		return Slot >= 0;
	}

	const char* MetaHumanContentFolder() { return "/Game/MetaHumans"; }
	const char* EditableMetahumansContentFolder() { return "/Game/EditableMetahumans"; }
	const char* PNGrassLibraryContentFolder() { return "/Game/PN_GrassLibrary"; }
	const char* MegascansContentFolder() { return "/Game/Megascans"; }
	const char* FabContentFolder() { return "/Game/Fab"; }
	const char* ValleySliceContentFolder() { return "/Game/ValleySlice"; }
	const char* ValleySliceMapPath() { return "/Game/Maps/ValleySlice"; }

	int VillagerMetaHumanPathCount(int Slot)
	{
		if (Slot < 0 || Slot >= PersonLookCount())
		{
			return 0;
		}
		return kFmtCount + (Slot == kMetaHumanMilestoneSlot ? kMaraExtraCount : 0);
	}

	const char* VillagerMetaHumanPathAt(int Slot, int Index)
	{
		static char Cache[16][8][256];
		const int Count = VillagerMetaHumanPathCount(Slot);
		if (Slot < 0 || Slot >= PersonLookCount() || Slot >= 16 || Index < 0 || Index >= Count || Index >= 8)
		{
			return "";
		}
		char* Out = Cache[Slot][Index];
		const char* Name = PersonLookAt(Slot).Name;
		if (!Name || !Name[0])
		{
			Out[0] = 0;
			return "";
		}
		if (Index < kFmtCount)
		{
			std::snprintf(Out, 256, kVillagerPathFmts[Index], Name, Name);
		}
		else
		{
			std::snprintf(Out, 256, "%s", kMaraExtraPaths[Index - kFmtCount]);
		}
		return Out;
	}

	int MetaHumanClassPathCount() { return VillagerMetaHumanPathCount(kMetaHumanMilestoneSlot); }
	const char* MetaHumanClassPathAt(int Index) { return VillagerMetaHumanPathAt(kMetaHumanMilestoneSlot, Index); }

	bool PackageMatchesVillager(const char* Path, const char* VillagerName)
	{
		if (!Path || !Path[0] || !VillagerName || !VillagerName[0])
		{
			return false;
		}
		const char* Short = LastComponent(Path);
		if (RejectPresentationAsset(Path, Short))
		{
			return false;
		}
		const bool bBP = StartsWithFold(Short, "BP_");
		const bool bNamedShort = EqualsFold(Short, VillagerName);
		const bool bBPNamed = bBP && EqualsFold(Short + 3, VillagerName);
		const bool bGenericInNamedFolder = bBP
			&& (EqualsFold(Short, "BP_MetaHuman") || EqualsFold(Short, "BP_MH"))
			&& PathHasSegment(Path, VillagerName);
		if (!bBP && !bNamedShort)
		{
			return false;
		}
		return bBPNamed || bNamedShort || bGenericInNamedFolder;
	}

	bool LooksLikeGenericMetaHumanBlueprint(const char* Path)
	{
		if (!Path || !Path[0])
		{
			return false;
		}
		if (!ContainsFold(Path, "/EditableMetahumans/") && !ContainsFold(Path, "/MetaHumans/")
			&& !ContainsFold(Path, "/ValleySlice/"))
		{
			return false;
		}
		const char* Short = LastComponent(Path);
		if (!StartsWithFold(Short, "BP_"))
		{
			return false;
		}
		if (RejectPresentationAsset(Path, Short))
		{
			return false;
		}
		if (ContainsFold(Short, "MHC") || ContainsFold(Short, "Groom") || ContainsFold(Short, "Face")
			|| ContainsFold(Short, "Body") || ContainsFold(Short, "Hair"))
		{
			return false;
		}
		if (!EqualsFold(Short, "BP_MetaHuman") && !EqualsFold(Short, "BP_MH")
			&& !StartsWithFold(Short, "BP_MetaHuman_"))
		{
			return false;
		}
		for (int I = 0; I < PersonLookCount(); ++I)
		{
			if (PackageMatchesVillager(Path, PersonLookAt(I).Name))
			{
				return false;
			}
		}
		return true;
	}

	int PreferredTreeScatterCount() { return 64; }
	int PreferredGrassScatterCount() { return 160; }
	int PreferredRockScatterCount() { return 28; }
	int ProceduralGrassTuftCount() { return 90; }

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
