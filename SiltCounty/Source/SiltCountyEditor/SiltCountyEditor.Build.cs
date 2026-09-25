using UnrealBuildTool;

public class SiltCountyEditor : ModuleRules
{
	public SiltCountyEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"UnrealEd",
			"MaterialEditor",
			"AssetRegistry"
		});
	}
}
