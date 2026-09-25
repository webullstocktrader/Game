#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SiltCountyGameMode.generated.h"

class ASiltContractActor;
class ASiltTruckPawn;

UCLASS()
class SILTCOUNTY_API ASiltCountyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASiltCountyGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

private:
	UPROPERTY()
	TObjectPtr<ASiltTruckPawn> ChiefTruck;

	UPROPERTY()
	TObjectPtr<ASiltTruckPawn> GoochTruck;

	UPROPERTY()
	TObjectPtr<ASiltContractActor> Contract;

	ASiltTruckPawn* SpawnTruck(int32 Index, const FString& Name, const FLinearColor& Paint, const FVector& Location, const FRotator& Rotation);
	ASiltTruckPawn* ClaimTruck();
};
