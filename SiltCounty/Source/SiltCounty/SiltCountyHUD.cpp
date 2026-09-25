#include "SiltCountyHUD.h"

#include "CanvasItem.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "EngineUtils.h"
#include "SiltContractActor.h"
#include "SiltCountyGameState.h"
#include "SiltTerrain.h"
#include "SiltTruckPawn.h"

void ASiltCountyHUD::DrawShadowedText(UFont* Font, const FString& Text, float X, float Y, const FLinearColor& Color, float Scale) const
{
	if (!Canvas || !Font)
	{
		return;
	}

	FCanvasTextItem Shadow(FVector2D(X + 1.5f, Y + 1.5f), FText::FromString(Text), Font, FLinearColor(0.f, 0.f, 0.f, 0.85f));
	Shadow.Scale = FVector2D(Scale, Scale);
	Canvas->DrawItem(Shadow);

	FCanvasTextItem Item(FVector2D(X, Y), FText::FromString(Text), Font, Color);
	Item.Scale = FVector2D(Scale, Scale);
	Canvas->DrawItem(Item);
}

void ASiltCountyHUD::DrawHUD()
{
	Super::DrawHUD();
	if (!Canvas || !GetWorld())
	{
		return;
	}

	UFont* Small = GEngine ? GEngine->GetSmallFont() : nullptr;
	UFont* Medium = GEngine ? GEngine->GetMediumFont() : nullptr;
	UFont* Large = GEngine ? GEngine->GetLargeFont() : nullptr;
	if (!Small || !Medium)
	{
		return;
	}

	const float Width = Canvas->SizeX;
	const float Height = Canvas->SizeY;
	const ASiltCountyGameState* GameState = GetWorld()->GetGameState<ASiltCountyGameState>();
	const ASiltTruckPawn* Truck = Cast<ASiltTruckPawn>(GetOwningPawn());

	DrawShadowedText(Medium, TEXT("SILT COUNTY"), 28.f, 24.f, FLinearColor(0.85f, 0.9f, 0.82f), 1.15f);
	if (Truck)
	{
		DrawShadowedText(Small, Truck->GetDriverName(), 28.f, 52.f, FLinearColor(0.95f, 0.85f, 0.55f));
	}

	if (GameState && !GameState->IsGameplay())
	{
		const FString Line = GameState->GetSubtitle().ToString();
		const float TextWidth = Line.Len() * 9.f;
		const float X = FMath::Max(24.f, (Width - TextWidth) * 0.5f);
		FCanvasTileItem Plate(FVector2D(X - 16.f, Height * 0.72f - 10.f), FVector2D(TextWidth + 32.f, 36.f), FLinearColor(0.f, 0.f, 0.f, 0.45f));
		Plate.BlendMode = SE_BLEND_Translucent;
		Canvas->DrawItem(Plate);
		DrawShadowedText(Medium, Line, X, Height * 0.72f, FLinearColor::White);
		DrawShadowedText(Small, TEXT("Enter  —  skip intro"), Width * 0.5f - 70.f, Height * 0.72f + 36.f, FLinearColor(0.8f, 0.8f, 0.75f));
		return;
	}

	FCanvasTileItem Panel(FVector2D(20.f, Height - 168.f), FVector2D(430.f, 140.f), FLinearColor(0.02f, 0.03f, 0.025f, 0.5f));
	Panel.BlendMode = SE_BLEND_Translucent;
	Canvas->DrawItem(Panel);

	const float Speed = Truck ? Truck->GetSpeedKmh() : 0.f;
	const ESiltSurface Surface = Truck ? Truck->GetCurrentSurface() : ESiltSurface::Dirt;
	FLinearColor SurfaceColor = FLinearColor(0.85f, 0.9f, 0.85f);
	if (Surface == ESiltSurface::DeepMud || Surface == ESiltSurface::Water)
	{
		SurfaceColor = FLinearColor(0.95f, 0.55f, 0.25f);
	}
	else if (Surface == ESiltSurface::Mud)
	{
		SurfaceColor = FLinearColor(0.85f, 0.7f, 0.35f);
	}
	else if (Surface == ESiltSurface::Asphalt)
	{
		SurfaceColor = FLinearColor(0.75f, 0.82f, 0.9f);
	}
	else if (Surface == ESiltSurface::Gravel || Surface == ESiltSurface::Road)
	{
		SurfaceColor = FLinearColor(0.82f, 0.78f, 0.68f);
	}

	DrawShadowedText(Large ? Large : Medium, FString::Printf(TEXT("%03.0f"), Speed), 36.f, Height - 156.f, FLinearColor::White, 1.2f);
	DrawShadowedText(Small, TEXT("KM/H"), 150.f, Height - 132.f, FLinearColor(0.75f, 0.78f, 0.75f));
	DrawShadowedText(Medium, SiltTerrain::SurfaceLabel(Surface), 36.f, Height - 96.f, SurfaceColor);
	if (Truck && Truck->GetSinkAlpha() > 0.35f)
	{
		DrawShadowedText(Small, TEXT("SINKING  —  keep the wheels turning"), 36.f, Height - 68.f, FLinearColor(0.95f, 0.45f, 0.2f));
	}
	DrawShadowedText(Small, TEXT("WASD drive    Space brake    Shift handbrake    R reset    RMB look"), 36.f, Height - 44.f, FLinearColor(0.7f, 0.72f, 0.68f), 0.9f);

	const ASiltContractActor* Contract = nullptr;
	for (TActorIterator<ASiltContractActor> It(GetWorld()); It; ++It)
	{
		Contract = *It;
		break;
	}

	FString ContractLine = TEXT("Contract radio is quiet.");
	FLinearColor ContractColor = FLinearColor(0.8f, 0.82f, 0.75f);
	if (Contract)
	{
		const float Meters = Truck ? FVector::Dist(Truck->GetActorLocation(), Contract->GetActorLocation()) / 100.f : 0.f;
		const float DropMeters = Truck ? FVector::Dist2D(Truck->GetActorLocation(), SiltTerrain::GetDropZone()) / 100.f : 0.f;
		switch (Contract->GetContractState())
		{
		case ESiltContractState::Available:
			if (Truck && FVector::Dist(Truck->GetActorLocation(), Contract->GetActorLocation()) <= Contract->GetHookRange())
			{
				ContractLine = TEXT("SOUTH SLOUGH PULL  —  press E to hook the tow");
				ContractColor = FLinearColor(1.f, 0.85f, 0.35f);
			}
			else
			{
				ContractLine = FString::Printf(TEXT("SOUTH SLOUGH PULL  —  reach the stranded van  (%0.0fm)"), Meters);
			}
			break;
		case ESiltContractState::Towing:
			ContractLine = FString::Printf(TEXT("TOWING  —  drag the van to the garage pad  (%0.0fm)"), DropMeters);
			ContractColor = FLinearColor(0.55f, 0.85f, 1.f);
			break;
		case ESiltContractState::Complete:
			ContractLine = TEXT("CONTRACT COMPLETE  —  you pulled them out of the slough.");
			ContractColor = FLinearColor(0.5f, 0.95f, 0.55f);
			break;
		default:
			break;
		}
	}

	DrawShadowedText(Small, ContractLine, 28.f, 78.f, ContractColor);
	if (GetWorld()->GetNumPlayerControllers() < 2)
	{
		DrawShadowedText(Small, TEXT("F9  —  local co-op, spawn Gooch"), 28.f, 100.f, FLinearColor(0.65f, 0.7f, 0.65f), 0.9f);
	}
}
