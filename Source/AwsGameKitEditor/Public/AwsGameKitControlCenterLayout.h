// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitCoreWrapper.h>
#include "AwsCredentialsManager.h"
#include "FeatureResourceManager.h"
#include "AwsGameKitFeatureControlCenter.h"

// Unreal
#include "Containers/UnrealString.h"
#include "IDetailCustomization.h"
#include "MessagingCommon/Public/MessageEndpoint.h"
#include "Templates/SharedPointer.h"

// Forward declarations
class SDockTab;
class FSpawnTabArgs;

// Type definitions
typedef TSharedPtr<FString, ESPMode::ThreadSafe> FComboBoxItem;

/**
 * This class defines the Control Center layout.
 */
class AWSGAMEKITEDITOR_API AwsGameKitControlCenterLayout : public IDetailCustomization
{
private:

    // Aws Environment and region
    TMap<FString, FString> environmentMapping;
    TMap<FString, FString> regionMapping;
    TArray<FComboBoxItem> environmentOptions;
    TArray<FComboBoxItem> regionOptions;
    FComboBoxItem currentEnvironment;
    FComboBoxItem currentRegion;
    TArray<FString> unsupportedRegions;

    // Input widgets
    TSharedPtr<SEditableTextBox> projectNameTextBox;
    TSharedPtr<SVerticalBox> projectNameBox;
    TSharedPtr<SBorder> projectNameValidation;
    TSharedPtr<SComboBox<FComboBoxItem>> environmentComboBox;
    TSharedPtr<SVerticalBox> customEnvironmentBox;
    TSharedPtr<SEditableTextBox> customEnvironmentNameTextBox;
    TSharedPtr<SBorder> customEnvironmentNameValidation;
    TSharedPtr<SEditableTextBox> customEnvironmentCodeTextBox;
    TSharedPtr<SBorder> customEnvironmentCodeValidation;
    TSharedPtr<SComboBox<FComboBoxItem>> regionComboBox;
    TSharedPtr<SBorder> submitValidation;
    TSharedPtr<SEditableTextBox> accessKeyTextBox;
    TSharedPtr<SBorder> accessKeyValidation;
    TSharedPtr<SEditableTextBox> secretKeyTextBox;
    TSharedPtr<SBorder> secretKeyValidation;
    TSharedPtr<SBox> accountLoadingAnimationBox;
    TSharedPtr<SCheckBox> storeCredentialsCheckBox;
    TSharedPtr<SButton> submitButton;
    TSharedPtr<SButton> featureControlCenterButton;
    TSharedPtr<SButton> switchEnvironmentsButton;
    TSharedPtr<SBorder> switchEnvironmentsNotification;
    TSharedPtr<SBorder> newEnvironmentNotification;
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> messageEndpoint;
    TSharedPtr<SBox> rightPane;
    TSharedPtr<SVerticalBox> rightPaneAwsText;
    TSharedPtr<SVerticalBox> rightPaneCredentialsSubmitted;

    // Dynamic text
    FText submitValidationText;
    FText environmentNameErrorText;
    FText environmentCodeErrorText;
    FText accountIdText;
    FText gameTitleText;

    // States
    void SetPartiallyCompleteState(const FString& gameName);

    // Settings retrieval
    bool TryFindConfigFile(FString& outConfigPath, FString& outGameName);
    bool TryParseGameNameFromConfig(const FString& configFilePath, FString& outGameName);
    void PopulateCustomEnvironments(const FString& gameName);
    void LoadLastUsedEnvironment();
    void LoadLastUsedRegion();
    bool TryLoadAwsCredentialsFromFile();
    bool IsGameNameValid(const FString& gameName);
    bool IsEnvironmentNameValid(const FString& environmentName) const;
    bool IsEnvironmentNameInUse(const FString& environmentName) const;
    bool IsEnvironmentCodeValid(const FString& environmentCode) const;
    bool IsEnvironmentCodeInUse(const FString& environmentCode) const;
    void RetrieveAccountId(const FString& accessKey, const FString& secretKey);

    // UI controls
    void PopulateEnvironments();
    void PopulateRegions();

    // UI Event handlers
    void OnProjectNameTextChanged(const FText& fieldText);
    void OnLoadCustomGameConfigFile();
    void OnRegionSelectionChanged(FComboBoxItem newValue, ESelectInfo::Type selectionType);
    void OnEnvironmentSelectionChanged(FComboBoxItem newValue, ESelectInfo::Type selectionType);
    void OnCustomEnvironmentNameChanged(const FText& fieldText);
    void OnCustomEnvironmentCodeChanged(const FText& fieldText);
    void OnAccessKeyChanged(const FText& fieldText);
    void OnSecretKeyChanged(const FText& fieldText);
    void OnAwsCredentialsChanged(bool areFieldsValid);
    void OnCheckFields();
    FReply OnSwitchEnvironment();
    FReply OnSubmit();
    EVisibility UpdateSwitchEnvironmentValidation() const;
    bool IsSwitchEnvironmentButtonEnabled() const;

    // Helpers (UI thread)
    TSharedRef<SDockTab> GetHomeTab();
    TSharedRef<SWidget> MakeWidget(FComboBoxItem item);
    FText GetRegionItemLabel() const;
    FText GetEnvironmentItemLabel() const;
    static bool TrySelectGameConfigFile(const FString& rootPath, FString& outFilePath);
    static FReply OpenBrowser(const FString& url);
    void EnableInputBoxes(bool isEnabled);
    static void BulkSetEnabled(const TArray<SWidget*> widgets, bool enabled);
    static void BulkSetVisibility(const TArray<SWidget*> widgets, EVisibility visibility);
    FString GetSelectedEnvironmentKey() const;
    FString GetCurrentRegionKey() const;
    void GetCustomEnvironment(FString& outKey, FString& outName) const;
    AccountDetails GetAccountDetails() const;
    void SetAccountDetails() const;

    // Warn user if they are closing home tab without submitting new environment
    bool GetCanCloseControlHomeTab();

public:
    AwsGameKitControlCenterLayout();

    ~AwsGameKitControlCenterLayout();

    /**
     * @brief Referenced in feature detail panels
    */
    static const FName ControlCenterHomeTabName;

    /**
     * Make a new instance of this detail layout class for a specific detail view requesting it.
     */
     //static TSharedRef<IDetailCustomization> MakeInstance();
    static TSharedRef<AwsGameKitControlCenterLayout> MakeInstance();

    virtual void CustomizeDetails(IDetailLayoutBuilder& detailBuilder) override;

    TSharedRef<SDockTab> OnSpawnHomeTab(const FSpawnTabArgs& spawnTabArgs);
    FReply OpenControlCenter();

    void SetInitialState();
};
