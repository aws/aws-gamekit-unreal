// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "Slate.h"

#define LOCTEXT_NAMESPACE "FAwsGameKitSessionManager"

FCriticalSection FAwsGameKitRuntimeModule::libLoadMutex;

void FAwsGameKitRuntimeModule::StartupModule()
{
    // Load AWS GameKitSession Manager Library
    UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::StartupModule()"));

    coreLibrary.CoreWrapper = MakeShareable(new AwsGameKitCoreWrapper());
    coreLibrary.CoreWrapper->Initialize();

    sessionManagerLibrary.SessionManagerWrapper = MakeShareable(new AwsGameKitSessionManagerWrapper());
    bool wrapperInitialized = sessionManagerLibrary.SessionManagerWrapper->Initialize();

#if WITH_EDITOR
    if (!wrapperInitialized)
    {
        FText title = FText::FromString("AWS GameKit");
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("You need to rebuild your Unreal project in Visual Studio before using AWS GameKit."), &title);
        FGenericPlatformMisc::RequestExit(true);
        return;
    }
#endif

    // Starts the SessionManager with an empty configuration file.
    // The configuration file can be reloaded by calling AwsGameKitSessionManagerWrapper::ReloadConfigFile()
    sessionManagerLibrary.SessionManagerInstanceHandle = sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerInstanceCreate(nullptr, Logging::LogCallBack);

#if UE_BUILD_SHIPPING
    ReloadConfigFile(FPaths::LaunchDir());
#endif
}

void FAwsGameKitRuntimeModule::ShutdownModule()
{
    // Unload AWS GameKitSession Manager Library
    UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitRuntimeModule::ShutdownModule()"));
    sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerInstanceRelease(sessionManagerLibrary.SessionManagerInstanceHandle);

    // Calling Shutdown() on this module gives exceptions after the editor is closed.

    coreLibrary.CoreWrapper.Reset();
    coreLibrary.CoreWrapper = nullptr;

    sessionManagerLibrary.SessionManagerWrapper.Reset();
    sessionManagerLibrary.SessionManagerWrapper = nullptr;
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

void FAwsGameKitRuntimeModule::loadIdentityLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (identityLibrary.IdentityWrapper == nullptr)
    {
        identityLibrary.IdentityWrapper = MakeShareable(new AwsGameKitIdentityWrapper());
        identityLibrary.IdentityWrapper->Initialize();

        identityLibrary.IdentityInstanceHandle = identityLibrary.IdentityWrapper->GameKitIdentityInstanceCreateWithSessionManager(GetSessionManagerInstance(), Logging::LogCallBack);
    }
}

void FAwsGameKitRuntimeModule::loadAchievementsLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (achievementsLibrary.AchievementsWrapper == nullptr)
    {
        achievementsLibrary.AchievementsWrapper = MakeShareable(new AwsGameKitAchievementsWrapper());
        achievementsLibrary.AchievementsWrapper->Initialize();

        achievementsLibrary.AchievementsInstanceHandle = achievementsLibrary.AchievementsWrapper->GameKitAchievementsInstanceCreateWithSessionManager(GetSessionManagerInstance(), "", Logging::LogCallBack);
    }
}

void FAwsGameKitRuntimeModule::loadGameSavingLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (gameSavingLibrary.GameSavingWrapper == nullptr)
    {
        gameSavingLibrary.GameSavingWrapper = MakeShareable(new AwsGameKitGameSavingWrapper());
        gameSavingLibrary.GameSavingWrapper->Initialize();

        gameSavingLibrary.GameSavingInstanceHandle = gameSavingLibrary.GameSavingWrapper->GameKitGameSavingInstanceCreateWithSessionManager(GetSessionManagerInstance(), Logging::LogCallBack, nullptr, 0, DefaultFileActions());
    }
}

void FAwsGameKitRuntimeModule::loadUserGameplayDataLibrary()
{
    FScopeLock scopeLock(&libLoadMutex);
    if (userGameplayDataLibrary.UserGameplayDataWrapper == nullptr)
    {
        userGameplayDataLibrary.UserGameplayDataWrapper = MakeShareable(new AwsGameKitUserGameplayDataWrapper());
        userGameplayDataLibrary.UserGameplayDataWrapper->Initialize();

        userGameplayDataLibrary.UserGameplayDataInstanceHandle = userGameplayDataLibrary.UserGameplayDataWrapper->GameKitUserGameplayDataInstanceCreateWithSessionManager(GetSessionManagerInstance(), Logging::LogCallBack);
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

IMPLEMENT_GAME_MODULE(FAwsGameKitRuntimeModule, AwsGameKitRuntime)
