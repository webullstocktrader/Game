#pragma once

#include "CoreMinimal.h"

namespace vg
{
	struct MaterialRecipe;
}

class UMaterial;

class FValleyContentFactory
{
public:
	static bool EnsureContent();

private:
	static UMaterial* CreateLit(const vg::MaterialRecipe& Recipe);
	static bool SaveAsset(UObject* Asset, const FString& PackagePath);
	static bool CreateSliceMap();
	static bool AssetExists(const FString& ObjectPath);
};
