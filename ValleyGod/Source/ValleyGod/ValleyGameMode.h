#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ValleyGameMode.generated.h"

class AValleyWorld;

UCLASS()
class VALLEYGOD_API AValleyGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AValleyGameMode();
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	TObjectPtr<AValleyWorld> Valley;
};
