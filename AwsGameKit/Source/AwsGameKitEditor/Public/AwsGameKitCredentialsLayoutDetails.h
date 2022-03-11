// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitEditor.h"
#include "FeatureResourceManager.h"

// Unreal
#include "Containers/UnrealString.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "Internationalization/Regex.h"
#include "MessagingCommon/Public/MessageEndpoint.h"
#include "Templates/SharedPointer.h"

// Forward declarations
class SDockTab;
class FSpawnTabArgs;

// Type definitions
typedef TSharedPtr<FString, ESPMode::ThreadSafe> FComboBoxItem;

class AWSGAMEKITEDITOR_API AwsGameKitCredentialsLayoutDetails : public IDetailCustomization
{
private:

    static const FString AWS_ACCOUNT_ID_EMPTY;

    IDetailCategoryBuilder* configCategoryBuilder = nullptr;
    FAwsGameKitEditorModule* editorModule;
    FReply OpenControlCenter();

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
    TSharedPtr<SHorizontalBox> projectNameBox;
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
    TSharedPtr<SButton> cancelEnvironmentSwitchButton;
    TSharedPtr<SBorder> switchEnvironmentsNotification;
    TSharedPtr<SBorder> newEnvironmentNotification;
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> messageEndpoint;
    TSharedPtr<SBox> rightPane;
    TSharedPtr<SVerticalBox> rightPaneAwsText;
    TSharedPtr<SVerticalBox> rightPaneCredentialsSubmitted;

    // Project name settings debounce
    double nextConfigFileCheckTimestamp = 0; // seconds
    bool configFileFieldChangedValid = false;
    const double configFileCheckDelay = 0.5; // seconds
    EActiveTimerReturnType ProjectNameStateTransitionCallback(double inCurrentTime, float inDeltaTime);
    TSharedPtr<FActiveTimerHandle> projectNameTimerHandle;

    // Dynamic text
    FText submitValidationText;
    FText environmentNameErrorText;
    FText environmentCodeErrorText;
    FText accountIdText = FText::FromString(AWS_ACCOUNT_ID_EMPTY);
    FText gameTitleText;
    FText projectNameValidationErrorText;

    // States
    void SetPartiallyCompleteState();
    bool isLoadingEnvironmentFromFile;

    // Settings retrieval
    bool TryFindConfigFile(FString& outConfigPath, FString& outGameName);
    bool ConfigFileExists(const FString& subfolder);
    bool TryParseGameNameFromConfig(const FString& configFilePath, FString& outGameName);
    void PopulateCustomEnvironments(const FString& gameName);
    void LoadLastUsedEnvironment();
    void LoadLastUsedRegion();
    bool TryLoadAwsCredentialsFromFile();
    bool IsGameNameValid(const FString& gameName);
    bool IsEnvironmentNameValid(const FString& environmentName);
    bool IsEnvironmentNameInUse(const FString& environmentName) const;
    bool IsEnvironmentCodeValid(const FString& environmentCode);
    bool IsEnvironmentCodeInUse(const FString& environmentCode);
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
    FReply OnChangeEnvironmentAndCredentials();
    FReply OnCancelEnvironmentAndCredentialsChange();
    FReply OnSubmit();
    bool IsSwitchEnvironmentButtonEnabled() const;
    EVisibility IsSwitchEnvironmentButtonVisible() const;

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
    static bool IsAccessKeyFieldValid(const FText& accessKeyFieldText);
    static bool IsSecretKeyFieldValid(const FText& accessKeyFieldText);
    static TArray<FString> GetInvalidRegexCharacters(const FRegexPattern& regexPattern, const FString& input);
    static bool InputContainsReservedKeyword(const FString& input, FString& reservedKeywordOutput);

public:
    AwsGameKitCredentialsLayoutDetails(FAwsGameKitEditorModule* editorModule);

    ~AwsGameKitCredentialsLayoutDetails();

    /**
    * @brief Creates a new instance.
    * @return A new property type customization.
    */
    static TSharedRef<IDetailCustomization> MakeInstance(FAwsGameKitEditorModule* editorModule);

    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

    void SetInitialState();
};
