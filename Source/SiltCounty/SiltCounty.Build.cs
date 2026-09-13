using UnrealBuildTool;

public class SiltCounty : ModuleRules
{
	public SiltCounty(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"ProceduralMeshComponent",
			"PhysicsCore",
			"RenderCore",
			"RHI",
			"UMG",
			"Slate",
			"SlateCore",
			"NetCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });
	}
}
