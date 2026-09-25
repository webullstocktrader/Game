#include "SiltWetMaterials.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "HAL/FileManager.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionAppendVector.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant2Vector.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionDistance.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionNoise.h"
#include "Materials/MaterialExpressionNormalize.h"
#include "Materials/MaterialExpressionOneMinus.h"
#include "Materials/MaterialExpressionPower.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionTextureCoordinate.h"
#include "Materials/MaterialExpressionTime.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogSiltWet, Log, All);

namespace SiltWetMaterials
{
	constexpr float Revision = 3.f;
	const TCHAR* RevisionKey = TEXT("SiltRevision");

	FAssetRegistryModule& AssetRegistry()
	{
		return FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	}

	void Link(FExpressionInput& Input, UMaterialExpression* Expr, int32 OutputIndex = 0)
	{
		if (Expr)
		{
			Input.Connect(OutputIndex, Expr);
		}
	}

	struct FGraph
	{
		UMaterial* Material = nullptr;
		bool bOk = true;

		explicit FGraph(UMaterial* InMaterial)
			: Material(InMaterial)
		{
			if (!Material)
			{
				bOk = false;
			}
		}

		UMaterialExpression* Add(TSubclassOf<UMaterialExpression> Class, int32 X, int32 Y)
		{
			if (!Material)
			{
				bOk = false;
				return nullptr;
			}
			UMaterialExpression* Expr = UMaterialEditingLibrary::CreateMaterialExpression(Material, Class, X, Y);
			if (!Expr)
			{
				bOk = false;
			}
			return Expr;
		}

		template <typename T>
		T* Node(int32 X, int32 Y)
		{
			return Cast<T>(Add(T::StaticClass(), X, Y));
		}

		UMaterialExpressionScalarParameter* Scalar(const TCHAR* Name, float Value, int32 X, int32 Y, float SliderMin, float SliderMax)
		{
			UMaterialExpressionScalarParameter* Param = Node<UMaterialExpressionScalarParameter>(X, Y);
			if (Param)
			{
				Param->ParameterName = Name;
				Param->DefaultValue = Value;
				Param->SliderMin = SliderMin;
				Param->SliderMax = SliderMax;
			}
			return Param;
		}

		UMaterialExpressionConstant* Const1(float Value, int32 X, int32 Y)
		{
			UMaterialExpressionConstant* Expr = Node<UMaterialExpressionConstant>(X, Y);
			if (Expr)
			{
				Expr->R = Value;
			}
			return Expr;
		}

		UMaterialExpressionConstant3Vector* Const3(const FLinearColor& Value, int32 X, int32 Y)
		{
			UMaterialExpressionConstant3Vector* Expr = Node<UMaterialExpressionConstant3Vector>(X, Y);
			if (Expr)
			{
				Expr->Constant = Value;
			}
			return Expr;
		}

		UMaterialExpressionConstant2Vector* Const2(float R, float G, int32 X, int32 Y)
		{
			UMaterialExpressionConstant2Vector* Expr = Node<UMaterialExpressionConstant2Vector>(X, Y);
			if (Expr)
			{
				Expr->R = R;
				Expr->G = G;
			}
			return Expr;
		}

		UMaterialExpressionComponentMask* Mask(UMaterialExpression* Source, bool bR, bool bG, bool bB, bool bA, int32 X, int32 Y)
		{
			UMaterialExpressionComponentMask* Expr = Node<UMaterialExpressionComponentMask>(X, Y);
			if (Expr)
			{
				Expr->R = bR ? 1 : 0;
				Expr->G = bG ? 1 : 0;
				Expr->B = bB ? 1 : 0;
				Expr->A = bA ? 1 : 0;
				Link(Expr->Input, Source);
			}
			return Expr;
		}

		UMaterialExpressionAdd* AddExpr(UMaterialExpression* A, UMaterialExpression* B, int32 X, int32 Y)
		{
			UMaterialExpressionAdd* Expr = Node<UMaterialExpressionAdd>(X, Y);
			if (Expr)
			{
				Link(Expr->A, A);
				Link(Expr->B, B);
			}
			return Expr;
		}

		UMaterialExpressionSubtract* Sub(UMaterialExpression* A, UMaterialExpression* B, int32 X, int32 Y)
		{
			UMaterialExpressionSubtract* Expr = Node<UMaterialExpressionSubtract>(X, Y);
			if (Expr)
			{
				Link(Expr->A, A);
				Link(Expr->B, B);
			}
			return Expr;
		}

		UMaterialExpressionMultiply* Mul(UMaterialExpression* A, UMaterialExpression* B, int32 X, int32 Y)
		{
			UMaterialExpressionMultiply* Expr = Node<UMaterialExpressionMultiply>(X, Y);
			if (Expr)
			{
				Link(Expr->A, A);
				Link(Expr->B, B);
			}
			return Expr;
		}

		UMaterialExpressionDivide* Div(UMaterialExpression* A, UMaterialExpression* B, int32 X, int32 Y)
		{
			UMaterialExpressionDivide* Expr = Node<UMaterialExpressionDivide>(X, Y);
			if (Expr)
			{
				Link(Expr->A, A);
				Link(Expr->B, B);
			}
			return Expr;
		}

		UMaterialExpressionLinearInterpolate* Lerp(UMaterialExpression* A, UMaterialExpression* B, UMaterialExpression* Alpha, int32 X, int32 Y)
		{
			UMaterialExpressionLinearInterpolate* Expr = Node<UMaterialExpressionLinearInterpolate>(X, Y);
			if (Expr)
			{
				Link(Expr->A, A);
				Link(Expr->B, B);
				Link(Expr->Alpha, Alpha);
			}
			return Expr;
		}

		UMaterialExpressionSaturate* Sat(UMaterialExpression* Input, int32 X, int32 Y)
		{
			UMaterialExpressionSaturate* Expr = Node<UMaterialExpressionSaturate>(X, Y);
			if (Expr)
			{
				Link(Expr->Input, Input);
			}
			return Expr;
		}

		UMaterialExpressionOneMinus* OneMinus(UMaterialExpression* Input, int32 X, int32 Y)
		{
			UMaterialExpressionOneMinus* Expr = Node<UMaterialExpressionOneMinus>(X, Y);
			if (Expr)
			{
				Link(Expr->Input, Input);
			}
			return Expr;
		}

		UMaterialExpressionPower* Pow(UMaterialExpression* Base, float Exponent, int32 X, int32 Y)
		{
			UMaterialExpressionPower* Expr = Node<UMaterialExpressionPower>(X, Y);
			if (Expr)
			{
				Link(Expr->Base, Base);
				Expr->ConstExponent = Exponent;
			}
			return Expr;
		}

		UMaterialExpressionAppendVector* Append(UMaterialExpression* A, UMaterialExpression* B, int32 X, int32 Y)
		{
			UMaterialExpressionAppendVector* Expr = Node<UMaterialExpressionAppendVector>(X, Y);
			if (Expr)
			{
				Link(Expr->A, A);
				Link(Expr->B, B);
			}
			return Expr;
		}

		UMaterialExpressionNormalize* Norm(UMaterialExpression* Input, int32 X, int32 Y)
		{
			UMaterialExpressionNormalize* Expr = Node<UMaterialExpressionNormalize>(X, Y);
			if (Expr)
			{
				Link(Expr->VectorInput, Input);
			}
			return Expr;
		}

		UMaterialExpressionNoise* NoiseAt(UMaterialExpression* Position, int32 X, int32 Y)
		{
			UMaterialExpressionNoise* Expr = Node<UMaterialExpressionNoise>(X, Y);
			if (Expr)
			{
				Expr->Scale = 1.f;
				Expr->Levels = 3;
				Expr->LevelScale = 2.f;
				Expr->OutputMin = 0.f;
				Expr->OutputMax = 1.f;
				Expr->bTurbulence = true;
				Expr->NoiseFunction = NOISEFUNCTION_GradientALU;
				Link(Expr->Position, Position);
			}
			return Expr;
		}
	};

	FString PackageNameFor(const TCHAR* AssetName)
	{
		return FString::Printf(TEXT("/Game/SiltCounty/Materials/%s"), AssetName);
	}

	FString ObjectPathFor(const TCHAR* AssetName)
	{
		return FString::Printf(TEXT("/Game/SiltCounty/Materials/%s.%s"), AssetName, AssetName);
	}

	bool SaveAsset(UObject* Asset)
	{
		if (!Asset)
		{
			return false;
		}

		if (UMaterial* Material = Cast<UMaterial>(Asset))
		{
			Material->bAutomaticallySetUsageInEditor = true;
			Material->PreEditChange(nullptr);
			Material->PostEditChange();
			UMaterialEditingLibrary::RecompileMaterial(Material);
		}
		else if (UMaterialInstanceConstant* Instance = Cast<UMaterialInstanceConstant>(Asset))
		{
			Instance->PreEditChange(nullptr);
			Instance->PostEditChange();
			UMaterialEditingLibrary::UpdateMaterialInstance(Instance);
		}
		else
		{
			Asset->PreEditChange(nullptr);
			Asset->PostEditChange();
		}

		UPackage* Package = Asset->GetPackage();
		const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);

		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		Args.SaveFlags = SAVE_NoError;
		const bool bSaved = UPackage::SavePackage(Package, Asset, *Filename, Args);
		UE_LOG(LogSiltWet, Display, TEXT("%s %s"), *Asset->GetName(), bSaved ? TEXT("saved") : TEXT("built in memory only"));
		return bSaved;
	}

	bool GraphRevisionMatches(UMaterial* Material)
	{
		UMaterialEditorOnlyData* Data = Material ? Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData()) : nullptr;
		if (!Data)
		{
			return false;
		}
		for (UMaterialExpression* Expr : Data->ExpressionCollection.Expressions)
		{
			const UMaterialExpressionScalarParameter* Param = Cast<UMaterialExpressionScalarParameter>(Expr);
			if (Param && Param->ParameterName == RevisionKey && FMath::IsNearlyEqual(Param->DefaultValue, Revision, 0.01f))
			{
				return true;
			}
		}
		return false;
	}

	UMaterial* AcquireMaterial(const TCHAR* AssetName, bool& bNeedsBuild)
	{
		bNeedsBuild = true;
		UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPathFor(AssetName));
		if (Existing && GraphRevisionMatches(Existing))
		{
			bNeedsBuild = false;
			return Existing;
		}

		if (Existing)
		{
			UMaterialEditingLibrary::DeleteAllMaterialExpressions(Existing);
			Existing->PhysMaterial = nullptr;
			return Existing;
		}

		const FString PackageName = PackageNameFor(AssetName);
		UPackage* Package = CreatePackage(*PackageName);
		UMaterial* Material = NewObject<UMaterial>(Package, FName(AssetName), RF_Public | RF_Standalone);
		if (!Material)
		{
			return nullptr;
		}
		AssetRegistry().Get().AssetCreated(Material);
		Package->MarkPackageDirty();
		return Material;
	}

	UMaterialExpression* ScaledWorldXY(FGraph& Graph, UMaterialExpression* WorldXY, float Frequency, int32 X, int32 Y)
	{
		UMaterialExpression* Freq = Graph.Const1(Frequency, X, Y);
		UMaterialExpression* Scaled = Graph.Mul(WorldXY, Freq, X + 160, Y);
		UMaterialExpression* Zero = Graph.Const1(0.f, X + 160, Y + 80);
		return Graph.Append(Scaled, Zero, X + 340, Y);
	}

	UMaterialExpression* OffsetPosition(FGraph& Graph, UMaterialExpression* Position, const FLinearColor& Offset, int32 X, int32 Y)
	{
		return Graph.AddExpr(Position, Graph.Const3(Offset, X, Y + 70), X + 180, Y);
	}

	struct FNormalSet
	{
		UMaterialExpression* Normal = nullptr;
		UMaterialExpression* Height = nullptr;
	};

	FNormalSet DerivativeNormal(FGraph& Graph, UMaterialExpression* Position, UMaterialExpression* Strength, float Epsilon, int32 X, int32 Y)
	{
		FNormalSet Result;
		Result.Height = Graph.NoiseAt(Position, X, Y);
		UMaterialExpression* HeightX = Graph.NoiseAt(OffsetPosition(Graph, Position, FLinearColor(Epsilon, 0.f, 0.f), X, Y + 120), X + 220, Y + 120);
		UMaterialExpression* HeightY = Graph.NoiseAt(OffsetPosition(Graph, Position, FLinearColor(0.f, Epsilon, 0.f), X, Y + 240), X + 220, Y + 240);
		UMaterialExpression* DeltaX = Graph.Mul(Graph.Sub(Result.Height, HeightX, X + 420, Y), Strength, X + 600, Y);
		UMaterialExpression* DeltaY = Graph.Mul(Graph.Sub(Result.Height, HeightY, X + 420, Y + 140), Strength, X + 600, Y + 140);
		UMaterialExpression* One = Graph.Const1(1.f, X + 600, Y + 260);
		UMaterialExpression* XY = Graph.Append(DeltaX, DeltaY, X + 780, Y);
		UMaterialExpression* XYZ = Graph.Append(XY, One, X + 960, Y);
		Result.Normal = Graph.Norm(XYZ, X + 1140, Y);
		return Result;
	}

	bool FinishMaterial(UMaterial* Material, const FGraph& Graph, UPhysicalMaterial* Phys)
	{
		if (!Material || !Graph.bOk)
		{
			UE_LOG(LogSiltWet, Error, TEXT("Failed to build %s"), Material ? *Material->GetName() : TEXT("material"));
			return false;
		}
		Material->PhysMaterial = Phys;
		return SaveAsset(Material);
	}

	UMaterial* BuildWetGround(UPhysicalMaterial* Phys, bool& bRebuilt)
	{
		bool bNeedsBuild = true;
		UMaterial* Material = AcquireMaterial(TEXT("M_WetGround"), bNeedsBuild);
		bRebuilt = Material && bNeedsBuild;
		if (!Material || !bNeedsBuild)
		{
			return Material;
		}

		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_ClearCoat);
		Material->TwoSided = true;
		Material->bTangentSpaceNormal = true;

		FGraph Graph(Material);
		UMaterialExpression* WorldPos = Graph.Node<UMaterialExpressionWorldPosition>(-1700, 0);
		UMaterialExpression* WorldXY = Graph.Mask(WorldPos, true, true, false, false, -1480, 0);
		UMaterialExpression* WorldZ = Graph.Mask(WorldPos, false, false, true, false, -1480, 120);
		UMaterialExpression* VertexColor = Graph.Node<UMaterialExpressionVertexColor>(-1700, 280);
		UMaterialExpression* VertexNormal = Graph.Node<UMaterialExpressionVertexNormalWS>(-1700, 460);
		UMaterialExpression* NormalZ = Graph.Mask(VertexNormal, false, false, true, false, -1480, 460);

		UMaterialExpression* PuddleAmount = Graph.Scalar(TEXT("PuddleAmount"), 0.60f, -1700, 640, 0.f, 1.f);
		UMaterialExpression* NormalStrength = Graph.Scalar(TEXT("NormalStrength"), 0.80f, -1700, 760, 0.f, 2.f);
		UMaterialExpression* WpoAmplitude = Graph.Scalar(TEXT("WpoAmplitude"), 6.f, -1700, 880, 0.f, 24.f);
		UMaterialExpression* ClearCoatBias = Graph.Scalar(TEXT("ClearCoatBias"), 0.40f, -1700, 1000, 0.f, 1.f);
		UMaterialExpression* PuddleRoughness = Graph.Scalar(TEXT("PuddleRoughness"), 0.045f, -1700, 1120, 0.02f, 0.2f);
		UMaterialExpression* WaterZ = Graph.Scalar(TEXT("WaterZ"), 720.f, -1700, 1240, 0.f, 5000.f);
		UMaterialExpression* ShoreBand = Graph.Scalar(TEXT("ShoreBand"), 900.f, -1700, 1360, 100.f, 4000.f);

		UMaterialExpression* MacroPos = ScaledWorldXY(Graph, WorldXY, 0.00028f, -1240, 0);
		UMaterialExpression* MesoPos = ScaledWorldXY(Graph, WorldXY, 0.0016f, -1240, 180);
		UMaterialExpression* MicroPos = ScaledWorldXY(Graph, WorldXY, 0.014f, -1240, 360);
		UMaterialExpression* Macro = Graph.NoiseAt(MacroPos, -820, 0);
		UMaterialExpression* Meso = Graph.NoiseAt(MesoPos, -820, 180);
		FNormalSet Micro = DerivativeNormal(Graph, MicroPos, NormalStrength, 0.12f, -820, 360);

		UMaterialExpression* FlatRaw = Graph.Sat(Graph.Mul(Graph.Sub(NormalZ, Graph.Const1(0.50f, -600, 640), -420, 640), Graph.Const1(2.f, -600, 720), -240, 640), -60, 640);
		UMaterialExpression* Patch = Graph.Mul(Graph.Mul(Graph.Sat(Graph.Pow(Meso, 2.4f, -420, 180), -240, 180), FlatRaw, -60, 180), PuddleAmount, 120, 180);
		UMaterialExpression* HeightAbove = Graph.Sub(WorldZ, WaterZ, -420, 860);
		UMaterialExpression* Shore = Graph.OneMinus(Graph.Sat(Graph.Div(HeightAbove, ShoreBand, -200, 860), 0, 860), 180, 860);

		UMaterialExpression* ShoreWeight = Graph.AddExpr(Graph.Mul(Shore, Graph.Const1(0.65f, 120, 980), 300, 980), Graph.Const1(0.35f, 300, 1100), 480, 980);
		UMaterialExpression* Film = Graph.Mul(ClearCoatBias, ShoreWeight, 660, 980);
		UMaterialExpression* ClearCoat = Graph.Sat(Graph.AddExpr(Film, Patch, 840, 180), 1020, 180);

		UMaterialExpression* Variation = Graph.Lerp(Graph.Const1(0.80f, -420, -80), Graph.Const1(1.14f, -240, -80), Macro, -60, -40);
		UMaterialExpression* BaseColor = Graph.Mul(VertexColor, Variation, 120, -40);
		UMaterialExpression* Darken = Graph.Const3(FLinearColor(0.42f, 0.38f, 0.34f), 120, 80);
		UMaterialExpression* DarkColor = Graph.Mul(VertexColor, Darken, 320, 40);
		UMaterialExpression* WetMix = Graph.Sat(Graph.AddExpr(Graph.Mul(Patch, Graph.Const1(0.90f, 320, 200), 500, 160), Graph.Mul(Film, Graph.Const1(0.22f, 320, 280), 500, 260), 680, 200), 860, 80);
		UMaterialExpression* Albedo = Graph.Sat(Graph.Lerp(BaseColor, DarkColor, WetMix, 1040, 0));

		UMaterialExpression* Grain = Graph.Lerp(Graph.Const1(0.85f, 320, 420), Graph.Const1(1.15f, 500, 420), Micro.Height, 680, 420);
		// VertexColor output 4 is authored roughness (alpha).
		UMaterialExpressionMultiply* BrokenRough = Graph.Mul(VertexColor, Grain, 860, 420);
		if (BrokenRough)
		{
			Link(BrokenRough->A, VertexColor, 4);
		}
		UMaterialExpression* Satin = Graph.Lerp(BrokenRough, Graph.Const1(0.40f, 860, 560), Graph.Mul(Film, Graph.Const1(0.70f, 680, 560), 860, 620), 1040, 480);
		UMaterialExpression* Roughness = Graph.Lerp(Satin, PuddleRoughness, Graph.Sat(Patch, 1040, 640), 1240, 480);

		UMaterialExpression* FlatNormal = Graph.Const3(FLinearColor(0.f, 0.f, 1.f), 1240, 700);
		UMaterialExpression* Normal = Graph.Norm(Graph.Lerp(Micro.Normal, FlatNormal, Graph.Sat(Patch, 1240, 860), 1420, 700), 1600, 700);

		UMaterialExpression* CoatRough = Graph.Lerp(Graph.Const1(0.20f, 1240, 980), PuddleRoughness, Graph.Sat(Patch, 1240, 1100), 1460, 980);
		UMaterialExpression* Ambient = Graph.Lerp(Graph.Const1(0.68f, 1240, 1220), Graph.Const1(1.f, 1420, 1220), Micro.Height, 1600, 1180);

		UMaterialExpression* HalfMacro = Graph.Sub(Macro, Graph.Const1(0.5f, 1460, 1340), 1640, 1340);
		UMaterialExpression* Bump = Graph.Mul(Graph.Mul(HalfMacro, WpoAmplitude, 1820, 1340), FlatRaw, 2000, 1340);
		UMaterialExpression* Zero = Graph.Const1(0.f, 1820, 1480);
		UMaterialExpression* WPO = Graph.Append(Graph.Append(Zero, Zero, 2000, 1480), Bump, 2180, 1400);

		UMaterialExpression* Specular = Graph.Const1(0.50f, 1600, 1500);
		UMaterialExpression* Metallic = Graph.Const1(0.f, 1600, 1620);

		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!EditorData)
		{
			Graph.bOk = false;
		}
		else
		{
			Link(EditorData->BaseColor, Albedo);
			Link(EditorData->Roughness, Roughness);
			Link(EditorData->Metallic, Metallic);
			Link(EditorData->Specular, Specular);
			Link(EditorData->Normal, Normal);
			Link(EditorData->ClearCoat, ClearCoat);
			Link(EditorData->ClearCoatRoughness, CoatRough);
			Link(EditorData->AmbientOcclusion, Ambient);
			Link(EditorData->WorldPositionOffset, WPO);
		}

		if (Graph.bOk)
		{
			Graph.Scalar(RevisionKey, Revision, -1700, 1500, 1.f, 20.f);
		}
		FinishMaterial(Material, Graph, Phys);
		return Graph.bOk ? Material : nullptr;
	}

	UMaterial* BuildFloodWater(UPhysicalMaterial* Phys, bool& bRebuilt)
	{
		bool bNeedsBuild = true;
		UMaterial* Material = AcquireMaterial(TEXT("M_FloodWater"), bNeedsBuild);
		bRebuilt = Material && bNeedsBuild;
		if (!Material || !bNeedsBuild)
		{
			return Material;
		}

		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);
		Material->TwoSided = true;
		Material->bTangentSpaceNormal = true;

		FGraph Graph(Material);
		UMaterialExpression* WorldXY = Graph.Mask(Graph.Node<UMaterialExpressionWorldPosition>(-1500, 0), true, true, false, false, -1280, 0);
		UMaterialExpression* Time = Graph.Node<UMaterialExpressionTime>(-1500, 160);
		UMaterialExpression* VertexColor = Graph.Node<UMaterialExpressionVertexColor>(-1500, 300);
		UMaterialExpression* Depth = Graph.Mask(VertexColor, true, false, false, false, -1280, 300);
		UMaterialExpression* Foam = Graph.Mask(VertexColor, false, true, false, false, -1280, 420);
		UMaterialExpression* RippleSpeed = Graph.Scalar(TEXT("RippleSpeed"), 28.f, -1500, 560, 0.f, 80.f);
		UMaterialExpression* FoamStrength = Graph.Scalar(TEXT("FoamStrength"), 0.55f, -1500, 680, 0.f, 1.5f);
		UMaterialExpression* WaterRoughness = Graph.Scalar(TEXT("WaterRoughness"), 0.04f, -1500, 800, 0.02f, 0.25f);
		UMaterialExpression* Strength = Graph.Const1(0.55f, -1500, 940);

		UMaterialExpression* Flow = Graph.Mul(Time, RippleSpeed, -1080, 160);
		UMaterialExpression* FlowA = Graph.Append(Flow, Graph.Mul(Flow, Graph.Const1(0.40f, -1080, 280), -900, 240), -720, 180);
		UMaterialExpression* FlowB = Graph.Append(Graph.Mul(Flow, Graph.Const1(-0.70f, -1080, 400), -900, 380), Graph.Mul(Flow, Graph.Const1(0.85f, -900, 480), -720, 460), -540, 400);
		UMaterialExpression* PosA = ScaledWorldXY(Graph, Graph.AddExpr(WorldXY, FlowA, -360, 0), 0.0045f, -160, 0);
		UMaterialExpression* PosB = ScaledWorldXY(Graph, Graph.AddExpr(WorldXY, FlowB, -360, 220), 0.009f, -160, 220);
		FNormalSet RippleA = DerivativeNormal(Graph, PosA, Strength, 0.15f, 200, 0);
		FNormalSet RippleB = DerivativeNormal(Graph, PosB, Strength, 0.15f, 200, 280);
		UMaterialExpression* Normal = Graph.Norm(Graph.AddExpr(RippleA.Normal, RippleB.Normal, 1600, 80), 1780, 80);

		UMaterialExpression* Shallow = Graph.Const3(FLinearColor(0.055f, 0.062f, 0.046f), 1600, 360);
		UMaterialExpression* Deep = Graph.Const3(FLinearColor(0.010f, 0.013f, 0.011f), 1600, 500);
		UMaterialExpression* Muddy = Graph.Lerp(Shallow, Deep, Depth, 1820, 400);
		UMaterialExpression* FoamMask = Graph.Sat(Graph.Mul(Foam, FoamStrength, 1600, 640), 1780, 640);
		UMaterialExpression* Foamed = Graph.Lerp(Muddy, Graph.Const3(FLinearColor(0.50f, 0.47f, 0.40f), 1820, 620), FoamMask, 2000, 480);
		UMaterialExpressionFresnel* Fresnel = Graph.Node<UMaterialExpressionFresnel>(1820, 780);
		if (Fresnel)
		{
			Fresnel->Exponent = 4.5f;
			Fresnel->BaseReflectFraction = 0.04f;
		}
		UMaterialExpression* Grazing = Graph.Mul(Fresnel, Graph.Const1(0.32f, 2000, 800), 2180, 760);
		UMaterialExpression* Albedo = Graph.Lerp(Foamed, Graph.Const3(FLinearColor(0.52f, 0.56f, 0.57f), 2180, 900), Grazing, 2360, 640);

		UMaterialExpression* RippleRough = Graph.Lerp(WaterRoughness, Graph.AddExpr(WaterRoughness, Graph.Const1(0.07f, 2000, 1040), 2180, 1040), RippleA.Height, 2360, 1000);
		UMaterialExpression* Roughness = Graph.Lerp(RippleRough, Graph.Const1(0.46f, 2360, 1160), FoamMask, 2540, 1040);

		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!EditorData)
		{
			Graph.bOk = false;
		}
		else
		{
			Link(EditorData->BaseColor, Albedo);
			Link(EditorData->Roughness, Roughness);
			Link(EditorData->Metallic, Graph.Const1(0.f, 2360, 1280));
			Link(EditorData->Specular, Graph.Const1(0.50f, 2360, 1400));
			Link(EditorData->Normal, Normal);
		}

		if (Graph.bOk)
		{
			Graph.Scalar(RevisionKey, Revision, -1500, 1080, 1.f, 20.f);
		}
		FinishMaterial(Material, Graph, Phys);
		return Graph.bOk ? Material : nullptr;
	}

	UMaterial* BuildPuddleDecal()
	{
		bool bNeedsBuild = true;
		UMaterial* Material = AcquireMaterial(TEXT("M_PuddleDecal"), bNeedsBuild);
		if (!Material || !bNeedsBuild)
		{
			return Material;
		}

		Material->MaterialDomain = MD_DeferredDecal;
		Material->BlendMode = BLEND_Translucent;
		Material->DecalBlendMode = DBM_DBuffer_ColorNormalRoughness;
		Material->SetShadingModel(MSM_DefaultLit);

		FGraph Graph(Material);
		UMaterialExpressionTextureCoordinate* UV = Graph.Node<UMaterialExpressionTextureCoordinate>(-700, 0);
		if (UV)
		{
			UV->CoordinateIndex = 0;
		}
		UMaterialExpression* Center = Graph.Const2(0.5f, 0.5f, -700, 140);
		UMaterialExpressionDistance* Distance = Graph.Node<UMaterialExpressionDistance>(-460, 40);
		if (Distance)
		{
			Link(Distance->A, UV);
			Link(Distance->B, Center);
		}
		UMaterialExpression* Edge = Graph.Sat(Graph.Mul(Graph.Sub(Graph.Const1(0.46f, -240, 40), Distance, -40, 40), Graph.Const1(6.25f, -40, 160), 140, 40), 320, 40);
		UMaterialExpression* WorldXY = Graph.Mask(Graph.Node<UMaterialExpressionWorldPosition>(-700, 300), true, true, false, false, -460, 300);
		UMaterialExpression* Breakup = Graph.Lerp(Graph.Const1(0.72f, -40, 300), Graph.Const1(1.f, 140, 300), Graph.NoiseAt(ScaledWorldXY(Graph, WorldXY, 0.003f, -240, 300), 140, 420), 360, 300);
		UMaterialExpression* Opacity = Graph.Mul(Graph.Mul(Edge, Breakup, 540, 80), Graph.Scalar(TEXT("PuddleOpacity"), 0.92f, 360, 180, 0.f, 1.f), 720, 80);

		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!EditorData)
		{
			Graph.bOk = false;
		}
		else
		{
			Link(EditorData->BaseColor, Graph.Const3(FLinearColor(0.012f, 0.009f, 0.007f), 720, -40));
			Link(EditorData->Roughness, Graph.Const1(0.035f, 720, 260));
			Link(EditorData->Normal, Graph.Const3(FLinearColor(0.f, 0.f, 1.f), 720, 380));
			Link(EditorData->Opacity, Opacity);
			Link(EditorData->Metallic, Graph.Const1(0.f, 720, 500));
		}

		if (Graph.bOk)
		{
			Graph.Scalar(RevisionKey, Revision, 360, 320, 1.f, 20.f);
		}
		FinishMaterial(Material, Graph, nullptr);
		return Graph.bOk ? Material : nullptr;
	}

	UPhysicalMaterial* EnsurePhys(const TCHAR* AssetName, float Friction, float StaticFriction, float Restitution, float Density)
	{
		UPhysicalMaterial* Phys = LoadObject<UPhysicalMaterial>(nullptr, *ObjectPathFor(AssetName));
		const bool bNew = Phys == nullptr;
		if (!Phys)
		{
			UPackage* Package = CreatePackage(*PackageNameFor(AssetName));
			Phys = NewObject<UPhysicalMaterial>(Package, FName(AssetName), RF_Public | RF_Standalone);
			if (!Phys)
			{
				return nullptr;
			}
			AssetRegistry().Get().AssetCreated(Phys);
		}

		const bool bChanged = bNew
			|| !FMath::IsNearlyEqual(Phys->Friction, Friction)
			|| !FMath::IsNearlyEqual(Phys->StaticFriction, StaticFriction)
			|| !FMath::IsNearlyEqual(Phys->Restitution, Restitution)
			|| !FMath::IsNearlyEqual(Phys->Density, Density)
			|| Phys->RestitutionCombineMode != ERestitutionCombineMode::Min;
		Phys->Friction = Friction;
		Phys->StaticFriction = StaticFriction;
		Phys->Restitution = Restitution;
		Phys->Density = Density;
		Phys->FrictionCombineMode = EFrictionCombineMode::Average;
		Phys->RestitutionCombineMode = ERestitutionCombineMode::Min;
		if (bChanged)
		{
			SaveAsset(Phys);
		}
		return Phys;
	}

	bool ScalarIs(const UMaterialInstance* Instance, const TCHAR* Name, float Expected)
	{
		float Value = 0.f;
		return Instance
			&& Instance->GetScalarParameterValue(FHashedMaterialParameterInfo(Name), Value)
			&& FMath::IsNearlyEqual(Value, Expected, 0.001f);
	}

	void EnsureInstance(const TCHAR* AssetName, UMaterial* Parent, UPhysicalMaterial* Phys, float PuddleAmount, float NormalStrength, float WpoAmplitude, float ClearCoatBias, bool bForce)
	{
		if (!Parent)
		{
			return;
		}

		UMaterialInstanceConstant* Instance = LoadObject<UMaterialInstanceConstant>(nullptr, *ObjectPathFor(AssetName));
		if (!bForce
			&& Instance
			&& Instance->Parent == Parent
			&& Instance->PhysMaterial == Phys
			&& ScalarIs(Instance, TEXT("PuddleAmount"), PuddleAmount)
			&& ScalarIs(Instance, TEXT("NormalStrength"), NormalStrength)
			&& ScalarIs(Instance, TEXT("WpoAmplitude"), WpoAmplitude)
			&& ScalarIs(Instance, TEXT("ClearCoatBias"), ClearCoatBias))
		{
			return;
		}

		const bool bNew = Instance == nullptr;
		if (!Instance)
		{
			UPackage* Package = CreatePackage(*PackageNameFor(AssetName));
			Instance = NewObject<UMaterialInstanceConstant>(Package, FName(AssetName), RF_Public | RF_Standalone);
			if (!Instance)
			{
				UE_LOG(LogSiltWet, Error, TEXT("Failed to create %s"), AssetName);
				return;
			}
			AssetRegistry().Get().AssetCreated(Instance);
		}

		UMaterialEditingLibrary::SetMaterialInstanceParent(Instance, Parent);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(Instance, TEXT("PuddleAmount"), PuddleAmount);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(Instance, TEXT("NormalStrength"), NormalStrength);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(Instance, TEXT("WpoAmplitude"), WpoAmplitude);
		UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(Instance, TEXT("ClearCoatBias"), ClearCoatBias);
		Instance->PhysMaterial = Phys;
		SaveAsset(Instance);
		if (bNew)
		{
			UE_LOG(LogSiltWet, Display, TEXT("Created surface instance %s"), AssetName);
		}
	}

	void Ensure()
	{
		// Friction stays at or above the engine default (0.7) on surfaces the chassis can rest on.
		// Wheel grip is the terrain query in SiltTruckPawn, not these assets. Density is g/cm^3.
		// Restitution is 0 so flood mud does not bounce the hull.
		UPhysicalMaterial* Road = EnsurePhys(TEXT("PM_WetRoad"), 0.82f, 0.90f, 0.02f, 2.30f);
		UPhysicalMaterial* Soil = EnsurePhys(TEXT("PM_WetSoil"), 0.74f, 0.80f, 0.01f, 1.55f);
		UPhysicalMaterial* Mud = EnsurePhys(TEXT("PM_Mud"), 0.70f, 0.74f, 0.00f, 1.75f);
		UPhysicalMaterial* Deep = EnsurePhys(TEXT("PM_DeepMud"), 0.70f, 0.72f, 0.00f, 1.90f);
		UPhysicalMaterial* Water = EnsurePhys(TEXT("PM_StandingWater"), 0.70f, 0.70f, 0.00f, 1.00f);

		bool bGroundRebuilt = false;
		UMaterial* Ground = BuildWetGround(Mud, bGroundRebuilt);
		bool bWaterRebuilt = false;
		BuildFloodWater(Water, bWaterRebuilt);
		BuildPuddleDecal();

		// WPO stays 0 on every section. A per-section offset would open a crack along surface borders.
		EnsureInstance(TEXT("MI_WetRoad"), Ground, Road, 0.50f, 0.22f, 0.f, 0.42f, bGroundRebuilt);
		EnsureInstance(TEXT("MI_WetSoil"), Ground, Soil, 0.28f, 0.70f, 0.f, 0.22f, bGroundRebuilt);
		EnsureInstance(TEXT("MI_Mud"), Ground, Mud, 0.84f, 1.05f, 0.f, 0.64f, bGroundRebuilt);
		EnsureInstance(TEXT("MI_DeepMud"), Ground, Deep, 1.00f, 0.80f, 0.f, 0.90f, bGroundRebuilt);
		EnsureInstance(TEXT("MI_SiltBed"), Ground, Deep, 0.20f, 0.35f, 0.f, 0.12f, bGroundRebuilt);
		(void)bWaterRebuilt;

		UE_LOG(LogSiltWet, Display, TEXT("Silt wet-ground, floodwater, puddle, and physical materials are revision %.0f"), Revision);
	}
}
