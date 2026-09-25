#include "SiltContentFactory.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/World.h"
#include "Factories/WorldFactory.h"
#include "HAL/FileManager.h"
#include "MaterialEditingLibrary.h"
#include "Materials/Material.h"
#include "Materials/MaterialExpressionCustom.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Materials/MaterialExpressionVertexNormalWS.h"
#include "Materials/MaterialExpressionWorldPosition.h"
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
	CreateLit(TEXT("M_DrumRed"), FLinearColor(0.28f, 0.045f, 0.02f), 0.35f, 0.46f, false, 0.f);
	CreateLit(TEXT("M_WindowWarm"), FLinearColor(0.62f, 0.34f, 0.12f), 0.f, 0.16f, false, 2.4f);
	CreateLit(TEXT("M_ShopFluorescent"), FLinearColor(0.78f, 0.88f, 0.96f), 0.f, 0.22f, false, 7.f);
	CreateLit(TEXT("M_SignBoard"), FLinearColor(0.045f, 0.04f, 0.035f), 0.08f, 0.55f, false, 0.f);
	CreateLit(TEXT("M_TrimCream"), FLinearColor(0.74f, 0.66f, 0.48f), 0.f, 0.5f, false, 0.f);
	CreateLit(TEXT("M_BannerRed"), FLinearColor(0.42f, 0.07f, 0.05f), 0.f, 0.55f, false, 0.f);
	CreateLit(TEXT("M_BannerCream"), FLinearColor(0.78f, 0.70f, 0.50f), 0.f, 0.52f, false, 0.f);

	CreateTownAndShopMaterials();
	return CreateSliceMap();
}

UMaterial* FSiltContentFactory::CreateWorldPattern(const FString& ShortName, const FString& Hlsl, float Metallic, float Roughness, bool bUseNormal)
{
	const FString PackagePath = FString::Printf(TEXT("/Game/Materials/%s"), *ShortName);
	const FString ObjectPath = PackagePath + TEXT(".") + ShortName;
	if (UMaterial* Existing = LoadObject<UMaterial>(nullptr, *ObjectPath))
	{
		return Existing;
	}

	UPackage* Package = CreatePackage(*PackagePath);
	UMaterial* Mat = NewObject<UMaterial>(Package, *ShortName, RF_Public | RF_Standalone);

	UMaterialExpressionWorldPosition* WorldPos = Cast<UMaterialExpressionWorldPosition>(
		UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionWorldPosition::StaticClass(), -860, 0));
	UMaterialExpressionVertexNormalWS* NormalWS = nullptr;
	if (bUseNormal)
	{
		NormalWS = Cast<UMaterialExpressionVertexNormalWS>(
			UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionVertexNormalWS::StaticClass(), -860, 200));
	}
	UMaterialExpressionCustom* Custom = Cast<UMaterialExpressionCustom>(
		UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionCustom::StaticClass(), -420, 0));
	if (!WorldPos || !Custom || (bUseNormal && !NormalWS))
	{
		SaveAsset(Mat, PackagePath);
		return Mat;
	}

	Custom->OutputType = CMOT_Float3;
	Custom->Description = ShortName;
	Custom->Code = Hlsl;
	Custom->Inputs.Reset();

	FCustomInput WorldIn;
	WorldIn.InputName = TEXT("WorldPos");
	Custom->Inputs.Add(WorldIn);
	if (bUseNormal)
	{
		FCustomInput NormalIn;
		NormalIn.InputName = TEXT("NormalWS");
		Custom->Inputs.Add(NormalIn);
	}

	UMaterialEditingLibrary::ConnectMaterialExpressions(WorldPos, TEXT(""), Custom, TEXT("WorldPos"));
	if (NormalWS)
	{
		UMaterialEditingLibrary::ConnectMaterialExpressions(NormalWS, TEXT(""), Custom, TEXT("NormalWS"));
	}
	UMaterialEditingLibrary::ConnectMaterialProperty(Custom, TEXT(""), MP_BaseColor);

	if (UMaterialExpressionScalarParameter* Metal = Cast<UMaterialExpressionScalarParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionScalarParameter::StaticClass(), -200, 240)))
	{
		Metal->ParameterName = TEXT("Metallic");
		Metal->DefaultValue = Metallic;
		UMaterialEditingLibrary::ConnectMaterialProperty(Metal, TEXT(""), MP_Metallic);
	}
	if (UMaterialExpressionScalarParameter* Rough = Cast<UMaterialExpressionScalarParameter>(
		UMaterialEditingLibrary::CreateMaterialExpression(Mat, UMaterialExpressionScalarParameter::StaticClass(), -200, 360)))
	{
		Rough->ParameterName = TEXT("Roughness");
		Rough->DefaultValue = Roughness;
		UMaterialEditingLibrary::ConnectMaterialProperty(Rough, TEXT(""), MP_Roughness);
	}

	UMaterialEditingLibrary::LayoutMaterialExpressions(Mat);
	UMaterialEditingLibrary::RecompileMaterial(Mat);
	SaveAsset(Mat, PackagePath);
	return Mat;
}

void FSiltContentFactory::CreateTownAndShopMaterials()
{
	const TCHAR* Brick = TEXT(
		"float3 P = WorldPos;\n"
		"float2 cell = float2(24.0, 14.0);\n"
		"float course = floor(P.z / cell.y);\n"
		"float shift = frac(course * 0.5);\n"
		"float2 fx = frac(float2(P.x / cell.x + shift, P.z / cell.y));\n"
		"float2 fy = frac(float2(P.y / cell.x + shift, P.z / cell.y));\n"
		"float mortar = max((fx.x < 0.08 || fx.y < 0.15) ? 1.0 : 0.0, (fy.x < 0.08 || fy.y < 0.15) ? 1.0 : 0.0);\n"
		"float n = frac(sin(dot(floor(P.xz / cell), float2(12.9898, 78.233))) * 43758.5453);\n"
		"float3 brick = lerp(float3(0.32, 0.11, 0.07), float3(0.48, 0.18, 0.10), n);\n"
		"return lerp(brick, float3(0.52, 0.49, 0.45), mortar);\n");

	const TCHAR* Stone = TEXT(
		"float3 P = WorldPos;\n"
		"float2 cell = float2(78.0, 36.0);\n"
		"float course = floor(P.z / cell.y);\n"
		"float shift = frac(course * 0.5);\n"
		"float2 fx = frac(float2(P.x / cell.x + shift, P.z / cell.y));\n"
		"float2 fy = frac(float2(P.y / cell.x + shift, P.z / cell.y));\n"
		"float mortar = max((fx.x < 0.05 || fx.y < 0.09) ? 1.0 : 0.0, (fy.x < 0.05 || fy.y < 0.09) ? 1.0 : 0.0);\n"
		"float n = frac(sin(dot(floor(P.xz / cell), float2(9.13, 23.71))) * 43758.5453);\n"
		"float3 rock = lerp(float3(0.40, 0.38, 0.34), float3(0.56, 0.53, 0.48), n);\n"
		"return lerp(rock, float3(0.28, 0.27, 0.25), mortar);\n");

	const TCHAR* Siding = TEXT(
		"float3 P = WorldPos;\n"
		"float plank = frac(P.z / 18.0);\n"
		"float gap = plank > 0.86 ? 1.0 : 0.0;\n"
		"float n = frac(sin(floor(P.z / 18.0) * 12.989 + P.x * 0.008) * 78.233);\n"
		"float3 wood = lerp(float3(0.28, 0.16, 0.08), float3(0.42, 0.24, 0.12), n);\n"
		"return lerp(wood, wood * 0.32, gap);\n");

	const TCHAR* Corrugated = TEXT(
		"float3 P = WorldPos;\n"
		"float3 Nn = abs(NormalWS);\n"
		"float rx = abs(sin(P.y * 0.52));\n"
		"float ry = abs(sin(P.x * 0.52));\n"
		"float rz = abs(sin(P.x * 0.48));\n"
		"float wave = (rx * Nn.x + ry * Nn.y + rz * Nn.z) / max(Nn.x + Nn.y + Nn.z, 0.001);\n"
		"float rust = frac(sin(dot(floor(P * 0.025), float3(41.1, 17.3, 9.7))) * 12345.67);\n"
		"float3 metal = lerp(float3(0.45, 0.38, 0.32), float3(0.30, 0.13, 0.06), rust);\n"
		"return metal * (0.46 + 0.54 * wave);\n");

	const TCHAR* Cinder = TEXT(
		"float3 P = WorldPos;\n"
		"float2 cell = float2(48.0, 22.0);\n"
		"float course = floor(P.z / cell.y);\n"
		"float shift = frac(course * 0.5);\n"
		"float2 fx = frac(float2(P.x / cell.x + shift, P.z / cell.y));\n"
		"float2 fy = frac(float2(P.y / cell.x + shift, P.z / cell.y));\n"
		"float mortar = max((fx.x < 0.055 || fx.y < 0.12) ? 1.0 : 0.0, (fy.x < 0.055 || fy.y < 0.12) ? 1.0 : 0.0);\n"
		"float n = frac(sin(dot(floor(P.xy / cell), float2(12.9, 78.2))) * 43758.5);\n"
		"float3 block = lerp(float3(0.46, 0.46, 0.44), float3(0.58, 0.57, 0.53), n);\n"
		"float soot = frac(sin((P.x + P.y) * 0.011) * 123.4);\n"
		"float3 col = lerp(block, float3(0.30, 0.30, 0.28), mortar);\n"
		"return lerp(col, col * 0.7, smoothstep(0.72, 1.0, soot) * 0.4);\n");

	const TCHAR* ShopFloor = TEXT(
		"float3 P = WorldPos;\n"
		"float n = frac(sin(dot(P.xy, float2(0.041, 0.027))) * 43758.5);\n"
		"float stain = frac(sin(dot(P.xy, float2(0.0047, 0.0061))) * 24634.6);\n"
		"float blob = smoothstep(0.62, 0.92, stain);\n"
		"float3 concrete = lerp(float3(0.23, 0.22, 0.20), float3(0.36, 0.34, 0.31), n);\n"
		"return lerp(concrete, float3(0.035, 0.03, 0.025), blob * 0.9);\n");

	const TCHAR* Plank = TEXT(
		"float3 P = WorldPos;\n"
		"float board = frac(P.y / 16.0);\n"
		"float gap = board > 0.88 ? 1.0 : 0.0;\n"
		"float n = frac(sin(floor(P.y / 16.0) * 17.13 + P.x * 0.006) * 91.32);\n"
		"float3 wood = lerp(float3(0.27, 0.15, 0.07), float3(0.40, 0.24, 0.11), n);\n"
		"return lerp(wood, wood * 0.28, gap);\n");

	const TCHAR* Concrete = TEXT(
		"float3 P = WorldPos;\n"
		"float2 f = frac(P.xy / 140.0);\n"
		"float joint = (f.x < 0.028 || f.y < 0.028) ? 1.0 : 0.0;\n"
		"float n = frac(sin(dot(floor(P.xy / 140.0), float2(19.1, 47.7))) * 123.45);\n"
		"float speckle = frac(sin(dot(P.xy, float2(0.19, 0.23))) * 43758.5);\n"
		"float3 mixcol = lerp(float3(0.38, 0.38, 0.36), float3(0.52, 0.51, 0.47), n);\n"
		"mixcol = lerp(mixcol, mixcol * 0.82, speckle * 0.3);\n"
		"return lerp(mixcol, float3(0.24, 0.24, 0.23), joint);\n");

	const TCHAR* Caliche = TEXT(
		"float3 P = WorldPos;\n"
		"float n = frac(sin(dot(P.xy, float2(0.023, 0.019))) * 43758.5);\n"
		"float pebble = frac(sin(dot(floor(P.xy / 18.0), float2(3.1, 7.7))) * 99.13);\n"
		"float3 dust = lerp(float3(0.45, 0.35, 0.22), float3(0.64, 0.51, 0.33), n);\n"
		"return lerp(dust, dust * 0.72, step(0.84, pebble));\n");

	const TCHAR* Adobe = TEXT(
		"float3 P = WorldPos;\n"
		"float n = frac(sin(dot(P.xyz, float3(0.017, 0.013, 0.021))) * 43758.5);\n"
		"float blot = frac(sin(dot(floor(P.xy * 0.01), float2(19.1, 47.7))) * 23421.6);\n"
		"float3 base = lerp(float3(0.52, 0.40, 0.26), float3(0.70, 0.55, 0.36), n);\n"
		"float3 col = lerp(base, float3(0.38, 0.26, 0.16), smoothstep(0.55, 0.95, blot) * 0.55);\n"
		"float crack = frac(sin(P.x * 0.07 + P.z * 0.11) * 43758.5) > 0.978 ? 1.0 : 0.0;\n"
		"return lerp(col, col * 0.48, crack);\n");

	const TCHAR* Tank = TEXT(
		"float3 P = WorldPos;\n"
		"float rib = abs(sin(P.z * 0.22));\n"
		"float band = frac(P.z / 220.0);\n"
		"float stripe = (band > 0.42 && band < 0.58) ? 1.0 : 0.0;\n"
		"float grime = frac(sin(dot(P.xy, float2(0.02, 0.017))) * 43758.5);\n"
		"float3 paint = lerp(float3(0.72, 0.74, 0.76), float3(0.48, 0.50, 0.52), grime);\n"
		"paint *= 0.72 + 0.28 * rib;\n"
		"return lerp(paint, float3(0.10, 0.16, 0.22), stripe);\n");

	auto Front = [](float R, float G, float B)
	{
		return FString::Printf(TEXT(
			"float3 P = WorldPos;\n"
			"float board = frac(P.x / 14.0);\n"
			"float gap = board > 0.9 ? 1.0 : 0.0;\n"
			"float n = frac(sin(floor(P.x / 14.0) * 19.17 + P.z * 0.013) * 91.73);\n"
			"float3 tint = float3(%.3ff, %.3ff, %.3ff);\n"
			"float3 paint = lerp(tint, tint * 0.72, n);\n"
			"float peel = (n > 0.84 && frac(P.z * 0.017) > 0.55) ? 1.0 : 0.0;\n"
			"float3 bare = float3(0.34, 0.22, 0.12);\n"
			"return lerp(paint, bare, max(gap, peel) * 0.85);\n"),
			R, G, B);
	};

	CreateWorldPattern(TEXT("M_Brick"), Brick, 0.f, 0.78f, false);
	CreateWorldPattern(TEXT("M_Stone"), Stone, 0.f, 0.84f, false);
	CreateWorldPattern(TEXT("M_Siding"), Siding, 0.f, 0.62f, false);
	CreateWorldPattern(TEXT("M_Corrugated"), Corrugated, 0.78f, 0.4f, true);
	CreateWorldPattern(TEXT("M_Cinder"), Cinder, 0.f, 0.86f, false);
	CreateWorldPattern(TEXT("M_ShopFloor"), ShopFloor, 0.02f, 0.45f, false);
	CreateWorldPattern(TEXT("M_Plank"), Plank, 0.f, 0.58f, false);
	CreateWorldPattern(TEXT("M_Concrete"), Concrete, 0.02f, 0.8f, false);
	CreateWorldPattern(TEXT("M_Caliche"), Caliche, 0.f, 0.92f, false);
	CreateWorldPattern(TEXT("M_Adobe"), Adobe, 0.f, 0.88f, false);
	CreateWorldPattern(TEXT("M_Tank"), Tank, 0.72f, 0.34f, false);
	CreateWorldPattern(TEXT("M_FalseFrontRed"), Front(0.45f, 0.09f, 0.06f), 0.02f, 0.5f, false);
	CreateWorldPattern(TEXT("M_FalseFrontGreen"), Front(0.09f, 0.20f, 0.11f), 0.02f, 0.48f, false);

	UE_LOG(LogTemp, Display, TEXT("SILT COUNTY: town and shop pattern materials ready."));
}
