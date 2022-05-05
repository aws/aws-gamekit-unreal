// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Libraries/include"));

            // Curl
            PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Libraries/include/curl"));
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/curl/libcurl.a"));

            // OpenSSL
            PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Libraries/include/openssl"));
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/openssl/libcrypto.a"));
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/openssl/libssl.a"));

            // nghttp
            PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Libraries/include/nghttp2"));
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/nghttp2/libnghttp2.a"));

            if (Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame || Target.Configuration == UnrealTargetConfiguration.Development)
            {
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-auth.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-cal.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-common.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-compression.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-event-stream.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-http.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-io.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-mqtt.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-c-s3.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-checksums.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-apigateway.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-cloudformation.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-cognito-idp.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-core.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-lambda.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-s3.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-secretsmanager.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-ssm.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-cpp-sdk-sts.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-crt-cpp.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-achievements.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-authentication.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-core.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-game-saving.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-identity.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-user-gameplay-data.a"));

                // yaml-cpp
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/yaml-cpp/libyaml-cppd.a"));

                // boost
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/boost/libboost_filesystem.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/boost/libboost_iostreams.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/boost/libboost_regex.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/boost/libboost.a"));
            }
            else
            {
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-auth.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-cal.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-common.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-compression.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-event-stream.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-http.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-io.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-mqtt.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-c-s3.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-checksums.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-apigateway.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-cloudformation.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-cognito-idp.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-core.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-lambda.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-s3.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-secretsmanager.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-ssm.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-cpp-sdk-sts.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-crt-cpp.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-gamekit-achievements.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-gamekit-authentication.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-gamekit-core.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-gamekit-game-saving.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-gamekit-identity.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libaws-gamekit-user-gameplay-data.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/yaml-cpp/libyaml-cppd.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/boost/libboost_filesystem.a"));
            }
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "AwsGameKitCore",
                "AwsGameKitRuntime",
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
