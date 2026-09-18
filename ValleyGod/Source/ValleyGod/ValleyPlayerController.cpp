#include "ValleyPlayerController.h"
#include "ValleyGodPawn.h"
#include "ValleyWorld.h"
#include "ValleyVillager.h"
#include "Sim/ValleySim.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Engine/LocalPlayer.h"
#include "CollisionQueryParams.h"
#include "Engine/EngineTypes.h"

AValleyPlayerController::AValleyPlayerController()
{
	bShowMouseCursor = false;
	bEnableClickEvents = false;
}

void AValleyPlayerController::BeginPlay()
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
	FInputModeGameOnly Mode;
	SetInputMode(Mode);
	bShowMouseCursor = false;
}

AValleyWorld* AValleyPlayerController::Valley() const
{
	return AValleyWorld::Get(GetWorld());
}

UInputAction* AValleyPlayerController::MakeAxis(const FName& Name)
{
	UInputAction* Action = NewObject<UInputAction>(this, Name);
	Action->ValueType = EInputActionValueType::Axis1D;
	return Action;
}

UInputAction* AValleyPlayerController::MakeButton(const FName& Name)
{
	UInputAction* Action = NewObject<UInputAction>(this, Name);
	Action->ValueType = EInputActionValueType::Boolean;
	return Action;
}

void AValleyPlayerController::MapKey(UInputAction* Action, const FKey& Key, bool bNegate)
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

void AValleyPlayerController::BuildMapping()
{
	if (Mapping)
	{
		return;
	}
	Mapping = NewObject<UInputMappingContext>(this, TEXT("ValleyIMC"));
	MoveAction = MakeAxis(TEXT("Move"));
	MoveRightAction = MakeAxis(TEXT("MoveRight"));
	MoveUpAction = MakeAxis(TEXT("MoveUp"));
	LookYawAction = MakeAxis(TEXT("LookYaw"));
	LookPitchAction = MakeAxis(TEXT("LookPitch"));
	ZoomAction = MakeAxis(TEXT("Zoom"));
	SprintAction = MakeButton(TEXT("Sprint"));
	RainAction = MakeButton(TEXT("Rain"));
	TornadoAction = MakeButton(TEXT("Tornado"));
	HurricaneAction = MakeButton(TEXT("Hurricane"));
	FloodAction = MakeButton(TEXT("Flood"));
	ClearAction = MakeButton(TEXT("ClearSky"));
	PauseAction = MakeButton(TEXT("Pause"));
	DayFastAction = MakeButton(TEXT("DayFast"));
	DaySlowAction = MakeButton(TEXT("DaySlow"));
	PinAction = MakeButton(TEXT("Pin"));
	CycleAction = MakeButton(TEXT("Cycle"));
	OverviewAction = MakeButton(TEXT("Overview"));

	MapKey(MoveAction, EKeys::W);
	MapKey(MoveAction, EKeys::S, true);
	MapKey(MoveRightAction, EKeys::D);
	MapKey(MoveRightAction, EKeys::A, true);
	MapKey(MoveUpAction, EKeys::E);
	MapKey(MoveUpAction, EKeys::Q, true);
	MapKey(LookYawAction, EKeys::MouseX);
	MapKey(LookPitchAction, EKeys::MouseY);
	MapKey(ZoomAction, EKeys::MouseWheelAxis);
	MapKey(SprintAction, EKeys::LeftShift);
	MapKey(RainAction, EKeys::One);
	MapKey(TornadoAction, EKeys::Two);
	MapKey(HurricaneAction, EKeys::Three);
	MapKey(FloodAction, EKeys::Four);
	MapKey(ClearAction, EKeys::Zero);
	MapKey(PauseAction, EKeys::P);
	MapKey(DayFastAction, EKeys::RightBracket);
	MapKey(DaySlowAction, EKeys::LeftBracket);
	MapKey(PinAction, EKeys::LeftMouseButton);
	MapKey(CycleAction, EKeys::Tab);
	MapKey(OverviewAction, EKeys::F);
}

void AValleyPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	BuildMapping();
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::OnMove);
		EIC->BindAction(MoveAction, ETriggerEvent::Completed, this, &ThisClass::OnMove);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Triggered, this, &ThisClass::OnMoveRight);
		EIC->BindAction(MoveRightAction, ETriggerEvent::Completed, this, &ThisClass::OnMoveRight);
		EIC->BindAction(MoveUpAction, ETriggerEvent::Triggered, this, &ThisClass::OnMoveUp);
		EIC->BindAction(MoveUpAction, ETriggerEvent::Completed, this, &ThisClass::OnMoveUp);
		EIC->BindAction(LookYawAction, ETriggerEvent::Triggered, this, &ThisClass::OnLookYaw);
		EIC->BindAction(LookPitchAction, ETriggerEvent::Triggered, this, &ThisClass::OnLookPitch);
		EIC->BindAction(ZoomAction, ETriggerEvent::Triggered, this, &ThisClass::OnZoom);
		EIC->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ThisClass::OnSprint);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &ThisClass::OnSprint);
		EIC->BindAction(RainAction, ETriggerEvent::Started, this, &ThisClass::OnRain);
		EIC->BindAction(TornadoAction, ETriggerEvent::Started, this, &ThisClass::OnTornado);
		EIC->BindAction(HurricaneAction, ETriggerEvent::Started, this, &ThisClass::OnHurricane);
		EIC->BindAction(FloodAction, ETriggerEvent::Started, this, &ThisClass::OnFlood);
		EIC->BindAction(ClearAction, ETriggerEvent::Started, this, &ThisClass::OnClear);
		EIC->BindAction(PauseAction, ETriggerEvent::Started, this, &ThisClass::OnPause);
		EIC->BindAction(DayFastAction, ETriggerEvent::Started, this, &ThisClass::OnDayFaster);
		EIC->BindAction(DaySlowAction, ETriggerEvent::Started, this, &ThisClass::OnDaySlower);
		EIC->BindAction(PinAction, ETriggerEvent::Started, this, &ThisClass::OnPin);
		EIC->BindAction(CycleAction, ETriggerEvent::Started, this, &ThisClass::OnCycle);
		EIC->BindAction(OverviewAction, ETriggerEvent::Started, this, &ThisClass::OnOverview);
	}
}

void AValleyPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (AValleyGodPawn* Pawn = Cast<AValleyGodPawn>(GetPawn()))
	{
		Pawn->SetFlyInput(ForwardAxis, RightAxis, UpAxis, bSprint);
		if (AValleyWorld* W = Valley())
		{
			if (W->GetPinnedId() >= 0)
			{
				if (AValleyVillager* V = W->FindVillager(W->GetPinnedId()))
				{
					Pawn->FollowActor(V, DeltaTime);
				}
			}
		}
	}
}

void AValleyPlayerController::OnMove(const FInputActionValue& Value)
{
	ForwardAxis = Value.Get<float>();
}

void AValleyPlayerController::OnMoveRight(const FInputActionValue& Value)
{
	RightAxis = Value.Get<float>();
}

void AValleyPlayerController::OnMoveUp(const FInputActionValue& Value)
{
	UpAxis = Value.Get<float>();
}

void AValleyPlayerController::OnLookYaw(const FInputActionValue& Value)
{
	if (AValleyGodPawn* Pawn = Cast<AValleyGodPawn>(GetPawn()))
	{
		if (!Pawn->bFollowing)
		{
			Pawn->AddLook(Value.Get<float>() * 0.7f, 0.f);
		}
	}
}

void AValleyPlayerController::OnLookPitch(const FInputActionValue& Value)
{
	if (AValleyGodPawn* Pawn = Cast<AValleyGodPawn>(GetPawn()))
	{
		if (!Pawn->bFollowing)
		{
			Pawn->AddLook(0.f, Value.Get<float>() * 0.7f);
		}
	}
}

void AValleyPlayerController::OnZoom(const FInputActionValue& Value)
{
	if (AValleyGodPawn* Pawn = Cast<AValleyGodPawn>(GetPawn()))
	{
		Pawn->AddZoom(Value.Get<float>());
	}
}

void AValleyPlayerController::OnSprint(const FInputActionValue& Value)
{
	bSprint = Value.Get<bool>();
}

void AValleyPlayerController::OnRain()
{
	if (AValleyWorld* W = Valley())
	{
		W->CommandWeather(vg::Weather::Rain);
	}
}

void AValleyPlayerController::OnTornado()
{
	if (AValleyWorld* W = Valley())
	{
		W->CommandWeather(vg::Weather::Tornado);
	}
}

void AValleyPlayerController::OnHurricane()
{
	if (AValleyWorld* W = Valley())
	{
		W->CommandWeather(vg::Weather::Hurricane);
	}
}

void AValleyPlayerController::OnFlood()
{
	if (AValleyWorld* W = Valley())
	{
		W->CommandWeather(vg::Weather::Flood);
	}
}

void AValleyPlayerController::OnClear()
{
	if (AValleyWorld* W = Valley())
	{
		W->CommandWeather(vg::Weather::Clear);
	}
}

void AValleyPlayerController::OnPause()
{
	if (AValleyWorld* W = Valley())
	{
		W->TogglePause();
	}
}

void AValleyPlayerController::OnDayFaster()
{
	if (AValleyWorld* W = Valley())
	{
		W->AdjustDayLength(-10.f);
	}
}

void AValleyPlayerController::OnDaySlower()
{
	if (AValleyWorld* W = Valley())
	{
		W->AdjustDayLength(10.f);
	}
}

void AValleyPlayerController::OnPin()
{
	FHitResult Hit;
	FVector Start;
	FRotator Rot;
	GetPlayerViewPoint(Start, Rot);
	const FVector End = Start + Rot.Vector() * 20000.f;
	FCollisionQueryParams Params(NAME_None, true, GetPawn());
	if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
	{
		if (AValleyVillager* V = Cast<AValleyVillager>(Hit.GetActor()))
		{
			if (AValleyWorld* W = Valley())
			{
				W->PinVillager(V->GetVillagerId());
			}
			if (AValleyGodPawn* Pawn = Cast<AValleyGodPawn>(GetPawn()))
			{
				Pawn->FollowActor(V, 0.f);
			}
		}
	}
}

void AValleyPlayerController::OnCycle()
{
	if (AValleyWorld* W = Valley())
	{
		const int32 Id = W->CyclePin();
		if (AValleyGodPawn* Pawn = Cast<AValleyGodPawn>(GetPawn()))
		{
			Pawn->FollowActor(W->FindVillager(Id), 0.f);
		}
	}
}

void AValleyPlayerController::OnOverview()
{
	if (AValleyWorld* W = Valley())
	{
		W->PinVillager(-1);
	}
	if (AValleyGodPawn* Pawn = Cast<AValleyGodPawn>(GetPawn()))
	{
		Pawn->GoOverview();
	}
}
