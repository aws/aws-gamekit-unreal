// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Core/AwsGameKitCoreWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"

#define LOCTEXT_NAMESPACE "AwsGameKitCoreWrapper"

std::string GameKit::StatusCodeToHexStr(const unsigned int statusCode)
{
    const size_t bufferSize = 17; // 16 characters + null terminator
    char buffer[bufferSize] = { '\0' };

    sprintf_s(buffer, bufferSize, "%#0x", statusCode);
    return buffer;
};

FString GameKit::StatusCodeToHexFStr(const unsigned int statusCode)
{
    return FString::Printf(TEXT("%#0x"), statusCode);
}

// Deprecated
FString FeatureToApiString(FeatureType feature)
{
    switch (feature)
    {
    case FeatureType::Main:
        return "main";
    case FeatureType::Identity:
        return "identity";
    case FeatureType::Authentication:
        return "authentication";
    case FeatureType::Achievements:
        return "achievements";
    case FeatureType::GameStateCloudSaving:
        return "gamesaving";
    case FeatureType::UserGameplayData:
        return "usergamedata";
    }

    return "";
}

// Deprecated
FString FeatureToUIString(FeatureType feature)
{
    switch (feature)
    {
    case FeatureType::Main:
        return "Main";
    case FeatureType::Identity:
        return "Identity And Authentication";
    case FeatureType::Authentication:
        return "Authentication";
    case FeatureType::Achievements:
        return "Achievements";
    case FeatureType::GameStateCloudSaving:
        return "Game State Cloud Saving";
    case FeatureType::UserGameplayData:
        return "User Gameplay Data";
    }

    return "";
}

// Deprecated
FString FeatureResourcesUIString(FeatureType feature)
{
    switch (feature)
    {
    case FeatureType::Identity:
        return "API Gateway, CloudWatch, Cognito, DynamoDB, IAM, Key Management Service, and Lambda. ";
    case FeatureType::Achievements:
        return "API Gateway, CloudFront, CloudWatch, Cognito, DynamoDB, Lambda, S3, and Security Token Service. ";
    case FeatureType::GameStateCloudSaving:
        return "API Gateway, CloudWatch, Cognito, DynamoDB, Lambda, and S3. ";
    case FeatureType::UserGameplayData:
        return "API Gateway, CloudWatch, Cognito, DynamoDB, and Lambda. ";
    default:
        return "";
    }
}

void AwsGameKitCoreWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitCoreWrapper::importFunctions()"));

    // Static functions
    LOAD_PLUGIN_FUNC(GameKitGetAwsAccountId, loadedDllHandle);
    
    // GameKit Account
    LOAD_PLUGIN_FUNC(GameKitAccountInstanceCreate, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountInstanceCreateWithRootPaths, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountGetRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountGetPluginRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountGetBaseCloudFormationPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountGetBaseFunctionsPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountGetInstanceCloudFormationPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountGetInstanceFunctionsPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountSetRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountSetPluginRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountHasValidCredentials, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountInstanceBootstrap, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountSaveSecret, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountCheckSecretExists, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountSaveFeatureInstanceTemplates, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountUploadAllDashboards, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountUploadLayers, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountUploadFunctions, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountCreateOrUpdateMainStack, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountCreateOrUpdateStacks, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAccountDeployApiGatewayStage, loadedDllHandle);

    // GameKit Resources
    LOAD_PLUGIN_FUNC(GameKitResourcesInstanceCreate, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesInstanceCreateWithRootPaths, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesGetRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesGetPluginRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesGetBaseCloudFormationPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesGetBaseFunctionsPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesGetInstanceCloudFormationPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesGetInstanceFunctionsPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesSetRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesSetPluginRootPath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesInstanceCreateOrUpdateStack, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesInstanceDeleteStack, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesGetCurrentStackStatus, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesIsCloudFormationInstanceTemplatePresent, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesSaveDeployedCloudFormationTemplate, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesSaveCloudFormationInstance, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesUpdateCloudFormationParameters, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesSaveLayerInstances, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesSaveFunctionInstances, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesUploadFeatureLayers, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesUploadFeatureFunctions, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitResourcesDescribeStackResources, loadedDllHandle);
    
    // GameKit Settings
    LOAD_PLUGIN_FUNC(GameKitSettingsInstanceCreate, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsSetGameName, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsSetLastUsedRegion, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsSetLastUsedEnvironment, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsAddCustomEnvironment, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsDeleteCustomEnvironment, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsActivateFeature, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsDeactivateFeature, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsSetFeatureVariables, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsDeleteFeatureVariable, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsSave, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetGameName, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetLastUsedRegion, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetLastUsedEnvironment, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetCustomEnvironments, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetCustomEnvironmentDescription, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsIsFeatureActive, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetFeatureVariables, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetFeatureVariable, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsGetSettingsFilePath, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSettingsReload, loadedDllHandle);
}

#pragma region GameKitAccount
unsigned int AwsGameKitCoreWrapper::GameKitGetAwsAccountId(DISPATCH_RECEIVER_HANDLE caller, CharPtrCallback resultcallback, const char* accessKey, const char* secretKey, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitGetAwsAccountId, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitGetAwsAccountId, caller, resultcallback, accessKey, secretKey, logCb);
}

GAMEKIT_ACCOUNT_INSTANCE_HANDLE AwsGameKitCoreWrapper::GameKitAccountInstanceCreate(AccountInfo accountInfo, AccountCredentials credentials, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountInstanceCreate, nullptr);

    return INVOKE_FUNC(GameKitAccountInstanceCreate, accountInfo, credentials, logCb);
}

GAMEKIT_ACCOUNT_INSTANCE_HANDLE AwsGameKitCoreWrapper::GameKitAccountInstanceCreateWithRootPaths(AccountInfo accountInfo, AccountCredentials credentials, const char* rootPath, const char* pluginRootPath, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountInstanceCreateWithRootPaths, nullptr);

    return INVOKE_FUNC(GameKitAccountInstanceCreateWithRootPaths, accountInfo, credentials, rootPath, pluginRootPath, logCb);
}

void AwsGameKitCoreWrapper::GameKitAccountInstanceRelease(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountInstanceRelease);

    INVOKE_FUNC(GameKitAccountInstanceRelease, accountInstance);
}

const char* AwsGameKitCoreWrapper::GameKitAccountGetRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountGetRootPath, nullptr);

    return INVOKE_FUNC(GameKitAccountGetRootPath, accountInstance);
}

const char* AwsGameKitCoreWrapper::GameKitAccountGetPluginRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountGetPluginRootPath, nullptr);

    return INVOKE_FUNC(GameKitAccountGetPluginRootPath, accountInstance);
}

const char* AwsGameKitCoreWrapper::GameKitAccountGetBaseCloudFormationPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountGetBaseCloudFormationPath, nullptr);

    return INVOKE_FUNC(GameKitAccountGetBaseCloudFormationPath, accountInstance);
}

const char* AwsGameKitCoreWrapper::GameKitAccountGetBaseFunctionsPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountGetBaseFunctionsPath, nullptr);

    return INVOKE_FUNC(GameKitAccountGetBaseFunctionsPath, accountInstance);
}

const char* AwsGameKitCoreWrapper::GameKitAccountGetInstanceCloudFormationPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountGetInstanceCloudFormationPath, nullptr);

    return INVOKE_FUNC(GameKitAccountGetInstanceCloudFormationPath, accountInstance);
}

const char* AwsGameKitCoreWrapper::GameKitAccountGetInstanceFunctionsPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountGetInstanceFunctionsPath, nullptr);

    return INVOKE_FUNC(GameKitAccountGetInstanceFunctionsPath, accountInstance);
}

void AwsGameKitCoreWrapper::GameKitAccountSetRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* rootPath)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountSetRootPath);

    INVOKE_FUNC(GameKitAccountSetRootPath, accountInstance, rootPath);
}

void AwsGameKitCoreWrapper::GameKitAccountSetPluginRootPath(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* pluginRootPath)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountSetPluginRootPath);

    INVOKE_FUNC(GameKitAccountSetPluginRootPath, accountInstance, pluginRootPath);
}

bool AwsGameKitCoreWrapper::GameKitAccountHasValidCredentials(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountHasValidCredentials, false);

    return INVOKE_FUNC(GameKitAccountHasValidCredentials, accountInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountInstanceBootstrap(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountInstanceBootstrap, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountInstanceBootstrap, accountInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountSaveSecret(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* secretName, const char* secretValue)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountSaveSecret, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountSaveSecret, accountInstance, secretName, secretValue);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountCheckSecretExists(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* secretName)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountCheckSecretExists, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountCheckSecretExists, accountInstance, secretName);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountSaveFeatureInstanceTemplates(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance, const char* const* varKeys, const char* const* varValues, int numKeys)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountSaveFeatureInstanceTemplates, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountSaveFeatureInstanceTemplates, accountInstance, varKeys, varValues, numKeys);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountUploadAllDashboards(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountUploadAllDashboards, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountUploadAllDashboards, accountInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountUploadLayers(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountUploadLayers, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountUploadLayers, accountInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountUploadFunctions(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountUploadFunctions, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountUploadFunctions, accountInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountCreateOrUpdateMainStack(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountCreateOrUpdateMainStack, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountCreateOrUpdateMainStack, accountInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountCreateOrUpdateStacks(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountCreateOrUpdateStacks, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountCreateOrUpdateStacks, accountInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitAccountDeployApiGatewayStage(GAMEKIT_ACCOUNT_INSTANCE_HANDLE accountInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitAccountDeployApiGatewayStage, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitAccountDeployApiGatewayStage, accountInstance);
}
#pragma endregion

#pragma region GameKitResources
GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE AwsGameKitCoreWrapper::GameKitResourcesInstanceCreate(AccountInfo accountInfo, AccountCredentials credentials, FeatureType featureType, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesInstanceCreate, nullptr);

    return INVOKE_FUNC(GameKitResourcesInstanceCreate, accountInfo, credentials, featureType, logCb);
}

GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE AwsGameKitCoreWrapper::GameKitResourcesInstanceCreateWithRootPaths(AccountInfo accountInfo, AccountCredentials credentials, FeatureType featureType, const char* rootPath, const char* pluginRootPath, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesInstanceCreateWithRootPaths, nullptr);

    return INVOKE_FUNC(GameKitResourcesInstanceCreateWithRootPaths, accountInfo, credentials, featureType, rootPath, pluginRootPath, logCb);
}

void AwsGameKitCoreWrapper::GameKitResourcesInstanceRelease(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesInstanceRelease);

    INVOKE_FUNC(GameKitResourcesInstanceRelease, resourceInstance);
}

const char* AwsGameKitCoreWrapper::GameKitResourcesGetRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesGetRootPath, nullptr);

    return INVOKE_FUNC(GameKitResourcesGetRootPath, resourceInstance);
}

const char* AwsGameKitCoreWrapper::GameKitResourcesGetPluginRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesGetPluginRootPath, nullptr);

    return INVOKE_FUNC(GameKitResourcesGetPluginRootPath, resourceInstance);
}

const char* AwsGameKitCoreWrapper::GameKitResourcesGetBaseCloudFormationPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesGetBaseCloudFormationPath, nullptr);

    return INVOKE_FUNC(GameKitResourcesGetBaseCloudFormationPath, resourceInstance);
}

const char* AwsGameKitCoreWrapper::GameKitResourcesGetBaseFunctionsPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesGetBaseFunctionsPath, nullptr);

    return INVOKE_FUNC(GameKitResourcesGetBaseFunctionsPath, resourceInstance);
}

const char* AwsGameKitCoreWrapper::GameKitResourcesGetInstanceCloudFormationPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesGetInstanceCloudFormationPath, nullptr);

    return INVOKE_FUNC(GameKitResourcesGetInstanceCloudFormationPath, resourceInstance);
}

const char* AwsGameKitCoreWrapper::GameKitResourcesGetInstanceFunctionsPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesGetInstanceFunctionsPath, nullptr);

    return INVOKE_FUNC(GameKitResourcesGetInstanceFunctionsPath, resourceInstance);
}

void AwsGameKitCoreWrapper::GameKitResourcesSetRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* rootPath)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesSetRootPath);

    INVOKE_FUNC(GameKitResourcesSetRootPath, resourceInstance, rootPath);
}

void AwsGameKitCoreWrapper::GameKitResourcesSetPluginRootPath(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* pluginRootPath)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesSetPluginRootPath)

        INVOKE_FUNC(GameKitResourcesSetPluginRootPath, resourceInstance, pluginRootPath);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesInstanceCreateOrUpdateStack(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesInstanceCreateOrUpdateStack, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesInstanceCreateOrUpdateStack, resourceInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesInstanceDeleteStack(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesInstanceDeleteStack, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesInstanceDeleteStack, resourceInstance);
}

const char* AwsGameKitCoreWrapper::GameKitResourcesGetCurrentStackStatus(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesGetCurrentStackStatus, nullptr);

    return INVOKE_FUNC(GameKitResourcesGetCurrentStackStatus, resourceInstance);
}

bool AwsGameKitCoreWrapper::GameKitResourcesIsCloudFormationInstanceTemplatePresent(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesIsCloudFormationInstanceTemplatePresent, nullptr);

    return INVOKE_FUNC(GameKitResourcesIsCloudFormationInstanceTemplatePresent, resourceInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesSaveDeployedCloudFormationTemplate(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesSaveDeployedCloudFormationTemplate, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesSaveDeployedCloudFormationTemplate, resourceInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesSaveCloudFormationInstance(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* const* varKeys, const char* const* varValues, int numKeys)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesSaveCloudFormationInstance, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesSaveCloudFormationInstance, resourceInstance, varKeys, varValues, numKeys);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesUpdateCloudFormationParameters(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, const char* const* varKeys, const char* const* varValues, int numKeys)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesUpdateCloudFormationParameters, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesUpdateCloudFormationParameters, resourceInstance, varKeys, varValues, numKeys);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesSaveLayerInstances(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesSaveLayerInstances, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesSaveLayerInstances, resourceInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesSaveFunctionInstances(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesSaveFunctionInstances, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesSaveFunctionInstances, resourceInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesUploadFeatureLayers(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesUploadFeatureLayers, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesUploadFeatureLayers, resourceInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesUploadFeatureFunctions(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesUploadFeatureFunctions, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesUploadFeatureFunctions, resourceInstance);
}

unsigned int AwsGameKitCoreWrapper::GameKitResourcesDescribeStackResources(GAMEKIT_FEATURERESOURCES_INSTANCE_HANDLE resourceInstance, FuncResourceInfoCallback resourceInfoCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitResourcesDescribeStackResources, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitResourcesDescribeStackResources, resourceInstance, resourceInfoCb);
}
#pragma endregion

#pragma region GameKitSettings
GAMEKIT_SETTINGS_INSTANCE_HANDLE AwsGameKitCoreWrapper::GameKitSettingsInstanceCreate(const char* rootPath, const char* pluginVersion, const char* shortGameName, const char* currentEnvironment, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsInstanceCreate, nullptr);

    return INVOKE_FUNC(GameKitSettingsInstanceCreate, rootPath, pluginVersion, shortGameName, currentEnvironment, logCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsInstanceRelease(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsInstanceRelease);

    INVOKE_FUNC(GameKitSettingsInstanceRelease, settingsInstance);
}

void AwsGameKitCoreWrapper::GameKitSettingsSetGameName(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* gameName)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsSetGameName);

    INVOKE_FUNC(GameKitSettingsSetGameName, settingsInstance, gameName); 
}

void AwsGameKitCoreWrapper::GameKitSettingsSetLastUsedRegion(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* region)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsSetLastUsedRegion);

    INVOKE_FUNC(GameKitSettingsSetLastUsedRegion, settingsInstance, region);
}

void AwsGameKitCoreWrapper::GameKitSettingsSetLastUsedEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* envCode)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsSetLastUsedEnvironment);

    INVOKE_FUNC(GameKitSettingsSetLastUsedEnvironment, settingsInstance, envCode);
}

void AwsGameKitCoreWrapper::GameKitSettingsAddCustomEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* envCode, const char* envDescription)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsAddCustomEnvironment);

    INVOKE_FUNC(GameKitSettingsAddCustomEnvironment, settingsInstance, envCode, envDescription);
}

void AwsGameKitCoreWrapper::GameKitSettingsDeleteCustomEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, const char* envCode)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsDeleteCustomEnvironment);

    INVOKE_FUNC(GameKitSettingsDeleteCustomEnvironment, settingsInstance, envCode);
}

void AwsGameKitCoreWrapper::GameKitSettingsActivateFeature(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsActivateFeature);

    INVOKE_FUNC(GameKitSettingsActivateFeature, settingsInstance, featureType);
}

void AwsGameKitCoreWrapper::GameKitSettingsDeactivateFeature(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsDeactivateFeature);

    INVOKE_FUNC(GameKitSettingsDeactivateFeature, settingsInstance, featureType);
}

void AwsGameKitCoreWrapper::GameKitSettingsSetFeatureVariables(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType, const char* const* varKeys, const char* const* varValues, size_t numKeys)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsSetFeatureVariables);

    INVOKE_FUNC(GameKitSettingsSetFeatureVariables, settingsInstance, featureType, varKeys, varValues, numKeys);
}

void AwsGameKitCoreWrapper::GameKitSettingsDeleteFeatureVariable(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType, const char* varName)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsDeleteFeatureVariable);

    INVOKE_FUNC(GameKitSettingsDeleteFeatureVariable, settingsInstance, featureType, varName);
}

unsigned int AwsGameKitCoreWrapper::GameKitSettingsSave(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsSave, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitSettingsSave, settingsInstance);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetGameName(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetGameName);

    INVOKE_FUNC(GameKitSettingsGetGameName, settingsInstance, receiver, resultsCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetLastUsedRegion(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetLastUsedRegion);

    INVOKE_FUNC(GameKitSettingsGetLastUsedRegion, settingsInstance, receiver, resultsCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetLastUsedEnvironment(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetLastUsedEnvironment);

    INVOKE_FUNC(GameKitSettingsGetLastUsedEnvironment, settingsInstance, receiver, resultsCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetCustomEnvironments(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, KeyValueCharPtrCallbackDispatcher resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetCustomEnvironments);

    INVOKE_FUNC(GameKitSettingsGetCustomEnvironments, settingsInstance, receiver, resultsCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetCustomEnvironmentDescription(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, const char* envCode, CharPtrCallback resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetCustomEnvironmentDescription);

    INVOKE_FUNC(GameKitSettingsGetCustomEnvironmentDescription, settingsInstance, receiver, envCode, resultsCb);
}

bool AwsGameKitCoreWrapper::GameKitSettingsIsFeatureActive(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, FeatureType featureType)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsIsFeatureActive, false);

    return INVOKE_FUNC(GameKitSettingsIsFeatureActive, settingsInstance, featureType);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetFeatureVariables(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, FeatureType featureType, KeyValueCharPtrCallbackDispatcher resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetFeatureVariables);

    INVOKE_FUNC(GameKitSettingsGetFeatureVariables, settingsInstance, receiver, featureType, resultsCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetFeatureVariable(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, FeatureType featureType, const char* varName, CharPtrCallback resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetFeatureVariable);

    INVOKE_FUNC(GameKitSettingsGetFeatureVariable, settingsInstance, receiver, featureType, varName, resultsCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsGetSettingsFilePath(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance, DISPATCH_RECEIVER_HANDLE receiver, CharPtrCallback resultsCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsGetSettingsFilePath);

    INVOKE_FUNC(GameKitSettingsGetSettingsFilePath, settingsInstance, receiver, resultsCb);
}

void AwsGameKitCoreWrapper::GameKitSettingsReload(GAMEKIT_SETTINGS_INSTANCE_HANDLE settingsInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Core, GameKitSettingsReload);

    INVOKE_FUNC(GameKitSettingsReload, settingsInstance);
}
#pragma endregion

#undef LOCTEXT_NAMESPACE
