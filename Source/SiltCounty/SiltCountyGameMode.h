#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SiltCountyGameMode.generated.h"

class ASiltWorldBuilder;
class ASiltIntroDirector;
class ASiltTruck;
class ASiltCountyPlayerController;

UCLASS()
class SILTCOUNTY_API ASiltCountyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASiltCountyGameMode();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

	void BeginCountyPlay();
	void AssignTruck(APlayerController* PC);

protected:
	void WatchContract();

	UPROPERTY()
	TObjectPtr<ASiltWorldBuilder> CountyWorld;

	UPROPERTY()
	TObjectPtr<ASiltIntroDirector> Intro;

	bool bPlayStarted = false;
	bool bSecondLocalSpawned = false;
};
