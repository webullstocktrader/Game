#pragma once

#include "CoreMinimal.h"

class UClass;
class UMaterialInterface;
class UStaticMesh;

namespace Valley
{
	struct FOptionalAssets
	{
		// Index matches villager slot. Missing entries stay null (procedural body).
		TArray<UClass*> VillagerClasses;
		UClass* MaraClass = nullptr;
		UMaterialInterface* Dirt = nullptr;
		UMaterialInterface* Grass = nullptr;
		UMaterialInterface* WetDirt = nullptr;
		TArray<UStaticMesh*> Trees;
		TArray<UStaticMesh*> GrassMeshes;
		TArray<UStaticMesh*> Rocks;
	};

	// Loads documented aliases, then scans Content/EditableMetahumans, MetaHumans,
	// PN_GrassLibrary, Megascans, Fab, and ValleySlice. Missing downloads yield
	// empty fields so callers keep procedural art.
	FOptionalAssets DiscoverOptionalAssets();
}
