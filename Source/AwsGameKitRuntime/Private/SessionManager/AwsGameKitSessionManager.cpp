﻿// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "SessionManager/AwsGameKitSessionManager.h"

// GameKit
#include "AwsGameKitRuntime.h"
#include "AwsGameKitCore.h"
#include "Models/AwsGameKitEnumConverter.h"

// Unreal
#include "Async/Async.h"
#include "Templates/Function.h"

SessionManagerLibrary AwsGameKitSessionManager::GetSessionManagerLibraryFromModule()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitIdentity::GetIdentityLibraryFromModule()"));
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    return runtimeModule->GetSessionManagerLibrary();
}

void AwsGameKitSessionManager::ReloadConfig()
{
    auto sessionManagerLibrary = GetSessionManagerLibraryFromModule();
    sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
}

bool AwsGameKitSessionManager::AreSettingsLoaded(FeatureType_E featureType)
{
    auto sessionManagerLibrary = GetSessionManagerLibraryFromModule();
    return sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerAreSettingsLoaded(sessionManagerLibrary.SessionManagerInstanceHandle, AwsGameKitEnumConverter::ConvertFeatureEnum(featureType));
}

FString AwsGameKitSessionManager::FeatureTypeToApiString(FeatureType_E featureType)
{
    return AwsGameKitEnumConverter::FeatureToApiString(AwsGameKitEnumConverter::ConvertFeatureEnum(featureType));
}

FString AwsGameKitSessionManager::FeatureTypeToUIString(FeatureType_E featureType)
{
   return AwsGameKitEnumConverter::FeatureToUIString(AwsGameKitEnumConverter::ConvertFeatureEnum(featureType));
}
