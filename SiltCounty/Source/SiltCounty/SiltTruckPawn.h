#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SiltTypes.h"
#include "SiltTruckPawn.generated.h"

class UBoxComponent;
class UCameraComponent;
class USpotLightComponent;
class USpringArmComponent;
class UStaticMeshComponent;
class UTextRenderComponent;

UCLASS()
class SILTCOUNTY_API ASiltTruckPawn : public APawn
{
	GENERATED_BODY()

public:
	ASiltTruckPawn();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void ConfigureIdentity(int32 InIndex, const FString& InName, const FLinearColor& InPaint);
	FVector GetHitchWorld() const;
	FString GetDriverName() const { return DriverName; }
	int32 GetTruckIndex() const { return TruckIndex; }
	ESiltSurface GetCurrentSurface() const { return CurrentSurface; }
	float GetSpeedKmh() const;
	float GetSinkAlpha() const { return SinkAlpha; }

protected:
	UFUNCTION(Server, Unreliable)
	void ServerDrive(float Throttle, float Steer, float Brake, bool bHandbrake);

	UFUNCTION(Server, Reliable)
	void ServerReset();

	UFUNCTION(Server, Reliable)
	void ServerHook();

	UFUNCTION()
	void OnRep_Identity();

private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Body;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> VisualBody;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Cab;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Hood;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Bed;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Bumper;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> Stack;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> LightBar;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WheelFL;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WheelFR;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WheelRL;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WheelRR;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UTextRenderComponent> NameText;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpotLightComponent> HeadlightL;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpotLightComponent> HeadlightR;

	UPROPERTY(ReplicatedUsing = OnRep_Identity)
	int32 TruckIndex = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Identity)
	FString DriverName;

	UPROPERTY(ReplicatedUsing = OnRep_Identity)
	FLinearColor PaintColor = FLinearColor(0.4f, 0.08f, 0.05f);

	UPROPERTY(Replicated)
	float NetSteer = 0.f;

	float ThrottleInput = 0.f;
	float SteerInput = 0.f;
	float BrakeInput = 0.f;
	bool bHandbrakeInput = false;
	float CameraYawOffset = 0.f;
	float CameraPitchOffset = 0.f;
	float WheelSpin = 0.f;
	float SinkAlpha = 0.f;
	ESiltSurface CurrentSurface = ESiltSurface::Dirt;

	UStaticMeshComponent* Wheels[4] = {};

	void InputThrottle(float Value);
	void InputSteer(float Value);
	void InputBrake(float Value);
	void InputLookYaw(float Value);
	void InputLookPitch(float Value);
	void InputHandbrakePressed();
	void InputHandbrakeReleased();
	void InputReset();
	void InputHook();

	void ApplyPhysicsRole();
	void ApplyCosmetics();
	void ResetUpright();
	bool IsDrivingAllowed() const;

	UStaticMeshComponent* MakeVisual(const FName& Name, UStaticMesh* Mesh, const FVector& RelativeLocation, const FVector& Scale, const FRotator& RelativeRotation);

	struct FWheelQuery
	{
		bool bGrounded = false;
		bool bFront = false;
		FVector MountWorld = FVector::ZeroVector;
		FVector HitNormal = FVector::UpVector;
		float AxleDrop = 20.f;
		ESiltSurface Surface = ESiltSurface::Dirt;
		FVector WheelForward = FVector::ForwardVector;
		FVector WheelRight = FVector::RightVector;
		FVector PointVelocity = FVector::ZeroVector;
	};

	void QueryWheels(TArray<FWheelQuery, TInlineAllocator<4>>& OutWheels) const;
	void StepVehicle(float DeltaSeconds, const TArray<FWheelQuery, TInlineAllocator<4>>& WheelHits);
	void UpdateWheelVisuals(float DeltaSeconds, const TArray<FWheelQuery, TInlineAllocator<4>>& WheelHits);
};
