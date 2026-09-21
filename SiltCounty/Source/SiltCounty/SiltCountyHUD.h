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
	void DrawShadowedText(UFont* Font, const FString& Text, float X, float Y, const FLinearColor& Color, float Scale = 1.f) const;
};
