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
	void PopulateLitMaterial(UMaterial* Mat, const vg::MaterialRecipe& Recipe);
	void EnsureMaterials();
}
