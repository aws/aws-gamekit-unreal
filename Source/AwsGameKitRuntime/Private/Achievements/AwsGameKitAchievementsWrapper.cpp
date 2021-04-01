// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Achievements/AwsGameKitAchievementsWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"

void AwsGameKitAchievementsWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievementsWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitAchievementsInstanceCreateWithSessionManager, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAchievementsInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitListAchievements, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUpdateAchievement, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetAchievement, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetAchievementIconsBaseUrl, loadedDllHandle);
}

GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE AwsGameKitAchievementsWrapper::GameKitAchievementsInstanceCreateWithSessionManager(void* sessionManager, const char* pluginRootPath, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAchievementsInstanceCreateWithSessionManager, nullptr);
    return INVOKE_FUNC(GameKitAchievementsInstanceCreateWithSessionManager, sessionManager, pluginRootPath, logCb);
}

void AwsGameKitAchievementsWrapper::GameKitAchievementsInstanceRelease(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAchievementsInstanceRelease);
    INVOKE_FUNC(GameKitAchievementsInstanceRelease, achievementsInstance);
}

unsigned int AwsGameKitAchievementsWrapper::GameKitListAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, unsigned int pageSize, bool waitForAllPages, 
    DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitListAchievements, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitListAchievements, achievementsInstance, pageSize, waitForAllPages, receiver, responseCallback);
}

unsigned int AwsGameKitAchievementsWrapper::GameKitUpdateAchievement(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char* achievementId, unsigned int incrementBy, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitUpdateAchievement, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitUpdateAchievement, achievementsInstance, achievementId, incrementBy, receiver, responseCallback);
}

unsigned int AwsGameKitAchievementsWrapper::GameKitGetAchievement(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char* achievementId, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitGetAchievement, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitGetAchievement, achievementsInstance, achievementId, receiver, responseCallback);
}

unsigned int AwsGameKitAchievementsWrapper::GameKitGetAchievementIconsBaseUrl(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const DISPATCH_RECEIVER_HANDLE dispatchReceiver, const CharPtrCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitGetAchievementIconsBaseUrl, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitGetAchievementIconsBaseUrl, achievementsInstance, dispatchReceiver, responseCallback);
}
