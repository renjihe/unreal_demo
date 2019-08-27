// Fill out your copyright notice in the Description page of Project Settings.

using System;
using System.IO;
using UnrealBuildTool;

public class SpGame : ModuleRules
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
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../../../new3d/include/new3d/")); }
    }

    public SpGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "RHI", "RenderCore", "GameplayAbilities", "UMG", "SlateCore", "Slate" });

        PublicIncludePaths.AddRange(
                new string[] {
                    New3dIncludePath,
                    SpLibraryPath + "include/summer",
				    // ... add public include paths required here ...
			    }
        );

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        LoadSpLibrary(Target);
        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        //bUseRTTI = true;
        //Definitions.Add("IN_UE4_ENGINE");
        Definitions.AddRange(new string[] { "HP_CLIENT", "UE_CLIENT", "WIN32", "NO_RTTI", "_CPPUNWIND" });
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
            case UnrealTargetConfiguration.Development:
                break;
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
            //PublicDelayLoadDLLs.Add("sp_common.dll");
            //RuntimeDependencies.Add(new RuntimeDependency(SpLibraryPathFull + "/sp_common.dll"));

            // CopyToBinaries cause hot-build failure, Move copying to sp_client.dllµÄPost-Build Event.
            //CopyToBinaries(SpLibraryPath + "../../bin/" + PlatformString + "/" + ConfigString + "/sp_client.dll", Target);
        }
        return isLibararySupported;
    }

    private void CopyToBinaries(string FilePath, ReadOnlyTargetRules Target)
    {
        string BinariesDir = Path.Combine(ModuleDirectory, "../../Binaries", Target.Platform.ToString());
        string FileName = Path.GetFileName(FilePath);

        if (!Directory.Exists(BinariesDir)) {
            Directory.CreateDirectory(BinariesDir);
        }
        //if (!File.Exists(Path.Combine(BinariesDir, FileName)))
        {
            File.Copy(FilePath, Path.Combine(BinariesDir, FileName), true);
        }
    }
}
