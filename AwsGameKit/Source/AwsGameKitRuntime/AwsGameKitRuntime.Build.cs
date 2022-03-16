// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

using System;
using System.IO;
using UnrealBuildTool;

public class AwsGameKitRuntime : ModuleRules
{
    public AwsGameKitRuntime(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public"),
                Path.Combine(ModuleDirectory, "Public/Achievements"),
                Path.Combine(ModuleDirectory, "Public/Common"),
                Path.Combine(ModuleDirectory, "Public/GameSaving"),
                Path.Combine(ModuleDirectory, "Public/Identity"),
                Path.Combine(ModuleDirectory, "Public/Models"),
                Path.Combine(ModuleDirectory, "Public/SessionManager"),
                Path.Combine(ModuleDirectory, "Public/UserGameplayData"),
                Path.Combine(ModuleDirectory, "Public/Utils")
            }
        );


        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private"),
                Path.Combine(ModuleDirectory, "Private/Achievements"),
                Path.Combine(ModuleDirectory, "Private/GameSaving"),
                Path.Combine(ModuleDirectory, "Private/Identity"),
                Path.Combine(ModuleDirectory, "Private/SessionManager"),
                Path.Combine(ModuleDirectory, "Private/UserGameplayData"),
                Path.Combine(ModuleDirectory, "Private/Utils")
            }
        );

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "AwsGameKitCore",
                "Engine",
                "Json"
            }
        );

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(
               new string[]
               {
                   "DesktopPlatform",
                   "EngineSettings",
                   "GameProjectGeneration",
                   "Projects"
               }
           );
       }

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Slate"
            }
        );

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );
    }
}
