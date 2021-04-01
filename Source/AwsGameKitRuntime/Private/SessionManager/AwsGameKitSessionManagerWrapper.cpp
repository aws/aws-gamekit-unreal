// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "SessionManager/AwsGameKitSessionManagerWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"

// Unreal
#include "HAL/FileManagerGeneric.h"

#if UE_BUILD_DEVELOPMENT && WITH_EDITOR
DEFINE_LOG_CATEGORY(LogAwsGameKit);
#endif

static const FString ClientConfigFile = "awsGameKitClientConfig.yml";

void AwsGameKitSessionManagerWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitSessionManagerWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitSessionManagerInstanceCreate, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSessionManagerInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSessionManagerAreSettingsLoaded, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSessionManagerReloadConfigFile, loadedDllHandle);
}

#if UE_BUILD_DEVELOPMENT && WITH_EDITOR
void AwsGameKitSessionManagerWrapper::ReloadConfig(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance, const FString& subfolder)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitSessionManagerWrapper::ReloadConfig(%s)"), *subfolder);
    FFileManagerGeneric fileManager;
    FString src = FPaths::ConvertRelativePathToFull(FPaths::GameSourceDir().Replace(TEXT("source/"), ToCStr(subfolder + ClientConfigFile), ESearchCase::IgnoreCase));
    FString dest = FPaths::Combine(FPaths::ProjectContentDir(), TEXT("GameKitConfig"), ClientConfigFile);
    uint32 result = fileManager.Copy(ToCStr(dest), ToCStr(src), true);
    
    if (result != COPY_OK)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("Error copying config, result code: %lu"), result);
        this->GameKitSessionManagerReloadConfigFile(sessionManagerInstance, "");
    }
    else
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("Copied config from %s to %s"), *src, *dest);
        this->GameKitSessionManagerReloadConfigFile(sessionManagerInstance, TCHAR_TO_UTF8(dest.GetCharArray().GetData()));
    }
}
#endif

void AwsGameKitSessionManagerWrapper::ReloadConfig(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitSessionManagerWrapper::ReloadConfig()"));
    TArray<FString> results;
    FFileManagerGeneric fileManager;

#if UE_BUILD_DEVELOPMENT && WITH_EDITOR
    FString searchPath = FPaths::GameSourceDir().Replace(TEXT("source/"), TEXT(""), ESearchCase::IgnoreCase);
#else
    FString searchPath = FPaths::LaunchDir();
#endif

    fileManager.FindFilesRecursive(results, ToCStr(searchPath), ToCStr(ClientConfigFile), true, false, true);
    if (results.Num() > 0)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("Loading config from %s"), *results[0]);
        this->GameKitSessionManagerReloadConfigFile(sessionManagerInstance, TCHAR_TO_UTF8(results[0].GetCharArray().GetData()));
    }
}

GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE AwsGameKitSessionManagerWrapper::GameKitSessionManagerInstanceCreate(const char* clientConfigFile, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerInstanceCreate, nullptr);

    return INVOKE_FUNC(GameKitSessionManagerInstanceCreate, clientConfigFile, logCb);
}

void AwsGameKitSessionManagerWrapper::GameKitSessionManagerInstanceRelease(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerInstanceRelease);

    INVOKE_FUNC(GameKitSessionManagerInstanceRelease, sessionManagerInstance);
}

bool AwsGameKitSessionManagerWrapper::GameKitSessionManagerAreSettingsLoaded(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance, FeatureType featureType)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerInstanceCreate, false);

    return INVOKE_FUNC(GameKitSessionManagerAreSettingsLoaded, sessionManagerInstance, featureType);
}

void AwsGameKitSessionManagerWrapper::GameKitSessionManagerReloadConfigFile(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance, const char* clientConfigFile)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerInstanceCreate);

    INVOKE_FUNC(GameKitSessionManagerReloadConfigFile, sessionManagerInstance, clientConfigFile);
}


#undef LOCTEXT_NAMESPACE
