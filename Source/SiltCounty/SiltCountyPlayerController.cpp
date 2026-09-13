#include "SiltCountyPlayerController.h"
#include "SiltTruck.h"
#include "SiltIntroDirector.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Engine/LocalPlayer.h"

ASiltCountyPlayerController::ASiltCountyPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
}

void ASiltCountyPlayerController::BeginPlay()
{
	Super::BeginPlay();
	BuildMapping();
	if (UEnhancedInputLocalPlayerSubsystem* Sub = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (Mapping)
		{
			Sub->AddMappingContext(Mapping, 0);
		}
	}
}

int32 ASiltCountyPlayerController::GetLocalIndex() const
{
	if (const ULocalPlayer* LP = GetLocalPlayer())
	{
		return LP->GetLocalPlayerIndex();
	}
	return 0;
}

void ASiltCountyPlayerController::PossessTruck(ASiltTruck* Truck)
{
	if (Truck)
	{
		Possess(Truck);
		SetViewTargetWithBlend(Truck, 0.8f);
	}
}

UInputAction* ASiltCountyPlayerController::MakeAxis(const FName& Name)
{
	UInputAction* Action = NewObject<UInputAction>(this, Name);
	Action->ValueType = EInputActionValueType::Axis1D;
	return Action;
}

UInputAction* ASiltCountyPlayerController::MakeButton(const FName& Name)
{
	UInputAction* Action = NewObject<UInputAction>(this, Name);
	Action->ValueType = EInputActionValueType::Boolean;
	return Action;
}

void ASiltCountyPlayerController::MapKey(UInputAction* Action, const FKey& Key, bool bNegate)
{
	if (!Mapping || !Action)
	{
		return;
	}
	FEnhancedActionKeyMapping& Mapped = Mapping->MapKey(Action, Key);
	if (bNegate)
	{
		UInputModifierNegate* Neg = NewObject<UInputModifierNegate>(this);
		Mapped.Modifiers.Add(Neg);
	}
}

void ASiltCountyPlayerController::BuildMapping()
{
	if (Mapping)
	{
		return;
	}

	Mapping = NewObject<UInputMappingContext>(this, TEXT("SiltIMC"));
	ThrottleAction = MakeAxis(TEXT("Throttle"));
	SteerAction = MakeAxis(TEXT("Steer"));
	BrakeAction = MakeAxis(TEXT("Brake"));
	HandbrakeAction = MakeButton(TEXT("Handbrake"));
	WinchToggleAction = MakeButton(TEXT("WinchToggle"));
	WinchInAction = MakeButton(TEXT("WinchIn"));
	WinchOutAction = MakeButton(TEXT("WinchOut"));
	AirDownAction = MakeButton(TEXT("AirDown"));
	AirUpAction = MakeButton(TEXT("AirUp"));
	InteractAction = MakeButton(TEXT("Interact"));
	ResetAction = MakeButton(TEXT("Reset"));
	SkipAction = MakeButton(TEXT("Skip"));

	const bool bP1 = GetLocalIndex() == 0;
	if (bP1)
	{
		MapKey(ThrottleAction, EKeys::W);
		MapKey(ThrottleAction, EKeys::S, true);
		MapKey(SteerAction, EKeys::D);
		MapKey(SteerAction, EKeys::A, true);
		MapKey(BrakeAction, EKeys::LeftShift);
		MapKey(HandbrakeAction, EKeys::SpaceBar);
		MapKey(WinchToggleAction, EKeys::Q);
		MapKey(WinchInAction, EKeys::E);
		MapKey(WinchOutAction, EKeys::C);
		MapKey(AirDownAction, EKeys::LeftBracket);
		MapKey(AirUpAction, EKeys::RightBracket);
		MapKey(InteractAction, EKeys::F);
		MapKey(ResetAction, EKeys::R);
		MapKey(SkipAction, EKeys::Enter);
		MapKey(SkipAction, EKeys::SpaceBar);
	}
	else
	{
		MapKey(ThrottleAction, EKeys::I);
		MapKey(ThrottleAction, EKeys::K, true);
		MapKey(SteerAction, EKeys::L);
		MapKey(SteerAction, EKeys::J, true);
		MapKey(BrakeAction, EKeys::RightShift);
		MapKey(HandbrakeAction, EKeys::RightAlt);
		MapKey(WinchToggleAction, EKeys::U);
		MapKey(WinchInAction, EKeys::O);
		MapKey(WinchOutAction, EKeys::Comma);
		MapKey(AirDownAction, EKeys::P);
		MapKey(AirUpAction, EKeys::Semicolon);
		MapKey(InteractAction, EKeys::Quote);
		MapKey(ResetAction, EKeys::Slash);
		MapKey(SkipAction, EKeys::Enter);
	}

	// Gamepads bind to whichever local player owns that controller.
	MapKey(ThrottleAction, EKeys::Gamepad_RightTriggerAxis);
	MapKey(ThrottleAction, EKeys::Gamepad_LeftTriggerAxis, true);
	MapKey(SteerAction, EKeys::Gamepad_LeftX);
	MapKey(BrakeAction, EKeys::Gamepad_LeftTriggerAxis);
	MapKey(HandbrakeAction, EKeys::Gamepad_FaceButton_Right);
	MapKey(WinchToggleAction, EKeys::Gamepad_LeftShoulder);
	MapKey(WinchInAction, EKeys::Gamepad_RightShoulder);
	MapKey(WinchOutAction, EKeys::Gamepad_DPad_Down);
	MapKey(AirDownAction, EKeys::Gamepad_DPad_Left);
	MapKey(AirUpAction, EKeys::Gamepad_DPad_Right);
	MapKey(InteractAction, EKeys::Gamepad_FaceButton_Bottom);
	MapKey(ResetAction, EKeys::Gamepad_FaceButton_Left);
	MapKey(SkipAction, EKeys::Gamepad_Special_Right);
}

void ASiltCountyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	BuildMapping();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &ThisClass::OnThrottle);
		EIC->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &ThisClass::OnThrottle);
		EIC->BindAction(SteerAction, ETriggerEvent::Triggered, this, &ThisClass::OnSteer);
		EIC->BindAction(SteerAction, ETriggerEvent::Completed, this, &ThisClass::OnSteer);
		EIC->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &ThisClass::OnBrake);
		EIC->BindAction(BrakeAction, ETriggerEvent::Completed, this, &ThisClass::OnBrake);
		EIC->BindAction(HandbrakeAction, ETriggerEvent::Triggered, this, &ThisClass::OnHandbrake);
		EIC->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &ThisClass::OnHandbrake);
		EIC->BindAction(WinchToggleAction, ETriggerEvent::Started, this, &ThisClass::OnWinchToggle);
		EIC->BindAction(WinchInAction, ETriggerEvent::Triggered, this, &ThisClass::OnWinchIn);
		EIC->BindAction(WinchInAction, ETriggerEvent::Completed, this, &ThisClass::OnWinchIn);
		EIC->BindAction(WinchOutAction, ETriggerEvent::Triggered, this, &ThisClass::OnWinchOut);
		EIC->BindAction(WinchOutAction, ETriggerEvent::Completed, this, &ThisClass::OnWinchOut);
		EIC->BindAction(AirDownAction, ETriggerEvent::Triggered, this, &ThisClass::OnAirDown);
		EIC->BindAction(AirDownAction, ETriggerEvent::Completed, this, &ThisClass::OnAirDown);
		EIC->BindAction(AirUpAction, ETriggerEvent::Triggered, this, &ThisClass::OnAirUp);
		EIC->BindAction(AirUpAction, ETriggerEvent::Completed, this, &ThisClass::OnAirUp);
		EIC->BindAction(InteractAction, ETriggerEvent::Started, this, &ThisClass::OnInteract);
		EIC->BindAction(ResetAction, ETriggerEvent::Triggered, this, &ThisClass::OnReset);
		EIC->BindAction(ResetAction, ETriggerEvent::Completed, this, &ThisClass::OnReset);
		EIC->BindAction(SkipAction, ETriggerEvent::Started, this, &ThisClass::OnSkip);
	}
}

void ASiltCountyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (ASiltTruck* Truck = Cast<ASiltTruck>(GetPawn()))
	{
		Truck->SetTruckInput(CurrentInput);
	}
	// Edge-triggered buttons clear after one consume in the truck.
	CurrentInput.bWinchToggle = false;
	CurrentInput.bInteract = false;
	CurrentInput.bSkipIntro = false;
}

void ASiltCountyPlayerController::OnThrottle(const FInputActionValue& Value)
{
	CurrentInput.Throttle = Value.Get<float>();
}

void ASiltCountyPlayerController::OnSteer(const FInputActionValue& Value)
{
	CurrentInput.Steer = Value.Get<float>();
}

void ASiltCountyPlayerController::OnBrake(const FInputActionValue& Value)
{
	CurrentInput.Brake = Value.Get<float>();
}

void ASiltCountyPlayerController::OnHandbrake(const FInputActionValue& Value)
{
	CurrentInput.bHandbrake = Value.Get<bool>();
}

void ASiltCountyPlayerController::OnWinchToggle(const FInputActionValue& Value)
{
	CurrentInput.bWinchToggle = true;
}

void ASiltCountyPlayerController::OnWinchIn(const FInputActionValue& Value)
{
	CurrentInput.bWinchIn = Value.Get<bool>();
}

void ASiltCountyPlayerController::OnWinchOut(const FInputActionValue& Value)
{
	CurrentInput.bWinchOut = Value.Get<bool>();
}

void ASiltCountyPlayerController::OnAirDown(const FInputActionValue& Value)
{
	CurrentInput.bAirDown = Value.Get<bool>();
}

void ASiltCountyPlayerController::OnAirUp(const FInputActionValue& Value)
{
	CurrentInput.bAirUp = Value.Get<bool>();
}

void ASiltCountyPlayerController::OnInteract(const FInputActionValue& Value)
{
	CurrentInput.bInteract = true;
}

void ASiltCountyPlayerController::OnReset(const FInputActionValue& Value)
{
	CurrentInput.bReset = Value.Get<bool>();
}

void ASiltCountyPlayerController::OnSkip(const FInputActionValue& Value)
{
	CurrentInput.bSkipIntro = true;
	if (IntroDirector)
	{
		IntroDirector->Skip();
	}
}
