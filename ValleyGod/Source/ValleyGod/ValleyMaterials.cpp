#include "ValleyTypes.h"
#include "Sim/ValleyLookPaths.h"
#include "Sim/ValleyPalette.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"

namespace
{
	TMap<FName, TObjectPtr<UMaterialInterface>> GRuntimeMats;
	TMap<FName, TObjectPtr<UMaterialInterface>> GRecipeMids;

	UMaterialInterface* LoadMat(const TCHAR* Path)
	{
		return LoadObject<UMaterialInterface>(nullptr, Path);
	}

	FLinearColor RecipeColor(const vg::MaterialRecipe& Recipe)
	{
		return FLinearColor(Recipe.R, Recipe.G, Recipe.B, Recipe.A);
	}

	bool HasBaseColorParam(UMaterialInterface* Mat)
	{
		if (!Mat)
		{
			return false;
		}
		TArray<FMaterialParameterInfo> Infos;
		TArray<FGuid> Ids;
		Mat->GetAllVectorParameterInfo(Infos, Ids);
		for (const FMaterialParameterInfo& Info : Infos)
		{
			if (Info.Name == TEXT("BaseColor"))
			{
				return true;
			}
		}
		return false;
	}

	void PlaceExpression(UMaterial* Mat, UMaterialExpression* Expr, int32 X, int32 Y)
	{
		if (!Mat || !Expr)
		{
			return;
		}
		Expr->Material = Mat;
#if WITH_EDITORONLY_DATA
		Expr->MaterialExpressionEditorX = X;
		Expr->MaterialExpressionEditorY = Y;
		if (UMaterialEditorOnlyData* Ed = Mat->GetEditorOnlyData())
		{
			Ed->ExpressionCollection.AddExpression(Expr);
		}
#endif
		if (UMaterialExpressionParameter* Param = Cast<UMaterialExpressionParameter>(Expr))
		{
			Param->UpdateParameterGuid(true, true);
		}
	}

	void ConnectProperty(UMaterial* Mat, UMaterialExpression* Expr, EMaterialProperty Property)
	{
#if WITH_EDITORONLY_DATA
		UMaterialEditorOnlyData* Ed = Mat ? Mat->GetEditorOnlyData() : nullptr;
		if (!Ed || !Expr)
		{
			return;
		}
		switch (Property)
		{
		case MP_BaseColor:
			Ed->BaseColor.Expression = Expr;
			Ed->BaseColor.OutputIndex = 0;
			break;
		case MP_Metallic:
			Ed->Metallic.Expression = Expr;
			Ed->Metallic.OutputIndex = 0;
			break;
		case MP_Specular:
			Ed->Specular.Expression = Expr;
			Ed->Specular.OutputIndex = 0;
			break;
		case MP_Roughness:
			Ed->Roughness.Expression = Expr;
			Ed->Roughness.OutputIndex = 0;
			break;
		case MP_EmissiveColor:
			Ed->EmissiveColor.Expression = Expr;
			Ed->EmissiveColor.OutputIndex = 0;
			break;
		case MP_Opacity:
			Ed->Opacity.Expression = Expr;
			Ed->Opacity.OutputIndex = 0;
			break;
		case MP_SubsurfaceColor:
			Ed->SubsurfaceColor.Expression = Expr;
			Ed->SubsurfaceColor.OutputIndex = 0;
			break;
		default:
			break;
		}
#else
		(void)Mat;
		(void)Expr;
		(void)Property;
#endif
	}

	UMaterialExpressionVectorParameter* AddVector(UMaterial* Mat, const TCHAR* Name, const FLinearColor& Value, int32 Y)
	{
		auto* Expr = NewObject<UMaterialExpressionVectorParameter>(Mat);
		Expr->ParameterName = Name;
		Expr->DefaultValue = Value;
		PlaceExpression(Mat, Expr, -420, Y);
		return Expr;
	}

	UMaterialExpressionScalarParameter* AddScalar(UMaterial* Mat, const TCHAR* Name, float Value, int32 Y)
	{
		auto* Expr = NewObject<UMaterialExpressionScalarParameter>(Mat);
		Expr->ParameterName = Name;
		Expr->DefaultValue = Value;
		PlaceExpression(Mat, Expr, -420, Y);
		return Expr;
	}
}

namespace Valley
{
	void PopulateLitMaterial(UMaterial* Mat, const vg::MaterialRecipe& Recipe)
	{
		if (!Mat)
		{
			return;
		}

		Mat->MaterialDomain = MD_Surface;
		Mat->TwoSided = Recipe.TwoSided;
		Mat->bUsedWithSkeletalMesh = false;
		Mat->bUsedWithParticleSprites = false;
		Mat->bUsedWithInstancedStaticMeshes = true;
		Mat->bAutomaticallySetUsageInEditor = true;

		switch (Recipe.Kind)
		{
		case vg::SurfaceKind::Translucent:
			Mat->BlendMode = BLEND_Translucent;
			Mat->SetShadingModel(MSM_DefaultLit);
			Mat->TranslucencyLightingMode = TLM_Surface;
			break;
		case vg::SurfaceKind::Subsurface:
			Mat->BlendMode = BLEND_Opaque;
			Mat->SetShadingModel(MSM_Subsurface);
			break;
		case vg::SurfaceKind::Emissive:
			Mat->BlendMode = BLEND_Opaque;
			Mat->SetShadingModel(MSM_Unlit);
			break;
		default:
			Mat->BlendMode = BLEND_Opaque;
			Mat->SetShadingModel(MSM_DefaultLit);
			break;
		}

#if WITH_EDITORONLY_DATA
		if (UMaterialEditorOnlyData* Ed = Mat->GetEditorOnlyData())
		{
			Ed->ExpressionCollection.Expressions.Empty();
			Ed->BaseColor.Expression = nullptr;
			Ed->Metallic.Expression = nullptr;
			Ed->Specular.Expression = nullptr;
			Ed->Roughness.Expression = nullptr;
			Ed->EmissiveColor.Expression = nullptr;
			Ed->Opacity.Expression = nullptr;
			Ed->SubsurfaceColor.Expression = nullptr;
		}
#endif

		const FLinearColor Color = RecipeColor(Recipe);
		ConnectProperty(Mat, AddVector(Mat, TEXT("BaseColor"), Color, 0), MP_BaseColor);
		ConnectProperty(Mat, AddScalar(Mat, TEXT("Metallic"), Recipe.Metallic, 140), MP_Metallic);
		ConnectProperty(Mat, AddScalar(Mat, TEXT("Specular"), Recipe.Specular, 220), MP_Specular);
		ConnectProperty(Mat, AddScalar(Mat, TEXT("Roughness"), Recipe.Roughness, 300), MP_Roughness);

		if (Recipe.Emissive > 0.f || Recipe.Kind == vg::SurfaceKind::Emissive)
		{
			const float Glow = Recipe.Emissive > 0.f ? Recipe.Emissive : 1.f;
			ConnectProperty(Mat, AddVector(Mat, TEXT("Emissive"), Color * Glow, 380), MP_EmissiveColor);
		}

		if (Recipe.Kind == vg::SurfaceKind::Translucent || Recipe.Kind == vg::SurfaceKind::Subsurface)
		{
			ConnectProperty(Mat, AddScalar(Mat, TEXT("Opacity"), Recipe.Opacity, 460), MP_Opacity);
		}

		if (Recipe.Kind == vg::SurfaceKind::Subsurface)
		{
			ConnectProperty(Mat, AddVector(Mat, TEXT("SubsurfaceColor"), FLinearColor(Recipe.SubR, Recipe.SubG, Recipe.SubB), 540),
				MP_SubsurfaceColor);
		}

#if WITH_EDITOR
		Mat->PreEditChange(nullptr);
		Mat->PostEditChange();
#endif
		Mat->MarkPackageDirty();
	}

	UMaterialInterface* FallbackMaterial()
	{
		if (UMaterialInterface* Mat = LoadMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
		{
			return Mat;
		}
		return UMaterial::GetDefaultMaterial(MD_Surface);
	}

	UMaterialInterface* Material(const TCHAR* ShortName)
	{
		const FName Key(ShortName);
		if (TObjectPtr<UMaterialInterface>* Found = GRuntimeMats.Find(Key))
		{
			if (Found->Get())
			{
				return Found->Get();
			}
		}

		const FString AssetPath = FString::Printf(TEXT("/Game/Materials/%s.%s"), ShortName, ShortName);
		if (UMaterialInterface* Asset = LoadMat(*AssetPath))
		{
			if (HasBaseColorParam(Asset))
			{
				GRuntimeMats.Add(Key, Asset);
				UE_LOG(LogTemp, Display, TEXT("Valley material %s from Content"), ShortName);
				return Asset;
			}
			UE_LOG(LogTemp, Warning, TEXT("Valley material %s on disk has no BaseColor; using runtime"), ShortName);
		}

		const vg::MaterialRecipe* Recipe = nullptr;
		{
			const auto Ansi = StringCast<ANSICHAR>(ShortName);
			Recipe = vg::FindMaterialRecipe(Ansi.Get());
		}
		if (!Recipe)
		{
			UE_LOG(LogTemp, Warning, TEXT("Valley material %s missing recipe; BasicShape fallback"), ShortName);
			return FallbackMaterial();
		}

		const FString RuntimeName = FString::Printf(TEXT("RT_%s"), ShortName);
		UMaterial* Mat = NewObject<UMaterial>(GetTransientPackage(), *RuntimeName, RF_Public | RF_Transient);
		PopulateLitMaterial(Mat, *Recipe);
		Mat->AddToRoot();
		GRuntimeMats.Add(Key, Mat);
		UE_LOG(LogTemp, Display, TEXT("Valley material %s from runtime fallback"), ShortName);
		return Mat;
	}

	UMaterialInterface* Tint(UObject* Outer, UMaterialInterface* Parent, const FLinearColor& Color, const FName& Name)
	{
		if (!Parent)
		{
			Parent = FallbackMaterial();
		}
		UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(Parent, Outer, Name);
		if (Dyn)
		{
			Dyn->SetVectorParameterValue(TEXT("BaseColor"), Color);
			Dyn->SetVectorParameterValue(TEXT("Color"), Color);
			return Dyn;
		}
		return Parent;
	}

	UMaterialInterface* RecipeColorParent(const TCHAR* ShortName)
	{
		const FString AssetPath = FString::Printf(TEXT("/Game/Materials/%s.%s"), ShortName, ShortName);
		if (UMaterialInterface* Asset = LoadMat(*AssetPath))
		{
			if (HasBaseColorParam(Asset))
			{
				const auto Converted = StringCast<ANSICHAR>(*Asset->GetPathName());
				if (!vg::IsBlueOrDefaultGroundPath(Converted.Get()))
				{
					return Asset;
				}
			}
		}

		const vg::MaterialRecipe* Recipe = nullptr;
		{
			const auto Ansi = StringCast<ANSICHAR>(ShortName);
			Recipe = vg::FindMaterialRecipe(Ansi.Get());
		}
		const bool bOpaque = !Recipe || Recipe->Kind == vg::SurfaceKind::Opaque;
		// Opaque ground, bark, and foliage use a compiled engine parent. An uncompiled
		// runtime UMaterial is what leaves the valley on the blue default.
		if (bOpaque)
		{
			if (UMaterialInterface* Basic = LoadMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
			{
				return Basic;
			}
		}

		if (UMaterialInterface* Runtime = Material(ShortName))
		{
			if (HasBaseColorParam(Runtime))
			{
				return Runtime;
			}
		}
		if (UMaterialInterface* Basic = LoadMat(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
		{
			return Basic;
		}
		return UMaterial::GetDefaultMaterial(MD_Surface);
	}

	UMaterialInterface* RecipeMid(UObject* Outer, const TCHAR* RecipeName, const FName& Name)
	{
		const FName Key(RecipeName ? RecipeName : TEXT("M_Dirt"));
		if (TObjectPtr<UMaterialInterface>* Found = GRecipeMids.Find(Key))
		{
			if (Found->Get())
			{
				return Found->Get();
			}
		}

		const auto Ansi = StringCast<ANSICHAR>(*Key.ToString());
		const vg::MaterialRecipe* Recipe = vg::FindMaterialRecipe(Ansi.Get());
		FLinearColor Color(vg::kGuaranteedDirtR, vg::kGuaranteedDirtG, vg::kGuaranteedDirtB, 1.f);
		float Roughness = 0.88f;
		float Specular = 0.28f;
		float Metallic = 0.f;
		if (Recipe)
		{
			Color = FLinearColor(Recipe->R, Recipe->G, Recipe->B, Recipe->A);
			Roughness = Recipe->Roughness;
			Specular = Recipe->Specular;
			Metallic = Recipe->Metallic;
		}
		if (Key == TEXT("M_Dirt"))
		{
			Color = FLinearColor(vg::kGuaranteedDirtR, vg::kGuaranteedDirtG, vg::kGuaranteedDirtB, 1.f);
		}
		else if (Key == TEXT("M_DirtWet"))
		{
			Color = FLinearColor(vg::kGuaranteedWetDirtR, vg::kGuaranteedWetDirtG, vg::kGuaranteedWetDirtB, 1.f);
		}

		UMaterialInterface* Parent = RecipeColorParent(*Key.ToString());
		const FName MidName = Name.IsNone() ? FName(*FString::Printf(TEXT("MID_%s"), *Key.ToString())) : Name;
		UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(Parent, GetTransientPackage(), MidName);
		if (!Dyn)
		{
			return Parent;
		}
		Dyn->SetVectorParameterValue(TEXT("BaseColor"), Color);
		Dyn->SetVectorParameterValue(TEXT("Color"), Color);
		Dyn->SetScalarParameterValue(TEXT("Roughness"), Roughness);
		Dyn->SetScalarParameterValue(TEXT("Specular"), Specular);
		Dyn->SetScalarParameterValue(TEXT("Metallic"), Metallic);
		Dyn->AddToRoot();
		GRecipeMids.Add(Key, Dyn);
		(void)Outer;
		UE_LOG(LogTemp, Display, TEXT("Valley MID %s BaseColor=(%.2f, %.2f, %.2f) parent=%s"),
			*Key.ToString(), Color.R, Color.G, Color.B, Parent ? *Parent->GetPathName() : TEXT("none"));
		return Dyn;
	}

	UMaterialInterface* ResolveScannedOrMid(UObject* Outer, UMaterialInterface* Scanned, const TCHAR* RecipeName, const FName& Name)
	{
		if (Scanned)
		{
			const auto Converted = StringCast<ANSICHAR>(*Scanned->GetPathName());
			if (vg::AcceptScannedGroundMaterial(Converted.Get()))
			{
				UE_LOG(LogTemp, Display, TEXT("Valley ground %s using scan %s"), RecipeName, *Scanned->GetPathName());
				return Scanned;
			}
			UE_LOG(LogTemp, Warning, TEXT("Valley ground rejected %s; forcing brown/recipe MID"), *Scanned->GetPathName());
		}
		return RecipeMid(Outer, RecipeName, Name);
	}

	void EnsureMaterials()
	{
		for (int32 I = 0; I < vg::MaterialRecipeCount(); ++I)
		{
			const FString Name = ANSI_TO_TCHAR(vg::MaterialRecipeAt(I).Name);
			Material(*Name);
		}
	}
}
