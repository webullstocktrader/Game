using UnrealBuildTool;

public class SiltCountyEditor : ModuleRules
{
	public SiltCountyEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"SiltCounty",
			"UnrealEd",
			"MaterialEditor",
			"AssetTools",
			"AssetRegistry",
			"EditorFramework"
		});
	}
}
