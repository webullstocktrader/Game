#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DefaultPawn.h"
#include "ValleyGodPawn.generated.h"

UCLASS()
class VALLEYGOD_API AValleyGodPawn : public ADefaultPawn
{
	GENERATED_BODY()

public:
	AValleyGodPawn();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void SetFlyInput(float Forward, float Right, float Up, bool bSprint);
	void AddLook(float Yaw, float Pitch);
	void AddZoom(float Wheel);
	void FollowActor(AActor* Target, float DeltaSeconds);
	void StopFollow();
	void GoOverview();
	void HideWatcherBody();

	bool bFollowing = false;

private:
	float ForwardAxis = 0.f;
	float RightAxis = 0.f;
	float UpAxis = 0.f;
	bool bSprint = false;
	float FlySpeed = 2200.f;
	float Fov = 80.f;
	TWeakObjectPtr<AActor> FollowTarget;
};
