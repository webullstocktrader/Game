#include "ValleyTypes.h"
#include "Engine/StaticMesh.h"

// Mesh helpers only. Valley::Material / FallbackMaterial live in ValleyMaterials.cpp.
// Putting them back here LNK2005s against a stale ValleyTypes.cpp.obj (unity/incremental).

namespace
{
	UStaticMesh* LoadMesh(const TCHAR* Path)
	{
		return LoadObject<UStaticMesh>(nullptr, Path);
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
}
