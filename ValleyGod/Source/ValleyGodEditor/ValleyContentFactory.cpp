#include "ValleyContentFactory.h"
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

bool FValleyContentFactory::AssetExists(const FString& ObjectPath)
{
	return LoadObject<UObject>(nullptr, *ObjectPath) != nullptr;
}

bool FValleyContentFactory::SaveAsset(UObject* Asset, const FString& PackagePath)
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

UMaterial* FValleyContentFactory::CreateLit(const FString& ShortName, const FLinearColor& Color, float Metallic, float Roughness, bool bTranslucent, float Emissive)
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

bool FValleyContentFactory::CreateSliceMap()
{
	if (AssetExists(TEXT("/Game/Maps/ValleySlice.ValleySlice")))
	{
		return true;
	}

	UPackage* Package = CreatePackage(TEXT("/Game/Maps/ValleySlice"));
	UWorldFactory* Factory = NewObject<UWorldFactory>();
	UWorld* World = Cast<UWorld>(Factory->FactoryCreateNew(
		UWorld::StaticClass(), Package, TEXT("ValleySlice"), RF_Public | RF_Standalone, nullptr, GWarn));
	if (!World)
	{
		return false;
	}
	return SaveAsset(World, TEXT("/Game/Maps/ValleySlice"));
}

bool FValleyContentFactory::EnsureContent()
{
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Materials")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Maps")), true);

	CreateLit(TEXT("M_Dirt"), FLinearColor(0.12f, 0.07f, 0.035f), 0.04f, 0.22f, false, 0.f);
	CreateLit(TEXT("M_DirtWet"), FLinearColor(0.06f, 0.035f, 0.018f), 0.08f, 0.12f, false, 0.f);
	CreateLit(TEXT("M_Grass"), FLinearColor(0.05f, 0.09f, 0.03f), 0.f, 0.55f, false, 0.f);
	CreateLit(TEXT("M_Water"), FLinearColor(0.03f, 0.06f, 0.05f, 0.42f), 0.f, 0.05f, true, 0.f);
	CreateLit(TEXT("M_Bark"), FLinearColor(0.05f, 0.03f, 0.02f), 0.f, 0.78f, false, 0.f);
	CreateLit(TEXT("M_Foliage"), FLinearColor(0.04f, 0.08f, 0.025f), 0.f, 0.5f, false, 0.f);
	CreateLit(TEXT("M_Wood"), FLinearColor(0.1f, 0.055f, 0.022f), 0.f, 0.52f, false, 0.f);
	CreateLit(TEXT("M_Hide"), FLinearColor(0.16f, 0.1f, 0.05f), 0.f, 0.48f, false, 0.f);
	CreateLit(TEXT("M_SkinWarm"), FLinearColor(0.42f, 0.28f, 0.18f), 0.f, 0.55f, false, 0.f);
	CreateLit(TEXT("M_ClothOchre"), FLinearColor(0.22f, 0.13f, 0.06f), 0.f, 0.6f, false, 0.f);
	CreateLit(TEXT("M_Hair"), FLinearColor(0.04f, 0.03f, 0.02f), 0.f, 0.7f, false, 0.f);
	CreateLit(TEXT("M_Stone"), FLinearColor(0.18f, 0.17f, 0.15f), 0.05f, 0.62f, false, 0.f);
	CreateLit(TEXT("M_Fire"), FLinearColor(1.f, 0.45f, 0.08f), 0.f, 1.f, false, 22.f);

	return CreateSliceMap();
}
