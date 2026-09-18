#pragma once

#include "CoreMinimal.h"

class UClass;
class UMaterialInterface;
class UStaticMesh;

namespace Valley
{
	struct FOptionalAssets
	{
		UClass* MaraClass = nullptr;
		UMaterialInterface* Dirt = nullptr;
		UMaterialInterface* Grass = nullptr;
		UMaterialInterface* WetDirt = nullptr;
		TArray<UStaticMesh*> Trees;
		TArray<UStaticMesh*> GrassMeshes;
		TArray<UStaticMesh*> Rocks;
	};

	// Loads documented aliases, then scans Content/Megascans, Fab, Materials, and PN_GrassLibrary.
	// Missing downloads yield empty fields so callers keep procedural art.
	FOptionalAssets DiscoverOptionalAssets();
}
