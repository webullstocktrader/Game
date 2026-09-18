#pragma once

#include "CoreMinimal.h"

class FValleyContentFactory
{
public:
	static bool EnsureContent();

private:
	static UMaterial* CreateLit(const FString& ShortName, const FLinearColor& Color, float Metallic, float Roughness, bool bTranslucent, float Emissive);
	static bool SaveAsset(UObject* Asset, const FString& PackagePath);
	static bool CreateSliceMap();
	static bool AssetExists(const FString& ObjectPath);
};
