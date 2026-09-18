#include "ValleyAssets.h"
#include "Sim/ValleyLookPaths.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Actor.h"
#include "HAL/FileManager.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Misc/StringConv.h"

DEFINE_LOG_CATEGORY_STATIC(LogValleyGodAssets, Log, All);

namespace
{
	FString Utf(const char* Path)
	{
		return UTF8_TO_TCHAR(Path ? Path : "");
	}

	FString ObjectPathOf(const FString& PackageName)
	{
		return PackageName + TEXT(".") + FPackageName::GetShortName(PackageName);
	}

	bool PackageOnDisk(const FString& PackageName)
	{
		if (PackageName.IsEmpty())
		{
			return false;
		}
		if (FPackageName::DoesPackageExist(PackageName))
		{
			return true;
		}
		FString Filename;
		if (!FPackageName::TryConvertLongPackageNameToFilename(PackageName, Filename, FPackageName::GetAssetPackageExtension()))
		{
			return false;
		}
		return IFileManager::Get().FileExists(*Filename);
	}

	template <typename T>
	T* LoadTyped(const FString& PackageName)
	{
		if (!PackageOnDisk(PackageName))
		{
			return nullptr;
		}
		return LoadObject<T>(nullptr, *ObjectPathOf(PackageName));
	}

	UClass* LoadActorClass(const FString& PackageName)
	{
		if (!PackageOnDisk(PackageName))
		{
			return nullptr;
		}
		const FString Short = FPackageName::GetShortName(PackageName);
		const FString ClassPath = PackageName + TEXT(".") + Short + TEXT("_C");
		if (UClass* Class = LoadClass<AActor>(nullptr, *ClassPath))
		{
			return Class;
		}
		if (UClass* Class = LoadObject<UClass>(nullptr, *ObjectPathOf(PackageName)))
		{
			if (Class->IsChildOf(AActor::StaticClass()))
			{
				return Class;
			}
		}
		return nullptr;
	}

	void CollectPackages(const FString& ContentRelative, TArray<FString>& Out, bool bRecursive)
	{
		const FString Root = FPaths::Combine(FPaths::ProjectContentDir(), ContentRelative);
		if (!IFileManager::Get().DirectoryExists(*Root))
		{
			return;
		}
		TArray<FString> Files;
		if (bRecursive)
		{
			IFileManager::Get().FindFilesRecursive(Files, *Root, TEXT("*.uasset"), true, false);
		}
		else
		{
			IFileManager::Get().FindFiles(Files, *FPaths::Combine(Root, TEXT("*.uasset")), true, false);
			for (FString& File : Files)
			{
				File = FPaths::Combine(Root, File);
			}
		}
		for (const FString& File : Files)
		{
			FString PackageName;
			if (FPackageName::TryConvertFilenameToLongPackageName(File, PackageName))
			{
				Out.AddUnique(PackageName);
			}
		}
	}

	template <typename T>
	void LoadDocumented(int Count, const char* (*At)(int), TArray<T*>& Out, int32 MaxCount)
	{
		for (int I = 0; I < Count && Out.Num() < MaxCount; ++I)
		{
			if (T* Obj = LoadTyped<T>(Utf(At(I))))
			{
				Out.AddUnique(Obj);
			}
		}
	}

	void ScanKindIntoMeshes(vg::ScanKind Kind, const TArray<FString>& Packages, TArray<UStaticMesh*>& Out, int32 MaxCount)
	{
		for (const FString& PackageName : Packages)
		{
			if (Out.Num() >= MaxCount)
			{
				break;
			}
			const auto Converted = StringCast<ANSICHAR>(*PackageName);
			if (!vg::ClassifyContentPath(Converted.Get(), Kind))
			{
				continue;
			}
			if (UStaticMesh* Mesh = LoadTyped<UStaticMesh>(PackageName))
			{
				Out.AddUnique(Mesh);
			}
		}
	}

	void ScanKindIntoMaterials(vg::ScanKind Kind, const TArray<FString>& Packages, TArray<UMaterialInterface*>& Out, int32 MaxCount)
	{
		for (const FString& PackageName : Packages)
		{
			if (Out.Num() >= MaxCount)
			{
				break;
			}
			const auto Converted = StringCast<ANSICHAR>(*PackageName);
			if (!vg::ClassifyContentPath(Converted.Get(), Kind))
			{
				continue;
			}
			if (UMaterialInterface* Mat = LoadTyped<UMaterialInterface>(PackageName))
			{
				Out.AddUnique(Mat);
			}
		}
	}

	int32 BlueprintRank(const FString& PackageName)
	{
		const FString Short = FPackageName::GetShortName(PackageName);
		if (Short.StartsWith(TEXT("BP_")))
		{
			return 0;
		}
		if (Short.Equals(TEXT("Mara"), ESearchCase::IgnoreCase))
		{
			return 1;
		}
		return 2;
	}

	UClass* FindMaraClass()
	{
		for (int32 I = 0; I < vg::MetaHumanClassPathCount(); ++I)
		{
			if (UClass* Class = LoadActorClass(Utf(vg::MetaHumanClassPathAt(I))))
			{
				UE_LOG(LogValleyGodAssets, Display, TEXT("Valley God: Mara MetaHuman class %s"), *Class->GetPathName());
				return Class;
			}
		}

		TArray<FString> Candidates;
		CollectPackages(TEXT("MetaHumans/Mara"), Candidates, true);
		Candidates.Sort([](const FString& A, const FString& B)
		{
			const int32 RA = BlueprintRank(A);
			const int32 RB = BlueprintRank(B);
			if (RA != RB)
			{
				return RA < RB;
			}
			return A.Len() < B.Len();
		});
		for (const FString& PackageName : Candidates)
		{
			UE_LOG(LogValleyGodAssets, Verbose, TEXT("Valley God: considering Mara package %s"), *PackageName);
			if (UClass* Class = LoadActorClass(PackageName))
			{
				UE_LOG(LogValleyGodAssets, Display, TEXT("Valley God: Mara MetaHuman scanned %s"), *Class->GetPathName());
				return Class;
			}
		}
		return nullptr;
	}
}

namespace Valley
{
	FOptionalAssets DiscoverOptionalAssets()
	{
		FOptionalAssets Found;
		Found.MaraClass = FindMaraClass();

		TArray<FString> Downloaded;
		CollectPackages(TEXT("Megascans"), Downloaded, true);
		CollectPackages(TEXT("Fab"), Downloaded, true);
		CollectPackages(TEXT("PN_GrassLibrary"), Downloaded, true);
		CollectPackages(TEXT("ValleySlice"), Downloaded, true);
		CollectPackages(TEXT("MSPresets"), Downloaded, true);
		CollectPackages(TEXT("Quixel"), Downloaded, true);

		TArray<UMaterialInterface*> Dirts;
		LoadDocumented<UMaterialInterface>(vg::DirtMaterialPathCount(), &vg::DirtMaterialPathAt, Dirts, 4);
		ScanKindIntoMaterials(vg::ScanKind::DirtMaterial, Downloaded, Dirts, 4);
		Found.Dirt = Dirts.Num() > 0 ? Dirts[0] : nullptr;

		TArray<UMaterialInterface*> Grasses;
		LoadDocumented<UMaterialInterface>(vg::GrassMaterialPathCount(), &vg::GrassMaterialPathAt, Grasses, 4);
		ScanKindIntoMaterials(vg::ScanKind::GrassMaterial, Downloaded, Grasses, 4);
		Found.Grass = Grasses.Num() > 0 ? Grasses[0] : nullptr;

		TArray<UMaterialInterface*> Wets;
		LoadDocumented<UMaterialInterface>(vg::WetDirtMaterialPathCount(), &vg::WetDirtMaterialPathAt, Wets, 4);
		ScanKindIntoMaterials(vg::ScanKind::WetDirtMaterial, Downloaded, Wets, 4);
		Found.WetDirt = Wets.Num() > 0 ? Wets[0] : nullptr;

		LoadDocumented<UStaticMesh>(vg::TreeMeshPathCount(), &vg::TreeMeshPathAt, Found.Trees, 32);
		ScanKindIntoMeshes(vg::ScanKind::TreeMesh, Downloaded, Found.Trees, 32);

		LoadDocumented<UStaticMesh>(vg::GrassMeshPathCount(), &vg::GrassMeshPathAt, Found.GrassMeshes, 32);
		ScanKindIntoMeshes(vg::ScanKind::GrassMesh, Downloaded, Found.GrassMeshes, 32);

		LoadDocumented<UStaticMesh>(vg::RockMeshPathCount(), &vg::RockMeshPathAt, Found.Rocks, 24);
		ScanKindIntoMeshes(vg::ScanKind::RockMesh, Downloaded, Found.Rocks, 24);

		UE_LOG(LogValleyGodAssets, Display,
			TEXT("Valley God assets: Mara=%s dirt=%s grass=%s trees=%d grassMeshes=%d rocks=%d"),
			Found.MaraClass ? TEXT("MetaHuman") : TEXT("procedural"),
			Found.Dirt ? TEXT("Quixel") : TEXT("procedural"),
			Found.Grass ? TEXT("Quixel") : TEXT("procedural"),
			Found.Trees.Num(), Found.GrassMeshes.Num(), Found.Rocks.Num());
		return Found;
	}
}
