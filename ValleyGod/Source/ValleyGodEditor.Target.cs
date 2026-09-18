using UnrealBuildTool;

public class ValleyGodEditorTarget : TargetRules
{
	public ValleyGodEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new[] { "ValleyGod", "ValleyGodEditor" });
	}
}
