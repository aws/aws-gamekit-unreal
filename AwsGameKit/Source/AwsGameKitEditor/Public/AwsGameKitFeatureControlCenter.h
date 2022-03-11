// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitMarshalling.h>
#include "AwsGameKitEditor.h"

// GameKit forward declarations
class FeatureResourceManager;

// Unreal
#include "Containers/UnrealString.h"
#include "HAL/CriticalSection.h"

// Unreal forward declarations
class FMessageEndpoint;
class FSlateStyleSet;
class SButton;
class SDockTab;
class SEditableTextBox;
class SWindow;

class AwsGameKitFeatureControlCenter
{
private:
    AccountDetails accountDetails;
    FAwsGameKitEditorModule* editorModule;
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> credentialsMessageEndpoint;
    TSet<FeatureType> availableFeatures =
    {
        FeatureType::Main,
        FeatureType::Identity,
        FeatureType::Achievements,
        FeatureType::GameStateCloudSaving,
        FeatureType::UserGameplayData
    };

    // Helpers
    void ConditionallyCreateOrUpdateFeatureResources(TSharedPtr<FeatureResourceManager> featureResourceManager, FeatureType feature);
    void ConditionallyCreateOrUpdateFeatureResources(TSharedPtr<FeatureResourceManager> featureResourceManager, FeatureType feature, FeatureType featureTypeStatusOverride);

    // UI Elements
    TSharedPtr<SEditableTextBox> deleteConfirmText;
    TSharedPtr<SButton> executeDeleteButton;
    TSharedPtr<SButton> executeCancelButton;
    TSharedPtr<SWindow> deleteResourcesWindow;

    // UI Actions and event handlers
    FeatureType featureToDelete;
    FReply DeleteResources(FeatureType feature);
    void OnConfirmDeleteChanged(const FText& text);
    FReply OnCancelDeleteChanged();
    void OpenDeleteDialog();

    // UI button state and message handling
    TMap<FeatureType, FString> featureStatusMessage;
    TMap<FeatureType, TSharedPtr<FText>> featureDeleteDetailsContent; // text in delete dialog

    TSet<FString> createEnabledStatuses = { FString(FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str()), FString(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()) };
    TSet<FString> redeployEnabledStatuses = { FString(FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str()), FString(FeatureResourceManager::ROLLBACK_COMPLETE_STATUS_TEXT.c_str()), FString(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()) };
    TSet<FString> deleteEnabledStatuses = { FString(FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str()), FString(FeatureResourceManager::ROLLBACK_COMPLETE_STATUS_TEXT.c_str()), FString(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()) };

    bool credentialsSubmitted;
    void CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context);

    FCriticalSection featureButtonsMutex;
    FCriticalSection featureStatusMessageMutex;
    
    // Helper to check the status of a dependent feature. if the status does not match append the feature name to the dependentFeatures FString 
    bool CheckDependentFeatureStatus(FeatureType feature, const char* status, TArray<FString>& dependentFeatures);

    bool IsFeatureInteractable(FeatureType feature, FString& outFeatureStatus);

    // Helper to track if the stack was retrieved for deletion 
    bool stackCanBeDeleted;

public:
    AwsGameKitFeatureControlCenter(FAwsGameKitEditorModule* editorModule);
    ~AwsGameKitFeatureControlCenter();

    void RefreshFeatureStatuses();
    void ResetFeatureStatuses();
    bool FeatureAvailable(FeatureType feature);
    FText GetStatus(FeatureType feature);

    // Helper to see if any feature is actively being deployed or updating
    bool IsAnyFeatureUpdating();
    bool IsFeatureUpdating(const FeatureType feature);

    // Helper tp see of a feature should have the ability to be refreshed
    bool IsRefreshAvailable();

    // Helper to check if a feature can be updated or deleted
    bool CanCreateOrUpdateDependentFeature(FeatureType feature);
    bool CanDeleteDependentFeature(FeatureType feature);

    // Store the current override tooltips for each feature
    TMap<FeatureType, FString> createOrUpdateOverrideTooltips;
    TMap<FeatureType, FString> deleteOverrideTooltips;

    // for details panels to access
    bool IsCreateEnabled(FeatureType feature);
    bool IsRedeployEnabled(FeatureType feature);
    bool IsDeleteEnabled(FeatureType feature);
    FReply CreateOrUpdateResources(FeatureType feature);
    FReply PrepareDeleteResources(FeatureType feature);
    FName GetIconStyle(FeatureType feature);
    void GetFeatureStatusAsync(FeatureType feature);
    bool IsValidProviderCredentialsInput(TSharedPtr<SCheckBox> providerCheckbox, TSharedPtr<SEditableTextBox> providerAppId, TSharedPtr<SEditableTextBox> providerAppSecret, FString secretId) const;
};
