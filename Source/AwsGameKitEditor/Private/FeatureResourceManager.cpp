// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "FeatureResourceManager.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitRuntime.h"
#include "EditorState.h"
#include "Core/AwsGameKitDispatcher.h"
#include "GameSaving/AwsGameKitGameSavingLayoutDetails.h"
#include "Identity/AwsGameKitIdentityLayoutDetails.h"

// Unreal
#include "Async/Async.h"
#include "Interfaces/IPluginManager.h"

TArray<FString> resourceInfoCache;

#define LOG_FEATURE_MESSAGE(message)                    \
{                                                       \
    UE_LOG(LogAwsGameKit, Log, TEXT("%s"), *message);       \
    featuresLog = message + "\n" + featuresLog;                      \
};

#define IDENTITY_NUM_KEYS 3 // Total number of Identity feature variables
#define GAMESAVING_NUM_KEYS 1 // Total number of Game Saving feature variables

#define CLOUDFORMATION_PARAMETER_IDENTITY_VAR_KEYS { \
    TCHAR_TO_UTF8(*AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_EMAIL_ENABLED), \
    TCHAR_TO_UTF8(*AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_ENABLED), \
    TCHAR_TO_UTF8(*AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_APP_ID), \
} \

#define CLOUDFORMATION_PARAMETER_GAMESAVING_VAR_KEYS { \
    TCHAR_TO_UTF8(*AwsGameKitGameSavingLayoutDetails::MAX_SAVE_SLOTS_PER_PLAYER), \
} \

#define CLOUDFORMATION_PARAMETER_IDENTITY_VAR_VALUES(identityIsEmailEnabled, identityIsFacebookEnabled, identityFacebookClientId) { \
    TCHAR_TO_UTF8(*identityIsEmailEnabled), \
    TCHAR_TO_UTF8(*identityIsFacebookEnabled), \
    TCHAR_TO_UTF8(*identityFacebookClientId), \
} \

#define CLOUDFORMATION_PARAMETER_GAMESAVING_VAR_VALUES(gameSavingMaxCloudSaveSlotsPerPlayer) { \
    TCHAR_TO_UTF8(*gameSavingMaxCloudSaveSlotsPerPlayer), \
} \

void ResourceInfoCallback(const char* logicalResourceId, const char* resourceType, const char* resourceStatus)
{
    // TODO: Make this thread safe.
    FString f = FString::Printf(L"'%s' resource with id '%s' in %s status.", ANSI_TO_TCHAR(resourceType), ANSI_TO_TCHAR(logicalResourceId), ANSI_TO_TCHAR(resourceStatus));
    resourceInfoCache.Add(f);
}

const std::string FeatureResourceManager::DEPLOYED_STATUS_TEXT = "Deployed";
const std::string FeatureResourceManager::UNDEPLOYED_STATUS_TEXT = "Undeployed";
const std::string FeatureResourceManager::ERROR_STATUS_TEXT = "Error";
const std::string FeatureResourceManager::ROLLBACK_COMPLETE_STATUS_TEXT = "Rollback Complete";
const std::string FeatureResourceManager::WORKING_STATUS_TEXT = "Running";
const std::string FeatureResourceManager::GENERATING_TEMPLATES_STATUS_TEXT = "Generating templates";
const std::string FeatureResourceManager::UPLOADING_DASHBOARDS_STATUS_TEXT = "Uploading dashboards";
const std::string FeatureResourceManager::UPLOADING_LAYERS_STATUS_TEXT = "Uploading layers";
const std::string FeatureResourceManager::UPLOADING_FUNCTIONS_STATUS_TEXT = "Uploading functions";
const std::string FeatureResourceManager::DEPLOYING_STATUS_TEXT = "Deploying resources";
const std::string FeatureResourceManager::DELETING_RESOURCES_STATUS_TEXT = "Deleting resources";
const std::string FeatureResourceManager::RETRIEVING_STATUS_TEXT = "Retrieving status";

FeatureResourceManager::~FeatureResourceManager()
{
    this->Shutdown();
}

void FeatureResourceManager::Initialize()
{
    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::Initialize()"));

    pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
    pluginRootPath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"));
    pluginRootPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*pluginRootPath);
    rootPath = FPaths::ProjectDir();
    rootPath.RemoveAt(rootPath.Len() - 1);
    rootPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*rootPath);

    Logging::AttachLogger(this);

    if (this->ftsc == nullptr)
    {
        this->ftsc = MakeShareable(new FThreadSafeCounter(0));
    }
}

CoreLibrary FeatureResourceManager::GetCoreLibraryFromModule() const
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    return runtimeModule->GetCoreLibrary();
}

void FeatureResourceManager::InitializeSettings(bool reinitialize)
{
    if (this->settingInstanceHandle != nullptr && reinitialize)
    {
        GetCoreLibraryFromModule().CoreWrapper->GameKitSettingsInstanceRelease(this->settingInstanceHandle);
        this->settingInstanceHandle = nullptr;
    }

    if (this->settingInstanceHandle == nullptr)
    {
        this->settingInstanceHandle = GetCoreLibraryFromModule().CoreWrapper->GameKitSettingsInstanceCreate(
            TCHAR_TO_UTF8(*this->GetRootPath()), TCHAR_TO_UTF8(*this->GetPluginVersion()), this->accountInfoCopy.gameName.c_str(), this->accountInfoCopy.environment.c_str(), Logging::LogCallBack);;
    }
}

void FeatureResourceManager::Log(unsigned int level, const FString& message)
{
    this->featuresLog += message + "\n";
}

void FeatureResourceManager::SetAccountDetails(const AccountDetails& accountDetails)
{
    this->accountInfoCopy = AccountInfoCopy(accountDetails);
    this->credentialsCopy = AccountCredentialsCopy(accountDetails);

    InitializeSettings(true);
}


FString FeatureResourceManager::GetDashboardURL(FString feature)
{
    FString const gameName = FString(this->accountInfoCopy.gameName.c_str());
    FString const env = FString(this->accountInfoCopy.environment.c_str());
    FString const region = FString(this->credentialsCopy.region.c_str());
    feature.RemoveSpacesInline();

    return "https://console.aws.amazon.com/cloudwatch/home?region=" + region + "#dashboards:name=GameKit-" + gameName + "-" + env + "-" + region + "-" + feature;
}

void FeatureResourceManager::SetAccountDetails(const TMap<FString, FString>& creds)
{
    AccountDetails accountDetails;
    accountDetails.environment = creds[EditorState::EDITOR_STATE_SELECTED_ENVIRONMENT];
    accountDetails.accountId = creds[EditorState::EDITOR_STATE_ACCOUNT_ID];
    accountDetails.gameName = creds[EditorState::EDITOR_STATE_SHORT_GAME_NAME];
    accountDetails.region = creds[EditorState::EDITOR_STATE_REGION];
    accountDetails.accessKey = creds[EditorState::EDITOR_STATE_ACCESS_KEY];
    accountDetails.accessSecret = creds[EditorState::EDITOR_STATE_ACCESS_SECRET];
    this->SetAccountDetails(accountDetails);
}

void FeatureResourceManager::SetGameName(const FString& gameName)
{
    this->accountInfoCopy.gameName = TCHAR_TO_UTF8(*gameName);
    InitializeSettings(true);
}

FString FeatureResourceManager::GetNavigationString()
{
    if (this->accountInfoCopy.gameName == "" ||
        this->accountInfoCopy.environment == "" ||
        this->credentialsCopy.region == "")
    {
        return FString("");
    }

    auto navigationString = this->accountInfoCopy.gameName + "  >  " +
        this->accountInfoCopy.environment + "  >  " +
        this->credentialsCopy.region;

    return FString(navigationString.c_str());
}

void FeatureResourceManager::Shutdown()
{
    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::Shutdown()"));

    if (this->ftsc != nullptr)
    {
        this->ftsc.Reset();
        this->ftsc = nullptr;
    }
    Logging::DetachLogger(this);
}

IntResult FeatureResourceManager::CreateOrUpdateFeatureResources(FeatureType featureType)
{
    featureRunningStates.FindOrAdd(featureType) = FeatureRunningState::Running;
    void* gamekitResourcesInstance = this->SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), FeatureType::Main);

    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    // Create the feature
    IntResult result(CreateOrUpdateResources(featureType));
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::CreateOrUpdateResources() for " + FeatureToUIString(featureType) + " feature: Could not create resources.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        GetCoreLibraryFromModule().CoreWrapper->GameKitResourcesInstanceRelease(gamekitResourcesInstance);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
        return result;
    }

    // Deploy feature
    void* accountInstance = coreLibrary.CoreWrapper->GameKitAccountInstanceCreateWithRootPaths(
        accountInfoCopy.ToCharPtrView(),
        credentialsCopy.ToCharPtrView(),
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );
    result = coreLibrary.CoreWrapper->GameKitAccountDeployApiGatewayStage(accountInstance);
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::CreateOrUpdateResources() for " + FeatureToUIString(featureType) + " feature: Could not Deploy to ApiGateway stage.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }

    featureRunningStates[featureType] = FeatureRunningState::NotRunning;
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(gamekitResourcesInstance);
    coreLibrary.CoreWrapper->GameKitAccountInstanceRelease(accountInstance);

    return result;
}

IntResult FeatureResourceManager::CreateOrUpdateResources(FeatureType featureType)
{
    if (featureRunningStates[featureType] != FeatureRunningState::Running)
    {
        LOG_FEATURE_MESSAGE(FString("Task status was not in Running state. Resource creation might fail"));
    }

    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::CreateOrUpdateResources()"));
    void* gamekitResourcesInstance = this->SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);
    IntResult result(coreLibrary.CoreWrapper->GameKitResourcesInstanceCreateOrUpdateStack(gamekitResourcesInstance));
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(gamekitResourcesInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::CreateOrUpdateResources() Creating/Updating stack failed.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
        LOG_FEATURE_MESSAGE(message);
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::CreateOrUpdateResources() SUCCESS."));
    }

    // Reload game configuration in session manager
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    runtimeModule->ReloadConfigFile(editorModule->GetFeatureResourceManager()->GetClientConfigSubdirectory());

    return result;
}

const FString& FeatureResourceManager::GetVariableOrDefault(const TMap<FString, FString>& vars, const FString& key, const FString& defaultString)
{
    if (vars.Contains(key))
    {
        return vars[key];
    }

    return defaultString;
}

IntResult FeatureResourceManager::DeleteFeatureResources(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    featureRunningStates[featureType] = FeatureRunningState::Running;

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::DeleteFeatureResources()"));
    void* gamekitResourcesInstance = this->SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);
    IntResult result(coreLibrary.CoreWrapper->GameKitResourcesInstanceDeleteStack(gamekitResourcesInstance));
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(gamekitResourcesInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::DeleteFeatureResources() for " + FeatureToUIString(featureType) + " feature: Failed to delete stack.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::DeleteFeatureResources() SUCCESS"));
    }

    featureRunningStates[featureType] = FeatureRunningState::NotRunning;
    return result;
}

IntResult FeatureResourceManager::DescribeFeatureResources(FeatureType featureType, TArray<FString>& outResources)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::DescribeFeatureResources()"));
    void* gamekitResourcesInstance = this->SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);

    resourceInfoCache.Reset();
    IntResult result(coreLibrary.CoreWrapper->GameKitResourcesDescribeStackResources(gamekitResourcesInstance, ResourceInfoCallback));
    outResources = resourceInfoCache;
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(gamekitResourcesInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::DescribeFeatureResources() Failed to retrieve stack resource information.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::DescribeFeatureResources() SUCCESS"));
    }

    return result;
}

FString FeatureResourceManager::GetAccountId(const FString& accessKey, const FString& secretKey)
{
    FString accountId;
    auto accountSetter = [&accountId](const char* retrievedAccountId)
    {
        accountId = retrievedAccountId;
    };
    typedef LambdaDispatcher<decltype(accountSetter), void, const char*> AccountSetter;

    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    IntResult result = coreLibrary.CoreWrapper->GameKitGetAwsAccountId((void*)&accountSetter, AccountSetter::Dispatch, TCHAR_TO_UTF8(*accessKey), TCHAR_TO_UTF8(*secretKey), Logging::LogCallBack);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::GetAccountId() Failed to retrieve account.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        accountId = "";
    }

    return accountId;
}

bool FeatureResourceManager::IsAccountInfoValid(const AccountDetails& accountDetails)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::IsAccountInfoValid()"));

    void* accountInstance = coreLibrary.CoreWrapper->GameKitAccountInstanceCreateWithRootPaths(
        AccountInfoCopy(accountDetails).ToCharPtrView(),
        AccountCredentialsCopy(accountDetails).ToCharPtrView(),
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );
    bool const hasValidCredentials = coreLibrary.CoreWrapper->GameKitAccountHasValidCredentials(accountInstance);
    coreLibrary.CoreWrapper->GameKitAccountInstanceRelease(accountInstance);
    return hasValidCredentials;
}

void* FeatureResourceManager::SetupResourcesInstance(const AccountInfo& accountInfo, const AccountCredentials& credentials, FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    void* gamekitResourcesInstance = coreLibrary.CoreWrapper->GameKitResourcesInstanceCreateWithRootPaths(
        accountInfo,
        credentials,
        featureType,
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );

    return gamekitResourcesInstance;
}

std::string FeatureResourceManager::GetResourcesStackStatus(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    if (featureRunningStates.Find(featureType) == nullptr)
    {
        // state is only set when creating or deleting resources, so it is safe to
        // assume not tasks are running before calling AWS
        featureRunningStates.FindOrAdd(featureType) = FeatureRunningState::NotRunning;
    }

    if (featureRunningStates[featureType] == FeatureRunningState::Running)
    {
        return WORKING_STATUS_TEXT;
    }

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::GetResourcesStackStatus()"));
    void* gamekitResourcesInstance = this->SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);
    std::string status = coreLibrary.CoreWrapper->GameKitResourcesGetCurrentStackStatus(gamekitResourcesInstance);
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(gamekitResourcesInstance);

    FString const message = FString("FeatureResourceManager::GetResourcesStackStatus() : ") + FeatureToUIString(featureType) + " : " + status.c_str();
    LOG_FEATURE_MESSAGE(message);

    // simplify status text
    if (status == "ROLLBACK_COMPLETE" ||
        status == "UPDATE_ROLLBACK_COMPLETE" ||
        status == "IMPORT_ROLLBACK_COMPLETE")
    {
        status = ROLLBACK_COMPLETE_STATUS_TEXT;
    }
    else if (status == "DELETE_COMPLETE" ||
        status == "UNDEPLOYED" ||
        status == "")
    {
        status = UNDEPLOYED_STATUS_TEXT;
    }
    else if (status.find("COMPLETE") != std::string::npos)
    {
        status = DEPLOYED_STATUS_TEXT;
    }
    else if (status.find("IN_PROGRESS") != std::string::npos)
    {
        status = WORKING_STATUS_TEXT;
    }
    else if (status.find("FAILED") != std::string::npos)
    {
        status = ERROR_STATUS_TEXT;
    }

    return status;
}

bool FeatureResourceManager::IsTaskInProgress(FeatureType featureType) const
{
    const FeatureRunningState* status = featureRunningStates.Find(featureType);

    return (status != nullptr && *status == FeatureRunningState::Running);
}

bool FeatureResourceManager::IsMainStackInProgress() const
{
    return IsTaskInProgress(FeatureType::Main);
}

bool FeatureResourceManager::IsFeatureCloudFormationInstanceTemplatePresent(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    void* resourceInstance = SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);
    const bool isPresent = coreLibrary.CoreWrapper->GameKitResourcesIsCloudFormationInstanceTemplatePresent(resourceInstance);
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);

    return isPresent;
}

IntResult FeatureResourceManager::SaveDeployedFeatureTemplate(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    void* resourceInstance = SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);
    const IntResult result(coreLibrary.CoreWrapper->GameKitResourcesSaveDeployedCloudFormationTemplate(resourceInstance));
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);

    return result;
}

IntResult FeatureResourceManager::BootstrapAccount()
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::BootstrapAccount()"));

    void* accountInstance = coreLibrary.CoreWrapper->GameKitAccountInstanceCreateWithRootPaths(
        this->accountInfoCopy.ToCharPtrView(),
        this->credentialsCopy.ToCharPtrView(),
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );
    IntResult result(coreLibrary.CoreWrapper->GameKitAccountInstanceBootstrap(accountInstance));
    coreLibrary.CoreWrapper->GameKitAccountInstanceRelease(accountInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = TEXT("Error: FeatureResourceManager::BootstrapAccount() Failed to create bucket.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::BootstrapAccount() SUCCESS"));
    }

    return result;
}

IntResult FeatureResourceManager::ProcessFeatureVariables(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, FeatureType featureType, TFunction<unsigned int(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char** varKeys, const char** varValues, int numKeys)> variablesHandler)
{
    IntResult result;
    auto const featureVars = this->GetFeatureVariables(featureType);
    if (featureType == FeatureType::Identity)
    {
        FString const identityIsEmailEnabled = GetVariableOrDefault(featureVars, AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_EMAIL_ENABLED, "false");
        FString const identityIsFacebookEnabled = GetVariableOrDefault(featureVars, AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_ENABLED, "false");
        FString const identityFacebookClientId = GetVariableOrDefault(featureVars, AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_APP_ID, "REPLACE_WITH_YOUR_FACEBOOK_CLIENT_ID");

        // If facebook is enabled and no client id, return an error and do not execute the handler
        if (identityIsFacebookEnabled == EditorState::TrueString && identityFacebookClientId.IsEmpty())
        {
            result.Result = GameKit::GAMEKIT_ERROR_GENERAL;
            result.ErrorMessage = "Please provide a Facebook App ID.";
            return result;
        }

        const char* varKeys[IDENTITY_NUM_KEYS] = CLOUDFORMATION_PARAMETER_IDENTITY_VAR_KEYS;
        const char* varValues[IDENTITY_NUM_KEYS] = CLOUDFORMATION_PARAMETER_IDENTITY_VAR_VALUES(identityIsEmailEnabled, identityIsFacebookEnabled, identityFacebookClientId);
        result = variablesHandler(resourceInstance, varKeys, varValues, IDENTITY_NUM_KEYS);
    }
    else if (featureType == FeatureType::GameStateCloudSaving)
    {
        auto gameSavingVars = this->GetFeatureVariables(FeatureType::GameStateCloudSaving);
        FString const gameSavingMaxCloudSaveSlotsPerPlayer = GetVariableOrDefault(gameSavingVars, AwsGameKitGameSavingLayoutDetails::MAX_SAVE_SLOTS_PER_PLAYER, FString::FromInt(AwsGameKitGameSavingLayoutDetails::DEFAULT_MAX_SAVE_SLOTS_PER_PLAYER));

        const char* varKeys[GAMESAVING_NUM_KEYS] = CLOUDFORMATION_PARAMETER_GAMESAVING_VAR_KEYS;
        const char* varValues[GAMESAVING_NUM_KEYS] = CLOUDFORMATION_PARAMETER_GAMESAVING_VAR_VALUES(gameSavingMaxCloudSaveSlotsPerPlayer);
        result = variablesHandler(resourceInstance, varKeys, varValues, GAMESAVING_NUM_KEYS);
    }
    else
    {
        // No feature-specific variables to save. But we'll execute the handler anyway
        result = variablesHandler(resourceInstance, {}, {}, 0);
    }

    return result;
}

IntResult FeatureResourceManager::GenerateFeatureTemplates(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    featureRunningStates.FindOrAdd(featureType) = FeatureRunningState::Running;
    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::GenerateFeatureTemplates()"));

    void* resourceInstance = coreLibrary.CoreWrapper->GameKitResourcesInstanceCreateWithRootPaths(
        this->accountInfoCopy.ToCharPtrView(),
        this->credentialsCopy.ToCharPtrView(),
        featureType,
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );

    IntResult result = ProcessFeatureVariables(resourceInstance, featureType, [this, coreLibrary](GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char** varKeys, const char** varValues, int numKeys)
    {
        return coreLibrary.CoreWrapper->GameKitResourcesSaveCloudFormationInstance(resourceInstance, varKeys, varValues, numKeys);
    });

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::GenerateFeatureTemplates() Failed to save feature CloudFormation instance template. " + result.ErrorMessage);
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
        coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);
        return result;
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::GenerateFeatureTemplates() CloudFormation instance template saved."));
    }

    // Save feature's Lambda Layer instance files
    result = coreLibrary.CoreWrapper->GameKitResourcesSaveLayerInstances(resourceInstance);
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::GenerateFeatureTemplates() Failed to save feature Lambda Layer instance files. " + result.ErrorMessage);
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
        coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);
        return result;
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::GenerateFeatureTemplates() Lambda Layer instance files saved."));
    }

    // Save feature's Lambda Function instance files
    result = coreLibrary.CoreWrapper->GameKitResourcesSaveFunctionInstances(resourceInstance);
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::GenerateFeatureTemplates() Failed to save feature Lambda Function instance files. " + result.ErrorMessage);
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
        coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);
        return result;
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::GenerateFeatureTemplates() Lambda Function instance files saved."));
    }

    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);

    return result;
}

IntResult FeatureResourceManager::UpdateFeatureParameters(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    featureRunningStates.FindOrAdd(featureType) = FeatureRunningState::Running;
    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UpdateFeatureParameters()"));
    void* resourceInstance = coreLibrary.CoreWrapper->GameKitResourcesInstanceCreateWithRootPaths(
        this->accountInfoCopy.ToCharPtrView(),
        this->credentialsCopy.ToCharPtrView(),
        featureType,
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );

    IntResult result = ProcessFeatureVariables(resourceInstance, featureType, [this, coreLibrary](GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char** varKeys, const char** varValues, int numKeys)
    {
        return coreLibrary.CoreWrapper->GameKitResourcesUpdateCloudFormationParameters(resourceInstance, varKeys, varValues, numKeys);
    });

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::UpdateFeatureParameters() Failed to save feature CloudFormation parameters. " + result.ErrorMessage);
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
        coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);
        return result;
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UpdateFeatureParameters() CloudFormation parameters saved."));
    }

    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(resourceInstance);

    return GameKit::GAMEKIT_SUCCESS;
}

IntResult FeatureResourceManager::UploadDashboards(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    featureRunningStates.FindOrAdd(featureType) = FeatureRunningState::Running;
    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UploadDashboards()"));

    void* accountInstance = coreLibrary.CoreWrapper->GameKitAccountInstanceCreateWithRootPaths(
        this->accountInfoCopy.ToCharPtrView(),
        this->credentialsCopy.ToCharPtrView(),
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );
    IntResult result(coreLibrary.CoreWrapper->GameKitAccountUploadAllDashboards(accountInstance));
    coreLibrary.CoreWrapper->GameKitAccountInstanceRelease(accountInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::UploadDashboards() Failed to upload dashboards.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UploadDashboards() SUCCESS"));
    }

    return result;
}

IntResult FeatureResourceManager::UploadLayers(FeatureType featureType)
{
    // Upload requested feature layers. Error messaging and logging already happening within UploadLayersByFeature.
    IntResult result(this->UploadLayersByFeature(featureType));
    return result;
}

IntResult FeatureResourceManager::UploadLayersByFeature(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    featureRunningStates.FindOrAdd(featureType) = FeatureRunningState::Running;
    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UploadLayers() for " + FeatureToUIString(featureType)));

    void* gameKitResourcesInstance = this->SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);
    IntResult result(coreLibrary.CoreWrapper->GameKitResourcesUploadFeatureLayers(gameKitResourcesInstance));
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(gameKitResourcesInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::UploadLayers() Failed to upload " + FeatureToUIString(featureType) + " layers.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UploadLayers() " + FeatureToUIString(featureType) + " SUCCESS"));
    }

    return result;
}

IntResult FeatureResourceManager::UploadFunctions(FeatureType featureType)
{
    // Upload requested feature functions. Error messaging and logging already happening within UploadFunctionsByFeature.
    IntResult result(this->UploadFunctionsByFeature(featureType));
    return result;
}

IntResult FeatureResourceManager::UploadFunctionsByFeature(FeatureType featureType)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    featureRunningStates.FindOrAdd(featureType) = FeatureRunningState::Running;
    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UploadFunctions() for " + FeatureToUIString(featureType)));

    void* gameKitResourcesInstance = this->SetupResourcesInstance(this->accountInfoCopy.ToCharPtrView(), this->credentialsCopy.ToCharPtrView(), featureType);
    IntResult result(coreLibrary.CoreWrapper->GameKitResourcesUploadFeatureFunctions(gameKitResourcesInstance));
    coreLibrary.CoreWrapper->GameKitResourcesInstanceRelease(gameKitResourcesInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::UploadFunctions() Failed to upload " + FeatureToUIString(featureType) + " functions.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
        featureRunningStates[featureType] = FeatureRunningState::NotRunning;
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::UploadFunctions() " + FeatureToUIString(featureType) + " SUCCESS"));
    }

    return result;
}

IntResult FeatureResourceManager::SaveSecret(const FString& secretName, const FString& secretValue)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::SaveSecret()"));

    void* accountInstance = coreLibrary.CoreWrapper->GameKitAccountInstanceCreateWithRootPaths(
        this->accountInfoCopy.ToCharPtrView(),
        this->credentialsCopy.ToCharPtrView(),
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );
    IntResult result(coreLibrary.CoreWrapper->GameKitAccountSaveSecret(accountInstance, TCHAR_TO_UTF8(*secretName), TCHAR_TO_UTF8(*secretValue)));
    coreLibrary.CoreWrapper->GameKitAccountInstanceRelease(accountInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::SaveSecret() Failed to save secret.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::SaveSecret() SUCCESS"));
    }

    return result;
}

IntResult FeatureResourceManager::CheckSecretExists(const FString& secretName)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::CheckSecretExists()"));

    void* accountInstance = coreLibrary.CoreWrapper->GameKitAccountInstanceCreateWithRootPaths(
        this->accountInfoCopy.ToCharPtrView(),
        this->credentialsCopy.ToCharPtrView(),
        TCHAR_TO_ANSI(*rootPath),
        TCHAR_TO_ANSI(*pluginRootPath),
        Logging::LogCallBack
    );
    IntResult result(coreLibrary.CoreWrapper->GameKitAccountCheckSecretExists(accountInstance, TCHAR_TO_UTF8(*secretName)));
    coreLibrary.CoreWrapper->GameKitAccountInstanceRelease(accountInstance);

    if (result.Result != GameKit::GAMEKIT_SUCCESS || result.Result != GameKit::GAMEKIT_WARNING_SECRETSMANAGER_SECRET_NOT_FOUND)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::CheckSecretExists() Failed to verify secret.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }
    else
    {
        LOG_FEATURE_MESSAGE(FString("FeatureResourceManager::CheckSecretExists() SUCCESS"));
    }

    return result;
}

void FeatureResourceManager::SaveCustomEnvironment(const FString& environmentKey, const FString& environmentValue)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    coreLibrary.CoreWrapper->GameKitSettingsAddCustomEnvironment(this->settingInstanceHandle, TCHAR_TO_UTF8(*environmentKey), TCHAR_TO_UTF8(*environmentValue));

    IntResult result = coreLibrary.CoreWrapper->GameKitSettingsSave(this->settingInstanceHandle);
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::SaveCustomEnvironment() Failed to save.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }
}

TMap<FString, FString> FeatureResourceManager::GetFeatureVariables(FeatureType featureType)
{
    TMap<FString, FString> vars;
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    auto varsSetter = [&vars](const char* key, const char* value)
    {
        vars.Add(key, value);
    };
    typedef LambdaDispatcher<decltype(varsSetter), void, const char*, const char*> VarsSetter;

    coreLibrary.CoreWrapper->GameKitSettingsGetFeatureVariables(
        this->settingInstanceHandle,
        (void*)&varsSetter,
        featureType,
        VarsSetter::Dispatch);

    return vars;
}

TMap<FString, FString> FeatureResourceManager::GetSettingsEnvironments() const
{
    TMap<FString, FString> environments;
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();

    FString settingsFile = this->GetRootPath() / FString(this->accountInfoCopy.gameName.c_str()) / "saveInfo.yml";
    if (!FPaths::FileExists(settingsFile))
    {
        return environments;
    }

    auto envSetter = [&environments](const char* key, const char* value)
    {
        environments.Add(key, value);
    };
    typedef LambdaDispatcher<decltype(envSetter), void, const char*, const char*> EnvSetter;

    coreLibrary.CoreWrapper->GameKitSettingsGetCustomEnvironments(
        this->settingInstanceHandle,
        (void*)&envSetter,
        EnvSetter::Dispatch);

    return environments;
}

FString FeatureResourceManager::GetGameName() const
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    if (this->accountInfoCopy.gameName.length() == 0)
    {
        FString gameName;
        auto gameNameSetter = [&gameName](const char* name)
        {
            gameName = name;
        };
        typedef LambdaDispatcher<decltype(gameNameSetter), void, const char*> GameNameSetter;

        coreLibrary.CoreWrapper->GameKitSettingsGetGameName(
            this->settingInstanceHandle,
            (void*)&gameNameSetter,
            GameNameSetter::Dispatch);

        return gameName;
    }

    return FString(this->accountInfoCopy.gameName.c_str());
}

FString FeatureResourceManager::GetLastUsedRegion() const
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    FString lastUsedRegion;
    auto lastUsedRegionSetter = [&lastUsedRegion](const char* region)
    {
        lastUsedRegion = region;
    };
    typedef LambdaDispatcher<decltype(lastUsedRegionSetter), void, const char*> LastUsedRegionSetter;

    coreLibrary.CoreWrapper->GameKitSettingsGetLastUsedRegion(
        this->settingInstanceHandle,
        (void*)&lastUsedRegionSetter,
        LastUsedRegionSetter::Dispatch);

    return lastUsedRegion;
}

FString FeatureResourceManager::GetLastUsedEnvironment(const FString& gameName) const
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    FString lastUsedEnv;
    FString settingsFile = this->GetRootPath() / gameName / "saveInfo.yml";

    if (!FPaths::FileExists(settingsFile))
    {
        return "dev";
    }

    auto lastUsedEnvSetter = [&lastUsedEnv](const char* env)
    {
        lastUsedEnv = env;
    };
    typedef LambdaDispatcher<decltype(lastUsedEnvSetter), void, const char*> LastUsedEnvSetter;

    coreLibrary.CoreWrapper->GameKitSettingsGetLastUsedEnvironment(
        this->settingInstanceHandle,
        (void*)&lastUsedEnvSetter,
        LastUsedEnvSetter::Dispatch);

    return lastUsedEnv;
}

FString FeatureResourceManager::GetLastUsedEnvironment() const
{
    return GetLastUsedEnvironment(FString(this->accountInfoCopy.gameName.c_str()));
}

void FeatureResourceManager::SetFeatureVariableIfUnset(FeatureType featureType, const FString& varName, const FString& varValue)
{
    const auto featureVars = this->GetFeatureVariables(featureType);
    if (!featureVars.Contains(varName))
    {
        this->SetFeatureVariable(featureType, varName, varValue);
    }
}

void FeatureResourceManager::SetFeatureVariable(FeatureType featureType, const FString& varName, const FString& varValue)
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    const char* key[1] = { TCHAR_TO_UTF8(*varName) };
    const char* val[1] = { TCHAR_TO_UTF8(*varValue) };

    coreLibrary.CoreWrapper->GameKitSettingsSetFeatureVariables(this->settingInstanceHandle, featureType, key, val, 1);
    // Debounce our writes so we don't thresh on IO
    int currentValue = this->ftsc->Increment();
    Async(EAsyncExecution::Thread, [this, coreLibrary, currentValue]()
    {
        FPlatformProcess::Sleep(3);

        if (currentValue == this->ftsc->GetValue())
        {
            coreLibrary.CoreWrapper->GameKitSettingsSave(this->settingInstanceHandle);
            this->ftsc->Reset();
        }
    });
}

void FeatureResourceManager::SaveSettings()
{
    CoreLibrary coreLibrary = GetCoreLibraryFromModule();
    FString pluginVersion = GetPluginVersion();
    FString settingsPath;
    auto pathSetter = [&settingsPath](const char* path)
    {
        settingsPath = path;
    };
    typedef LambdaDispatcher<decltype(pathSetter), void, const char*> PathSetter;

    coreLibrary.CoreWrapper->GameKitSettingsGetSettingsFilePath(this->settingInstanceHandle, (void*)&pathSetter, PathSetter::Dispatch);

    if (!settingsPath.IsEmpty())
    {
        FString parentDir = FPaths::GetPath(settingsPath);

        if (!FPaths::DirectoryExists(parentDir))
        {
            IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();
            bool created = platformFile.CreateDirectoryTree(*parentDir);
            FString message = "Created " + parentDir + " already exists: " + (created ? "yes" : "no");
            LOG_FEATURE_MESSAGE(message);
        }
    }

    coreLibrary.CoreWrapper->GameKitSettingsSetGameName(this->settingInstanceHandle, this->accountInfoCopy.gameName.c_str());
    coreLibrary.CoreWrapper->GameKitSettingsSetLastUsedEnvironment(this->settingInstanceHandle, this->accountInfoCopy.environment.c_str());
    coreLibrary.CoreWrapper->GameKitSettingsSetLastUsedRegion(this->settingInstanceHandle, this->credentialsCopy.region.c_str());

    IntResult result = coreLibrary.CoreWrapper->GameKitSettingsSave(this->settingInstanceHandle);
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: FeatureResourceManager::SaveSettings() Failed to save.");
        FString const error = GameKit::StatusCodeToHexFStr(result.Result);
        FString const message = result.ErrorMessage + " : " + error;
        LOG_FEATURE_MESSAGE(message);
    }
}

FString FeatureResourceManager::GetPluginVersion() const
{
    const FPluginDescriptor& descriptor = IPluginManager::Get().FindPlugin("AwsGameKit")->GetDescriptor();
    return descriptor.VersionName;
}

const FString& FeatureResourceManager::GetRootPath() const
{
    return rootPath;
}

const FString FeatureResourceManager::GetClientConfigSubdirectory() const
{
    std::string dir = this->accountInfoCopy.gameName + "/" + this->accountInfoCopy.environment + "/";
    return FString(dir.c_str());
}