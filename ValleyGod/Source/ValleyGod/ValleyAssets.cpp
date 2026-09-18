#include "ValleyAssets.h"
#include "Sim/ValleyLookPaths.h"
#include "Sim/ValleyPalette.h"
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

	int32 BlueprintRank(const FString& PackageName, const FString& VillagerName)
	{
		const FString Short = FPackageName::GetShortName(PackageName);
		if (Short.Equals(TEXT("BP_") + VillagerName, ESearchCase::IgnoreCase))
		{
			return 0;
		}
		if (Short.StartsWith(TEXT("BP_")))
		{
			return 1;
		}
		if (Short.Equals(VillagerName, ESearchCase::IgnoreCase))
		{
			return 2;
		}
		return 3;
	}

	bool PathMatchesVillager(const FString& PackageName, const char* VillagerName)
	{
		const auto Converted = StringCast<ANSICHAR>(*PackageName);
		return vg::PackageMatchesVillager(Converted.Get(), VillagerName);
	}

	bool PathLooksGeneric(const FString& PackageName)
	{
		const auto Converted = StringCast<ANSICHAR>(*PackageName);
		return vg::LooksLikeGenericMetaHumanBlueprint(Converted.Get());
	}

	UClass* LoadDocumentedVillagerClass(int Slot)
	{
		for (int32 I = 0; I < vg::VillagerMetaHumanPathCount(Slot); ++I)
		{
			if (UClass* Class = LoadActorClass(Utf(vg::VillagerMetaHumanPathAt(Slot, I))))
			{
				return Class;
			}
		}
		return nullptr;
	}

	UClass* FindNamedClass(const char* VillagerName, const TArray<FString>& Candidates)
	{
		if (!VillagerName || !VillagerName[0])
		{
			return nullptr;
		}
		const FString Name = UTF8_TO_TCHAR(VillagerName);
		TArray<FString> Named;
		for (const FString& PackageName : Candidates)
		{
			if (PathMatchesVillager(PackageName, VillagerName))
			{
				Named.Add(PackageName);
			}
		}
		Named.Sort([&Name](const FString& A, const FString& B)
		{
			const int32 RA = BlueprintRank(A, Name);
			const int32 RB = BlueprintRank(B, Name);
			if (RA != RB)
			{
				return RA < RB;
			}
			return A.Len() < B.Len();
		});
		for (const FString& PackageName : Named)
		{
			if (UClass* Class = LoadActorClass(PackageName))
			{
				UE_LOG(LogValleyGodAssets, Display, TEXT("Valley God: %s MetaHuman scanned %s"),
					UTF8_TO_TCHAR(VillagerName), *Class->GetPathName());
				return Class;
			}
		}
		return nullptr;
	}

	void DiscoverVillagerClasses(TArray<UClass*>& OutClasses, const TArray<FString>& MetaPackages)
	{
		const int32 AdultCount = vg::PersonLookCount();
		OutClasses.SetNum(AdultCount);
		TSet<UClass*> Claimed;

		for (int32 Slot = 0; Slot < AdultCount; ++Slot)
		{
			const char* Name = vg::PersonLookAt(Slot).Name;
			UClass* Class = LoadDocumentedVillagerClass(Slot);
			if (!Class)
			{
				Class = FindNamedClass(Name, MetaPackages);
			}
			if (Class)
			{
				OutClasses[Slot] = Class;
				Claimed.Add(Class);
				UE_LOG(LogValleyGodAssets, Display, TEXT("Valley God: slot %d (%s) MetaHuman class %s"),
					Slot, UTF8_TO_TCHAR(Name ? Name : ""), *Class->GetPathName());
			}
		}

		TArray<UClass*> Generics;
		for (const FString& PackageName : MetaPackages)
		{
			if (!PathLooksGeneric(PackageName))
			{
				continue;
			}
			if (UClass* Class = LoadActorClass(PackageName))
			{
				if (!Claimed.Contains(Class))
				{
					Generics.AddUnique(Class);
					Claimed.Add(Class);
				}
			}
		}

		int32 GenericIndex = 0;
		for (int32 Slot = 0; Slot < OutClasses.Num() && GenericIndex < Generics.Num(); ++Slot)
		{
			if (!OutClasses[Slot])
			{
				OutClasses[Slot] = Generics[GenericIndex++];
				UE_LOG(LogValleyGodAssets, Display, TEXT("Valley God: slot %d using generic MetaHuman %s"),
					Slot, *OutClasses[Slot]->GetPathName());
			}
		}

		for (; GenericIndex < Generics.Num(); ++GenericIndex)
		{
			OutClasses.Add(Generics[GenericIndex]);
		}
	}
}

namespace Valley
{
	FOptionalAssets DiscoverOptionalAssets()
	{
		FOptionalAssets Found;

		TArray<FString> MetaPackages;
		CollectPackages(TEXT("EditableMetahumans"), MetaPackages, true);
		CollectPackages(TEXT("MetaHumans"), MetaPackages, true);
		CollectPackages(TEXT("ValleySlice"), MetaPackages, true);

		DiscoverVillagerClasses(Found.VillagerClasses, MetaPackages);
		Found.MaraClass = Found.VillagerClasses.IsValidIndex(0) ? Found.VillagerClasses[0] : nullptr;

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

		int32 NamedMH = 0;
		for (UClass* Class : Found.VillagerClasses)
		{
			if (Class)
			{
				++NamedMH;
			}
		}
		UE_LOG(LogValleyGodAssets, Display,
			TEXT("Valley God assets: MetaHumans=%d/%d dirt=%s grass=%s trees=%d grassMeshes=%d rocks=%d"),
			NamedMH, Found.VillagerClasses.Num(),
			Found.Dirt ? TEXT("Quixel") : TEXT("procedural"),
			Found.Grass ? TEXT("Quixel") : TEXT("procedural"),
			Found.Trees.Num(), Found.GrassMeshes.Num(), Found.Rocks.Num());
		return Found;
	}
}
