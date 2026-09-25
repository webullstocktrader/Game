#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltTypes.h"
#include "SiltContractActor.generated.h"

class ASiltTruckPawn;
class UBoxComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class SILTCOUNTY_API ASiltContractActor : public AActor
{
	GENERATED_BODY()

public:
	ASiltContractActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void TryHook(ASiltTruckPawn* Truck);
	ESiltContractState GetContractState() const { return State; }
	float GetHookRange() const { return HookRange; }
	ASiltTruckPawn* GetHookedTruck() const { return HookedTruck; }

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Van;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> VanBody;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Cabin;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Person;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> HelpText;

	UPROPERTY(ReplicatedUsing = OnRep_State)
	ESiltContractState State = ESiltContractState::Locked;

	UPROPERTY(Replicated)
	TObjectPtr<ASiltTruckPawn> HookedTruck;

	float HookRange = 1400.f;

	UFUNCTION()
	void OnRep_State();

	void ApplyPhysicsRole();
	void RefreshText() const;
	void StepTow(float DeltaSeconds);
	void TryComplete();
};
