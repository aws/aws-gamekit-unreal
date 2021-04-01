// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

using System;
using System.IO;
using UnrealBuildTool;

public class AwsGameKitCore : ModuleRules
{
    public AwsGameKitCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "Public/Core")
            }
        );


        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private"),
                Path.Combine(ModuleDirectory, "Private/Core")
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "Engine"                
            }
        );


        PrivateDependencyModuleNames.AddRange(
            new string[]
            {                
            }
        );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            if (Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame || Target.Configuration == UnrealTargetConfiguration.Development)
            {
                RuntimeDependencies.Add("$(ProjectDir)/Binaries/Win64/aws-*.dll", Path.Combine(PluginDirectory, "Libraries/Win64/Debug/aws-*.dll"));
                RuntimeDependencies.Add("$(ProjectDir)/Binaries/Win64/aws-*.pdb", Path.Combine(PluginDirectory, "Libraries/Win64/Debug/aws-*.pdb"));
            }
            else
            {
                RuntimeDependencies.Add("$(ProjectDir)/Binaries/Win64/aws-*.dll", Path.Combine(PluginDirectory, "Libraries/Win64/Release/aws-*.dll"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            // TODO:: Add mac libraries
        }
    }
}
