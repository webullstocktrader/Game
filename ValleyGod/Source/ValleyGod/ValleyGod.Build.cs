using UnrealBuildTool;

public class ValleyGod : ModuleRules
{
	public ValleyGod(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(ModuleDirectory);
		// Force this module to rebuild if Material() moved between cpp files.
		// Stale unity/incremental ValleyTypes.cpp.obj otherwise LNK2005s against ValleyMaterials.cpp.
		PrivateDefinitions.Add("VALLEYGOD_MATERIAL_SPLIT=1");

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ProceduralMeshComponent",
			"RenderCore",
			"RHI",
			"UMG",
			"Slate",
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
