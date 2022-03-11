// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "Achievements/AwsGameKitAchievementsWrapper.h"
#include "Core/AwsGameKitCoreWrapper.h"
#include "Core/AwsGameKitMarshalling.h"
#include "GameSaving/AwsGameKitGameSavingWrapper.h"
#include "Identity/AwsGameKitIdentityWrapper.h"
#include "SessionManager/AwsGameKitSessionManagerWrapper.h"
#include "UserGameplayData/AwsGameKitUserGameplayDataWrapper.h"

// Unreal
#include "AwsGameKitUserGameplayDataStateHandler.h"
#include "Delegates/Delegate.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"

#include "AwsGameKitRuntime.generated.h"

struct CoreLibrary
{
    TSharedPtr<AwsGameKitCoreWrapper> CoreWrapper;
};

struct IdentityLibrary
{
    TSharedPtr<AwsGameKitIdentityWrapper> IdentityWrapper;
    GAMEKIT_IDENTITY_INSTANCE_HANDLE IdentityInstanceHandle = nullptr;
};

struct SessionManagerLibrary
{
    TSharedPtr<AwsGameKitSessionManagerWrapper> SessionManagerWrapper;
    GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE SessionManagerInstanceHandle = nullptr;
};

struct AchievementsLibrary
{
    TSharedPtr<AwsGameKitAchievementsWrapper> AchievementsWrapper;
    GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE AchievementsInstanceHandle = nullptr;
};

struct GameSavingLibrary
{
    TSharedPtr<AwsGameKitGameSavingWrapper> GameSavingWrapper;
    GAMEKIT_GAME_SAVING_INSTANCE_HANDLE GameSavingInstanceHandle = nullptr;
};

struct UserGameplayDataLibrary
{
    TSharedPtr<AwsGameKitUserGameplayDataWrapper> UserGameplayDataWrapper;
    GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE UserGameplayDataInstanceHandle = nullptr;
    TSharedPtr<AwsGameKitUserGameplayDataStateHandler> UserGameplayDataStateHandler;
};

/**
 * @brief Delegate for notifying changes in the Network status. Network can be Ok (true) or in Error state (false).
 */
UDELEGATE(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data | Network Status Change Delegate")
DECLARE_DYNAMIC_DELEGATE_TwoParams(FNetworkStatusChangeDelegate, bool, isConnectionOk, FString, connectionClient);
class AWSGAMEKITRUNTIME_API FAwsGameKitRuntimeModule : public IModuleInterface
{
private:
    CoreLibrary coreLibrary;
    SessionManagerLibrary sessionManagerLibrary;
    IdentityLibrary identityLibrary;
    AchievementsLibrary achievementsLibrary;
    GameSavingLibrary gameSavingLibrary;
    UserGameplayDataLibrary userGameplayDataLibrary;

    static FCriticalSection libLoadMutex;

    bool initializeWrappers();
    void loadIdentityLibrary();
    void loadAchievementsLibrary();
    void loadGameSavingLibrary();
    void loadUserGameplayDataLibrary();

    // Delegate that notifies other objects about network state changes
    FNetworkStatusChangeDelegate onNetworkStatusChangeDelegate;

    // This function gets invoked by the internal AwsGameKitUserGameplayDataWrapper on a network state change.
    // This calls the FNetworkStatusChangeDelegate
    void OnNetworkStatusChange(bool isConnectionOk, const char* connectionClient);

protected:

public:
    // ------ IModuleInterface implementation ------
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    // ------ SessionManager methods ------
    /**
     * @brief Get the singleton GameKitSessionManager instance residing in the DLL.
     */
    GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE GetSessionManagerInstance() const;

    /**
     * @brief Get the singleton AwsGameKitSessionManagerWrapper.
     */
    AwsGameKitSessionManagerWrapper* GetSessionManagerWrapper() const;

    /**
     * @brief See AwsGameKitSessionManagerWrapper::GameKitSessionManagerAreSettingsLoaded().
     *
     * @return True if all settings for the feature are loaded from the "awsGameKitClientConfig.yml" file.
     */
    bool AreFeatureSettingsLoaded(FeatureType type) const;

    /**
     * @brief See AwsGameKitSessionManagerWrapper::ReloadConfig().
     *
     * @return True if the "awsGameKitClientConfig.yml" was reloaded successfully, false otherwise.
     */
    bool ReloadConfigFile(const FString& subdirectory) const;

    // ------ Library Getters ------
    CoreLibrary GetCoreLibrary();
    SessionManagerLibrary GetSessionManagerLibrary();
    IdentityLibrary GetIdentityLibrary();
    AchievementsLibrary GetAchievementsLibrary();
    GameSavingLibrary GetGameSavingLibrary();
    UserGameplayDataLibrary GetUserGameplayDataLibrary();

    // Runtime delegates
    void SetNetworkChangeDelegate(const FNetworkStatusChangeDelegate& networkStatusChangeDelegate);

    // Helper dispatcher for network state change
    typedef FunctorDispatcher<void(FAwsGameKitRuntimeModule::*)(bool, const char*), &FAwsGameKitRuntimeModule::OnNetworkStatusChange> OnNetworkStatusChangeDispatcher;
};
