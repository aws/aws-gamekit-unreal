// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "SessionManager/AwsGameKitSessionManagerWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"

// Unreal
#include "HAL/FileManagerGeneric.h"
#include "Misc/App.h"
#if PLATFORM_IOS
#include "IOS/IOSPlatformFile.h"
#endif
#if PLATFORM_ANDROID
#include "Android/AndroidPlatformFile.h"
#endif

#if UE_BUILD_DEVELOPMENT && WITH_EDITOR
DEFINE_LOG_CATEGORY(LogAwsGameKit);
#endif

static const FString ClientConfigFile = "awsGameKitClientConfig.yml";

AwsGameKitSessionManagerWrapper::~AwsGameKitSessionManagerWrapper()
{
    Shutdown();
}
void AwsGameKitSessionManagerWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitSessionManagerWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitSessionManagerInstanceCreate, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSessionManagerInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSessionManagerAreSettingsLoaded, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSessionManagerReloadConfigFile, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSessionManagerSetToken, loadedDllHandle);
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
#elif PLATFORM_ANDROID
    // Paths to config and cert are different depending on the deployment method. 
    // Packaged games that are installed should have the config in ProjectContentDir and the CA Certificate bundled as an asset.
    FString contentDir = FPaths::ProjectContentDir();
    FString searchPath = contentDir;
#elif UE_BUILD_SHIPPING
    FString searchPath = FPaths::ProjectDir();
#elif PLATFORM_MAC
    FString searchPath = FPaths::ProjectContentDir();
#else
    FString searchPath = FPaths::LaunchDir();
#endif

    FString clientConfigFileToSearch = ClientConfigFile;
#if PLATFORM_IOS
    searchPath += FString(FApp::GetProjectName()).ToLower();
    clientConfigFileToSearch = clientConfigFileToSearch.ToLower();
#endif

    UE_LOG(LogAwsGameKit, Display, TEXT("Searching for config %s recursively starting at %s"), *clientConfigFileToSearch, *searchPath);
    fileManager.FindFilesRecursive(results, ToCStr(searchPath), ToCStr(clientConfigFileToSearch), true, false, true);
    if (results.Num() > 0)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("Loading config from %s"), *results[0]);
#if PLATFORM_WINDOWS || PLATFORM_MAC
        this->GameKitSessionManagerReloadConfigFile(sessionManagerInstance, TCHAR_TO_UTF8(*results[0]));
#elif PLATFORM_ANDROID
        FString configFileContents;
        if (FFileHelper::LoadFileToString(configFileContents, *results[0], FFileHelper::EHashOptions::None))
        {
            // Copy CA Cert asset to a readable path and add it to settings
            FString caCertPath = FString(searchPath + "certs/cacert.pem"); // searchPath ends in foo/content/
            FString savePath = FPaths::ProjectSavedDir() + "Config/cacert.pem";
            FString saveAndroidFilePath = IAndroidPlatformFile::GetPlatformPhysical().ConvertToAbsolutePathForExternalAppForWrite(*savePath);
            if (IAndroidPlatformFile::GetPlatformPhysical().CopyFile(*saveAndroidFilePath, *caCertPath))
            {
                FString caCertAndroidFilePath = IAndroidPlatformFile::GetPlatformPhysical().ConvertToAbsolutePathForExternalAppForRead(*caCertPath);
                configFileContents.Append("\n").Append("ca_cert_file: ").Append(saveAndroidFilePath).Append("\n");
                this->GameKitSessionManagerReloadConfigContents(sessionManagerInstance, TCHAR_TO_UTF8(configFileContents.GetCharArray().GetData()));
            }
            else
            {
                UE_LOG(LogAwsGameKit, Error, TEXT("Could not copy CA Cert from %s to %s."), *caCertPath, *saveAndroidFilePath);
            }
        }
        else
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("Could not load config from %s."), *results[0]);
        }
#elif PLATFORM_IOS
        FString configFileContents;
        if (FFileHelper::LoadFileToString(configFileContents, *results[0], FFileHelper::EHashOptions::None))
        {
            // Inject CA Cert Path to settings
            FString caCertPath = FString(FApp::GetProjectName()).ToLower() + "/content/certs/cacert.pem";
            FIOSPlatformFile iosPlatformFile = FIOSPlatformFile();
            FString caCertIosFilePath = iosPlatformFile.ConvertToAbsolutePathForExternalAppForRead(*caCertPath);
            configFileContents.Append("\n").Append("ca_cert_file: ").Append(caCertIosFilePath).Append("\n");
            this->GameKitSessionManagerReloadConfigContents(sessionManagerInstance, TCHAR_TO_UTF8(*configFileContents));
        }
        else
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("Could not load config from %s."), *results[0]);
        }
#endif
    }
    else
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("Did not find config to load at %s."), *searchPath);

#if PLATFORM_ANDROID
        // If config file wasn't found in ProjectContentDir, the game must've been deployed using the Editor's Launch button.
        // Try to convert the path to Absolute path for External Read. CA Cert doesn't need to be copied.

        searchPath = IAndroidPlatformFile::GetPlatformPhysical().ConvertToAbsolutePathForExternalAppForRead(ToCStr(contentDir));
        UE_LOG(LogAwsGameKit, Display, TEXT("Searching for config %s recursively starting at %s"), *clientConfigFileToSearch, *searchPath);
        fileManager.FindFilesRecursive(results, ToCStr(searchPath), ToCStr(clientConfigFileToSearch), true, false, true);
        if (results.Num() > 0)
        {
            FString configFileContents;
            if (FFileHelper::LoadFileToString(configFileContents, *results[0], FFileHelper::EHashOptions::None))
            {
                // Inject CA Cert Path to settings
                FString caCertPath = FString(searchPath + "certs/cacert.pem"); // searchPath ends in foo/content/
                FString caCertAndroidFilePath = IAndroidPlatformFile::GetPlatformPhysical().ConvertToAbsolutePathForExternalAppForRead(*caCertPath);
                configFileContents.Append("\n").Append("ca_cert_file: ").Append(caCertAndroidFilePath).Append("\n");
                this->GameKitSessionManagerReloadConfigContents(sessionManagerInstance, TCHAR_TO_UTF8(configFileContents.GetCharArray().GetData()));
            }
            else
            {
                UE_LOG(LogAwsGameKit, Error, TEXT("Could not load config from %s."), *results[0]);
            }
        }
        else
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("Did not find config to load at %s."), *searchPath);
        }
#endif
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
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerAreSettingsLoaded, false);

    return INVOKE_FUNC(GameKitSessionManagerAreSettingsLoaded, sessionManagerInstance, featureType);
}

void AwsGameKitSessionManagerWrapper::GameKitSessionManagerReloadConfigFile(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance, const char* clientConfigFile)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerReloadConfigFile);

    INVOKE_FUNC(GameKitSessionManagerReloadConfigFile, sessionManagerInstance, clientConfigFile);
}

void AwsGameKitSessionManagerWrapper::GameKitSessionManagerReloadConfigContents(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance, const char* clientConfigFileContents)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerReloadConfigContents);

    INVOKE_FUNC(GameKitSessionManagerReloadConfigContents, sessionManagerInstance, clientConfigFileContents);
}

void AwsGameKitSessionManagerWrapper::GameKitSessionManagerSetToken(GAMEKIT_SESSION_MANAGER_INSTANCE_HANDLE sessionManagerInstance, GameKit::TokenType tokenType, const char* value)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(SessionManager, GameKitSessionManagerSetToken);

    INVOKE_FUNC(GameKitSessionManagerSetToken, sessionManagerInstance, tokenType, value);
}

#undef LOCTEXT_NAMESPACE
