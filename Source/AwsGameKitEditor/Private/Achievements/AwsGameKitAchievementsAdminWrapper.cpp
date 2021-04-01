// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Achievements/AwsGameKitAchievementsAdminWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"

void AwsGameKitAchievementsAdminWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievementsAdminWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitAchievementsInstanceCreateWithSessionManager, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAchievementsInstanceRelease, loadedDllHandle);

    LOAD_PLUGIN_FUNC(GameKitAdminListAchievements, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAdminAddAchievements, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAdminDeleteAchievements, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetAchievementIconsBaseUrl, loadedDllHandle);
}

GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE AwsGameKitAchievementsAdminWrapper::GameKitAchievementsInstanceCreateWithSessionManager(void* sessionManager, const char* pluginRootPath, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAchievementsInstanceCreateWithSessionManager, nullptr);
    return INVOKE_FUNC(GameKitAchievementsInstanceCreateWithSessionManager, sessionManager, pluginRootPath, logCb);
}

void AwsGameKitAchievementsAdminWrapper::GameKitAchievementsInstanceRelease(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAchievementsInstanceRelease);
    INVOKE_FUNC(GameKitAchievementsInstanceRelease, achievementsInstance);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitAdminListAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, 
    const AccountInfo* accountInfo, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminListAchievements, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitAdminListAchievements, achievementsInstance, accountCredentials, accountInfo, pageSize, waitForAllPages, receiver, responseCallback);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitAdminAddAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, Achievement* achievement, unsigned int batchSize)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminAddAchievements, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitAdminAddAchievements, achievementsInstance, accountCredentials, accountInfo, achievement, batchSize);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitAdminDeleteAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, const char** achievementIdentifiers, unsigned int batchSize)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminDeleteAchievements, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitAdminDeleteAchievements, achievementsInstance, accountCredentials, accountInfo, achievementIdentifiers, batchSize);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitGetAchievementIconsBaseUrl(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const DISPATCH_RECEIVER_HANDLE dispatchReceiver, const CharPtrCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitGetAchievementIconsBaseUrl, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitGetAchievementIconsBaseUrl, achievementsInstance, dispatchReceiver, responseCallback);
}
