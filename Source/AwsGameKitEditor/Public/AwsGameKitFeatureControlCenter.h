// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitMarshalling.h>

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
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> messageEndpoint;
    TArray<FeatureType> availableFeatures;

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
    TMap<FeatureType, bool> featureDeleteButtonsState;
    TMap<FeatureType, bool> featureCreateButtonsState;
    TMap<FeatureType, bool> featureRedeployButtonsState;
    TMap<FeatureType, bool> featureDeleteOverrideButtonsState;
    TMap<FeatureType, bool> featureCreateOverrideButtonsState;
    TMap<FeatureType, bool> featureRedeployOverrideButtonsState;
    TMap<FeatureType, TSharedPtr<FText>> featureDeleteDetailsContent; // text in delete dialog
    
    // Deployment states and handlers
    bool anyDeploymentRunning;
    FeatureType currentlyDeployingFeature;
    void DeploymentStateMessageHandler(const struct FMsgDeploymentState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context);

    FCriticalSection featureButtonsMutex;
    FCriticalSection featureStatusMessageMutex;
    FCriticalSection anyDeploymentRunningMutex;
    
    void EnableFeatureButtons(FeatureType feature, bool createEnabled, bool redeployEnabled, bool deleteEnabled);
    void EnableFeatureButtonsAsync(FeatureType feature, bool createEnabled, bool redeployEnabled, bool deleteEnabled);
    void EnableDeleteButtonAsync(bool enabled);

    // Helper to check the status of a dependent feature. if the status does not match append the feature name to the dependentFeatures FString 
    bool CheckDependentFeatureStatus(FeatureType feature, const char* status, TArray<FString>& dependentFeatures);

    // Helper to track if the stack was retrieved for deletion 
    bool stackCanBeDeleted;

public:
    AwsGameKitFeatureControlCenter();
    ~AwsGameKitFeatureControlCenter();

    void RefreshFeatureStatuses();
    void ResetFeatureStatuses();
    FText GetStatus(FeatureType feature);

    // Helper to see if any feature is actively being deployed or updating
    bool IsAnyFeatureUpdating();
    bool IsFeatureUpdating(const FeatureType feature);

    // Helper to check if a feature can be updated or deleted
    bool CanCreateOrUpdateDependentFeature(FeatureType feature);
    bool CanDeleteDependentFeature(FeatureType feature);

    // Store the current override tooltips for each feature
    TMap<FeatureType, FString> createOrUpdateOverrideTooltips;
    TMap<FeatureType, FString> deleteOverrideTooltips;

    // for details panels to access
    bool IsCreateEnabled(FeatureType feature, bool defaultValue);
    bool IsRedeployEnabled(FeatureType feature, bool defaultValue);
    bool IsDeleteEnabled(FeatureType feature, bool defaultValue);
    FReply CreateOrUpdateResources(FeatureType feature);
    FReply PrepareDeleteResources(FeatureType feature);
    FName GetIconStyle(FeatureType feature);
    void GetFeatureStatusAsync(FeatureType feature);
    bool IsValidProviderCredentialsInput(TSharedPtr<SCheckBox> providerCheckbox, TSharedPtr<SEditableTextBox> providerAppId, TSharedPtr<SEditableTextBox> providerAppSecret, FString secretId) const;
};
