using UnrealBuildTool;
using System.Collections.Generic;

public class SiltCountyTarget : TargetRules
{
	public SiltCountyTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("SiltCounty");
	}
}
