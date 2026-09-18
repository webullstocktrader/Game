#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ValleyHUD.generated.h"

UCLASS()
class VALLEYGOD_API AValleyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawBar(float X, float Y, float W, float H, const FLinearColor& Color);
	void ShadowText(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale = 1.f);
	FString Clock(float Hours) const;
	FString Countdown(float Seconds) const;
};
