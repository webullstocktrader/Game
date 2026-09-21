using UnrealBuildTool;
using System.Collections.Generic;

public class SiltCountyEditorTarget : TargetRules
{
	public SiltCountyEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("SiltCounty");
	}
}
