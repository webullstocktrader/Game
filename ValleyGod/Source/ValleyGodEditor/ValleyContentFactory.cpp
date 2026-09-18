#include "ValleyContentFactory.h"
#include "ValleyTypes.h"
#include "Sim/ValleyPalette.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Factories/WorldFactory.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "HAL/FileManager.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
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
		UE_LOG(LogTemp, Error, TEXT("ValleyPrep: no asset for %s"), *PackagePath);
		return false;
	}
	UPackage* Package = Asset->GetOutermost();
	Package->FullyLoad();
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Asset);

	FString Filename;
	if (!FPackageName::TryConvertLongPackageNameToFilename(PackagePath, Filename, FPackageName::GetAssetPackageExtension()))
	{
		UE_LOG(LogTemp, Error, TEXT("ValleyPrep: cannot convert %s to a filename"), *PackagePath);
		return false;
	}

	IFileManager::Get().MakeDirectory(*FPaths::GetPath(Filename), true);

	FSavePackageArgs Args;
	Args.TopLevelFlags = RF_Public | RF_Standalone;
	Args.Error = GWarn;
	Args.SaveFlags = SAVE_None;
	const bool bOk = UPackage::SavePackage(Package, Asset, *Filename, Args);
	UE_LOG(LogTemp, Display, TEXT("ValleyPrep save %s -> %s (%s)"), *PackagePath, *Filename, bOk ? TEXT("ok") : TEXT("FAIL"));
	return bOk;
}

UMaterial* FValleyContentFactory::CreateLit(const vg::MaterialRecipe& Recipe)
{
	const FString ShortName = ANSI_TO_TCHAR(Recipe.Name);
	const FString PackagePath = FString::Printf(TEXT("/Game/Materials/%s"), *ShortName);
	const FString ObjectPath = PackagePath + TEXT(".") + ShortName;

	UPackage* Package = CreatePackage(*PackagePath);
	Package->FullyLoad();

	UMaterial* Mat = FindObject<UMaterial>(Package, *ShortName);
	if (!Mat)
	{
		Mat = LoadObject<UMaterial>(nullptr, *ObjectPath);
	}
	if (!Mat)
	{
		Mat = NewObject<UMaterial>(Package, *ShortName, RF_Public | RF_Standalone);
	}

	Valley::PopulateLitMaterial(Mat, Recipe);
	UMaterialEditingLibrary::LayoutMaterialExpressions(Mat);
	UMaterialEditingLibrary::RecompileMaterial(Mat);
	if (!SaveAsset(Mat, PackagePath))
	{
		return nullptr;
	}
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
		UE_LOG(LogTemp, Error, TEXT("ValleyPrep: WorldFactory failed for ValleySlice"));
		return false;
	}
	return SaveAsset(World, TEXT("/Game/Maps/ValleySlice"));
}

bool FValleyContentFactory::EnsureContent()
{
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Materials")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Maps")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("MetaHumans/Mara")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("EditableMetahumans")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("PN_GrassLibrary")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Megascans")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("MSPresets")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Quixel")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("ValleySlice")), true);
	IFileManager::Get().MakeDirectory(*FPaths::Combine(FPaths::ProjectContentDir(), TEXT("Fab")), true);

	int32 Saved = 0;
	int32 Failed = 0;
	for (int32 I = 0; I < vg::MaterialRecipeCount(); ++I)
	{
		if (CreateLit(vg::MaterialRecipeAt(I)))
		{
			++Saved;
		}
		else
		{
			++Failed;
		}
	}

	const bool bMap = CreateSliceMap();
	FEditorFileUtils::SaveDirtyPackages(true, true, false);

	UE_LOG(LogTemp, Display, TEXT("ValleyPrep materials saved=%d failed=%d map=%s"), Saved, Failed, bMap ? TEXT("ok") : TEXT("FAIL"));
	return Failed == 0 && bMap;
}
