#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SiltIntroDirector.generated.h"

class ASiltWorldBuilder;
class ACameraActor;
class ASiltCountyGameMode;

UCLASS()
class SILTCOUNTY_API ASiltIntroDirector : public AActor
{
	GENERATED_BODY()

public:
	ASiltIntroDirector();
	virtual void Tick(float DeltaSeconds) override;

	void StartIntro(ASiltWorldBuilder* InWorld, ASiltCountyGameMode* InGameMode);
	void Skip();
	FString GetSubtitle() const { return Subtitle; }
	bool IsRunning() const { return bRunning; }

private:
	void SetCam(const FVector& Loc, const FVector& LookAt);

	UPROPERTY()
	TObjectPtr<ASiltWorldBuilder> CountyWorld;

	UPROPERTY()
	TObjectPtr<ASiltCountyGameMode> GameMode;

	UPROPERTY()
	TObjectPtr<ACameraActor> Rig;

	float Time = 0.f;
	bool bRunning = false;
	bool bFinished = false;
	FString Subtitle;
};
