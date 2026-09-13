#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltTypes.h"
#include "SiltProp.generated.h"

UCLASS()
class SILTCOUNTY_API ASiltAnchor : public AActor
{
	GENERATED_BODY()

public:
	ASiltAnchor();
	void BuildStump(const FVector& Location);
	void BuildStranded(const FVector& Location, const FRotator& Rotation);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Body;
};

UCLASS()
class SILTCOUNTY_API ASiltCrate : public AActor
{
	GENERATED_BODY()

public:
	ASiltCrate();
	void Place(const FVector& Location);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Box;
};

UCLASS()
class SILTCOUNTY_API ASiltDropZone : public AActor
{
	GENERATED_BODY()

public:
	ASiltDropZone();
	void Place(const FVector& Location);
	bool Contains(const FVector& Point) const;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Pad;
};
