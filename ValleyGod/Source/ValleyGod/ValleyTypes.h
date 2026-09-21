#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UMaterial;
class UMaterialInterface;
class UObject;

namespace vg
{
	struct MaterialRecipe;
}

namespace Valley
{
	UStaticMesh* CubeMesh();
	UStaticMesh* SphereMesh();
	UStaticMesh* CylinderMesh();
	UStaticMesh* ConeMesh();
	UStaticMesh* PlaneMesh();
	UMaterialInterface* Material(const TCHAR* ShortName);
	UMaterialInterface* FallbackMaterial();
	UMaterialInterface* Tint(UObject* Outer, UMaterialInterface* Parent, const FLinearColor& Color, const FName& Name);
	// Compiled parent + MID. Sets BaseColor and Color so BasicShape cannot stay blue/white.
	UMaterialInterface* RecipeMid(UObject* Outer, const TCHAR* RecipeName, const FName& Name);
	// Quixel/Megascans/PN material when the path is a real ground scan; otherwise RecipeMid.
	UMaterialInterface* ResolveScannedOrMid(UObject* Outer, UMaterialInterface* Scanned, const TCHAR* RecipeName, const FName& Name);
	void PopulateLitMaterial(UMaterial* Mat, const vg::MaterialRecipe& Recipe);
	void EnsureMaterials();
}
