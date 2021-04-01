// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

using System;
using System.IO;
using UnrealBuildTool;

public class AwsGameKitEditor : ModuleRules
{
    public AwsGameKitEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Public/Achievements"),
                Path.Combine(ModuleDirectory, "Public/GameSaving"),
                Path.Combine(ModuleDirectory, "Public/Identity"),
                Path.Combine(ModuleDirectory, "Public/UserGameplayData"),
                Path.Combine(ModuleDirectory, "Public/Utils")
            }
        );


        PrivateIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModuleDirectory, "Private/Achievements"),
                Path.Combine(ModuleDirectory, "Private/GameSaving"),
                Path.Combine(ModuleDirectory, "Private/Identity"),
                Path.Combine(ModuleDirectory, "Private/UserGameplayData"),
                Path.Combine(ModuleDirectory, "Private/Utils")
            }
        );


        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "AwsGameKitCore",
                "AwsGameKitRuntime"
            }
        );

        if (Target.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.AddRange(
                new string[]
                {
                    "Core",
                    "CoreUObject",
                    "Engine",
                    "Json",
                    "UnrealEd",
                    "Slate",
                    "SlateCore",
                    "EditorStyle",
                    "Projects",
                    "InputCore",
                    "AwsGameKitCore",
                    "AwsGameKitRuntime",
                    "Http",
                    "ImageWrapper"
                }
            );
        }

        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
            }
        );

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // Add any libraries not part of AwsGameKit but needed by AwsGameKitRuntime
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            // Add any libraries not part of AwsGameKit but needed by AwsGameKitRuntime
        }
    }
}
