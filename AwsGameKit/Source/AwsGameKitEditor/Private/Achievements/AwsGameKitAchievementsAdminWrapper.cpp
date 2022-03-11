// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Achievements/AwsGameKitAchievementsAdminWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"

void AwsGameKitAchievementsAdminWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievementsAdminWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitAdminAchievementsInstanceCreateWithSessionManager, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAdminAchievementsInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAdminCredentialsChanged, loadedDllHandle);

    LOAD_PLUGIN_FUNC(GameKitAdminListAchievements, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAdminAddAchievements, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAdminDeleteAchievements, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetAchievementIconsBaseUrl, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIsAchievementIdValid, loadedDllHandle);
}

GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE AwsGameKitAchievementsAdminWrapper::GameKitAdminAchievementsInstanceCreateWithSessionManager(void* sessionManager, const char* cloudResourcesPath, const AccountCredentials accountCredentials, const AccountInfo accountInfo, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminAchievementsInstanceCreateWithSessionManager, nullptr);
    return INVOKE_FUNC(GameKitAdminAchievementsInstanceCreateWithSessionManager, sessionManager, cloudResourcesPath, accountCredentials, accountInfo, logCb);
}

void AwsGameKitAchievementsAdminWrapper::GameKitAdminAchievementsInstanceRelease(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminAchievementsInstanceRelease);
    INVOKE_FUNC(GameKitAdminAchievementsInstanceRelease, achievementsInstance);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitAdminCredentialsChanged(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials accountCredentials, const AccountInfo accountInfo)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminCredentialsChanged, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitAdminCredentialsChanged, achievementsInstance, accountCredentials, accountInfo);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitAdminListAchievements(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminListAchievements, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitAdminListAchievements, achievementsInstance, pageSize, waitForAllPages, receiver, responseCallback);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitAdminAddAchievements(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, GameKit::Achievement* achievement, unsigned int batchSize)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminAddAchievements, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitAdminAddAchievements, achievementsInstance, achievement, batchSize);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitAdminDeleteAchievements(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char** achievementIdentifiers, unsigned int batchSize)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitAdminDeleteAchievements, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitAdminDeleteAchievements, achievementsInstance, achievementIdentifiers, batchSize);
}

unsigned int AwsGameKitAchievementsAdminWrapper::GameKitGetAchievementIconsBaseUrl(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const DISPATCH_RECEIVER_HANDLE dispatchReceiver, const CharPtrCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitGetAchievementIconsBaseUrl, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitGetAchievementIconsBaseUrl, achievementsInstance, dispatchReceiver, responseCallback);
}

bool AwsGameKitAchievementsAdminWrapper::GameKitIsAchievementIdValid(const char* achievementId)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Achievements, GameKitIsAchievementIdValid, false);
    return INVOKE_FUNC(GameKitIsAchievementIdValid, achievementId);
}
