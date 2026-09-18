#pragma once

#include "CoreMinimal.h"

class UStaticMesh;
class UMaterialInterface;

namespace Valley
{
	UStaticMesh* CubeMesh();
	UStaticMesh* SphereMesh();
	UStaticMesh* CylinderMesh();
	UStaticMesh* ConeMesh();
	UStaticMesh* PlaneMesh();
	UMaterialInterface* Material(const TCHAR* ShortName);
	UMaterialInterface* FallbackMaterial();
}
