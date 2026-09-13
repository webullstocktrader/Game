#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SiltTypes.h"
#include "SiltCountyPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class ASiltTruck;
class ASiltIntroDirector;

UCLASS()
class SILTCOUNTY_API ASiltCountyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ASiltCountyPlayerController();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	void PossessTruck(ASiltTruck* Truck);
	void BindIntro(ASiltIntroDirector* Intro) { IntroDirector = Intro; }
	int32 GetLocalIndex() const;

	FSiltTruckInput CurrentInput;

private:
	void BuildMapping();
	void ApplyAction(const FName& Name, float Axis);
	void ApplyButton(const FName& Name, bool bDown);

	void OnThrottle(const struct FInputActionValue& Value);
	void OnSteer(const struct FInputActionValue& Value);
	void OnBrake(const struct FInputActionValue& Value);
	void OnHandbrake(const struct FInputActionValue& Value);
	void OnWinchToggle(const struct FInputActionValue& Value);
	void OnWinchIn(const struct FInputActionValue& Value);
	void OnWinchOut(const struct FInputActionValue& Value);
	void OnAirDown(const struct FInputActionValue& Value);
	void OnAirUp(const struct FInputActionValue& Value);
	void OnInteract(const struct FInputActionValue& Value);
	void OnReset(const struct FInputActionValue& Value);
	void OnSkip(const struct FInputActionValue& Value);

	UInputAction* MakeAxis(const FName& Name);
	UInputAction* MakeButton(const FName& Name);
	void MapKey(UInputAction* Action, const FKey& Key, bool bNegate = false);

	UPROPERTY()
	TObjectPtr<UInputMappingContext> Mapping;

	UPROPERTY()
	TObjectPtr<UInputAction> ThrottleAction;
	UPROPERTY()
	TObjectPtr<UInputAction> SteerAction;
	UPROPERTY()
	TObjectPtr<UInputAction> BrakeAction;
	UPROPERTY()
	TObjectPtr<UInputAction> HandbrakeAction;
	UPROPERTY()
	TObjectPtr<UInputAction> WinchToggleAction;
	UPROPERTY()
	TObjectPtr<UInputAction> WinchInAction;
	UPROPERTY()
	TObjectPtr<UInputAction> WinchOutAction;
	UPROPERTY()
	TObjectPtr<UInputAction> AirDownAction;
	UPROPERTY()
	TObjectPtr<UInputAction> AirUpAction;
	UPROPERTY()
	TObjectPtr<UInputAction> InteractAction;
	UPROPERTY()
	TObjectPtr<UInputAction> ResetAction;
	UPROPERTY()
	TObjectPtr<UInputAction> SkipAction;

	UPROPERTY()
	TObjectPtr<ASiltIntroDirector> IntroDirector;
};
