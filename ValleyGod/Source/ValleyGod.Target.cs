using UnrealBuildTool;

public class ValleyGodTarget : TargetRules
{
	public ValleyGodTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("ValleyGod");
	}
}
