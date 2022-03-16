// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
 * @brief Interface for the Achievements low level C API.
 */

#pragma once

// GameKit Unreal Plugin
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryWrapper.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryUtils.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>
#include <AwsGameKitCore/Public/Core/Logging.h>

// GameKit
#if PLATFORM_IOS || PLATFORM_ANDROID
#include <aws/gamekit/achievements/exports.h>
#endif

// Standard library
#include <string>

/**
 * @brief Pointer to an instance of an Achievements class created in the imported Achievements C library.
 *
 * Most GameKit C APIs require an instance handle to be passed in.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit C libraries expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE;

/**
 * Callback pointer declarations
 */
typedef void(*FuncDispatcherResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* responseValue);

class AWSGAMEKITRUNTIME_API AwsGameKitAchievementsWrapper : public AwsGameKitLibraryWrapper
{

private:
    /**
    * Function pointer handles and declarations
    */
    DEFINE_FUNC_HANDLE(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE, GameKitAchievementsInstanceCreateWithSessionManager, (void* sessionManager, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitAchievementsInstanceRelease, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance));

    DEFINE_FUNC_HANDLE(unsigned int, GameKitListAchievements, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitUpdateAchievement, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char* achievementId, unsigned int incrementBy, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetAchievement, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char* achievementId, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetAchievementIconsBaseUrl, (GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, DISPATCH_RECEIVER_HANDLE receiver, const CharPtrCallback responseCallback));

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
     * @details Make sure to call GameKitAchievementsInstanceRelease to destroy the returned object when finished with it.
     *
     * @param sessionManager Pointer to a session manager object which manages tokens and configuration.
     * @param pluginRootPath Root path for plugin resources to read config files.
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new Achievements instance.
    */
    GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE GameKitAchievementsInstanceCreateWithSessionManager(void* sessionManager, FuncLogCallback logCb);

    /**
     * @brief Destroys the passed in achievements instance.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
    */
    virtual void GameKitAchievementsInstanceRelease(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance);

    /**
     * @brief Passes info on the current player's progress for all achievements to a callback function.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param pageSize The number of dynamo records to scan before the callback is called, max 100.
     * @param waitForAllPages Determines if all achievements should be scanned before calling the callback.
     * @param receiver Object that responseCallback is a member of.
     * @param responseCallback Callback method to write decoded JSON response with achievement info to.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitListAchievements(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, unsigned int pageSize, bool waitForAllPages, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback);

    /**
     * @brief Updates the player's progress for a specific achievement in dynamoDB.
     * @details Stateless achievements have a completion requirement of 1 increment which is the default increment value E.g. "Complete Campaign."
     * If called with an increment value of 4 on an achievement like "Eat 10 bananas," it'll move it's a previous completion rate of 3/10 to 7/10.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param achievementsId ID of achievement that will be incremented.
     * @param incrementBy How much to progress the specified achievement by.
     * @param receiver Object that responseCallback is a member of.
     * @param responseCallback Callback method to write decoded JSON response with achievement info to.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitUpdateAchievement(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char* achievementId, unsigned int incrementBy, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback);

    /**
     * @brief Passes info about the progress of a specific achievement for the current player to a callback function.
     *
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param achievementId ID of achievement that will be retrieved.
     * @param receiver Object that responseCallback is a member of.
     * @param responseCallback Callback method to write decoded JSON response with specific achievement info to.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult GameKit errors.h file for details.
    */
    virtual unsigned int GameKitGetAchievement(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const char* achievementId, DISPATCH_RECEIVER_HANDLE receiver, FuncDispatcherResponseCallback responseCallback);

    /**
     * @brief Retrieve base url for achievement icons.
     * @param achievementsInstance Pointer to GameKit::Achievements instance created with GameKitAchievementsInstanceCreateWithSessionManager()
     * @param dispatchReceiver Object that responseCallback is a member of.
     * @param responseCallback Callback method to write the results
    */
    virtual unsigned int GameKitGetAchievementIconsBaseUrl(GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE achievementsInstance, const DISPATCH_RECEIVER_HANDLE dispatchReceiver, const CharPtrCallback responseCallback);
};
