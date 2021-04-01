// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryUtils.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryWrapper.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitMarshalling.h>

// Standard library
#include <string>

/**
 * Structs/Enums
 */

struct Achievement
{
    const char* achievementId;
    const char* title;
    const char* lockedDescription;
    const char* unlockedDescription;
    const char* lockedIcon;
    const char* unlockedIcon;

    unsigned int requiredAmount;
    unsigned int points;
    unsigned int sortOrder;

    bool isStateful;
    bool isSecret;
    bool isHidden;
};

/**
 * Handle to an instance of GameKit::Achievements created inside the GameKit.
 *
 * An instance handle must be passed to all GameKit APIs which operate on a GameKit::Achievements class instance.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit DLLs expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE;

/**
 * Callback pointer declarations
 */
typedef void(*FuncDispatcherResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* responseValue);

class AWSGAMEKITEDITOR_API AwsGameKitAchievementsAdminWrapper : public AwsGameKitLibraryWrapper
{

private:
    /**
    * Function pointer handles and declarations
    */
    DEFINE_FUNC_HANDLE(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE, GameKitAchievementsInstanceCreateWithSessionManager, (void* sessionManager, const char* pluginRootPath, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitAchievementsInstanceRelease, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance));

    DEFINE_FUNC_HANDLE(unsigned int, GameKitAdminListAchievements, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAdminAddAchievements, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, Achievement* achievement, unsigned int batchSize));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAdminDeleteAchievements, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, const char** achievementIdentifiers, unsigned int batchSize));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetAchievementIconsBaseUrl, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, DISPATCH_RECEIVER_HANDLE receiver, const CharPtrCallback responseCallback));

protected:
    virtual std::string getLibraryFilename() override
    {
        return "aws-gamekit-achievements";
    }

    virtual void importFunctions(void* loadedDllHandle) override;

public:
    /**
     * @brief Creates an achievements instance, which can be used to access the Achievements API.
     *
     * @details Make sure to call GameKitAchievementsInstanceRelease to destroy the returned object when finished with it.
     *
     * @param sessionManager Pointer to a session manager object which manages tokens and configuration.
     * @param pluginRootPath Root path for plugin resources to read config files.
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new Achievements instance.
    */
    GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE GameKitAchievementsInstanceCreateWithSessionManager(void* sessionManager, const char* pluginRootPath, FuncLogCallback logCb);

    /**
     * @brief Destroys the passed in achievements instance.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
    */
    virtual void GameKitAchievementsInstanceRelease(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance);

    /**
     * @brief Passes all the metadata for every achievement for the current game and environment to a callback function.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param accountCredentials Struct containing AWS access key, secret access key, region, and accountID.
     * @param accountInfo Struct containing environment and game name
     * @param pageSize The number of dynamo records to scan before the callback is called, max 100.
     * @param waitForAllPages Determines if all achievements should be scanned before calling the callback.
     * @param receiver Object that responseCallback is a member of.
     * @param responseCallback Callback function to write decoded JSON response with achievement info to.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitAdminListAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback);

    /**
     * @brief Adds or updates the achievements table in dynamoDB for the current game and environment to have new metadata items.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param accountCredentials Struct containing AWS access key, secret access key, region, and accountID.
     * @param accountInfo Struct containing environment and game name
     * @param achievements Array of structs containing all the fields and values of an achievements item in dynamoDB.
     * @param batchSize The number of items that achievements contains.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitAdminAddAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, Achievement* achievements, unsigned int batchSize);

    /**
     * @brief Deletes the achievements in the table in dynamoDB for the current game and environment specified ID's
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param accountCredentials Struct containing AWS access key, secret access key, region, and accountID.
     * @param accountInfo Struct containing environment and game name
     * @param achievementIdentifiers Array of achievement ID's, which is used as the partion key in dynamoDB.
     * @param batchSize The number of items achievementIdentifiers contains.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitAdminDeleteAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const AccountCredentials* accountCredentials, const AccountInfo* accountInfo, const char** achievementIdentifiers, unsigned int batchSize);

    /**
     * @brief Retrieve base url for achievement icons.
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param dispatchReceiver Object that responseCallback is a member of.
     * @param responseCallback Callback method to write the results
    */
    virtual unsigned int GameKitGetAchievementIconsBaseUrl(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const DISPATCH_RECEIVER_HANDLE dispatchReceiver, const CharPtrCallback responseCallback);
};