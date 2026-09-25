#include "SiltTypes.h"
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

namespace Silt
{
	UStaticMesh* CubeMesh()
	{
		if (UStaticMesh* Mesh = LoadMesh(TEXT("/Engine/BasicShapes/Cube.Cube")))
		{
			return Mesh;
		}
		return nullptr;
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
		return LoadMesh(TEXT("/Engine/BasicShapes/Cone.Cone"));
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

	FLinearColor SurfaceColor(ESiltSurface Surface)
	{
		switch (Surface)
		{
		case ESiltSurface::Pavement: return FLinearColor(0.025f, 0.025f, 0.028f);
		case ESiltSurface::Gravel:   return FLinearColor(0.09f, 0.08f, 0.07f);
		case ESiltSurface::Grass:    return FLinearColor(0.04f, 0.07f, 0.03f);
		case ESiltSurface::Water:    return FLinearColor(0.03f, 0.05f, 0.045f);
		case ESiltSurface::Wood:     return FLinearColor(0.08f, 0.05f, 0.02f);
		case ESiltSurface::Mud:
		default:                     return FLinearColor(0.08f, 0.05f, 0.025f);
		}
	}

	const TCHAR* DriverName(ESiltDriver Driver)
	{
		return Driver == ESiltDriver::Chief ? TEXT("CHIEF") : TEXT("GOOCH");
	}

	const TCHAR* SurfaceName(ESiltSurface Surface)
	{
		switch (Surface)
		{
		case ESiltSurface::Pavement: return TEXT("PAVE");
		case ESiltSurface::Gravel:   return TEXT("GRAVEL");
		case ESiltSurface::Grass:    return TEXT("GRASS");
		case ESiltSurface::Water:    return TEXT("FORD");
		case ESiltSurface::Wood:     return TEXT("WOOD");
		default:                     return TEXT("MUD");
		}
	}
}
