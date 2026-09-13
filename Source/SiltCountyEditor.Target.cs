using UnrealBuildTool;

public class SiltCountyEditorTarget : TargetRules
{
	public SiltCountyEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new[] { "SiltCounty", "SiltCountyEditor" });
	}
}
