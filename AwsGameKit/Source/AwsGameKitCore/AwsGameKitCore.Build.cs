// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

using System;
using System.Collections.Generic;
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

        PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Libraries/include"));
        if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            // Curl
            PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Libraries/include/curl"));
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/curl/libcurl.a"));

            // nghttp
            PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Libraries/include/nghttp2"));
            PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/nghttp2/libnghttp2.a"));

            if (Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame || Target.Configuration == UnrealTargetConfiguration.Development)
            {
                // Aws SDK
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

                // GameKit
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-achievements.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-authentication.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-core.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-game-saving.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-identity.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libaws-gamekit-user-gameplay-data.a"));

                // yaml-cpp
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Debug/libyaml-cppd.a"));

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
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/libyaml-cppd.a"));
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries/IOS/Release/boost/libboost_filesystem.a"));
            }
        }

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            IEnumerable<string> libs = new List<string>
            {
                // Aws sdk
                "libaws-c-auth.a",
                "libaws-c-cal.a",
                "libaws-c-common.a",
                "libaws-c-compression.a",
                "libaws-c-event-stream.a",
                "libaws-c-http.a",
                "libaws-c-io.a",
                "libaws-c-mqtt.a",
                "libaws-c-s3.a",
                "libaws-checksums.a",
                "libaws-cpp-sdk-apigateway.a",
                "libaws-cpp-sdk-cloudformation.a",
                "libaws-cpp-sdk-cognito-idp.a",
                "libaws-cpp-sdk-core.a", 
                "libaws-cpp-sdk-lambda.a",
                "libaws-cpp-sdk-s3.a",
                "libaws-cpp-sdk-secretsmanager.a",
                "libaws-cpp-sdk-ssm.a",
                "libaws-cpp-sdk-sts.a",
                "libaws-crt-cpp.a",
                "libs2n.a",

                // SSL
                "libcrypto.a",
                "libssl.a",

                // Curl
                "libcurl.a",

                // Yaml
                "libyaml-cpp.a",

                // Boost
                "libboost_filesystem-mt-d-a32.a",
                "libboost_iostreams-mt-d-a32.a",

                // GameKit
                "libaws-gamekit-achievements.a",
                "libaws-gamekit-authentication.a",
                "libaws-gamekit-core.a",
                "libaws-gamekit-game-saving.a",
                "libaws-gamekit-identity.a",
                "libaws-gamekit-user-gameplay-data.a"
            };

            string buildFlavor = string.Empty;
            if (Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame || Target.Configuration == UnrealTargetConfiguration.Development)
            {
                buildFlavor = "Debug";
            }
            else
            {
                buildFlavor = "Release";
            }

            foreach (var lib in libs)
            {
                PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Libraries", "Android", buildFlavor, lib));
            }
        }

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
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
            if (Target.Configuration == UnrealTargetConfiguration.Debug || Target.Configuration == UnrealTargetConfiguration.DebugGame || Target.Configuration == UnrealTargetConfiguration.Development)
            {
                RuntimeDependencies.Add("$(ProjectDir)/Binaries/Mac/libaws-*.dylib", Path.Combine(PluginDirectory, "Libraries/Mac/Debug/libaws-*.dylib"));
            }
            else
            {
                RuntimeDependencies.Add("$(ProjectDir)/Binaries/Mac/libaws-*.dylib", Path.Combine(PluginDirectory, "Libraries/Mac/Release/libaws-*.dylib"));
            }
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS || Target.Platform == UnrealTargetPlatform.Android)
        {
            RuntimeDependencies.Add("$(ProjectDir)/Content/certs/*.pem", Path.Combine(PluginDirectory, "Libraries/certs/*.pem"));
        }
    }
}
