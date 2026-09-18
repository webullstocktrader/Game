using UnrealBuildTool;

public class ValleyGodEditor : ModuleRules
{
	public ValleyGodEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"ValleyGod",
			"UnrealEd",
			"MaterialEditor",
			"AssetTools",
			"AssetRegistry",
			"EditorFramework"
		});
		PrivateIncludePathModuleNames.Add("ValleyGod");
	}
}
