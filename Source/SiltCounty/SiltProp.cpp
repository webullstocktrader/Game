#include "SiltProp.h"
#include "Components/StaticMeshComponent.h"

ASiltAnchor::ASiltAnchor()
{
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(Root);
	Body->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Tags.Add(Silt::TagAnchor());
}

void ASiltAnchor::BuildStump(const FVector& Location)
{
	SetActorLocation(Location);
	if (UStaticMesh* Cyl = Silt::CylinderMesh())
	{
		Body->SetStaticMesh(Cyl);
		Body->SetWorldScale3D(FVector(0.7f, 0.7f, 0.55f));
		if (UMaterialInterface* Wood = Silt::Material(TEXT("M_Wood")))
		{
			Body->SetMaterial(0, Wood);
		}
	}
#if WITH_EDITOR
	SetActorLabel(TEXT("Stump"));
#endif
}

void ASiltAnchor::BuildStranded(const FVector& Location, const FRotator& Rotation)
{
	SetActorLocationAndRotation(Location, Rotation);
	if (UStaticMesh* Cube = Silt::CubeMesh())
	{
		Body->SetStaticMesh(Cube);
		Body->SetWorldScale3D(FVector(4.6f, 1.8f, 1.4f));
		if (UMaterialInterface* Metal = Silt::Material(TEXT("M_Metal")))
		{
			Body->SetMaterial(0, Metal);
		}
	}

	auto Add = [&](const FName& Name, const FVector& Loc, const FVector& Scale, UMaterialInterface* Mat)
	{
		UStaticMeshComponent* Comp = NewObject<UStaticMeshComponent>(this, Name);
		Comp->SetStaticMesh(Silt::CubeMesh());
		Comp->SetRelativeLocation(Loc);
		Comp->SetRelativeScale3D(Scale);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Mat)
		{
			Comp->SetMaterial(0, Mat);
		}
		Comp->SetupAttachment(Root);
		Comp->RegisterComponent();
	};

	Add(TEXT("Cab"), FVector(140.f, 0.f, 80.f), FVector(1.6f, 1.6f, 1.1f), Silt::Material(TEXT("M_TruckGooch")));
	Add(TEXT("Bed"), FVector(-120.f, 0.f, 40.f), FVector(2.0f, 1.6f, 0.7f), Silt::Material(TEXT("M_Metal")));
#if WITH_EDITOR
	SetActorLabel(TEXT("StrandedCountyRig"));
#endif
}

ASiltCrate::ASiltCrate()
{
	Box = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Box"));
	SetRootComponent(Box);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Tags.Add(Silt::TagCrate());
}

void ASiltCrate::Place(const FVector& Location)
{
	SetActorLocation(Location);
	if (UStaticMesh* Cube = Silt::CubeMesh())
	{
		Box->SetStaticMesh(Cube);
		Box->SetWorldScale3D(FVector(0.9f, 0.7f, 0.7f));
		if (UMaterialInterface* Wood = Silt::Material(TEXT("M_Crate")))
		{
			Box->SetMaterial(0, Wood);
		}
	}
#if WITH_EDITOR
	SetActorLabel(TEXT("CountyCrate"));
#endif
}

ASiltDropZone::ASiltDropZone()
{
	Pad = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pad"));
	SetRootComponent(Pad);
	Pad->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASiltDropZone::Place(const FVector& Location)
{
	SetActorLocation(Location);
	if (UStaticMesh* Cube = Silt::CubeMesh())
	{
		Pad->SetStaticMesh(Cube);
		Pad->SetWorldScale3D(FVector(4.5f, 3.2f, 0.08f));
		if (UMaterialInterface* Paint = Silt::Material(TEXT("M_Asphalt")))
		{
			Pad->SetMaterial(0, Paint);
		}
	}
#if WITH_EDITOR
	SetActorLabel(TEXT("Highway6LayBy"));
#endif
}

bool ASiltDropZone::Contains(const FVector& Point) const
{
	return FVector::Dist2D(Point, GetActorLocation()) < 380.f;
}
