using System.IO;
using UnrealBuildTool;

public class InnoactivePortal : ModuleRules
{
	public InnoactivePortal(ReadOnlyTargetRules Target) : base(Target)
	{        
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine", "Slate", "SlateCore",
                "Blutility",
                "Projects",
                "UATHelper", "DesktopPlatform", "UnrealEd", "ProjectTargetPlatformEditor", "GameProjectGeneration", "EngineSettings", "CoreUObject", "DeveloperToolSettings",
                "ToolMenus",
                "UMGEditor",
                "DerivedDataCache",
            }
            );
    }
}
