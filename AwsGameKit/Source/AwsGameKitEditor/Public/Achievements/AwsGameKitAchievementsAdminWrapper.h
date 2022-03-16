// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit Unreal Plugin
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryUtils.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryWrapper.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitMarshalling.h>

// GameKit
#if PLATFORM_IOS
#include <aws/gamekit/achievements/exports.h>
#endif
#include <aws/gamekit/achievements/gamekit_achievements_models.h>

// Standard library
#include <string>

/**
 * Handle to an instance of GameKit::Achievements created inside the GameKit.
 *
 * An instance handle must be passed to all GameKit APIs which operate on a GameKit::Achievements class instance.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit DLLs expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE;

/**
 * Callback pointer declarations
 */
typedef void(*FuncDispatcherResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* responseValue);

using namespace GameKit;

class AWSGAMEKITEDITOR_API AwsGameKitAchievementsAdminWrapper : public AwsGameKitLibraryWrapper
{

private:
    /**
    * Function pointer handles and declarations
    */
    DEFINE_FUNC_HANDLE(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE, GameKitAdminAchievementsInstanceCreateWithSessionManager, (void* sessionManager, const char* cloudResourcesPath, const AccountCredentials accountCredentials, const AccountInfo accountInfo, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitAdminAchievementsInstanceRelease, (GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAdminCredentialsChanged, (GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials accountCredentials, const AccountInfo accountInfo));

    DEFINE_FUNC_HANDLE(unsigned int, GameKitAdminListAchievements, (GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAdminAddAchievements, (GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, GameKit::Achievement* achievement, unsigned int batchSize));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAdminDeleteAchievements, (GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char** achievementIdentifiers, unsigned int batchSize));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetAchievementIconsBaseUrl, (GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, DISPATCH_RECEIVER_HANDLE receiver, const CharPtrCallback responseCallback));
    DEFINE_FUNC_HANDLE(bool, GameKitIsAchievementIdValid, (const char* achievementId));

protected:
    virtual std::string getLibraryFilename() override
    {
#if PLATFORM_WINDOWS
        return "aws-gamekit-achievements";
#elif PLATFORM_MAC
        return "libaws-gamekit-achievements";
#else
        return "";
#endif
    }

    virtual void importFunctions(void* loadedDllHandle) override;

public:
    /**
     * @brief Creates an achievements instance, which can be used to access the Achievements API.
     *
     * @details Make sure to call GameKitAdminAchievementsInstanceRelease to destroy the returned object when finished with it.
     *
     * @param sessionManager Pointer to a session manager object which manages tokens and configuration.
     * @param cloudResourcesPath Root path for plugin resources to read config files.
     * @param accountCredentials Struct containing AWS access key, secret access key, region, and accountID.
     * @param accountInfo Struct containing environment and game name
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new Achievements instance.
    */
    GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE GameKitAdminAchievementsInstanceCreateWithSessionManager(void* sessionManager, const char* cloudResourcesPath, const AccountCredentials accountCredentials, const AccountInfo accountInfo, FuncLogCallback logCb);

    /**
     * @brief Destroys the passed in achievements instance.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
    */
    virtual void GameKitAdminAchievementsInstanceRelease(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance);

    /**
     * @brief Changes the credentials used to sign requests and retrieve session tokens for admin requests.
     *
     * @param accountCredentials Struct containing Aws account credentials.
     * @param accountInfo Struct containing information about Aws account, and your game.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitAdminCredentialsChanged(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const GameKit::AccountCredentials accountCredentials, const GameKit::AccountInfo accountInfo);

    /**
     * @brief Passes all the metadata for every achievement for the current game and environment to a callback function.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param pageSize The number of dynamo records to scan before the callback is called, max 100.
     * @param waitForAllPages Determines if all achievements should be scanned before calling the callback.
     * @param receiver Object that responseCallback is a member of.
     * @param responseCallback Callback function to write decoded JSON response with achievement info to.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitAdminListAchievements(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback);

    /**
     * @brief Adds or updates the achievements table in dynamoDB for the current game and environment to have new metadata items.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param achievements Array of structs containing all the fields and values of an achievements item in dynamoDB.
     * @param batchSize The number of items that achievements contains.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitAdminAddAchievements(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, GameKit::Achievement* achievements, unsigned int batchSize);

    /**
     * @brief Deletes the achievements in the table in dynamoDB for the current game and environment specified ID's
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param achievementIdentifiers Array of achievement ID's, which is used as the partion key in dynamoDB.
     * @param batchSize The number of items achievementIdentifiers contains.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitAdminDeleteAchievements(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char** achievementIdentifiers, unsigned int batchSize);

    /**
     * @brief Retrieve base url for achievement icons.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param dispatchReceiver Object that responseCallback is a member of.
     * @param responseCallback Callback method to write the results
    */
    virtual unsigned int GameKitGetAchievementIconsBaseUrl(GAMEKIT_ADMIN_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const DISPATCH_RECEIVER_HANDLE dispatchReceiver, const CharPtrCallback responseCallback);

    /**
     * @brief Returns whether the achievement ID has invalid characters or length
     *
     * @param achievementId The ID to check.
     * @return Returns true if the achievement ID is valid else false.
    */
    virtual bool GameKitIsAchievementIdValid(const char* achievementId);
};
