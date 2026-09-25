#include "SiltCountyEditor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Containers/Ticker.h"
#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionActorPositionWS.h"
#include "Materials/MaterialExpressionAdd.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionDivide.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionFrac.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionNoise.h"
#include "Materials/MaterialExpressionSaturate.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionSubtract.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexColor.h"
#include "Materials/MaterialExpressionWorldPosition.h"
#include "MaterialEditingLibrary.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogSiltEditor, Log, All);

namespace SiltMaterialBootstrap
{
	UMaterialExpression* AddExpr(UMaterial* Material, TSubclassOf<UMaterialExpression> Class, int32 X, int32 Y)
	{
		return UMaterialEditingLibrary::CreateMaterialExpression(Material, Class, X, Y);
	}

	UMaterial* CreatePackageMaterial(const TCHAR* Name)
	{
		const FString PackageName = FString::Printf(TEXT("/Game/SiltCounty/Materials/%s"), Name);
		if (FPackageName::DoesPackageExist(PackageName))
		{
			return nullptr;
		}

		UPackage* Package = CreatePackage(*PackageName);
		UMaterial* Material = NewObject<UMaterial>(Package, FName(Name), RF_Public | RF_Standalone);
		if (!Material)
		{
			return nullptr;
		}
		FAssetRegistryModule& AssetRegistry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		AssetRegistry.Get().AssetCreated(Material);
		Package->MarkPackageDirty();
		return Material;
	}

	bool SaveMaterial(UMaterial* Material)
	{
		if (!Material)
		{
			return false;
		}
		Material->bAutomaticallySetUsageInEditor = true;
		Material->PreEditChange(nullptr);
		Material->PostEditChange();
		UMaterialEditingLibrary::RecompileMaterial(Material);

		UPackage* Package = Material->GetOutermost();
		const FString Filename = FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension());
		IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);

		FSavePackageArgs Args;
		Args.TopLevelFlags = RF_Public | RF_Standalone;
		Args.SaveFlags = SAVE_NoError;
		const bool bSaved = UPackage::SavePackage(Package, Material, *Filename, Args);
		UE_LOG(LogSiltEditor, Display, TEXT("Silt material %s %s"), *Material->GetName(), bSaved ? TEXT("saved") : TEXT("created in memory only"));
		return bSaved;
	}

	void BuildWetGround()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_WetGround"));
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);
		Material->TwoSided = true;

		UMaterialExpressionVertexColor* VertexColor = Cast<UMaterialExpressionVertexColor>(AddExpr(Material, UMaterialExpressionVertexColor::StaticClass(), -500, 0));
		UMaterialExpressionConstant* DarkAmount = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -500, 180));
		UMaterialExpressionMultiply* Dark = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -260, 80));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -500, 320));
		UMaterialExpressionConstant* FresnelScale = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -500, 460));
		UMaterialExpressionMultiply* FresnelMask = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -240, 360));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 0, 40));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 0, 220));
		UMaterialExpressionConstant* Metallic = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 0, 340));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!VertexColor || !DarkAmount || !Dark || !Fresnel || !FresnelScale || !FresnelMask || !Albedo || !Specular || !Metallic || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_WetGround graph"));
			return;
		}

		DarkAmount->R = 0.4f;
		Fresnel->Exponent = 4.5f;
		FresnelScale->R = 0.45f;
		Specular->R = 0.55f;
		Metallic->R = 0.f;
		Dark->A.Connect(0, VertexColor);
		Dark->B.Connect(0, DarkAmount);
		FresnelMask->A.Connect(0, Fresnel);
		FresnelMask->B.Connect(0, FresnelScale);
		Albedo->A.Connect(0, VertexColor);
		Albedo->B.Connect(0, Dark);
		Albedo->Alpha.Connect(0, FresnelMask);
		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(4, VertexColor);
		EditorData->Specular.Connect(0, Specular);
		EditorData->Metallic.Connect(0, Metallic);
		SaveMaterial(Material);
	}

	void BuildFloodWater()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_FloodWater"));
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);
		Material->TwoSided = false;

		UMaterialExpressionConstant3Vector* Deep = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -420, 0));
		UMaterialExpressionConstant3Vector* Pale = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -420, 160));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -420, 320));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), -80, 80));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -80, 240));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -80, 360));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Deep || !Pale || !Fresnel || !Albedo || !Roughness || !Specular || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_FloodWater graph"));
			return;
		}

		Deep->Constant = FLinearColor(0.025f, 0.055f, 0.05f);
		Pale->Constant = FLinearColor(0.42f, 0.48f, 0.46f);
		Fresnel->Exponent = 5.f;
		Roughness->R = 0.04f;
		Specular->R = 0.7f;
		Albedo->A.Connect(0, Deep);
		Albedo->B.Connect(0, Pale);
		Albedo->Alpha.Connect(0, Fresnel);
		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, Roughness);
		EditorData->Specular.Connect(0, Specular);
		SaveMaterial(Material);
	}

	void SetUnitRange(UMaterialExpressionScalarParameter* Param)
	{
		if (!Param)
		{
			return;
		}
		Param->SliderMin = 0.f;
		Param->SliderMax = 1.f;
	}

	bool IsPassAClamped(const UMaterial* Material)
	{
		if (!Material)
		{
			return false;
		}
		for (UMaterialExpression* Expr : Material->GetExpressions())
		{
			const UMaterialExpressionScalarParameter* Scalar = Cast<UMaterialExpressionScalarParameter>(Expr);
			if (Scalar && Scalar->ParameterName == TEXT("WetAmount"))
			{
				return Scalar->SliderMax > 0.5f;
			}
		}
		return false;
	}

	bool HasUnitScalar(const UMaterial* Material, FName Name)
	{
		if (!Material)
		{
			return false;
		}
		for (UMaterialExpression* Expr : Material->GetExpressions())
		{
			const UMaterialExpressionScalarParameter* Scalar = Cast<UMaterialExpressionScalarParameter>(Expr);
			if (Scalar && Scalar->ParameterName == Name)
			{
				return Scalar->SliderMax > 0.5f;
			}
		}
		return false;
	}

	UMaterial* AcquireTruckPaint()
	{
		const TCHAR* ObjectPath = TEXT("/Game/SiltCounty/Materials/M_TruckPaint.M_TruckPaint");
		if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, ObjectPath))
		{
			if (IsPassAClamped(Existing))
			{
				UE_LOG(LogSiltEditor, Display, TEXT("Silt truck paint pass A ready"));
				return nullptr;
			}
			UMaterialEditingLibrary::DeleteAllMaterialExpressions(Existing);
			return Existing;
		}
		return CreatePackageMaterial(TEXT("M_TruckPaint"));
	}

	void BuildTruckPaint()
	{
		UMaterial* Material = AcquireTruckPaint();
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);

		UMaterialExpressionVectorParameter* Paint = Cast<UMaterialExpressionVectorParameter>(AddExpr(Material, UMaterialExpressionVectorParameter::StaticClass(), -980, 0));
		UMaterialExpressionVectorParameter* DirtColor = Cast<UMaterialExpressionVectorParameter>(AddExpr(Material, UMaterialExpressionVectorParameter::StaticClass(), -980, 180));
		UMaterialExpressionConstant* Shade = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -980, 360));
		UMaterialExpressionMultiply* Dark = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -720, 40));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -980, 500));
		UMaterialExpressionConstant* FresnelScale = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -980, 660));
		UMaterialExpressionMultiply* FresnelMask = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -700, 520));
		UMaterialExpressionLinearInterpolate* WetAlbedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), -460, 40));

		UMaterialExpressionWorldPosition* WorldPos = Cast<UMaterialExpressionWorldPosition>(AddExpr(Material, UMaterialExpressionWorldPosition::StaticClass(), -980, 860));
		UMaterialExpressionActorPositionWS* ActorPos = Cast<UMaterialExpressionActorPositionWS>(AddExpr(Material, UMaterialExpressionActorPositionWS::StaticClass(), -980, 1020));
		UMaterialExpressionSubtract* LocalPos = Cast<UMaterialExpressionSubtract>(AddExpr(Material, UMaterialExpressionSubtract::StaticClass(), -740, 900));
		UMaterialExpressionComponentMask* LocalZ = Cast<UMaterialExpressionComponentMask>(AddExpr(Material, UMaterialExpressionComponentMask::StaticClass(), -540, 900));
		UMaterialExpressionConstant* HeightLift = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -540, 1060));
		UMaterialExpressionAdd* LiftedZ = Cast<UMaterialExpressionAdd>(AddExpr(Material, UMaterialExpressionAdd::StaticClass(), -360, 940));
		UMaterialExpressionConstant* HeightSpan = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -360, 1100));
		UMaterialExpressionDivide* HeightDiv = Cast<UMaterialExpressionDivide>(AddExpr(Material, UMaterialExpressionDivide::StaticClass(), -180, 960));
		UMaterialExpressionSaturate* Height01 = Cast<UMaterialExpressionSaturate>(AddExpr(Material, UMaterialExpressionSaturate::StaticClass(), 0, 960));
		UMaterialExpressionScalarParameter* Coverage = Cast<UMaterialExpressionScalarParameter>(AddExpr(Material, UMaterialExpressionScalarParameter::StaticClass(), -180, 1160));
		UMaterialExpressionSubtract* CoverageGap = Cast<UMaterialExpressionSubtract>(AddExpr(Material, UMaterialExpressionSubtract::StaticClass(), 180, 1040));
		UMaterialExpressionConstant* CoverageBand = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 180, 1200));
		UMaterialExpressionDivide* CoverageDiv = Cast<UMaterialExpressionDivide>(AddExpr(Material, UMaterialExpressionDivide::StaticClass(), 360, 1080));
		UMaterialExpressionSaturate* Vertical = Cast<UMaterialExpressionSaturate>(AddExpr(Material, UMaterialExpressionSaturate::StaticClass(), 540, 1080));
		UMaterialExpressionNoise* Breakup = Cast<UMaterialExpressionNoise>(AddExpr(Material, UMaterialExpressionNoise::StaticClass(), 360, 1280));
		UMaterialExpressionConstant* BreakupMin = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 360, 1460));
		UMaterialExpressionConstant* BreakupMax = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 360, 1580));
		UMaterialExpressionLinearInterpolate* BreakupScale = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 560, 1360));
		UMaterialExpressionMultiply* BrokenVertical = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), 740, 1160));
		UMaterialExpressionScalarParameter* DirtAmount = Cast<UMaterialExpressionScalarParameter>(AddExpr(Material, UMaterialExpressionScalarParameter::StaticClass(), 740, 1360));
		UMaterialExpressionMultiply* DirtRaw = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), 920, 1220));
		UMaterialExpressionSaturate* SatWet = Cast<UMaterialExpressionSaturate>(AddExpr(Material, UMaterialExpressionSaturate::StaticClass(), 900, 340));
		UMaterialExpressionSaturate* SatDirt = Cast<UMaterialExpressionSaturate>(AddExpr(Material, UMaterialExpressionSaturate::StaticClass(), 900, 1360));
		UMaterialExpressionSaturate* SatCoverage = Cast<UMaterialExpressionSaturate>(AddExpr(Material, UMaterialExpressionSaturate::StaticClass(), 40, 1160));
		UMaterialExpressionSaturate* DirtMask = Cast<UMaterialExpressionSaturate>(AddExpr(Material, UMaterialExpressionSaturate::StaticClass(), 1100, 1220));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 1280, 80));

		UMaterialExpressionScalarParameter* Roughness = Cast<UMaterialExpressionScalarParameter>(AddExpr(Material, UMaterialExpressionScalarParameter::StaticClass(), 740, 200));
		UMaterialExpressionScalarParameter* WetAmount = Cast<UMaterialExpressionScalarParameter>(AddExpr(Material, UMaterialExpressionScalarParameter::StaticClass(), 740, 340));
		UMaterialExpressionConstant* WetRoughTarget = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 740, 480));
		UMaterialExpressionLinearInterpolate* WetRough = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 960, 280));
		UMaterialExpressionConstant* DirtRoughTarget = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 960, 460));
		UMaterialExpressionLinearInterpolate* FinalRough = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 1160, 340));

		UMaterialExpressionConstant* SpecDry = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 740, 640));
		UMaterialExpressionConstant* SpecWet = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 740, 760));
		UMaterialExpressionLinearInterpolate* WetSpec = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 960, 680));
		UMaterialExpressionConstant* SpecDirt = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 960, 860));
		UMaterialExpressionLinearInterpolate* FinalSpec = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 1160, 740));

		UMaterialExpressionConstant* MetalClean = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 960, 1020));
		UMaterialExpressionConstant* MetalDirt = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 960, 1140));
		UMaterialExpressionLinearInterpolate* FinalMetal = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 1160, 1040));

		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Paint || !DirtColor || !Shade || !Dark || !Fresnel || !FresnelScale || !FresnelMask || !WetAlbedo
			|| !WorldPos || !ActorPos || !LocalPos || !LocalZ || !HeightLift || !LiftedZ || !HeightSpan || !HeightDiv || !Height01
			|| !Coverage || !SatCoverage || !CoverageGap || !CoverageBand || !CoverageDiv || !Vertical || !Breakup || !BreakupMin || !BreakupMax || !BreakupScale || !BrokenVertical
			|| !DirtAmount || !SatDirt || !DirtRaw || !DirtMask || !Albedo
			|| !Roughness || !WetAmount || !SatWet || !WetRoughTarget || !WetRough || !DirtRoughTarget || !FinalRough
			|| !SpecDry || !SpecWet || !WetSpec || !SpecDirt || !FinalSpec
			|| !MetalClean || !MetalDirt || !FinalMetal || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_TruckPaint graph"));
			return;
		}

		Paint->ParameterName = TEXT("PaintColor");
		Paint->DefaultValue = FLinearColor(0.42f, 0.08f, 0.05f);
		DirtColor->ParameterName = TEXT("DirtColor");
		DirtColor->DefaultValue = FLinearColor(0.12f, 0.09f, 0.06f);
		Shade->R = 0.45f;
		Fresnel->Exponent = 4.f;
		FresnelScale->R = 0.4f;
		Roughness->ParameterName = TEXT("Roughness");
		Roughness->DefaultValue = 0.38f;
		WetAmount->ParameterName = TEXT("WetAmount");
		WetAmount->DefaultValue = 0.55f;
		SetUnitRange(WetAmount);
		WetRoughTarget->R = 0.12f;
		DirtRoughTarget->R = 0.86f;
		SpecDry->R = 0.42f;
		SpecWet->R = 0.7f;
		SpecDirt->R = 0.16f;
		MetalClean->R = 0.22f;
		MetalDirt->R = 0.02f;
		DirtAmount->ParameterName = TEXT("DirtAmount");
		DirtAmount->DefaultValue = 0.35f;
		SetUnitRange(DirtAmount);
		Coverage->ParameterName = TEXT("DirtCoverageBias");
		Coverage->DefaultValue = 0.6f;
		SetUnitRange(Coverage);
		HeightLift->R = 50.f;
		HeightSpan->R = 220.f;
		CoverageBand->R = 0.35f;
		Breakup->Scale = 0.06f;
		Breakup->bTurbulence = true;
		Breakup->Levels = 3;
		Breakup->OutputMin = 0.f;
		Breakup->OutputMax = 1.f;
		BreakupMin->R = 0.82f;
		BreakupMax->R = 1.f;
		LocalZ->R = false;
		LocalZ->G = false;
		LocalZ->B = true;
		LocalZ->A = false;

		Dark->A.Connect(0, Paint);
		Dark->B.Connect(0, Shade);
		FresnelMask->A.Connect(0, Fresnel);
		FresnelMask->B.Connect(0, FresnelScale);
		WetAlbedo->A.Connect(0, Paint);
		WetAlbedo->B.Connect(0, Dark);
		WetAlbedo->Alpha.Connect(0, FresnelMask);

		LocalPos->A.Connect(0, WorldPos);
		LocalPos->B.Connect(0, ActorPos);
		LocalZ->Input.Connect(0, LocalPos);
		LiftedZ->A.Connect(0, LocalZ);
		LiftedZ->B.Connect(0, HeightLift);
		HeightDiv->A.Connect(0, LiftedZ);
		HeightDiv->B.Connect(0, HeightSpan);
		Height01->Input.Connect(0, HeightDiv);
		SatCoverage->Input.Connect(0, Coverage);
		CoverageGap->A.Connect(0, SatCoverage);
		CoverageGap->B.Connect(0, Height01);
		CoverageDiv->A.Connect(0, CoverageGap);
		CoverageDiv->B.Connect(0, CoverageBand);
		Vertical->Input.Connect(0, CoverageDiv);
		BreakupScale->A.Connect(0, BreakupMin);
		BreakupScale->B.Connect(0, BreakupMax);
		BreakupScale->Alpha.Connect(0, Breakup);
		BrokenVertical->A.Connect(0, Vertical);
		BrokenVertical->B.Connect(0, BreakupScale);
		SatDirt->Input.Connect(0, DirtAmount);
		DirtRaw->A.Connect(0, BrokenVertical);
		DirtRaw->B.Connect(0, SatDirt);
		DirtMask->Input.Connect(0, DirtRaw);
		Albedo->A.Connect(0, WetAlbedo);
		Albedo->B.Connect(0, DirtColor);
		Albedo->Alpha.Connect(0, DirtMask);

		SatWet->Input.Connect(0, WetAmount);
		WetRough->A.Connect(0, Roughness);
		WetRough->B.Connect(0, WetRoughTarget);
		WetRough->Alpha.Connect(0, SatWet);
		FinalRough->A.Connect(0, WetRough);
		FinalRough->B.Connect(0, DirtRoughTarget);
		FinalRough->Alpha.Connect(0, DirtMask);

		WetSpec->A.Connect(0, SpecDry);
		WetSpec->B.Connect(0, SpecWet);
		WetSpec->Alpha.Connect(0, SatWet);
		FinalSpec->A.Connect(0, WetSpec);
		FinalSpec->B.Connect(0, SpecDirt);
		FinalSpec->Alpha.Connect(0, DirtMask);

		FinalMetal->A.Connect(0, MetalClean);
		FinalMetal->B.Connect(0, MetalDirt);
		FinalMetal->Alpha.Connect(0, DirtMask);

		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, FinalRough);
		EditorData->Metallic.Connect(0, FinalMetal);
		EditorData->Specular.Connect(0, FinalSpec);
		SaveMaterial(Material);
		UE_LOG(LogSiltEditor, Display, TEXT("Silt truck paint pass A ready"));
	}

	UMaterial* AcquirePuddle()
	{
		const TCHAR* ObjectPath = TEXT("/Game/SiltCounty/Materials/M_Puddle.M_Puddle");
		if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, ObjectPath))
		{
			if (HasUnitScalar(Existing, TEXT("Wetness")))
			{
				UE_LOG(LogSiltEditor, Display, TEXT("Silt shallow puddles pass A ready"));
				return nullptr;
			}
			UMaterialEditingLibrary::DeleteAllMaterialExpressions(Existing);
			return Existing;
		}
		return CreatePackageMaterial(TEXT("M_Puddle"));
	}

	void BuildPuddle()
	{
		UMaterial* Material = AcquirePuddle();
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);
		Material->TwoSided = true;

		// Same mud / flood colors and fresnel as M_WetGround and M_FloodWater.
		// Wetness 0–1 only darkens toward flood water and keeps roughness at or under 0.06.
		UMaterialExpressionConstant3Vector* Mud = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -680, -20));
		UMaterialExpressionConstant3Vector* Deep = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -680, 160));
		UMaterialExpressionConstant3Vector* Pale = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -680, 340));
		UMaterialExpressionScalarParameter* Wetness = Cast<UMaterialExpressionScalarParameter>(AddExpr(Material, UMaterialExpressionScalarParameter::StaticClass(), -680, 520));
		UMaterialExpressionSaturate* SatWet = Cast<UMaterialExpressionSaturate>(AddExpr(Material, UMaterialExpressionSaturate::StaticClass(), -420, 520));
		UMaterialExpressionLinearInterpolate* Body = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), -220, 40));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -420, 300));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 20, 80));
		UMaterialExpressionConstant* RoughDry = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -220, 260));
		UMaterialExpressionConstant* RoughWet = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -220, 380));
		UMaterialExpressionLinearInterpolate* Rough = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 20, 280));
		UMaterialExpressionConstant* SpecDry = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -220, 500));
		UMaterialExpressionConstant* SpecWet = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -220, 620));
		UMaterialExpressionLinearInterpolate* Spec = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 20, 500));
		UMaterialExpressionConstant* Metallic = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 20, 660));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Mud || !Deep || !Pale || !Wetness || !SatWet || !Body || !Fresnel || !Albedo
			|| !RoughDry || !RoughWet || !Rough || !SpecDry || !SpecWet || !Spec || !Metallic || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_Puddle graph"));
			return;
		}

		Mud->Constant = FLinearColor(0.09f, 0.055f, 0.028f);
		Deep->Constant = FLinearColor(0.025f, 0.055f, 0.05f);
		Pale->Constant = FLinearColor(0.42f, 0.48f, 0.46f);
		Wetness->ParameterName = TEXT("Wetness");
		Wetness->DefaultValue = 0.85f;
		SetUnitRange(Wetness);
		Fresnel->Exponent = 5.f;
		RoughDry->R = 0.06f;
		RoughWet->R = 0.035f;
		SpecDry->R = 0.72f;
		SpecWet->R = 0.92f;
		Metallic->R = 0.f;

		SatWet->Input.Connect(0, Wetness);
		Body->A.Connect(0, Mud);
		Body->B.Connect(0, Deep);
		Body->Alpha.Connect(0, SatWet);
		Albedo->A.Connect(0, Body);
		Albedo->B.Connect(0, Pale);
		Albedo->Alpha.Connect(0, Fresnel);
		Rough->A.Connect(0, RoughDry);
		Rough->B.Connect(0, RoughWet);
		Rough->Alpha.Connect(0, SatWet);
		Spec->A.Connect(0, SpecDry);
		Spec->B.Connect(0, SpecWet);
		Spec->Alpha.Connect(0, SatWet);

		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, Rough);
		EditorData->Specular.Connect(0, Spec);
		EditorData->Metallic.Connect(0, Metallic);
		SaveMaterial(Material);
		UE_LOG(LogSiltEditor, Display, TEXT("Silt shallow puddles pass A ready"));
	}

	void BuildBeacon()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_Beacon"));
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_Unlit);

		UMaterialExpressionVectorParameter* Paint = Cast<UMaterialExpressionVectorParameter>(AddExpr(Material, UMaterialExpressionVectorParameter::StaticClass(), -360, 0));
		UMaterialExpressionConstant* Gain = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -360, 160));
		UMaterialExpressionMultiply* Emissive = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -120, 40));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Paint || !Gain || !Emissive || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_Beacon graph"));
			return;
		}

		Paint->ParameterName = TEXT("PaintColor");
		Paint->DefaultValue = FLinearColor(1.f, 0.65f, 0.15f);
		Gain->R = 6.f;
		Emissive->A.Connect(0, Paint);
		Emissive->B.Connect(0, Gain);
		EditorData->EmissiveColor.Connect(0, Emissive);
		EditorData->BaseColor.Connect(0, Paint);
		SaveMaterial(Material);
	}

	void BuildCourseMaterial(
		const TCHAR* Name,
		const FLinearColor& Face,
		const FLinearColor& Joint,
		const FLinearColor& Wet,
		float ZFrequency,
		float JointWidth,
		float RoughDry,
		float RoughWet,
		float SpecularValue)
	{
		UMaterial* Material = CreatePackageMaterial(Name);
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);

		UMaterialExpressionWorldPosition* WorldPos = Cast<UMaterialExpressionWorldPosition>(AddExpr(Material, UMaterialExpressionWorldPosition::StaticClass(), -980, 0));
		UMaterialExpressionComponentMask* HeightMask = Cast<UMaterialExpressionComponentMask>(AddExpr(Material, UMaterialExpressionComponentMask::StaticClass(), -760, 0));
		UMaterialExpressionConstant* Frequency = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -760, 160));
		UMaterialExpressionMultiply* ScaledZ = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -540, 40));
		UMaterialExpressionFrac* Course = Cast<UMaterialExpressionFrac>(AddExpr(Material, UMaterialExpressionFrac::StaticClass(), -340, 40));
		UMaterialExpressionConstant* JointAt = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -340, 200));
		UMaterialExpressionConstant3Vector* FaceColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -340, 340));
		UMaterialExpressionConstant3Vector* JointColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -340, 500));
		UMaterialExpressionIf* Courses = Cast<UMaterialExpressionIf>(AddExpr(Material, UMaterialExpressionIf::StaticClass(), -80, 160));
		UMaterialExpressionConstant3Vector* WetColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -80, 420));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -80, 580));
		UMaterialExpressionConstant* FresnelScale = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -80, 720));
		UMaterialExpressionMultiply* FresnelMask = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), 140, 560));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 320, 120));
		UMaterialExpressionConstant* DryRough = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 140, 280));
		UMaterialExpressionConstant* WetRough = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 140, 400));
		UMaterialExpressionLinearInterpolate* Roughness = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 320, 320));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 320, 480));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!WorldPos || !HeightMask || !Frequency || !ScaledZ || !Course || !JointAt || !FaceColor || !JointColor || !Courses
			|| !WetColor || !Fresnel || !FresnelScale || !FresnelMask || !Albedo || !DryRough || !WetRough || !Roughness || !Specular || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build %s graph"), Name);
			return;
		}

		HeightMask->R = 0;
		HeightMask->G = 0;
		HeightMask->B = 1;
		HeightMask->A = 0;
		Frequency->R = ZFrequency;
		JointAt->R = JointWidth;
		FaceColor->Constant = Face;
		JointColor->Constant = Joint;
		WetColor->Constant = Wet;
		Fresnel->Exponent = 4.5f;
		FresnelScale->R = 0.42f;
		DryRough->R = RoughDry;
		WetRough->R = RoughWet;
		Specular->R = SpecularValue;

		HeightMask->Input.Connect(0, WorldPos);
		ScaledZ->A.Connect(0, HeightMask);
		ScaledZ->B.Connect(0, Frequency);
		Course->Input.Connect(0, ScaledZ);
		Courses->A.Connect(0, Course);
		Courses->B.Connect(0, JointAt);
		Courses->ALessThanB.Connect(0, JointColor);
		Courses->AEqualsB.Connect(0, JointColor);
		Courses->AGreaterThanB.Connect(0, FaceColor);
		FresnelMask->A.Connect(0, Fresnel);
		FresnelMask->B.Connect(0, FresnelScale);
		Albedo->A.Connect(0, Courses);
		Albedo->B.Connect(0, WetColor);
		Albedo->Alpha.Connect(0, FresnelMask);
		Roughness->A.Connect(0, DryRough);
		Roughness->B.Connect(0, WetRough);
		Roughness->Alpha.Connect(0, FresnelMask);
		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, Roughness);
		EditorData->Specular.Connect(0, Specular);
		SaveMaterial(Material);
	}

	void BuildTownBrick()
	{
		BuildCourseMaterial(
			TEXT("M_TownBrick"),
			FLinearColor(0.34f, 0.13f, 0.09f),
			FLinearColor(0.40f, 0.38f, 0.34f),
			FLinearColor(0.10f, 0.06f, 0.045f),
			0.07f,
			0.18f,
			0.88f,
			0.22f,
			0.32f);
	}

	void BuildTownClapboard()
	{
		BuildCourseMaterial(
			TEXT("M_TownClapboard"),
			FLinearColor(0.40f, 0.33f, 0.24f),
			FLinearColor(0.15f, 0.11f, 0.08f),
			FLinearColor(0.16f, 0.13f, 0.10f),
			0.042f,
			0.12f,
			0.78f,
			0.28f,
			0.26f);
	}

	void BuildConcreteBlock()
	{
		BuildCourseMaterial(
			TEXT("M_ConcreteBlock"),
			FLinearColor(0.44f, 0.43f, 0.40f),
			FLinearColor(0.20f, 0.20f, 0.18f),
			FLinearColor(0.18f, 0.19f, 0.18f),
			0.022f,
			0.08f,
			0.9f,
			0.16f,
			0.5f);
	}

	void BuildMunicipalPaint()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_MunicipalPaint"));
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);

		UMaterialExpressionVectorParameter* Paint = Cast<UMaterialExpressionVectorParameter>(AddExpr(Material, UMaterialExpressionVectorParameter::StaticClass(), -520, 0));
		UMaterialExpressionConstant* Shade = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -520, 180));
		UMaterialExpressionMultiply* Dark = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -280, 80));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -520, 320));
		UMaterialExpressionConstant* FresnelScale = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -520, 460));
		UMaterialExpressionMultiply* FresnelMask = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -260, 360));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), -20, 40));
		UMaterialExpressionScalarParameter* Roughness = Cast<UMaterialExpressionScalarParameter>(AddExpr(Material, UMaterialExpressionScalarParameter::StaticClass(), -20, 220));
		UMaterialExpressionConstant* Metallic = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -20, 340));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -20, 460));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Paint || !Shade || !Dark || !Fresnel || !FresnelScale || !FresnelMask || !Albedo || !Roughness || !Metallic || !Specular || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_MunicipalPaint graph"));
			return;
		}

		Paint->ParameterName = TEXT("PaintColor");
		Paint->DefaultValue = FLinearColor(0.36f, 0.42f, 0.38f);
		Shade->R = 0.55f;
		Fresnel->Exponent = 4.5f;
		FresnelScale->R = 0.35f;
		Roughness->ParameterName = TEXT("Roughness");
		Roughness->DefaultValue = 0.52f;
		Metallic->R = 0.04f;
		Specular->R = 0.42f;
		Dark->A.Connect(0, Paint);
		Dark->B.Connect(0, Shade);
		FresnelMask->A.Connect(0, Fresnel);
		FresnelMask->B.Connect(0, FresnelScale);
		Albedo->A.Connect(0, Paint);
		Albedo->B.Connect(0, Dark);
		Albedo->Alpha.Connect(0, FresnelMask);
		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, Roughness);
		EditorData->Metallic.Connect(0, Metallic);
		EditorData->Specular.Connect(0, Specular);
		SaveMaterial(Material);
	}

	void BuildRain()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_Rain"));
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Translucent;
		Material->SetShadingModel(MSM_Unlit);
		Material->TwoSided = true;
		Material->bUsedWithInstancedStaticMeshes = true;

		UMaterialExpressionConstant3Vector* Color = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -300, 0));
		UMaterialExpressionConstant* Opacity = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -300, 160));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Color || !Opacity || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_Rain graph"));
			return;
		}

		Color->Constant = FLinearColor(0.75f, 0.8f, 0.82f);
		Opacity->R = 0.22f;
		EditorData->EmissiveColor.Connect(0, Color);
		EditorData->Opacity.Connect(0, Opacity);
		SaveMaterial(Material);
	}

	void BuildShopFloor()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_ShopFloor"));
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);

		UMaterialExpressionWorldPosition* WorldPos = Cast<UMaterialExpressionWorldPosition>(AddExpr(Material, UMaterialExpressionWorldPosition::StaticClass(), -980, 0));
		UMaterialExpressionComponentMask* SpanMask = Cast<UMaterialExpressionComponentMask>(AddExpr(Material, UMaterialExpressionComponentMask::StaticClass(), -760, 0));
		UMaterialExpressionConstant* Frequency = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -760, 160));
		UMaterialExpressionMultiply* Scaled = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -540, 40));
		UMaterialExpressionFrac* StainFrac = Cast<UMaterialExpressionFrac>(AddExpr(Material, UMaterialExpressionFrac::StaticClass(), -340, 40));
		UMaterialExpressionConstant* StainAt = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -340, 200));
		UMaterialExpressionConstant3Vector* ConcreteColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -340, 340));
		UMaterialExpressionConstant3Vector* OilColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -340, 500));
		UMaterialExpressionIf* Stain = Cast<UMaterialExpressionIf>(AddExpr(Material, UMaterialExpressionIf::StaticClass(), -80, 160));
		UMaterialExpressionConstant3Vector* WetColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -80, 420));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -80, 580));
		UMaterialExpressionConstant* FresnelScale = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -80, 720));
		UMaterialExpressionMultiply* FresnelMask = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), 140, 560));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 320, 120));
		UMaterialExpressionConstant* DryRough = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 140, 280));
		UMaterialExpressionConstant* WetRough = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 140, 400));
		UMaterialExpressionLinearInterpolate* Roughness = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 320, 320));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 320, 480));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!WorldPos || !SpanMask || !Frequency || !Scaled || !StainFrac || !StainAt || !ConcreteColor || !OilColor || !Stain
			|| !WetColor || !Fresnel || !FresnelScale || !FresnelMask || !Albedo || !DryRough || !WetRough || !Roughness || !Specular || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_ShopFloor graph"));
			return;
		}

		SpanMask->R = 1;
		SpanMask->G = 0;
		SpanMask->B = 0;
		SpanMask->A = 0;
		Frequency->R = 0.008f;
		StainAt->R = 0.22f;
		ConcreteColor->Constant = FLinearColor(0.26f, 0.25f, 0.22f);
		OilColor->Constant = FLinearColor(0.05f, 0.04f, 0.03f);
		WetColor->Constant = FLinearColor(0.08f, 0.09f, 0.09f);
		Fresnel->Exponent = 3.5f;
		FresnelScale->R = 0.62f;
		DryRough->R = 0.58f;
		WetRough->R = 0.06f;
		Specular->R = 0.55f;

		SpanMask->Input.Connect(0, WorldPos);
		Scaled->A.Connect(0, SpanMask);
		Scaled->B.Connect(0, Frequency);
		StainFrac->Input.Connect(0, Scaled);
		Stain->A.Connect(0, StainFrac);
		Stain->B.Connect(0, StainAt);
		Stain->ALessThanB.Connect(0, OilColor);
		Stain->AEqualsB.Connect(0, OilColor);
		Stain->AGreaterThanB.Connect(0, ConcreteColor);
		FresnelMask->A.Connect(0, Fresnel);
		FresnelMask->B.Connect(0, FresnelScale);
		Albedo->A.Connect(0, Stain);
		Albedo->B.Connect(0, WetColor);
		Albedo->Alpha.Connect(0, FresnelMask);
		Roughness->A.Connect(0, DryRough);
		Roughness->B.Connect(0, WetRough);
		Roughness->Alpha.Connect(0, FresnelMask);
		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, Roughness);
		EditorData->Specular.Connect(0, Specular);
		SaveMaterial(Material);
	}

	void BuildShopWall()
	{
		BuildCourseMaterial(
			TEXT("M_ShopWall"),
			FLinearColor(0.32f, 0.30f, 0.26f),
			FLinearColor(0.16f, 0.15f, 0.13f),
			FLinearColor(0.12f, 0.13f, 0.12f),
			0.05f,
			0.14f,
			0.82f,
			0.18f,
			0.28f);
	}

	void Ensure()
	{
		static bool bOnce = false;
		if (bOnce)
		{
			return;
		}
		bOnce = true;
		BuildWetGround();
		BuildFloodWater();
		BuildTruckPaint();
		BuildPuddle();
		BuildBeacon();
		BuildRain();
		BuildTownBrick();
		BuildTownClapboard();
		BuildConcreteBlock();
		BuildMunicipalPaint();
		BuildShopFloor();
		BuildShopWall();
		UE_LOG(LogSiltEditor, Display, TEXT("Silt County materials are ready under /Game/SiltCounty/Materials"));
	}
}

void FSiltCountyEditorModule::StartupModule()
{
	if (!GIsEditor || IsRunningCommandlet())
	{
		return;
	}

	// Wait until the editor finishes booting before compiling materials.
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateLambda([](float DeltaTime)
	{
		(void)DeltaTime;
		SiltMaterialBootstrap::Ensure();
		return false;
	}), 1.0f);
}

IMPLEMENT_MODULE(FSiltCountyEditorModule, SiltCountyEditor);
