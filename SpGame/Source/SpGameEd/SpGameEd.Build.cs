// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.IO;
using UnrealBuildTool;

public class SpGameEd : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string SpLibraryPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../../sp_p/")); }
    }

    private string New3dIncludePath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../../../new3d/")); }
    }

    public SpGameEd(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore", "UnrealEd", "LevelEditor", "Slate", "SlateCore", "EditorStyle", "AssetTools" });
        PrivateDependencyModuleNames.AddRange(new string[] { "SpGame" });

        PublicIncludePaths.AddRange(
                new string[] {
                    New3dIncludePath,
                    SpLibraryPath + "include/summer",
				    // ... add public include paths required here ...
			    }
        );

        LoadSpLibrary(Target);
		
        Definitions.AddRange(new string[] { "HP_CLIENT", "UE_CLIENT", "WIN32", "NO_RTTI" });
    }

    public bool LoadSpLibrary(ReadOnlyTargetRules Target)
    {
        string ConfigString = "Release";
        UnrealTargetConfiguration CheckConfig = Target.Configuration;
        switch (CheckConfig)
        {
            default:
            case UnrealTargetConfiguration.Debug:
            case UnrealTargetConfiguration.DebugGame:
            case UnrealTargetConfiguration.Test:
                break;
            case UnrealTargetConfiguration.Development:
            case UnrealTargetConfiguration.Shipping:
                ConfigString = "Release_optimize";
                break;
        }

        bool isLibararySupported = false;
        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            isLibararySupported = true;
            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "x64" : "Win32";
            string SpLibraryPathFull = SpLibraryPath + "../../lib/" + PlatformString + "/" + ConfigString;
            PublicLibraryPaths.Add(SpLibraryPathFull);
            PublicAdditionalLibraries.Add(SpLibraryPathFull + "/sp_client.lib");
        }
        return isLibararySupported;
    }
}
