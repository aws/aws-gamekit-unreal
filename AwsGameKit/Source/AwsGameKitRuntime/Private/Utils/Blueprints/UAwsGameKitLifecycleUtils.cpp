// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Utils/Blueprints/UAwsGameKitLifecycleUtils.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"

void UAwsGameKitLifecycleUtils::ShutdownGameKit()
{
    UE_LOG(LogAwsGameKit, Log, TEXT("UAwsGameKitLifecycleUtils::ShutdownGameKit()"));
    FAwsGameKitCoreModule* coreModule = FModuleManager::GetModulePtr<FAwsGameKitCoreModule>("AwsGameKitCore");
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    UE_LOG(LogAwsGameKit, Log, TEXT("Shutting down AwsGameKitRuntime..."));
    runtimeModule->ShutdownModule();

    UE_LOG(LogAwsGameKit, Log, TEXT("Shutting down AwsGameKitCore..."));
    coreModule->ShutdownModule();
}
