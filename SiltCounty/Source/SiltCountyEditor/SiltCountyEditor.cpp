#include "SiltCountyEditor.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Containers/Ticker.h"
#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionConstant.h"
#include "Materials/MaterialExpressionConstant3Vector.h"
#include "Materials/MaterialExpressionFresnel.h"
#include "Materials/MaterialExpressionLinearInterpolate.h"
#include "Materials/MaterialExpressionMultiply.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexColor.h"
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
