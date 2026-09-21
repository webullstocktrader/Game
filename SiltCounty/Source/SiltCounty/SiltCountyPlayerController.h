#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SiltCountyPlayerController.generated.h"

class ACameraActor;

UCLASS()
class SILTCOUNTY_API ASiltCountyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASiltCountyPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

	void RequestSkipIntro();
	void RequestSplitScreen();

protected:
	UFUNCTION(Server, Reliable)
	void ServerSkipIntro();

private:
	UPROPERTY()
	TObjectPtr<ACameraActor> IntroCamera;

	bool bIntroView = false;

	void UpdateIntroCamera();
};
