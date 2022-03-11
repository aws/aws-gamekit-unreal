// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitRuntime.h"
#include "Core/AwsGameKitCoreWrapper.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/AwsGameKitMarshalling.h"
#include "Core/Logging.h"

// Standard library
#include <string>

// Unreal
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"

class FeatureResourceManager : IChildLogger
{
private:
    void* SetupResourcesInstance(const AccountInfo accountInfo, const AccountCredentials credentials, FeatureType featureType);

    IntResult CreateOrUpdateResources(FeatureType featureType);
    IntResult UploadLayersByFeature(FeatureType feature);
    IntResult UploadFunctionsByFeature(FeatureType feature);
    IntResult ValidateFeatureVariables(FeatureType featureType);
    static const FString& GetVariableOrDefault(const TMap<FString, FString>& vars, const FString& key, const FString& defaultString);

    FString pluginBaseDir;
    FString pluginRootPath;
    FString rootPath;

    std::string retrievedAccountId;
    FString featuresLog;

    AccountInfoCopy accountInfoCopy;
    AccountCredentialsCopy credentialsCopy;


protected:
    TSharedPtr<FThreadSafeCounter, ESPMode::ThreadSafe> ftsc;
    GAMEKIT_SETTINGS_INSTANCE_HANDLE settingInstanceHandle;

public:
    ~FeatureResourceManager();
    void Initialize();
    CoreLibrary GetCoreLibraryFromModule() const;
    void InitializeSettings(bool reinitialize);
    void SetAccountDetails(const AccountDetails& accountDetails);
    void SetAccountDetails(const TMap<FString, FString>& editorSettings);
    void SetGameName(const FString& gameName);
    FString GetDashboardURL(FString feature);
    FString GetNavigationString();
    void Shutdown();

    void Log(unsigned int level, const FString& message) override;

    inline const FString& GetLog() const { return featuresLog; };

    // Feature task running status tracking
    enum class FeatureRunningState : uint8
    {
        NotRunning = 0,
        Running = 1
    };

    TMap<FeatureType, FeatureRunningState> featureRunningStates;

    static const std::string DEPLOYED_STATUS_TEXT;
    static const std::string UNDEPLOYED_STATUS_TEXT;
    static const std::string ERROR_STATUS_TEXT;
    static const std::string ROLLBACK_COMPLETE_STATUS_TEXT;
    static const std::string WORKING_STATUS_TEXT;
    static const std::string GENERATING_TEMPLATES_STATUS_TEXT;
    static const std::string UPLOADING_DASHBOARDS_STATUS_TEXT;
    static const std::string UPLOADING_LAYERS_STATUS_TEXT;
    static const std::string UPLOADING_FUNCTIONS_STATUS_TEXT;
    static const std::string DEPLOYING_STATUS_TEXT;
    static const std::string DELETING_RESOURCES_STATUS_TEXT;
    static const std::string RETRIEVING_STATUS_TEXT;

    // Account
    bool IsAccountInfoValid(const AccountDetails& accountDetails);
    FString GetAccountId(const FString& accessKey, const FString& secretKey);
    IntResult BootstrapAccount();
    IntResult CreateEmptyClientConfigFile();
    IntResult CreateOrUpdateFeatureResources(FeatureType featureType);
    IntResult DeleteFeatureResources(FeatureType featureType);
    IntResult DescribeFeatureResources(FeatureType featureType, TArray<FString>& outResources);
    std::string GetResourcesStackStatus(FeatureType featureType);
    bool IsTaskInProgress(FeatureType featureType) const;
    bool IsMainStackInProgress() const;
    bool IsFeatureCloudFormationInstanceTemplatePresent(FeatureType featureType);
    IntResult SaveDeployedFeatureTemplate(FeatureType featureType);
    IntResult GenerateFeatureInstanceFiles(FeatureType featureType);
    IntResult ValidateFeatureParameters(FeatureType featureType);
    IntResult UploadDashboards(FeatureType featureType);
    IntResult UploadLayers(FeatureType featureType);
    IntResult UploadFunctions(FeatureType featureType);
    IntResult SaveSecret(const FString& secretName, const FString& secretValue);
    IntResult CheckSecretExists(const FString& secretName);

    // GameKit Settings
    void SaveCustomEnvironment(const FString& environmentKey, const FString& environmentValue);
    TMap<FString, FString> GetFeatureVariables(FeatureType featureType);
    void SetFeatureVariableIfUnset(FeatureType featureType, const FString& varName, const FString& varValue);
    void SetFeatureVariable(FeatureType featureType, const FString& varName, const FString& varValue);
    TMap<FString, FString> GetSettingsEnvironments() const;
    FString GetGameName() const;
    FString GetLastUsedRegion() const;
    FString GetLastUsedEnvironment() const;
    void SaveSettings();

    FString GetPluginVersion() const;
    const FString& GetRootPath() const;
    const FString GetClientConfigSubdirectory() const;
};
