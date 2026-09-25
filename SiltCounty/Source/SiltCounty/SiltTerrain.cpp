#include "SiltTerrain.h"

namespace SiltTerrain
{
	namespace
	{
		constexpr float CountyMin = -400000.f;
		constexpr float CountyMax = 400000.f;
		constexpr float WaterLevel = 720.f;

		struct FRoadSpan
		{
			FVector2D A;
			FVector2D B;
		};

		const TArray<FRoadSpan>& RoadSpans()
		{
			static const TArray<FRoadSpan> Spans = {
				{ FVector2D(0.f, 86000.f), FVector2D(0.f, 62000.f) },
				{ FVector2D(0.f, 62000.f), FVector2D(3500.f, 30000.f) },
				{ FVector2D(3500.f, 30000.f), FVector2D(5500.f, 6000.f) },
				{ FVector2D(5500.f, 6000.f), FVector2D(-2500.f, -6000.f) },
				{ FVector2D(-2500.f, -6000.f), FVector2D(-16000.f, -14000.f) }
			};
			return Spans;
		}

		float HashSigned(int32 X, int32 Y)
		{
			uint32 H = static_cast<uint32>(X) * 374761393u + static_cast<uint32>(Y) * 668265263u;
			H = (H ^ (H >> 13)) * 1274126177u;
			const float Unit = static_cast<float>(H & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
			return Unit * 2.f - 1.f;
		}

		float Fade(float T)
		{
			return T * T * (3.f - 2.f * T);
		}

		float ValueNoise(float X, float Y)
		{
			const int32 X0 = FMath::FloorToInt(X);
			const int32 Y0 = FMath::FloorToInt(Y);
			const float Tx = Fade(X - static_cast<float>(X0));
			const float Ty = Fade(Y - static_cast<float>(Y0));
			const float V00 = HashSigned(X0, Y0);
			const float V10 = HashSigned(X0 + 1, Y0);
			const float V01 = HashSigned(X0, Y0 + 1);
			const float V11 = HashSigned(X0 + 1, Y0 + 1);
			return FMath::Lerp(FMath::Lerp(V00, V10, Tx), FMath::Lerp(V01, V11, Tx), Ty);
		}

		float Fbm(float X, float Y)
		{
			float Sum = 0.f;
			float Amp = 0.55f;
			float Freq = 1.f;
			for (int32 Octave = 0; Octave < 4; ++Octave)
			{
				Sum += Amp * ValueNoise(X * Freq, Y * Freq);
				Freq *= 2.03f;
				Amp *= 0.5f;
			}
			return Sum;
		}

		float SegmentDistance(const FVector2D& P, const FVector2D& A, const FVector2D& B)
		{
			const FVector2D AB = B - A;
			const float Denom = AB.SizeSquared();
			if (Denom < 1.f)
			{
				return FVector2D::Distance(P, A);
			}
			const float T = FMath::Clamp(FVector2D::DotProduct(P - A, AB) / Denom, 0.f, 1.f);
			return FVector2D::Distance(P, A + AB * T);
		}

		FVector WithGroundClearance(float X, float Y, float Clearance)
		{
			return FVector(X, Y, SampleHeight(X, Y) + Clearance);
		}
	}

	float GetCountyMin()
	{
		return CountyMin;
	}

	float GetCountyMax()
	{
		return CountyMax;
	}

	float GetWaterLevel()
	{
		return WaterLevel;
	}

	float DistanceToRoad(float X, float Y)
	{
		const FVector2D P(X, Y);
		float Best = TNumericLimits<float>::Max();
		for (const FRoadSpan& Span : RoadSpans())
		{
			Best = FMath::Min(Best, SegmentDistance(P, Span.A, Span.B));
		}
		return Best;
	}

	FVector2D GetContractXY()
	{
		return FVector2D(-30000.f, -24000.f);
	}

	float SampleHeight(float X, float Y)
	{
		const float Xm = X * 0.01f;
		const float Ym = Y * 0.01f;

		float Height = 900.f;
		Height += 980.f * Fbm(Xm * 0.0015f, Ym * 0.0015f);
		Height += 260.f * Fbm(Xm * 0.0065f + 4.2f, Ym * 0.0065f - 1.3f);
		Height += 55.f * Fbm(Xm * 0.02f, Ym * 0.02f);

		const float RiverCenter = 2200.f * FMath::Sin(Ym * 0.0045f) + 1400.f * FMath::Sin(Ym * 0.0017f + 1.4f);
		const float RiverDist = FMath::Abs(Xm - RiverCenter);
		const float River = 1.f - FMath::SmoothStep(18.f, 70.f, RiverDist);
		Height -= River * 560.f;

		const float TownDist = FVector2D(X, Y - 52000.f).Size();
		Height -= (1.f - FMath::SmoothStep(2500.f, 20000.f, TownDist)) * 460.f;

		const FVector2D Contract = GetContractXY();
		const float PitDist = FVector2D(X - Contract.X, Y - Contract.Y).Size();
		Height -= (1.f - FMath::SmoothStep(500.f, 6200.f, PitDist)) * 300.f;

		const float GarageDist = FVector2D(X, Y - 80000.f).Size();
		if (GarageDist < 8000.f)
		{
			const float Blend = 1.f - FMath::SmoothStep(2800.f, 8000.f, GarageDist);
			Height = FMath::Lerp(Height, 1220.f, Blend);
		}

		const float RoadDist = DistanceToRoad(X, Y);
		if (RoadDist < 2400.f)
		{
			const float Raised = FMath::Max(Height, WaterLevel + 130.f);
			const float Blend = 1.f - FMath::SmoothStep(500.f, 2400.f, RoadDist);
			Height = FMath::Lerp(Height, Raised + 35.f, Blend);
		}

		return Height;
	}

	ESiltSurface SampleSurface(float X, float Y)
	{
		const float Height = SampleHeight(X, Y);
		const float RoadDist = DistanceToRoad(X, Y);
		const float TrackNoise = FMath::Abs(ValueNoise(X * 0.0022f, Y * 0.0022f));

		// Highway crown: wet black asphalt. Shoulder band: crushed gravel.
		if (Height > WaterLevel + 50.f)
		{
			if (RoadDist < 700.f)
			{
				return ESiltSurface::Asphalt;
			}
			if (RoadDist < 1100.f)
			{
				return ESiltSurface::Gravel;
			}
			// Soft mud tracks just off the gravel shoulder (duals leave the crown).
			if (RoadDist < 1600.f && TrackNoise > 0.35f)
			{
				return ESiltSurface::Mud;
			}
		}

		if (Height < WaterLevel - 60.f)
		{
			return ESiltSurface::Water;
		}
		if (Height < WaterLevel + 50.f)
		{
			return ESiltSurface::DeepMud;
		}
		if (Height < WaterLevel + 240.f)
		{
			return ESiltSurface::Mud;
		}
		// Default upland: wet olive soil, not dry farm dirt.
		return ESiltSurface::Dirt;
	}

	const TCHAR* SurfaceLabel(ESiltSurface Surface)
	{
		switch (Surface)
		{
		case ESiltSurface::Asphalt: return TEXT("WET ASPHALT");
		case ESiltSurface::Gravel: return TEXT("WET GRAVEL");
		case ESiltSurface::Road: return TEXT("WET GRAVEL");
		case ESiltSurface::Dirt: return TEXT("WET SOIL");
		case ESiltSurface::Mud: return TEXT("MUD TRACK");
		case ESiltSurface::DeepMud: return TEXT("DEEP MUD");
		case ESiltSurface::Water: return TEXT("FLOODWATER");
		default: return TEXT("GROUND");
		}
	}

	FRotator GetTruckYaw()
	{
		return FRotator(0.f, -90.f, 0.f);
	}

	FVector GetChiefSpawn()
	{
		return WithGroundClearance(-650.f, 78500.f, 190.f);
	}

	FVector GetGoochSpawn()
	{
		return WithGroundClearance(650.f, 78500.f, 190.f);
	}

	FVector GetContractSpawn()
	{
		const FVector2D XY = GetContractXY();
		const float Height = SampleHeight(XY.X, XY.Y);
		const float Z = FMath::Max(Height, WaterLevel - 30.f) + 90.f;
		return FVector(XY.X, XY.Y, Z);
	}

	FVector GetDropZone()
	{
		return WithGroundClearance(2200.f, 74800.f, 40.f);
	}
}
