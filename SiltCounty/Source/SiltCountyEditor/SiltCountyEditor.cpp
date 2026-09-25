#include "SiltCountyEditor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Containers/Ticker.h"
#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionComponentMask.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionIf.h"
#include "Materials/MaterialExpressionFrac.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionScalarParameter.h"
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

	void BuildTruckPaint()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_TruckPaint"));
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
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_TruckPaint graph"));
			return;
		}

		Paint->ParameterName = TEXT("PaintColor");
		Paint->DefaultValue = FLinearColor(0.42f, 0.08f, 0.05f);
		Shade->R = 0.45f;
		Fresnel->Exponent = 4.f;
		FresnelScale->R = 0.4f;
		Roughness->ParameterName = TEXT("Roughness");
		Roughness->DefaultValue = 0.38f;
		Metallic->R = 0.22f;
		Specular->R = 0.5f;
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

	void BuildGarageFloor()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_GarageFloor"));
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
		UMaterialExpressionVectorParameter* ConcreteColor = Cast<UMaterialExpressionVectorParameter>(AddExpr(Material, UMaterialExpressionVectorParameter::StaticClass(), -340, 340));
		UMaterialExpressionConstant3Vector* OilColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -340, 500));
		UMaterialExpressionIf* Stain = Cast<UMaterialExpressionIf>(AddExpr(Material, UMaterialExpressionIf::StaticClass(), -80, 160));
		UMaterialExpressionConstant3Vector* WetColor = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -80, 420));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -80, 580));
		UMaterialExpressionConstant* FresnelScale = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -80, 720));
		UMaterialExpressionMultiply* FresnelMask = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), 140, 560));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 320, 120));
		UMaterialExpressionScalarParameter* DryRough = Cast<UMaterialExpressionScalarParameter>(AddExpr(Material, UMaterialExpressionScalarParameter::StaticClass(), 140, 280));
		UMaterialExpressionConstant* WetRough = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 140, 400));
		UMaterialExpressionLinearInterpolate* Roughness = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 320, 320));
		UMaterialExpressionConstant* SpecularDry = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 140, 520));
		UMaterialExpressionConstant* SpecularWet = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), 140, 640));
		UMaterialExpressionLinearInterpolate* Specular = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), 320, 480));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!WorldPos || !SpanMask || !Frequency || !Scaled || !StainFrac || !StainAt || !ConcreteColor || !OilColor || !Stain
			|| !WetColor || !Fresnel || !FresnelScale || !FresnelMask || !Albedo || !DryRough || !WetRough || !Roughness
			|| !SpecularDry || !SpecularWet || !Specular || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_GarageFloor graph"));
			return;
		}

		SpanMask->R = 1;
		SpanMask->G = 0;
		SpanMask->B = 0;
		SpanMask->A = 0;
		Frequency->R = 0.006f;
		StainAt->R = 0.28f;
		ConcreteColor->ParameterName = TEXT("PaintColor");
		ConcreteColor->DefaultValue = FLinearColor(0.30f, 0.34f, 0.36f);
		OilColor->Constant = FLinearColor(0.045f, 0.04f, 0.038f);
		WetColor->Constant = FLinearColor(0.12f, 0.16f, 0.19f);
		Fresnel->Exponent = 3.2f;
		FresnelScale->R = 0.7f;
		DryRough->ParameterName = TEXT("Roughness");
		DryRough->DefaultValue = 0.22f;
		WetRough->R = 0.03f;
		SpecularDry->R = 0.28f;
		SpecularWet->R = 0.88f;

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
		Specular->A.Connect(0, SpecularDry);
		Specular->B.Connect(0, SpecularWet);
		Specular->Alpha.Connect(0, FresnelMask);
		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, Roughness);
		EditorData->Specular.Connect(0, Specular);
		SaveMaterial(Material);
	}

	void BuildGarageWall()
	{
		BuildCourseMaterial(
			TEXT("M_GarageWall"),
			FLinearColor(0.40f, 0.41f, 0.36f),
			FLinearColor(0.16f, 0.15f, 0.13f),
			FLinearColor(0.18f, 0.17f, 0.14f),
			0.055f,
			0.16f,
			0.78f,
			0.24f,
			0.22f);
	}

	void BuildGarageRoof()
	{
		UMaterial* Material = CreatePackageMaterial(TEXT("M_GarageRoof"));
		if (!Material)
		{
			return;
		}
		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Opaque;
		Material->SetShadingModel(MSM_DefaultLit);

		UMaterialExpressionConstant3Vector* Metal = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -520, 0));
		UMaterialExpressionConstant3Vector* Edge = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -520, 160));
		UMaterialExpressionFresnel* Fresnel = Cast<UMaterialExpressionFresnel>(AddExpr(Material, UMaterialExpressionFresnel::StaticClass(), -520, 320));
		UMaterialExpressionConstant* FresnelScale = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -520, 460));
		UMaterialExpressionMultiply* FresnelMask = Cast<UMaterialExpressionMultiply>(AddExpr(Material, UMaterialExpressionMultiply::StaticClass(), -260, 360));
		UMaterialExpressionLinearInterpolate* Albedo = Cast<UMaterialExpressionLinearInterpolate>(AddExpr(Material, UMaterialExpressionLinearInterpolate::StaticClass(), -20, 40));
		UMaterialExpressionConstant* Roughness = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -20, 200));
		UMaterialExpressionConstant* Metallic = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -20, 320));
		UMaterialExpressionConstant* Specular = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -20, 440));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Metal || !Edge || !Fresnel || !FresnelScale || !FresnelMask || !Albedo || !Roughness || !Metallic || !Specular || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_GarageRoof graph"));
			return;
		}

		Metal->Constant = FLinearColor(0.07f, 0.075f, 0.08f);
		Edge->Constant = FLinearColor(0.16f, 0.17f, 0.18f);
		Fresnel->Exponent = 4.5f;
		FresnelScale->R = 0.35f;
		Roughness->R = 0.42f;
		Metallic->R = 0.78f;
		Specular->R = 0.5f;
		FresnelMask->A.Connect(0, Fresnel);
		FresnelMask->B.Connect(0, FresnelScale);
		Albedo->A.Connect(0, Metal);
		Albedo->B.Connect(0, Edge);
		Albedo->Alpha.Connect(0, FresnelMask);
		EditorData->BaseColor.Connect(0, Albedo);
		EditorData->Roughness.Connect(0, Roughness);
		EditorData->Metallic.Connect(0, Metallic);
		EditorData->Specular.Connect(0, Specular);
		SaveMaterial(Material);
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
		BuildBeacon();
		BuildRain();
		BuildTownBrick();
		BuildTownClapboard();
		BuildConcreteBlock();
		BuildMunicipalPaint();
		BuildGarageFloor();
		BuildGarageWall();
		BuildGarageRoof();
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
