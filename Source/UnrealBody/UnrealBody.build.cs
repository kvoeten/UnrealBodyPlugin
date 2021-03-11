// Some copyright should be here...

using UnrealBuildTool;
using System.IO;

public class UnrealBody : ModuleRules
{
	public UnrealBody(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
        bEnableExceptions = true;
		
        PublicIncludePaths.AddRange(
			new string[] { 
				Path.Combine(ModuleDirectory, "Public")
			}
		);

        PrivateIncludePaths.AddRange(
			new string[] { 
				Path.Combine(ModuleDirectory, "Private")
			}
		);
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
                "Core",
                "CoreUObject",
                "Sockets",
                "Networking",
                "Engine",
                "InputCore"
			}
		);

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(
            new string[] {
                "UnrealEd",
                "EditorStyle",
                "DesktopWidgets",
                "DesktopPlatform"
            });
        }


        PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore"
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				
			}
		);
    }
}
