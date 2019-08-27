// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

[SupportedPlatforms(UnrealPlatformClass.Server)]
public class SpGameServerTarget : TargetRules    // Change this line of code as shown previously
{
    public SpGameServerTarget(TargetInfo Target) : base(Target) // Change this line of code as shown previously
    {
        Type = TargetType.Server;
        ExtraModuleNames.Add("SpGame");   // Change this line of code as shown previously
        //ExtraModuleNames.AddRange( new string[] { "SpGame" } );
    }
}