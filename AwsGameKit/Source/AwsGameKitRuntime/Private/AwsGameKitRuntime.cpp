// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitRuntime.h"

// GameKit
#include "AwsGameKitCore.h"
#if WITH_EDITOR
#include "AwsGameKitEditor/Public/AwsGameKitEditor.h"
#endif

// Unreal
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Misc/MessageDialog.h"

// Unreal public module dependency
#if WITH_EDITOR
#include "DesktopPlatformModule.h"
#include "GameProjectUtils.h"
#include "GeneralProjectSettings.h"
#include "ModuleDescriptor.h"
#endif
#include "Framework/Application/SlateApplication.h"

#define LOCTEXT_NAMESPACE "FAwsGameKitSessionManager"

FCriticalSection FAwsGameKitRuntimeModule::libLoadMutex;

void FAwsGameKitRuntimeModule::StartupModule()
{
    UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::StartupModule()"));
    const bool wrappersInitialized = initializeWrappers();

    // Starts the SessionManager with an empty configuration file.
    // The configuration file can be reloaded by calling AwsGameKitSessionManagerWrapper::ReloadConfigFile()
    sessionManagerLibrary.SessionManagerInstanceHandle = sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerInstanceCreate(nullptr, FGameKitLogging::LogCallBack);

#if PLATFORM_ANDROID || PLATFORM_IOS
    ReloadConfigFile(""); // Mobile platforms have logic to determine the path in the device file system
#elif UE_BUILD_SHIPPING || !WITH_EDITOR
    ReloadConfigFile(FPaths::LaunchDir());
#endif
}

void FAwsGameKitRuntimeModule::ShutdownModule()
{
    // Unload AWS GameKitSession Manager Library
    UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule()"));

    // Calling Shutdown() on this module gives exceptions after the editor is closed.

    if (identityLibrary.IdentityWrapper != nullptr)
    {
        UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule(): Releasing Identity Library"));
        identityLibrary.IdentityWrapper->GameKitIdentityInstanceRelease(identityLibrary.IdentityInstanceHandle);
        identityLibrary.IdentityWrapper = nullptr;
    }

    if (achievementsLibrary.AchievementsWrapper != nullptr)
    {
        UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule(): Releasing Achievements Library"));
        achievementsLibrary.AchievementsWrapper->GameKitAchievementsInstanceRelease(achievementsLibrary.AchievementsInstanceHandle);
        achievementsLibrary.AchievementsWrapper = nullptr;
    }

    if (gameSavingLibrary.GameSavingWrapper != nullptr)
    {
        UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule(): Releasing Game Saving Library"));
        gameSavingLibrary.GameSavingWrapper->GameKitGameSavingInstanceRelease(gameSavingLibrary.GameSavingInstanceHandle);
        gameSavingLibrary.GameSavingWrapper  = nullptr;
    }

    if (userGameplayDataLibrary.UserGameplayDataWrapper != nullptr)
    {
        UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule(): Releasing User Gameplay Data Library"));
        userGameplayDataLibrary.UserGameplayDataWrapper->GameKitUserGameplayDataInstanceRelease(userGameplayDataLibrary.UserGameplayDataInstanceHandle);
        userGameplayDataLibrary.UserGameplayDataWrapper = nullptr;
        userGameplayDataLibrary.UserGameplayDataStateHandler = nullptr;
    }

    if (coreLibrary.CoreWrapper != nullptr)
    {
        UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule(): Releasing Core Library"));
        coreLibrary.CoreWrapper = nullptr;
    }

    if (sessionManagerLibrary.SessionManagerWrapper != nullptr)
    {
        UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule(): Releasing SessionManager Library"));
        sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerInstanceRelease(sessionManagerLibrary.SessionManagerInstanceHandle);
        sessionManagerLibrary.SessionManagerWrapper = nullptr;
    }
}

bool FAwsGameKitRuntimeModule::AreFeatureSettingsLoaded(FeatureType type) const
{
    bool loaded = sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerAreSettingsLoaded(sessionManagerLibrary.SessionManagerInstanceHandle, type);
    return loaded;
}

GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE FAwsGameKitRuntimeModule::GetSessionManagerInstance() const
{
    return sessionManagerLibrary.SessionManagerInstanceHandle;
}

AwsGameKitSessionManagerWrapper* FAwsGameKitRuntimeModule::GetSessionManagerWrapper() const
{
    return sessionManagerLibrary.SessionManagerWrapper.Get();
}

bool FAwsGameKitRuntimeModule::ReloadConfigFile(const FString& subdirectory) const
{
#if WITH_EDITOR
    this->sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle, subdirectory);
#else
    this->sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
#endif
    return
        AreFeatureSettingsLoaded(FeatureType::Identity) &&
        AreFeatureSettingsLoaded(FeatureType::Achievements) &&
        AreFeatureSettingsLoaded(FeatureType::UserGameplayData) &&
        AreFeatureSettingsLoaded(FeatureType::GameStateCloudSaving);
}

CoreLibrary FAwsGameKitRuntimeModule::GetCoreLibrary()
{
    return coreLibrary;
}

SessionManagerLibrary FAwsGameKitRuntimeModule::GetSessionManagerLibrary()
{
    return sessionManagerLibrary;
}

IdentityLibrary FAwsGameKitRuntimeModule::GetIdentityLibrary()
{
    loadIdentityLibrary();
    return identityLibrary;
}

AchievementsLibrary FAwsGameKitRuntimeModule::GetAchievementsLibrary()
{
    loadAchievementsLibrary();
    return achievementsLibrary;
}

GameSavingLibrary FAwsGameKitRuntimeModule::GetGameSavingLibrary()
{
    loadGameSavingLibrary();
    return gameSavingLibrary;
}

UserGameplayDataLibrary FAwsGameKitRuntimeModule::GetUserGameplayDataLibrary()
{
    loadUserGameplayDataLibrary();
    return userGameplayDataLibrary;
}

void FAwsGameKitRuntimeModule::SetNetworkChangeDelegate(const FNetworkStatusChangeDelegate& networkStatusChangeDelegate)
{
    if (networkStatusChangeDelegate.IsBound())
    {
        this->onNetworkStatusChangeDelegate = networkStatusChangeDelegate;
    }
}

bool FAwsGameKitRuntimeModule::initializeWrappers()
{
    // Load AWS GameKit Core Library
    coreLibrary.CoreWrapper = MakeShareable(new AwsGameKitCoreWrapper());
    coreLibrary.CoreWrapper->Initialize();

    // Load AWS GameKit SessionManager Library
    sessionManagerLibrary.SessionManagerWrapper = MakeShareable(new AwsGameKitSessionManagerWrapper());
    return sessionManagerLibrary.SessionManagerWrapper->Initialize();
}

void FAwsGameKitRuntimeModule::loadIdentityLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (identityLibrary.IdentityWrapper == nullptr)
    {
        identityLibrary.IdentityWrapper = MakeShareable(new AwsGameKitIdentityWrapper());
        identityLibrary.IdentityWrapper->Initialize();

        identityLibrary.IdentityInstanceHandle = identityLibrary.IdentityWrapper->GameKitIdentityInstanceCreateWithSessionManager(GetSessionManagerInstance(), FGameKitLogging::LogCallBack);
    }
}

void FAwsGameKitRuntimeModule::loadAchievementsLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (achievementsLibrary.AchievementsWrapper == nullptr)
    {
        achievementsLibrary.AchievementsWrapper = MakeShareable(new AwsGameKitAchievementsWrapper());
        achievementsLibrary.AchievementsWrapper->Initialize();

        achievementsLibrary.AchievementsInstanceHandle = achievementsLibrary.AchievementsWrapper->GameKitAchievementsInstanceCreateWithSessionManager(GetSessionManagerInstance(), FGameKitLogging::LogCallBack);
    }
}

void FAwsGameKitRuntimeModule::loadGameSavingLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (gameSavingLibrary.GameSavingWrapper == nullptr)
    {
        gameSavingLibrary.GameSavingWrapper = MakeShareable(new AwsGameKitGameSavingWrapper());
        gameSavingLibrary.GameSavingWrapper->Initialize();

        gameSavingLibrary.GameSavingInstanceHandle = gameSavingLibrary.GameSavingWrapper->GameKitGameSavingInstanceCreateWithSessionManager(GetSessionManagerInstance(), FGameKitLogging::LogCallBack, nullptr, 0, DefaultFileActions());
    }
}

void FAwsGameKitRuntimeModule::loadUserGameplayDataLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (userGameplayDataLibrary.UserGameplayDataWrapper == nullptr)
    {
        userGameplayDataLibrary.UserGameplayDataWrapper = MakeShareable(new AwsGameKitUserGameplayDataWrapper());
        userGameplayDataLibrary.UserGameplayDataWrapper->Initialize();

        userGameplayDataLibrary.UserGameplayDataInstanceHandle = userGameplayDataLibrary.UserGameplayDataWrapper->GameKitUserGameplayDataInstanceCreateWithSessionManager(GetSessionManagerInstance(), FGameKitLogging::LogCallBack);
    }

    if (userGameplayDataLibrary.UserGameplayDataStateHandler == nullptr)
    {
        userGameplayDataLibrary.UserGameplayDataStateHandler = MakeShareable(new AwsGameKitUserGameplayDataStateHandler());
    }
}

void FAwsGameKitRuntimeModule::OnNetworkStatusChange(bool isConnectionOk, const char* connectionClient)
{
    FString client(connectionClient);
    AsyncTask(ENamedThreads::GameThread, [this, isConnectionOk, client]()
    {
        this->onNetworkStatusChangeDelegate.ExecuteIfBound(isConnectionOk, client);
    });
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE(FAwsGameKitRuntimeModule, AwsGameKitRuntime);
