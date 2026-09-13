#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SiltCountyHUD.generated.h"

UCLASS()
class SILTCOUNTY_API ASiltCountyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;

private:
	void DrawBar(float X, float Y, float W, float H, const FLinearColor& Color);
	void ShadowText(float X, float Y, const FString& Text, const FLinearColor& Color, float Scale = 1.f);
};
