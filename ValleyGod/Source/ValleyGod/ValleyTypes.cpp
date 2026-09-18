#include "ValleyTypes.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"

namespace
{
	UStaticMesh* LoadMesh(const TCHAR* Path)
	{
		return LoadObject<UStaticMesh>(nullptr, Path);
	}

	UMaterialInterface* LoadMat(const TCHAR* Path)
	{
		return LoadObject<UMaterialInterface>(nullptr, Path);
	}
}

namespace Valley
{
	UStaticMesh* CubeMesh()
	{
		return LoadMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	}

	UStaticMesh* SphereMesh()
	{
		return LoadMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	}

	UStaticMesh* CylinderMesh()
	{
		return LoadMesh(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	}

	UStaticMesh* ConeMesh()
	{
		if (UStaticMesh* Cone = LoadMesh(TEXT("/Engine/BasicShapes/Cone.Cone")))
		{
			return Cone;
		}
		return CylinderMesh();
	}

	UStaticMesh* PlaneMesh()
	{
		return LoadMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
	}

	UMaterialInterface* FallbackMaterial()
	{
		if (UMaterialInterface* Mat = LoadMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
		{
			return Mat;
		}
		return UMaterial::GetDefaultMaterial(MD_Surface);
	}

	UMaterialInterface* Material(const TCHAR* ShortName)
	{
		const FString Path = FString::Printf(TEXT("/Game/Materials/%s.%s"), ShortName, ShortName);
		if (UMaterialInterface* Mat = LoadMat(*Path))
		{
			return Mat;
		}
		return FallbackMaterial();
	}
}
