/*
*   This file is part of the Unreal Body Plugin by Kaz Voeten.
*   Copyright (C) 2021 Kaz Voeten
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

using UnrealBuildTool;
using System.IO;

public class UnrealBody : ModuleRules
{
	public UnrealBody(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_1;
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
