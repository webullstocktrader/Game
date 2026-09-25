#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltWorldBuilder.generated.h"

class ASiltTerrain;
class ASiltTruck;
class ASiltCrate;
class ASiltDropZone;
class ASiltDeer;
class ASiltAnchor;

UCLASS()
class SILTCOUNTY_API ASiltWorldBuilder : public AActor
{
	GENERATED_BODY()

public:
	ASiltWorldBuilder();
	virtual void Tick(float DeltaSeconds) override;

	void BuildSlice();

	ASiltTruck* GetChief() const { return Chief; }
	ASiltTruck* GetGooch() const { return Gooch; }
	ASiltCrate* GetCrate() const { return Crate; }
	ASiltDropZone* GetDropZone() const { return DropZone; }
	ASiltTerrain* GetTerrain() const { return Terrain; }
	FVector GetGarageLookAt() const;
	FVector GetKeyRackLocation() const { return KeyRackLocation; }

private:
	void StripTemplateActors();
	void SpawnAtmosphere();
	void SpawnGarage();
	void SpawnGarageDressing();
	void SpawnTreesAndStumps();
	void SpawnTown();
	void SpawnRain();
	void UpdateRain(float DeltaSeconds);
	UStaticMeshComponent* PlaceMesh(UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat, const FName& Name);
	void PlaceLabel(const FName& Name, const FString& Text, const FVector& Loc, float YawDeg, float WorldSize, const FColor& Color);

	UPROPERTY()
	TObjectPtr<ASiltTerrain> Terrain;

	UPROPERTY()
	TObjectPtr<ASiltTruck> Chief;

	UPROPERTY()
	TObjectPtr<ASiltTruck> Gooch;

	UPROPERTY()
	TObjectPtr<ASiltCrate> Crate;

	UPROPERTY()
	TObjectPtr<ASiltDropZone> DropZone;

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> RainStreaks;

	FVector KeyRackLocation = FVector::ZeroVector;
	float RainClock = 0.f;
};
