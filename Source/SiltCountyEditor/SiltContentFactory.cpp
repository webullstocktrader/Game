#include "SiltContentFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Factories/WorldFactory.h"
#include "HAL/FileManager.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "UObject/SavePackage.h"

bool FSiltContentFactory::AssetExists(const FString& ObjectPath)
{
	return LoadObject<UObject>(nullptr, *ObjectPath) != nullptr;
}

bool FSiltContentFactory::SaveAsset(UObject* Asset, const FString& PackagePath)
{
	if (!Asset)
	{
		return false;
	}
	UPackage* Package = Asset->GetOutermost();
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Asset);

	FString Filename;
	if (!FPackageName::TryConvertLongPackageNameToFilename(PackagePath, Filename, FPackageName::GetAssetPackageExtension()))
	{
		return false;
	}

	FSavePackageArgs Args;
	Args.TopLevelFlags = RF_Public | RF_Standalone;
	Args.SaveFlags = SAVE_NoError;
	return UPackage::SavePackage(Package, Asset, *Filename, Args);
}

UMaterial* FSiltContentFactory::CreateLit(const FString& ShortName, const FLinearColor& Color, float Metallic, float Roughness, bool bTranslucent, float Emissive)
{
	const FString PackagePath = FString::Printf(TEXT("/Game/Materials/%s"), *ShortName);
	const FString ObjectPath = PackagePath + TEXT(".") + ShortName;
	if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPath))
	{
		return Existing;
	}

	UPackage* Package = CreatePackage(*PackagePath);
	UMaterial* Mat = NewObject<UMaterial>(Package, *ShortName, RF_Public | RF_Standalone);
	if (bTranslucent)
	{
		Mat->BlendMode = BLEND_Translucent;
		Mat->TranslucencyLightingMode = TLM_VolumetricPerVertexNonDirectional;
	}

	auto* ColorExpr = Cast<UMaterialExpressionVectorParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionVectorParameter::StaticClass(), -420, 0));
	if (!ColorExpr)
	{
		SaveAsset(Mat, PackagePath);
		return Mat;
	}
	ColorExpr->ParameterName = TEXT("BaseColor");
	ColorExpr->DefaultValue = Color;
	UMaterialEditingLibrary::ConnectMaterialProperty(ColorExpr, TEXT(""), MP_BaseColor);

	auto* MetalExpr = Cast<UMaterialExpressionScalarParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionScalarParameter::StaticClass(), -420, 160));
	MetalExpr->ParameterName = TEXT("Metallic");
	MetalExpr->DefaultValue = Metallic;
	UMaterialEditingLibrary::ConnectMaterialProperty(MetalExpr, TEXT(""), MP_Metallic);

	auto* RoughExpr = Cast<UMaterialExpressionScalarParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionScalarParameter::StaticClass(), -420, 280));
	RoughExpr->ParameterName = TEXT("Roughness");
	RoughExpr->DefaultValue = Roughness;
	UMaterialEditingLibrary::ConnectMaterialProperty(RoughExpr, TEXT(""), MP_Roughness);

	if (Emissive > 0.f)
	{
		auto* Em = Cast<UMaterialExpressionVectorParameter>(
			UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionVectorParameter::StaticClass(), -420, 400));
		Em->ParameterName = TEXT("Emissive");
		Em->DefaultValue = FLinearColor(Color.R, Color.G, Color.B) * Emissive;
		UMaterialEditingLibrary::ConnectMaterialProperty(Em, TEXT(""), MP_EmissiveColor);
	}

	if (bTranslucent)
	{
		auto* Op = Cast<UMaterialExpressionScalarParameter>(
			UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionScalarParameter::StaticClass(), -420, 520));
		Op->ParameterName = TEXT("Opacity");
		Op->DefaultValue = Color.A > 0.f ? Color.A : 0.35f;
		UMaterialEditingLibrary::ConnectMaterialProperty(Op, TEXT(""), MP_Opacity);
	}

	UMaterialEditingLibrary::LayoutMaterialExpressions(Mat);
	UMaterialEditingLibrary::RecompileMaterial(Mat);
	SaveAsset(Mat, PackagePath);
	return Mat;
}

bool FSiltContentFactory::CreateSliceMap()
{
	if (AssetExists(TEXT("/Game/Maps/SiltCountySlice.SiltCountySlice")))
	{
		return true;
	}

	UPackage* Package = CreatePackage(TEXT("/Game/Maps/SiltCountySlice"));
	UWorldFactory* Factory = NewObject<UWorldFactory>();
	UWorld* World = Cast<UWorld>(Factory->FactoryCreateNew(
		UWorld::StaticClass(), Package, TEXT("SiltCountySlice"), RF_Public | RF_Standalone, nullptr, GWarn));
	if (!World)
	{
		return false;
	}
	return SaveAsset(World, TEXT("/Game/Maps/SiltCountySlice"));
}

bool FSiltContentFactory::EnsureContent()
{
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Materials")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Maps")), true);

	CreateLit(TEXT("M_TruckChief"), FLinearColor(0.045f, 0.09f, 0.035f), 0.55f, 0.28f, false, 0.f);
	CreateLit(TEXT("M_TruckGooch"), FLinearColor(0.18f, 0.035f, 0.02f), 0.5f, 0.32f, false, 0.f);
	CreateLit(TEXT("M_Rubber"), FLinearColor(0.02f, 0.02f, 0.02f), 0.f, 0.55f, false, 0.f);
	CreateLit(TEXT("M_Mud"), FLinearColor(0.08f, 0.05f, 0.025f), 0.f, 0.22f, false, 0.f);
	CreateLit(TEXT("M_Asphalt"), FLinearColor(0.02f, 0.02f, 0.022f), 0.12f, 0.18f, false, 0.f);
	CreateLit(TEXT("M_Gravel"), FLinearColor(0.09f, 0.08f, 0.07f), 0.05f, 0.42f, false, 0.f);
	CreateLit(TEXT("M_Water"), FLinearColor(0.03f, 0.05f, 0.04f, 0.42f), 0.f, 0.06f, true, 0.f);
	CreateLit(TEXT("M_Metal"), FLinearColor(0.16f, 0.16f, 0.17f), 0.82f, 0.28f, false, 0.f);
	CreateLit(TEXT("M_Glass"), FLinearColor(0.02f, 0.03f, 0.04f, 0.28f), 0.f, 0.05f, true, 0.f);
	CreateLit(TEXT("M_Chrome"), FLinearColor(0.65f, 0.66f, 0.68f), 1.f, 0.16f, false, 0.f);
	CreateLit(TEXT("M_Emissive"), FLinearColor(1.f, 0.86f, 0.55f), 0.f, 1.f, false, 18.f);
	CreateLit(TEXT("M_Bark"), FLinearColor(0.04f, 0.028f, 0.018f), 0.f, 0.72f, false, 0.f);
	CreateLit(TEXT("M_Foliage"), FLinearColor(0.03f, 0.06f, 0.02f), 0.f, 0.48f, false, 0.f);
	CreateLit(TEXT("M_Wood"), FLinearColor(0.09f, 0.05f, 0.02f), 0.f, 0.5f, false, 0.f);
	CreateLit(TEXT("M_Crate"), FLinearColor(0.22f, 0.13f, 0.04f), 0.f, 0.48f, false, 0.f);

	return CreateSliceMap();
}
