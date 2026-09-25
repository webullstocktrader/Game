#include "SiltCountyEditor.h"
#include "SiltWetMaterials.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Containers/Ticker.h"
#include "HAL/FileManager.h"
#include "Modules/ModuleManager.h"
#include "Materials/Material.h"
#include "Materials/MaterialEditorOnlyData.h"
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

	// Soft translucent streak. Opacity stays near 0.3 so the cubes do not read as sheets.
	void ApplyRainOpacity(UMaterial* Material)
	{
		if (!Material)
		{
			return;
		}

		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Translucent;
		Material->SetShadingModel(MSM_Unlit);
		Material->TwoSided = true;
		Material->bUsedWithInstancedStaticMeshes = true;

		bool bSetOpacity = false;
		if (UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData()))
		{
			for (UMaterialExpression* Expr : EditorData->ExpressionCollection.Expressions)
			{
				if (UMaterialExpressionConstant* Opacity = Cast<UMaterialExpressionConstant>(Expr))
				{
					Opacity->R = 0.3f;
					bSetOpacity = true;
				}
			}
		}

		if (!bSetOpacity)
		{
			UE_LOG(LogSiltEditor, Warning, TEXT("M_Rain has no opacity constant. Delete Content/SiltCounty/Materials/M_Rain and reopen the editor."));
		}
		SaveMaterial(Material);
	}

	void BuildRain()
	{
		if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, TEXT("/Game/SiltCounty/Materials/M_Rain.M_Rain")))
		{
			ApplyRainOpacity(Existing);
			UE_LOG(LogSiltEditor, Display, TEXT("M_Rain kept translucent, opacity 0.3"));
			return;
		}

		UMaterial* Material = CreatePackageMaterial(TEXT("M_Rain"));
		if (!Material)
		{
			return;
		}

		UMaterialExpressionConstant3Vector* Color = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -300, 0));
		UMaterialExpressionConstant* Opacity = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -300, 160));
		UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
		if (!Color || !Opacity || !EditorData)
		{
			UE_LOG(LogSiltEditor, Error, TEXT("Failed to build M_Rain graph"));
			return;
		}

		Color->Constant = FLinearColor(0.75f, 0.8f, 0.82f);
		Opacity->R = 0.3f;
		EditorData->EmissiveColor.Connect(0, Color);
		EditorData->Opacity.Connect(0, Opacity);
		ApplyRainOpacity(Material);
		UE_LOG(LogSiltEditor, Display, TEXT("M_Rain built as a translucent streak, opacity 0.3"));
	}

	void BuildSoftCard(const TCHAR* Name, const FLinearColor& Color, float OpacityValue)
	{
		const FString ObjectPath = FString::Printf(TEXT("/Game/SiltCounty/Materials/%s.%s"), Name, Name);
		UMaterial* Material = LoadObject<UMaterial>(nullptr, *ObjectPath);
		const bool bNew = Material == nullptr;
		if (bNew)
		{
			Material = CreatePackageMaterial(Name);
		}
		if (!Material)
		{
			return;
		}

		Material->MaterialDomain = MD_Surface;
		Material->BlendMode = BLEND_Translucent;
		Material->SetShadingModel(MSM_Unlit);
		Material->TwoSided = true;
		Material->bUsedWithInstancedStaticMeshes = true;

		if (bNew)
		{
			UMaterialExpressionConstant3Vector* Tint = Cast<UMaterialExpressionConstant3Vector>(AddExpr(Material, UMaterialExpressionConstant3Vector::StaticClass(), -300, 0));
			UMaterialExpressionConstant* Opacity = Cast<UMaterialExpressionConstant>(AddExpr(Material, UMaterialExpressionConstant::StaticClass(), -300, 160));
			UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData());
			if (!Tint || !Opacity || !EditorData)
			{
				UE_LOG(LogSiltEditor, Error, TEXT("Failed to build %s"), Name);
				return;
			}
			Tint->Constant = FLinearColor(Color.R, Color.G, Color.B, 1.f);
			Opacity->R = OpacityValue;
			EditorData->EmissiveColor.Connect(0, Tint);
			EditorData->Opacity.Connect(0, Opacity);
		}
		else if (UMaterialEditorOnlyData* EditorData = Cast<UMaterialEditorOnlyData>(Material->GetEditorOnlyData()))
		{
			for (UMaterialExpression* Expr : EditorData->ExpressionCollection.Expressions)
			{
				if (UMaterialExpressionConstant* Opacity = Cast<UMaterialExpressionConstant>(Expr))
				{
					Opacity->R = OpacityValue;
				}
			}
		}

		SaveMaterial(Material);
		UE_LOG(LogSiltEditor, Display, TEXT("%s translucent opacity %.2f"), Name, OpacityValue);
	}

	void Ensure()
	{
		static bool bOnce = false;
		if (bOnce)
		{
			return;
		}
		bOnce = true;
		SiltWetMaterials::Ensure();
		BuildTruckPaint();
		BuildBeacon();
		BuildRain();
		BuildSoftCard(TEXT("M_TireSpray"), FLinearColor(0.62f, 0.66f, 0.68f), 0.28f);
		BuildSoftCard(TEXT("M_MudKick"), FLinearColor(0.18f, 0.12f, 0.07f), 0.45f);
		BuildSoftCard(TEXT("M_GroundMist"), FLinearColor(0.58f, 0.60f, 0.56f), 0.08f);
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
