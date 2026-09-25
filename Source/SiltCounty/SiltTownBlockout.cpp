#include "SiltWorldBuilder.h"
#include "SiltTerrain.h"
#include "SiltTypes.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"

void ASiltWorldBuilder::PlaceLabel(const FName& Name, const FString& Text, const FVector& Loc, float YawDeg, float WorldSize, const FColor& Color)
{
	UTextRenderComponent* Sign = NewObject<UTextRenderComponent>(this, Name);
	Sign->SetText(FText::FromString(Text));
	Sign->SetWorldSize(WorldSize);
	Sign->SetTextRenderColor(Color);
	Sign->SetHorizontalAlignment(EHTA_Center);
	Sign->SetVerticalAlignment(EVRTA_TextCenter);
	Sign->SetCastShadow(false);
	Sign->SetWorldLocation(Loc);
	Sign->SetWorldRotation(FRotator(0.f, YawDeg, 0.f));
	Sign->SetupAttachment(GetRootComponent());
	Sign->RegisterComponent();
}

void ASiltWorldBuilder::SpawnGarageDressing()
{
	// Lock B: props only. The open east bay, pad, and key rack are already placed.
	UStaticMesh* Cube = Silt::CubeMesh();
	UStaticMesh* Cyl = Silt::CylinderMesh();
	if (!Cube || !Cyl || !Terrain)
	{
		return;
	}

	const FVector C = Terrain->GetGarageCenter();
	const float Z = Terrain->Query(C).Position.Z;
	UMaterialInterface* Steel = Silt::Material(TEXT("M_Metal"));
	UMaterialInterface* Tin = Silt::Material(TEXT("M_Corrugated"));
	UMaterialInterface* Gravel = Silt::Material(TEXT("M_Gravel"));
	UMaterialInterface* Caliche = Silt::Material(TEXT("M_Caliche"));
	UMaterialInterface* Cream = Silt::Material(TEXT("M_TrimCream"));
	UMaterialInterface* Plank = Silt::Material(TEXT("M_Plank"));
	UMaterialInterface* Glass = Silt::Material(TEXT("M_Glass"));
	UMaterialInterface* Rubber = Silt::Material(TEXT("M_Rubber"));
	UMaterialInterface* Drum = Silt::Material(TEXT("M_DrumRed"));
	UMaterialInterface* Crate = Silt::Material(TEXT("M_Crate"));
	UMaterialInterface* Board = Silt::Material(TEXT("M_SignBoard"));
	UMaterialInterface* Tube = Silt::Material(TEXT("M_ShopFluorescent"));
	UMaterialInterface* Lamp = Silt::Material(TEXT("M_Emissive"));

	PlaceMesh(Cyl, FVector(C.X + 380.f, C.Y, Z + 468.f), FRotator(0.f, 0.f, 90.f), FVector(0.32f, 0.32f, 17.2f), Tin, TEXT("DoorRoll"));
	PlaceMesh(Cube, FVector(C.X - 400.f, C.Y + 900.f, Z + 252.f), FRotator::ZeroRotator, FVector(16.f, 0.16f, 0.28f), Cream, TEXT("FasciaN"));
	PlaceMesh(Cube, FVector(C.X - 400.f, C.Y - 900.f, Z + 252.f), FRotator::ZeroRotator, FVector(16.f, 0.16f, 0.28f), Cream, TEXT("FasciaS"));
	PlaceMesh(Cube, FVector(C.X - 1180.f, C.Y, Z + 252.f), FRotator::ZeroRotator, FVector(0.16f, 18.f, 0.28f), Cream, TEXT("FasciaW"));

	const float WinZ = Z + 310.f;
	const float WinX[3] = { C.X - 860.f, C.X - 420.f, C.X + 20.f };
	for (int32 I = 0; I < 3; ++I)
	{
		PlaceMesh(Cube, FVector(WinX[I], C.Y + 868.f, WinZ), FRotator::ZeroRotator, FVector(0.85f, 0.05f, 0.42f), Glass, FName(*FString::Printf(TEXT("ShopWinN_%d"), I)));
		PlaceMesh(Cube, FVector(WinX[I], C.Y - 868.f, WinZ), FRotator::ZeroRotator, FVector(0.85f, 0.05f, 0.42f), Glass, FName(*FString::Printf(TEXT("ShopWinS_%d"), I)));
	}

	PlaceMesh(Cube, FVector(C.X - 760.f, C.Y + 760.f, Z + 92.f), FRotator::ZeroRotator, FVector(2.2f, 0.62f, 0.08f), Plank, TEXT("BenchTop"));
	PlaceMesh(Cube, FVector(C.X - 850.f, C.Y + 760.f, Z + 46.f), FRotator::ZeroRotator, FVector(0.08f, 0.5f, 0.9f), Steel, TEXT("BenchLegL"));
	PlaceMesh(Cube, FVector(C.X - 670.f, C.Y + 760.f, Z + 46.f), FRotator::ZeroRotator, FVector(0.08f, 0.5f, 0.9f), Steel, TEXT("BenchLegR"));

	PlaceMesh(Cube, FVector(C.X - 1040.f, C.Y + 640.f, Z + 55.f), FRotator::ZeroRotator, FVector(0.42f, 1.5f, 0.08f), Steel, TEXT("ShelfLow"));
	PlaceMesh(Cube, FVector(C.X - 1040.f, C.Y + 640.f, Z + 130.f), FRotator::ZeroRotator, FVector(0.42f, 1.5f, 0.08f), Steel, TEXT("ShelfHigh"));
	PlaceMesh(Cube, FVector(C.X - 1040.f, C.Y + 420.f, Z + 130.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 2.5f), Steel, TEXT("ShelfPostA"));
	PlaceMesh(Cube, FVector(C.X - 1040.f, C.Y + 860.f, Z + 130.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 2.5f), Steel, TEXT("ShelfPostB"));
	PlaceMesh(Cube, FVector(C.X - 1040.f, C.Y + 520.f, Z + 78.f), FRotator::ZeroRotator, FVector(0.38f, 0.32f, 0.28f), Crate, TEXT("PartsCrateA"));
	PlaceMesh(Cube, FVector(C.X - 1040.f, C.Y + 760.f, Z + 154.f), FRotator::ZeroRotator, FVector(0.34f, 0.3f, 0.26f), Crate, TEXT("PartsCrateB"));

	PlaceMesh(Cyl, FVector(C.X - 980.f, C.Y + 280.f, Z + 48.f), FRotator::ZeroRotator, FVector(0.56f, 0.56f, 0.9f), Drum, TEXT("DrumA"));
	PlaceMesh(Cyl, FVector(C.X - 860.f, C.Y + 360.f, Z + 48.f), FRotator::ZeroRotator, FVector(0.56f, 0.56f, 0.9f), Drum, TEXT("DrumB"));

	for (int32 I = 0; I < 3; ++I)
	{
		PlaceMesh(Cyl, FVector(C.X - 520.f, C.Y - 760.f, Z + 18.f + I * 22.f), FRotator::ZeroRotator, FVector(0.7f, 0.7f, 0.2f), Rubber, FName(*FString::Printf(TEXT("TireStack_%d"), I)));
	}

	PlaceMesh(Cube, FVector(C.X - 1148.f, C.Y + 220.f, Z + 175.f), FRotator::ZeroRotator, FVector(0.05f, 1.3f, 0.9f), Board, TEXT("ToolBoard"));

	const float TubeY[3] = { C.Y - 420.f, C.Y, C.Y + 420.f };
	for (int32 I = 0; I < 3; ++I)
	{
		PlaceMesh(Cube, FVector(C.X - 180.f, TubeY[I], Z + 258.f), FRotator::ZeroRotator, FVector(2.4f, 0.1f, 0.05f), Tube, FName(*FString::Printf(TEXT("ShopTube_%d"), I)));
	}

	PlaceMesh(Cube, FVector(C.X + 1400.f, C.Y, Z + 2.f), FRotator::ZeroRotator, FVector(16.f, 20.f, 0.06f), Caliche, TEXT("ShopCaliche"));
	PlaceMesh(Cube, FVector(C.X + 1400.f, C.Y, Z + 12.f), FRotator::ZeroRotator, FVector(10.f, 14.f, 0.1f), Gravel, TEXT("ShopApron"));
	PlaceMesh(Cyl, FVector(C.X + 1520.f, C.Y + 1180.f, Z + 260.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 5.2f), Steel, TEXT("YardPole"));
	PlaceMesh(Cube, FVector(C.X + 1520.f, C.Y + 1180.f, Z + 530.f), FRotator::ZeroRotator, FVector(0.42f, 0.28f, 0.16f), Lamp, TEXT("YardLamp"));

	const FVector Spouts[4] = {
		FVector(C.X - 1120.f, C.Y - 840.f, Z + 200.f),
		FVector(C.X - 1120.f, C.Y + 840.f, Z + 200.f),
		FVector(C.X + 320.f, C.Y - 840.f, Z + 200.f),
		FVector(C.X + 320.f, C.Y + 840.f, Z + 200.f)
	};
	for (int32 I = 0; I < 4; ++I)
	{
		PlaceMesh(Cyl, Spouts[I], FRotator::ZeroRotator, FVector(0.07f, 0.07f, 4.0f), Steel, FName(*FString::Printf(TEXT("Downspout_%d"), I)));
	}
}

void ASiltWorldBuilder::SpawnTown()
{
	UStaticMesh* Cube = Silt::CubeMesh();
	UStaticMesh* Cyl = Silt::CylinderMesh();
	UStaticMesh* Cone = Silt::ConeMesh();
	if (!Cube || !Cyl || !Terrain)
	{
		return;
	}

	const FVector Town(12000.f, 7200.f, 0.f);
	const FSiltGroundHit Hit = Terrain->Query(Town);

	UMaterialInterface* Tin = Silt::Material(TEXT("M_Corrugated"));
	UMaterialInterface* Brick = Silt::Material(TEXT("M_Brick"));
	UMaterialInterface* Stone = Silt::Material(TEXT("M_Stone"));
	UMaterialInterface* Siding = Silt::Material(TEXT("M_Siding"));
	UMaterialInterface* Plank = Silt::Material(TEXT("M_Plank"));
	UMaterialInterface* Adobe = Silt::Material(TEXT("M_Adobe"));
	UMaterialInterface* Concrete = Silt::Material(TEXT("M_Concrete"));
	UMaterialInterface* Caliche = Silt::Material(TEXT("M_Caliche"));
	UMaterialInterface* RedFront = Silt::Material(TEXT("M_FalseFrontRed"));
	UMaterialInterface* GreenFront = Silt::Material(TEXT("M_FalseFrontGreen"));
	UMaterialInterface* TankMat = Silt::Material(TEXT("M_Tank"));
	UMaterialInterface* Glass = Silt::Material(TEXT("M_Glass"));
	UMaterialInterface* Warm = Silt::Material(TEXT("M_WindowWarm"));
	UMaterialInterface* Board = Silt::Material(TEXT("M_SignBoard"));
	UMaterialInterface* Cream = Silt::Material(TEXT("M_TrimCream"));
	UMaterialInterface* Metal = Silt::Material(TEXT("M_Metal"));
	UMaterialInterface* Wood = Silt::Material(TEXT("M_Wood"));
	UMaterialInterface* Asphalt = Silt::Material(TEXT("M_Asphalt"));
	UMaterialInterface* BannerRed = Silt::Material(TEXT("M_BannerRed"));
	UMaterialInterface* BannerCream = Silt::Material(TEXT("M_BannerCream"));
	UMaterialInterface* Lamp = Silt::Material(TEXT("M_Emissive"));

	// Retained landmarks. Footprints and component names stay.
	PlaceMesh(Cube, FVector(Town.X, Town.Y, Hit.Position.Z + 140.f), FRotator::ZeroRotator, FVector(4.2f, 3.1f, 2.8f), Tin, TEXT("FeedStore"));
	PlaceMesh(Cube, FVector(Town.X + 700.f, Town.Y - 200.f, Hit.Position.Z + 110.f), FRotator(0.f, 15.f, 0.f), FVector(3.2f, 2.4f, 2.2f), Siding, TEXT("Chapel"));
	PlaceMesh(Cube, FVector(Town.X - 600.f, Town.Y + 180.f, Hit.Position.Z + 90.f), FRotator(0.f, -8.f, 0.f), FVector(2.4f, 2.8f, 1.8f), Tin, TEXT("GasShed"));
	PlaceMesh(Cube, FVector(Town.X, Town.Y, Hit.Position.Z + 292.f), FRotator::ZeroRotator, FVector(4.55f, 3.4f, 0.12f), Tin, TEXT("FeedRoof"));
	PlaceMesh(Cube, FVector(Town.X + 700.f, Town.Y - 200.f, Hit.Position.Z + 232.f), FRotator(0.f, 15.f, 0.f), FVector(3.5f, 2.7f, 0.1f), Tin, TEXT("ChapelRoof"));
	PlaceMesh(Cube, FVector(Town.X - 600.f, Town.Y + 180.f, Hit.Position.Z + 192.f), FRotator(0.f, -8.f, 0.f), FVector(2.7f, 3.1f, 0.1f), Tin, TEXT("GasRoof"));

	UTextRenderComponent* SiltSign = NewObject<UTextRenderComponent>(this, TEXT("SiltSign"));
	SiltSign->SetText(FText::FromString(TEXT("SILT")));
	SiltSign->SetWorldSize(64.f);
	SiltSign->SetTextRenderColor(FColor(200, 40, 40));
	SiltSign->SetHorizontalAlignment(EHTA_Center);
	SiltSign->SetWorldLocation(FVector(Town.X, Town.Y + 400.f, Hit.Position.Z + 260.f));
	SiltSign->SetWorldRotation(FRotator(0.f, 180.f, 0.f));
	SiltSign->SetupAttachment(GetRootComponent());
	SiltSign->RegisterComponent();

	PlaceMesh(Cube, FVector(Town.X, Town.Y - 175.f, Hit.Position.Z + 250.f), FRotator::ZeroRotator, FVector(2.4f, 0.08f, 0.5f), Board, TEXT("FeedBoard"));
	PlaceLabel(TEXT("FeedLabel"), TEXT("FEED"), FVector(Town.X, Town.Y - 185.f, Hit.Position.Z + 250.f), -90.f, 36.f, FColor(235, 220, 180));

	if (Cone)
	{
		PlaceMesh(Cyl, FVector(Town.X + 700.f, Town.Y - 200.f, Hit.Position.Z + 360.f), FRotator::ZeroRotator, FVector(0.42f, 0.42f, 2.6f), Siding, TEXT("ChapelSteeple"));
		PlaceMesh(Cone, FVector(Town.X + 700.f, Town.Y - 200.f, Hit.Position.Z + 545.f), FRotator::ZeroRotator, FVector(0.9f, 0.9f, 1.15f), Tin, TEXT("ChapelSpire"));
	}
	PlaceLabel(TEXT("ChapelLabel"), TEXT("CHAPEL"), FVector(Town.X + 700.f, Town.Y - 340.f, Hit.Position.Z + 200.f), -90.f, 28.f, FColor(245, 236, 214));

	PlaceMesh(Cube, FVector(Town.X - 600.f, Town.Y - 40.f, Hit.Position.Z + 210.f), FRotator::ZeroRotator, FVector(3.8f, 2.4f, 0.1f), Tin, TEXT("GasCanopy"));
	PlaceMesh(Cyl, FVector(Town.X - 760.f, Town.Y - 120.f, Hit.Position.Z + 110.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 2.1f), Metal, TEXT("GasPostA"));
	PlaceMesh(Cyl, FVector(Town.X - 440.f, Town.Y - 120.f, Hit.Position.Z + 110.f), FRotator::ZeroRotator, FVector(0.16f, 0.16f, 2.1f), Metal, TEXT("GasPostB"));
	PlaceLabel(TEXT("GasLabel"), TEXT("GAS"), FVector(Town.X - 600.f, Town.Y - 165.f, Hit.Position.Z + 200.f), -90.f, 32.f, FColor(230, 210, 140));

	auto GZ = [&](float X, float Y)
	{
		return Terrain->Query(FVector(X, Y, 0.f)).Position.Z;
	};
	auto Block = [&](const FName& Name, float X, float Y, float SX, float SY, float SZ, UMaterialInterface* Mat, float Lift)
	{
		const float Base = GZ(X, Y) + Lift;
		return PlaceMesh(Cube, FVector(X, Y, Base + SZ * 50.f), FRotator::ZeroRotator, FVector(SX, SY, SZ), Mat, Name);
	};

	const float StreetY = Town.Y - 2000.f;
	const float SouthY = StreetY - 1750.f;
	const float NorthY = StreetY + 1750.f;
	const float X0 = Town.X - 2100.f;
	const float X1 = Town.X + 5600.f;
	const float Seg = 1500.f;
	int32 SegN = 0;
	for (float X = X0 + Seg * 0.5f; X < X1; X += Seg - 80.f, ++SegN)
	{
		const float Z = GZ(X, StreetY);
		PlaceMesh(Cube, FVector(X, StreetY, Z + 8.f), FRotator::ZeroRotator, FVector(Seg / 100.f, 11.f, 0.14f), Asphalt, FName(*FString::Printf(TEXT("MainStreet_%d"), SegN)));
		PlaceMesh(Cube, FVector(X, StreetY + 690.f, Z + 16.f), FRotator::ZeroRotator, FVector(Seg / 100.f, 2.5f, 0.1f), Concrete, FName(*FString::Printf(TEXT("WalkN_%d"), SegN)));
		PlaceMesh(Cube, FVector(X, StreetY - 690.f, Z + 16.f), FRotator::ZeroRotator, FVector(Seg / 100.f, 2.5f, 0.1f), Concrete, FName(*FString::Printf(TEXT("WalkS_%d"), SegN)));
		if (SegN % 2 == 0)
		{
			PlaceMesh(Cyl, FVector(X, StreetY + 780.f, Z + 230.f), FRotator::ZeroRotator, FVector(0.1f, 0.1f, 4.4f), Metal, FName(*FString::Printf(TEXT("LampPoleN_%d"), SegN)));
			PlaceMesh(Cube, FVector(X, StreetY + 780.f, Z + 460.f), FRotator::ZeroRotator, FVector(0.36f, 0.22f, 0.14f), Lamp, FName(*FString::Printf(TEXT("LampHeadN_%d"), SegN)));
			PlaceMesh(Cyl, FVector(X, StreetY - 780.f, Z + 230.f), FRotator::ZeroRotator, FVector(0.1f, 0.1f, 4.4f), Metal, FName(*FString::Printf(TEXT("LampPoleS_%d"), SegN)));
			PlaceMesh(Cube, FVector(X, StreetY - 780.f, Z + 460.f), FRotator::ZeroRotator, FVector(0.36f, 0.22f, 0.14f), Lamp, FName(*FString::Printf(TEXT("LampHeadS_%d"), SegN)));
		}
	}

	auto Crosswalk = [&](float X, const TCHAR* Tag)
	{
		const float Z = GZ(X, StreetY) + 20.f;
		for (int32 I = 0; I < 6; ++I)
		{
			const float Ox = (I - 2.5f) * 90.f;
			PlaceMesh(Cube, FVector(X + Ox, StreetY, Z), FRotator::ZeroRotator, FVector(0.38f, 9.2f, 0.04f), Cream, FName(*FString::Printf(TEXT("%s_%d"), Tag, I)));
		}
	};
	Crosswalk(Town.X - 200.f, TEXT("CrossBank"));
	Crosswalk(Town.X + 4300.f, TEXT("CrossSchool"));

	const float StreetEndX = Town.X + 4900.f;
	PlaceMesh(Cube, FVector(StreetEndX, StreetY, GZ(StreetEndX, StreetY) + 8.f), FRotator::ZeroRotator, FVector(12.f, 11.f, 0.14f), Asphalt, TEXT("MainStreet_End"));
	PlaceMesh(Cube, FVector(StreetEndX, StreetY + 690.f, GZ(StreetEndX, StreetY) + 16.f), FRotator::ZeroRotator, FVector(12.f, 2.5f, 0.1f), Concrete, TEXT("WalkN_End"));
	PlaceMesh(Cube, FVector(StreetEndX, StreetY - 690.f, GZ(StreetEndX, StreetY) + 16.f), FRotator::ZeroRotator, FVector(12.f, 2.5f, 0.1f), Concrete, TEXT("WalkS_End"));

	const float WestSignX = Town.X - 2750.f;
	const float EastSignX = Town.X + 5200.f;
	const float SignY = StreetY + 1000.f;
	PlaceMesh(Cube, FVector(WestSignX, SignY, GZ(WestSignX, SignY) + 230.f), FRotator::ZeroRotator, FVector(0.1f, 3.2f, 0.7f), Board, TEXT("MainWestBoard"));
	PlaceLabel(TEXT("MainWest"), TEXT("MAIN STREET"), FVector(WestSignX - 12.f, SignY, GZ(WestSignX, SignY) + 230.f), 180.f, 42.f, FColor(240, 236, 220));
	PlaceMesh(Cube, FVector(EastSignX, SignY, GZ(EastSignX, SignY) + 230.f), FRotator::ZeroRotator, FVector(0.1f, 3.2f, 0.7f), Board, TEXT("MainEastBoard"));
	PlaceLabel(TEXT("MainEast"), TEXT("MAIN STREET"), FVector(EastSignX + 12.f, SignY, GZ(EastSignX, SignY) + 230.f), 0.f, 42.f, FColor(240, 236, 220));

	auto FaceSign = [&](const FName& Label, const FString& Text, float X, float Y, float Z, float Yaw, float WidthM, float TextSize, const FColor& Ink)
	{
		const bool bWidthOnX = FMath::Abs(FMath::Cos(FMath::DegreesToRadians(Yaw))) < 0.5f;
		const FVector Scale = bWidthOnX ? FVector(WidthM, 0.08f, 0.62f) : FVector(0.08f, WidthM, 0.62f);
		PlaceMesh(Cube, FVector(X, Y, Z), FRotator::ZeroRotator, Scale, Board, FName(*(Label.ToString() + TEXT("Board"))));
		const FVector N = FRotator(0.f, Yaw, 0.f).Vector();
		PlaceLabel(Label, Text, FVector(X, Y, Z) + N * 8.f, Yaw, TextSize, Ink);
	};
	auto WindowRow = [&](const FString& Prefix, float X, float FaceY, float Z, int32 Count, float Spacing, UMaterialInterface* Mat, float WinW, float WinH, float SkipCenterCm)
	{
		for (int32 I = 0; I < Count; ++I)
		{
			const float Ox = (I - (Count - 1) * 0.5f) * Spacing;
			if (FMath::Abs(Ox) < SkipCenterCm)
			{
				continue;
			}
			UMaterialInterface* Use = (I % 2 == 0) ? Mat : Glass;
			PlaceMesh(Cube, FVector(X + Ox, FaceY, Z), FRotator::ZeroRotator, FVector(WinW, 0.06f, WinH), Use, FName(*FString::Printf(TEXT("%s_%d"), *Prefix, I)));
		}
	};

	const float BankX = Town.X - 700.f;
	const float BankFaceY = SouthY + 620.f;
	Block(TEXT("BankFoot"), BankX, SouthY, 16.4f, 12.4f, 0.35f, Concrete, 0.f);
	Block(TEXT("CountyTrustBank"), BankX, SouthY, 16.f, 12.f, 7.2f, Stone, 35.f);
	Block(TEXT("BankCap"), BankX, SouthY, 16.5f, 12.5f, 0.4f, Stone, 35.f + 720.f);
	const float BankZ = GZ(BankX, SouthY);
	for (int32 I = 0; I < 4; ++I)
	{
		const float Ox = (I - 1.5f) * 320.f;
		PlaceMesh(Cyl, FVector(BankX + Ox, BankFaceY + 40.f, BankZ + 35.f + 320.f), FRotator::ZeroRotator, FVector(0.62f, 0.62f, 6.1f), Stone, FName(*FString::Printf(TEXT("BankColumn_%d"), I)));
	}
	PlaceMesh(Cube, FVector(BankX, BankFaceY + 20.f, BankZ + 35.f + 640.f), FRotator::ZeroRotator, FVector(13.5f, 0.7f, 0.55f), Cream, TEXT("BankLintel"));
	PlaceMesh(Cube, FVector(BankX, BankFaceY + 8.f, BankZ + 35.f + 180.f), FRotator::ZeroRotator, FVector(1.6f, 0.12f, 2.8f), Board, TEXT("BankDoor"));
	for (int32 Step = 0; Step < 3; ++Step)
	{
		const float Sy = BankFaceY + 90.f + Step * 55.f;
		const float Sz = BankZ + 12.f + Step * 14.f;
		PlaceMesh(Cube, FVector(BankX, Sy, Sz), FRotator::ZeroRotator, FVector(7.f - Step * 0.3f, 0.7f, 0.16f), Concrete, FName(*FString::Printf(TEXT("BankStep_%d"), Step)));
	}
	WindowRow(TEXT("BankWin"), BankX, BankFaceY + 8.f, BankZ + 35.f + 430.f, 5, 240.f, Warm, 0.9f, 1.5f, 110.f);
	FaceSign(TEXT("BankSign"), TEXT("COUNTY TRUST BANK"), BankX, BankFaceY + 130.f, BankZ + 35.f + 760.f, 90.f, 8.2f, 46.f, FColor(236, 224, 190));

	const float HallX = Town.X + 1800.f;
	const float HallFaceY = SouthY + 650.f;
	Block(TEXT("HallFoot"), HallX, SouthY, 16.6f, 13.2f, 0.35f, Concrete, 0.f);
	Block(TEXT("TownHall"), HallX, SouthY, 16.f, 12.6f, 7.6f, Brick, 35.f);
	Block(TEXT("HallRoof"), HallX, SouthY, 16.4f, 13.f, 0.18f, Tin, 35.f + 760.f);
	const float HallZ = GZ(HallX, SouthY);
	const FVector TurretOff[4] = {
		FVector(-760.f, -580.f, 0.f), FVector(760.f, -580.f, 0.f),
		FVector(-760.f, 580.f, 0.f), FVector(760.f, 580.f, 0.f)
	};
	for (int32 I = 0; I < 4; ++I)
	{
		const float TX = HallX + TurretOff[I].X;
		const float TY = SouthY + TurretOff[I].Y;
		const float TZ = GZ(TX, TY);
		PlaceMesh(Cyl, FVector(TX, TY, TZ + 35.f + 560.f), FRotator::ZeroRotator, FVector(2.15f, 2.15f, 11.2f), Brick, FName(*FString::Printf(TEXT("HallTurret_%d"), I)));
		if (Cone)
		{
			PlaceMesh(Cone, FVector(TX, TY, TZ + 35.f + 1120.f + 70.f), FRotator::ZeroRotator, FVector(2.7f, 2.7f, 1.5f), Tin, FName(*FString::Printf(TEXT("HallTurretCap_%d"), I)));
		}
		PlaceMesh(Cube, FVector(TX, TY + (TurretOff[I].Y > 0.f ? 110.f : -110.f), TZ + 35.f + 700.f), FRotator::ZeroRotator, FVector(0.28f, 0.06f, 0.7f), Warm, FName(*FString::Printf(TEXT("TurretSlit_%d"), I)));
	}
	PlaceMesh(Cube, FVector(HallX, HallFaceY + 8.f, HallZ + 35.f + 200.f), FRotator::ZeroRotator, FVector(1.8f, 0.12f, 3.2f), Board, TEXT("HallDoor"));
	PlaceMesh(Cube, FVector(HallX, HallFaceY + 6.f, HallZ + 35.f + 620.f), FRotator::ZeroRotator, FVector(14.f, 0.16f, 0.35f), Cream, TEXT("HallTrim"));
	WindowRow(TEXT("HallWin"), HallX, HallFaceY + 10.f, HallZ + 35.f + 460.f, 4, 260.f, Warm, 0.85f, 1.35f, 140.f);
	FaceSign(TEXT("HallSign"), TEXT("TOWN HALL"), HallX, HallFaceY + 80.f, HallZ + 35.f + 700.f, 90.f, 5.4f, 48.f, FColor(245, 236, 214));
	const float PoleX = HallX + 420.f;
	const float PoleY = HallFaceY + 220.f;
	const float PoleZ = GZ(PoleX, PoleY);
	PlaceMesh(Cyl, FVector(PoleX, PoleY, PoleZ + 280.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 5.6f), Metal, TEXT("HallFlagPole"));
	PlaceMesh(Cube, FVector(PoleX + 55.f, PoleY, PoleZ + 500.f), FRotator::ZeroRotator, FVector(1.0f, 0.04f, 0.42f), BannerRed, TEXT("HallBannerA"));
	PlaceMesh(Cube, FVector(PoleX + 55.f, PoleY, PoleZ + 458.f), FRotator::ZeroRotator, FVector(1.0f, 0.04f, 0.28f), BannerCream, TEXT("HallBannerB"));

	const float RoomsX = Town.X + 550.f;
	const float RoomsY = SouthY + 180.f;
	Block(TEXT("RoomsFoot"), RoomsX, RoomsY, 6.2f, 8.4f, 0.3f, Caliche, 0.f);
	Block(TEXT("AdobeRooms"), RoomsX, RoomsY, 5.8f, 8.f, 4.4f, Adobe, 30.f);
	Block(TEXT("RoomsRoof"), RoomsX, RoomsY, 6.3f, 8.5f, 0.22f, Adobe, 30.f + 440.f);
	const float RoomsZ = GZ(RoomsX, RoomsY);
	const float RoomsFace = RoomsY + 400.f;
	PlaceMesh(Cube, FVector(RoomsX, RoomsFace + 6.f, RoomsZ + 30.f + 140.f), FRotator::ZeroRotator, FVector(1.1f, 0.1f, 2.2f), Wood, TEXT("RoomsDoor"));
	PlaceMesh(Cube, FVector(RoomsX - 160.f, RoomsFace + 6.f, RoomsZ + 30.f + 250.f), FRotator::ZeroRotator, FVector(0.7f, 0.06f, 0.9f), Warm, TEXT("RoomsWinL"));
	PlaceMesh(Cube, FVector(RoomsX + 160.f, RoomsFace + 6.f, RoomsZ + 30.f + 250.f), FRotator::ZeroRotator, FVector(0.7f, 0.06f, 0.9f), Glass, TEXT("RoomsWinR"));
	FaceSign(TEXT("RoomsSign"), TEXT("ROOMS"), RoomsX, RoomsFace + 40.f, RoomsZ + 30.f + 390.f, 90.f, 2.6f, 36.f, FColor(245, 232, 200));

	const float SchoolX = Town.X + 4300.f;
	const float SchoolFace = SouthY + 560.f;
	Block(TEXT("SchoolFoot"), SchoolX, SouthY, 22.4f, 11.4f, 0.35f, Concrete, 0.f);
	Block(TEXT("SiltCountyHigh"), SchoolX, SouthY, 22.f, 11.f, 6.4f, Brick, 35.f);
	Block(TEXT("SchoolRoof"), SchoolX, SouthY, 22.6f, 11.6f, 0.16f, Tin, 35.f + 640.f);
	const float SchoolZ = GZ(SchoolX, SouthY);
	WindowRow(TEXT("SchoolWinLow"), SchoolX, SchoolFace + 8.f, SchoolZ + 35.f + 180.f, 8, 230.f, Warm, 0.8f, 1.05f, 140.f);
	WindowRow(TEXT("SchoolWinHigh"), SchoolX, SchoolFace + 8.f, SchoolZ + 35.f + 380.f, 8, 230.f, Glass, 0.8f, 1.05f, 0.f);
	PlaceMesh(Cube, FVector(SchoolX, SchoolFace + 8.f, SchoolZ + 35.f + 160.f), FRotator::ZeroRotator, FVector(1.8f, 0.12f, 2.6f), Board, TEXT("SchoolDoor"));
	FaceSign(TEXT("SchoolSign"), TEXT("SILT COUNTY HIGH"), SchoolX, SchoolFace + 70.f, SchoolZ + 35.f + 590.f, 90.f, 9.5f, 48.f, FColor(245, 236, 214));
	const float GymY = SouthY - 1350.f;
	Block(TEXT("GymFoot"), SchoolX, GymY, 14.4f, 9.4f, 0.3f, Concrete, 0.f);
	Block(TEXT("SchoolGym"), SchoolX, GymY, 14.f, 9.f, 5.2f, Brick, 30.f);
	Block(TEXT("GymRoof"), SchoolX, GymY, 14.5f, 9.5f, 0.16f, Tin, 30.f + 520.f);
	const float SPoleX = SchoolX - 700.f;
	const float SPoleY = SchoolFace + 240.f;
	const float SPoleZ = GZ(SPoleX, SPoleY);
	PlaceMesh(Cyl, FVector(SPoleX, SPoleY, SPoleZ + 250.f), FRotator::ZeroRotator, FVector(0.08f, 0.08f, 5.f), Metal, TEXT("SchoolFlagPole"));
	PlaceMesh(Cube, FVector(SPoleX + 50.f, SPoleY, SPoleZ + 450.f), FRotator::ZeroRotator, FVector(0.9f, 0.04f, 0.36f), BannerCream, TEXT("SchoolBannerA"));
	PlaceMesh(Cube, FVector(SPoleX + 50.f, SPoleY, SPoleZ + 414.f), FRotator::ZeroRotator, FVector(0.9f, 0.04f, 0.24f), BannerRed, TEXT("SchoolBannerB"));

	const float AlleyX = Town.X + 2850.f;
	PlaceMesh(Cube, FVector(AlleyX, SouthY, GZ(AlleyX, SouthY) + 6.f), FRotator::ZeroRotator, FVector(4.5f, 10.f, 0.08f), Caliche, TEXT("HallSchoolAlley"));

	auto FalseFront = [&](const FName& Name, float X, float FaceY, float Z, float Width, float Height, UMaterialInterface* Mat)
	{
		PlaceMesh(Cube, FVector(X, FaceY, Z + Height * 50.f), FRotator::ZeroRotator, FVector(Width, 0.28f, Height), Mat, Name);
	};
	auto Porch = [&](const FString& Prefix, float X, float FaceY, float Z, float Width, float Depth, float Toward)
	{
		const float DeckY = FaceY + Toward * (Depth * 50.f + 20.f);
		PlaceMesh(Cube, FVector(X, DeckY, Z + 18.f), FRotator::ZeroRotator, FVector(Width, Depth, 0.1f), Plank, FName(*FString::Printf(TEXT("%sDeck"), *Prefix)));
		PlaceMesh(Cube, FVector(X, DeckY, Z + 280.f), FRotator::ZeroRotator, FVector(Width + 0.3f, Depth + 0.2f, 0.1f), Tin, FName(*FString::Printf(TEXT("%sPorchRoof"), *Prefix)));
		const float HalfW = Width * 50.f - 30.f;
		const float HalfD = Depth * 50.f - 20.f;
		const FVector Posts[4] = {
			FVector(X - HalfW, DeckY - Toward * HalfD, Z + 150.f),
			FVector(X + HalfW, DeckY - Toward * HalfD, Z + 150.f),
			FVector(X - HalfW, DeckY + Toward * HalfD, Z + 150.f),
			FVector(X + HalfW, DeckY + Toward * HalfD, Z + 150.f)
		};
		for (int32 I = 0; I < 4; ++I)
		{
			PlaceMesh(Cyl, Posts[I], FRotator::ZeroRotator, FVector(0.16f, 0.16f, 2.7f), Wood, FName(*FString::Printf(TEXT("%sPost_%d"), *Prefix, I)));
		}
	};

	const float MillX = Town.X - 1900.f;
	const float MillFace = NorthY - 520.f;
	Block(TEXT("MillardFoot"), MillX, NorthY, 14.4f, 10.4f, 0.28f, Caliche, 0.f);
	Block(TEXT("Millards"), MillX, NorthY, 14.f, 10.f, 4.8f, Siding, 28.f);
	Block(TEXT("MillardRoof"), MillX, NorthY, 14.3f, 10.3f, 0.12f, Tin, 28.f + 480.f);
	const float MillZ = GZ(MillX, NorthY);
	FalseFront(TEXT("MillardFront"), MillX, MillFace - 8.f, MillZ + 28.f, 14.4f, 7.1f, RedFront);
	Porch(TEXT("Millard"), MillX, MillFace, MillZ, 14.f, 2.4f, -1.f);
	PlaceMesh(Cube, FVector(MillX, MillFace - 6.f, MillZ + 28.f + 150.f), FRotator::ZeroRotator, FVector(1.4f, 0.1f, 2.4f), Wood, TEXT("MillardDoor"));
	WindowRow(TEXT("MillardWin"), MillX, MillFace - 12.f, MillZ + 28.f + 280.f, 4, 250.f, Warm, 0.75f, 1.15f, 120.f);
	FaceSign(TEXT("MillardSign"), TEXT("MILLARD'S"), MillX, MillFace - 30.f, MillZ + 28.f + 560.f, -90.f, 6.4f, 52.f, FColor(245, 236, 214));

	const float TateX = Town.X + 2200.f;
	const float TateFace = NorthY - 500.f;
	Block(TEXT("TateFoot"), TateX, NorthY, 15.2f, 9.6f, 0.28f, Caliche, 0.f);
	Block(TEXT("TatesWestern"), TateX, NorthY, 14.8f, 9.2f, 4.6f, Siding, 28.f);
	Block(TEXT("TateRoof"), TateX, NorthY, 15.1f, 9.5f, 0.12f, Tin, 28.f + 460.f);
	const float TateZ = GZ(TateX, NorthY);
	FalseFront(TEXT("TateFront"), TateX, TateFace - 8.f, TateZ + 28.f, 15.2f, 6.8f, GreenFront);
	Porch(TEXT("Tate"), TateX, TateFace, TateZ, 14.8f, 2.3f, -1.f);
	PlaceMesh(Cube, FVector(TateX, TateFace - 6.f, TateZ + 28.f + 145.f), FRotator::ZeroRotator, FVector(1.35f, 0.1f, 2.3f), Wood, TEXT("TateDoor"));
	WindowRow(TEXT("TateWin"), TateX, TateFace - 12.f, TateZ + 28.f + 270.f, 4, 260.f, Warm, 0.7f, 1.2f, 120.f);
	FaceSign(TEXT("TateSign"), TEXT("TATE'S WESTERN"), TateX, TateFace - 30.f, TateZ + 28.f + 540.f, -90.f, 7.6f, 44.f, FColor(236, 224, 180));
	const float HitchY = TateFace - 280.f;
	PlaceMesh(Cyl, FVector(TateX - 280.f, HitchY, TateZ + 55.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 1.0f), Wood, TEXT("HitchPostA"));
	PlaceMesh(Cyl, FVector(TateX + 280.f, HitchY, TateZ + 55.f), FRotator::ZeroRotator, FVector(0.12f, 0.12f, 1.0f), Wood, TEXT("HitchPostB"));
	PlaceMesh(Cyl, FVector(TateX, HitchY, TateZ + 95.f), FRotator(90.f, 0.f, 0.f), FVector(0.08f, 0.08f, 5.6f), Wood, TEXT("HitchRail"));

	const float ElevX = Town.X - 2700.f;
	const float ElevY = SouthY - 1500.f;
	const float ElevZ = GZ(ElevX, ElevY);
	PlaceMesh(Cube, FVector(ElevX, ElevY, ElevZ + 60.f), FRotator::ZeroRotator, FVector(12.4f, 5.2f, 1.15f), Concrete, TEXT("ElevatorBase"));
	const float BinOff[3] = { -380.f, 0.f, 380.f };
	for (int32 I = 0; I < 3; ++I)
	{
		PlaceMesh(Cyl, FVector(ElevX + BinOff[I], ElevY, ElevZ + 115.f + 760.f), FRotator::ZeroRotator, FVector(3.3f, 3.3f, 15.2f), Tin, FName(*FString::Printf(TEXT("GrainBin_%d"), I)));
	}
	PlaceMesh(Cube, FVector(ElevX, ElevY, ElevZ + 115.f + 1520.f + 140.f), FRotator::ZeroRotator, FVector(12.f, 4.4f, 2.8f), Metal, TEXT("ElevatorHead"));
	PlaceMesh(Cube, FVector(ElevX, ElevY + 280.f, ElevZ + 900.f), FRotator::ZeroRotator, FVector(1.3f, 1.5f, 18.f), Tin, TEXT("ElevatorLeg"));
	FaceSign(TEXT("GrainSign"), TEXT("SILT GRAIN"), ElevX, ElevY + 360.f, ElevZ + 115.f + 1580.f, 90.f, 4.8f, 40.f, FColor(240, 236, 220));

	const float TowX = Town.X + 250.f;
	const float TowY = Town.Y + 1900.f;
	const float TowZ = GZ(TowX, TowY);
	const FVector LegOff[4] = {
		FVector(-190.f, -190.f, 0.f), FVector(190.f, -190.f, 0.f),
		FVector(-190.f, 190.f, 0.f), FVector(190.f, 190.f, 0.f)
	};
	for (int32 I = 0; I < 4; ++I)
	{
		PlaceMesh(Cyl, FVector(TowX + LegOff[I].X, TowY + LegOff[I].Y, TowZ + 600.f), FRotator::ZeroRotator, FVector(0.22f, 0.22f, 12.f), Metal, FName(*FString::Printf(TEXT("TowerLeg_%d"), I)));
	}
	PlaceMesh(Cube, FVector(TowX, TowY, TowZ + 420.f), FRotator::ZeroRotator, FVector(4.2f, 0.08f, 0.08f), Metal, TEXT("TowerBraceX"));
	PlaceMesh(Cube, FVector(TowX, TowY, TowZ + 420.f), FRotator::ZeroRotator, FVector(0.08f, 4.2f, 0.08f), Metal, TEXT("TowerBraceY"));
	PlaceMesh(Cube, FVector(TowX, TowY, TowZ + 1210.f), FRotator::ZeroRotator, FVector(5.4f, 5.4f, 0.28f), Metal, TEXT("TowerDeck"));
	PlaceMesh(Cyl, FVector(TowX, TowY, TowZ + 1210.f + 180.f), FRotator::ZeroRotator, FVector(4.5f, 4.5f, 3.3f), TankMat, TEXT("WaterTower"));
	if (Cone)
	{
		PlaceMesh(Cone, FVector(TowX, TowY, TowZ + 1210.f + 330.f + 70.f), FRotator::ZeroRotator, FVector(5.0f, 5.0f, 1.3f), Tin, TEXT("TowerHat"));
	}
	PlaceMesh(Cube, FVector(TowX, TowY - 230.f, TowZ + 600.f), FRotator::ZeroRotator, FVector(0.08f, 0.06f, 12.f), Metal, TEXT("TowerLadder"));
	FaceSign(TEXT("TowerSign"), TEXT("SILT COUNTY"), TowX, TowY - 250.f, TowZ + 1210.f + 180.f, -90.f, 3.6f, 36.f, FColor(236, 228, 200));
}
