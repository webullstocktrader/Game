#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ValleyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class AValleyWorld;

UCLASS()
class VALLEYGOD_API AValleyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AValleyPlayerController();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

private:
	void BuildMapping();
	UInputAction* MakeAxis(const FName& Name);
	UInputAction* MakeButton(const FName& Name);
	void MapKey(UInputAction* Action, const FKey& Key, bool bNegate = false);

	void OnMove(const struct FInputActionValue& Value);
	void OnMoveRight(const struct FInputActionValue& Value);
	void OnMoveUp(const struct FInputActionValue& Value);
	void OnLookYaw(const struct FInputActionValue& Value);
	void OnLookPitch(const struct FInputActionValue& Value);
	void OnZoom(const struct FInputActionValue& Value);
	void OnSprint(const struct FInputActionValue& Value);
	void OnRain();
	void OnTornado();
	void OnHurricane();
	void OnFlood();
	void OnClear();
	void OnPause();
	void OnDayFaster();
	void OnDaySlower();
	void OnPin();
	void OnCycle();
	void OnOverview();
	void OnNextLand();

	AValleyWorld* Valley() const;

	UPROPERTY()
	TObjectPtr<UInputMappingContext> Mapping;

	UPROPERTY()
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY()
	TObjectPtr<UInputAction> MoveRightAction;
	UPROPERTY()
	TObjectPtr<UInputAction> MoveUpAction;
	UPROPERTY()
	TObjectPtr<UInputAction> LookYawAction;
	UPROPERTY()
	TObjectPtr<UInputAction> LookPitchAction;
	UPROPERTY()
	TObjectPtr<UInputAction> ZoomAction;
	UPROPERTY()
	TObjectPtr<UInputAction> SprintAction;
	UPROPERTY()
	TObjectPtr<UInputAction> RainAction;
	UPROPERTY()
	TObjectPtr<UInputAction> TornadoAction;
	UPROPERTY()
	TObjectPtr<UInputAction> HurricaneAction;
	UPROPERTY()
	TObjectPtr<UInputAction> FloodAction;
	UPROPERTY()
	TObjectPtr<UInputAction> ClearAction;
	UPROPERTY()
	TObjectPtr<UInputAction> PauseAction;
	UPROPERTY()
	TObjectPtr<UInputAction> DayFastAction;
	UPROPERTY()
	TObjectPtr<UInputAction> DaySlowAction;
	UPROPERTY()
	TObjectPtr<UInputAction> PinAction;
	UPROPERTY()
	TObjectPtr<UInputAction> CycleAction;
	UPROPERTY()
	TObjectPtr<UInputAction> OverviewAction;
	UPROPERTY()
	TObjectPtr<UInputAction> NextLandAction;

	float ForwardAxis = 0.f;
	float RightAxis = 0.f;
	float UpAxis = 0.f;
	bool bSprint = false;
};
