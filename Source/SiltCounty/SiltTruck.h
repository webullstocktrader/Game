#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SiltTypes.h"
#include "SiltTruck.generated.h"

class ASiltTerrain;
class USpringArmComponent;
class UCameraComponent;
class UBoxComponent;
class UAudioComponent;
class USoundWaveProcedural;
class UTextRenderComponent;

USTRUCT()
struct FSiltWheel
{
	GENERATED_BODY()

	FVector LocalOffset = FVector::ZeroVector;
	float RadiusCm = 42.f;
	float RestLengthCm = 38.f;
	float Spring = 52000.f;
	float Damper = 4200.f;
	float Compression = 0.f;
	float LastCompression = 0.f;
	float AngularVel = 0.f;
	float SteerAngle = 0.f;
	float Slip = 0.f;
	bool bGrounded = false;
	bool bSteered = false;
	FVector ContactPos = FVector::ZeroVector;
	FVector ContactNormal = FVector::UpVector;
	ESiltSurface Surface = ESiltSurface::Mud;
	float SinkCm = 0.f;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Tire = nullptr;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> Rim = nullptr;
};

UCLASS()
class SILTCOUNTY_API ASiltTruck : public APawn
{
	GENERATED_BODY()

public:
	ASiltTruck();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	void Configure(ESiltDriver InDriver, ASiltTerrain* InTerrain);
	void SetTruckInput(const FSiltTruckInput& InInput) { Input = InInput; }
	void SetPartner(ASiltTruck* Other) { Partner = Other; }

	ESiltDriver GetDriver() const { return Driver; }
	ESiltGear GetGear() const { return Gear; }
	float GetPSI() const { return PSI; }
	float GetSpeedMPH() const;
	float GetRPM() const { return RPM; }
	bool IsWinchConnected() const { return WinchTarget != nullptr; }
	FString GetWinchLabel() const;
	FVector GetFrontHook() const;
	FVector GetRearHook() const;
	UFUNCTION(BlueprintPure)
	FString GetHudName() const { return Silt::DriverName(Driver); }

	void TryToggleWinch();
	void TryInteract();
	void AttachCrate(AActor* Crate);
	bool IsCarryingCrate() const { return CarriedCrate != nullptr; }
	AActor* GetCarriedCrate() const { return CarriedCrate; }

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> Chassis;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAudioComponent> EngineAudio;

protected:
	void BuildVisuals();
	UStaticMeshComponent* AddPart(const FName& Name, UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot, const FVector& Scale, UMaterialInterface* Mat);
	void Simulate(float Dt);
	void IntegrateWheels(float Dt, FVector& OutForce, FVector& OutTorque);
	void SimulateEngine(float Dt);
	void SimulateWinch(float Dt, FVector& OutForce);
	void UpdateVisuals(float Dt);
	void UpdateAudio(float Dt);
	void FindWinchTarget();
	void ResetUpright();
	void OnEngineAudioNeeded(USoundWaveProcedural* Wave, int32 SamplesRequired);

	UPROPERTY()
	ESiltDriver Driver = ESiltDriver::Chief;

	UPROPERTY()
	TObjectPtr<ASiltTerrain> Terrain = nullptr;

	UPROPERTY()
	TObjectPtr<ASiltTruck> Partner = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> WinchTarget = nullptr;

	UPROPERTY()
	TObjectPtr<AActor> CarriedCrate = nullptr;

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> CableMesh = nullptr;

	UPROPERTY()
	TObjectPtr<USoundWaveProcedural> EngineWave = nullptr;

	UPROPERTY()
	TObjectPtr<UTextRenderComponent> DoorSign = nullptr;

	FSiltTruckInput Input;
	TArray<FSiltWheel> Wheels;

	FVector Velocity = FVector::ZeroVector;
	FVector AngularVel = FVector::ZeroVector;

	float MassKg = 2650.f;
	float PSI = 28.f;
	float RPM = 800.f;
	ESiltGear Gear = ESiltGear::Drive;
	float SteerAngle = 0.f;
	float WinchLength = 0.f;
	bool bWinchWanted = false;
	float ResetHold = 0.f;
	float AudioPhase = 0.f;
	float BodySway = 0.f;
};
