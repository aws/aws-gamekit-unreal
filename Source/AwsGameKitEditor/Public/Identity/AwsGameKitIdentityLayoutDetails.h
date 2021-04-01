// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitFeatureLayoutDetails.h"

// Unreal
#include "Containers/UnrealString.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"

// Unreal forward declarations
class FMessageEndpoint;
class IMessageContext;
class SButton;
class SCheckBox;
class SEditableTextBox;
class SVerticalBox;
enum class ECheckBoxState : uint8;
struct FMsgCredentialsState;

class AWSGAMEKITEDITOR_API AwsGameKitIdentityLayoutDetails : public AwsGameKitFeatureLayoutDetails
{
public:
    static const FString GAMEKIT_IDENTITY_SECRET_SECURED;
    static const FString GAMEKIT_IDENTITY_EMAIL_ENABLED;
    static const FString GAMEKIT_IDENTITY_FACEBOOK_ENABLED;
    static const FString GAMEKIT_IDENTITY_FACEBOOK_APP_ID;
    static const FString GAMEKIT_IDENTITY_FACEBOOK_APP_SECRET;

    AwsGameKitIdentityLayoutDetails(const FAwsGameKitEditorModule* editorModule);
    ~AwsGameKitIdentityLayoutDetails() = default;

    /**
    * @brief Creates a new instance.
    * @return A new property type customization.
    */
    static TSharedRef<IDetailCustomization> MakeInstance(const FAwsGameKitEditorModule* editorModule);

    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

private:
    // for managing when to populate from config files
    bool facebookSectionDisplayed;

    // UI elements
    TSharedPtr<SVerticalBox> fieldsSection;
    TSharedPtr<SVerticalBox> facebookSection;

    IDetailCategoryBuilder* identityCategoryBuilder = nullptr;

    // email
    TSharedPtr<SCheckBox> emailCheckbox;

    // facebook
    TSharedPtr<SEditableTextBox> facebookAppId;
    TSharedPtr<SEditableTextBox> facebookAppSecret;
    TSharedPtr<SCheckBox> facebookCheckbox;

    void DisplayFacebookSection(bool display);

    // Event Handlers
    void ToggleEmail(ECheckBoxState NewCheckboxState);
    void ToggleFacebook(ECheckBoxState NewCheckboxState);
    void SetFacebookEnabledControls();
    void SetEmailEnabledControls();
    void OnConfigFieldCommitted(const FText& fieldText, ETextCommit::Type type);
    void SaveSettings();

    virtual void CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context) override;

    // Pricing warning for Identity (first) deployment
    TSharedRef<SWidget> GenerateAWSCostWarning(const FSimpleDelegate& InOnCreate, const FSimpleDelegate& InOnCancel);
    void OpenFirstDeploymentDialog(EAppReturnType::Type* serviceCostWarningResponse);

    // Deployment override
    FReply DeployFeature() override;
};