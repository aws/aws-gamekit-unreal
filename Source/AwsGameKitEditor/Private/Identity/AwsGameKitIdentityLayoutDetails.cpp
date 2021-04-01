// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Identity/AwsGameKitIdentityLayoutDetails.h"

// GameKit
#include "AwsGameKitControlCenterLayout.h"
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitFeatureControlCenter.h"
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"
#include "Utils/AwsGameKitEditorUtils.h"
#include "Utils/AwsGameKitProjectSettingsUtils.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Editor.h"
#include "MessageEndpointBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/SlateStyle.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitLoginSettings"

#define INIT_PROVIDER_FIELDS(provider, idRef, secretRef) \
SNew(SVerticalBox)                          \
+ SVerticalBox::Slot()                      \
.AutoHeight()                               \
.Padding(0, 5)                              \
[                                           \
    SNew(SHorizontalBox)                    \
    + SHorizontalBox::Slot()                \
    [                                       \
        SNew(STextBlock)                    \
        .Text(FText::FromString(provider))  \
        .Font(AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold10")) \
    ]                                       \
]                                           \
+ SVerticalBox::Slot()                      \
.VAlign(VAlign_Top)                         \
.AutoHeight()                               \
[                                           \
    PROJECT_SETTINGS_ROW(                   \
        SNew(STextBlock)                    \
        .Text(FText::FromString("App ID:")),\
        SAssignNew(idRef, SEditableTextBox) \
        .OnTextCommitted(this, &AwsGameKitIdentityLayoutDetails::OnConfigFieldCommitted) \
    )                                       \
]                                           \
+ SVerticalBox::Slot()                      \
.VAlign(VAlign_Top)                         \
.AutoHeight()                               \
[                                           \
    PROJECT_SETTINGS_ROW(                   \
        SNew(STextBlock)                    \
        .Text(FText::FromString("App Secret:")), \
        SAssignNew(secretRef, SEditableTextBox) \
        .IsPassword(true)                   \
        .OnTextCommitted(this, &AwsGameKitIdentityLayoutDetails::OnConfigFieldCommitted) \
    )                                       \
]                                               

#define INIT_PROVIDER_CHECKBOX(provider, checkboxRef, changeHandler) \
+ SScrollBox::Slot()                                \
.Padding(2)                                         \
[                                                   \
    SNew(SHorizontalBox)                            \
    + SHorizontalBox::Slot()                        \
    .AutoWidth()                                    \
    .HAlign(HAlign_Left)                            \
    [                                               \
        SAssignNew(checkboxRef, SCheckBox)          \
        .OnCheckStateChanged(this, changeHandler)   \
    ]                                               \
    + SHorizontalBox::Slot()                        \
    .AutoWidth()                                    \
    .HAlign(HAlign_Left)                            \
    .Padding(5)                                     \
    [                                               \
        SNew(STextBlock)                            \
        .Text(FText::FromString(provider))          \
    ]                                               \
]

const FString AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_SECRET_SECURED = "Secured in AWS Secrets Manager";
const FString AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_EMAIL_ENABLED = "is_email_enabled";
const FString AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_ENABLED = "is_facebook_enabled";
const FString AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_APP_ID = "facebook_client_id";
const FString AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_APP_SECRET = "facebook_client_secret";

AwsGameKitIdentityLayoutDetails::AwsGameKitIdentityLayoutDetails(const FAwsGameKitEditorModule* editorModule) : AwsGameKitFeatureLayoutDetails(FeatureType::Identity, editorModule)
{
}

TSharedRef<IDetailCustomization> AwsGameKitIdentityLayoutDetails::MakeInstance(const FAwsGameKitEditorModule* editorModule)
{
    // WHEN THIS IS CALLED FOR FIRST TIME FEATURE IS IMPLICITLY ACTIVATED
    // ADD NECESSARY INFO TO saveInfo.yml

    TSharedRef<AwsGameKitIdentityLayoutDetails> layoutDetails = MakeShareable(new AwsGameKitIdentityLayoutDetails(editorModule));
    TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    // Get editor module and update feature status
    if (editorState->AreCredentialsValid())
    {
        // Set Account details
        editorModule->GetFeatureResourceManager()->SetAccountDetails(editorState->GetCredentials());

        // Update feature status
        editorModule->GetFeatureControlCenter()->GetFeatureStatusAsync(FeatureType::Identity);
    }

    return layoutDetails;
}

void AwsGameKitIdentityLayoutDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
    // Add Identity UI 
    static const FName BindingsCategory = TEXT("Identity and Authentication");
    featureCategoryBuilder = &DetailBuilder.EditCategory(BindingsCategory);

    this->facebookSection = INIT_PROVIDER_FIELDS("Facebook", this->facebookAppId, this->facebookAppSecret);
    this->facebookSectionDisplayed = false;

    const FSlateFontInfo robotoBold11 = AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold11");

    featureCategoryBuilder->AddCustomRow(LOCTEXT("IdentityDescriptionRowFilter", "Identity | Description | GameKit | Game Kit | AWS"))
    [
        GetFeatureHeader(LOCTEXT("IdentityDescription", 
            "<DescriptionBoldText>This feature must be deployed before you can work with other AWS GameKit features.</> Sign players into your game to create player IDs, authenticate players to prevent cheating and fraud."))
    ];

    featureCategoryBuilder->AddCustomRow(LOCTEXT("IdentityConfigRowFilter", "Identity | Config | GameKit | Game Kit | AWS"))
    [
        SNew(SScrollBox)
        .IsEnabled_Raw(this, &AwsGameKitIdentityLayoutDetails::CanEditConfiguration)
        + SScrollBox::Slot()
        .Padding(0, 5)
        [
            SNew(STextBlock)
            .Text(FText::FromString("Login mechanism (one or more are required)"))
            .Font(robotoBold11)
        ]

        INIT_PROVIDER_CHECKBOX("Email / Password", emailCheckbox, &AwsGameKitIdentityLayoutDetails::ToggleEmail)
        INIT_PROVIDER_CHECKBOX("Facebook", facebookCheckbox, &AwsGameKitIdentityLayoutDetails::ToggleFacebook)

        + SScrollBox::Slot()
        [
            SNew(SVerticalBox)
            .Visibility_Lambda([this]
                {
                    if (this->facebookCheckbox->IsChecked())
                    {
                        return EVisibility::Visible;
                    }
                    return EVisibility::Collapsed;
                })
            + SVerticalBox::Slot()
            .Padding(0, 5)
            .MaxHeight(2.0f)
            [
                SNew(SBorder)
                .BorderBackgroundColor(FLinearColor::Black)
            ]
            + SVerticalBox::Slot()
            .Padding(0, 5, 0, 2)
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(FText::FromString("Identity provider credentials"))
                .Font(robotoBold11)
            ]
            + SVerticalBox::Slot()
            .VAlign(VAlign_Top)
            .AutoHeight()   
            .Padding(0, 5)
            [
                SNew(SHorizontalBox)                    
                + SHorizontalBox::Slot()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("To save credentials changes, deploy or update your Identity feature."))
                ]
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SAssignNew(this->fieldsSection, SVerticalBox)
            ]
        ]
    ];

    SetFacebookEnabledControls();
    SetEmailEnabledControls();

    featureCategoryBuilder->AddCustomRow(LOCTEXT("IdentityDeployRowFilter", "Identity | Deploy | GameKit | Game Kit | AWS"))
    [
        this->GetDeployControls()
    ];
}

void AwsGameKitIdentityLayoutDetails::ToggleFacebook(ECheckBoxState checkboxState)
{
    DisplayFacebookSection(checkboxState == ECheckBoxState::Checked);
}

void AwsGameKitIdentityLayoutDetails::DisplayFacebookSection(bool display)
{
    if (display && !this->facebookSectionDisplayed)
    {
        this->fieldsSection->AddSlot()[this->facebookSection.ToSharedRef()];
    }
    else if (!display && this->facebookSectionDisplayed)
    {
        this->fieldsSection->RemoveSlot(this->facebookSection.ToSharedRef());
    }

    this->facebookSectionDisplayed = display;
}

void AwsGameKitIdentityLayoutDetails::ToggleEmail(ECheckBoxState NewCheckboxState)
{

}

void AwsGameKitIdentityLayoutDetails::OnConfigFieldCommitted(const FText& fieldText, ETextCommit::Type type)
{
    AwsGameKitIdentityLayoutDetails::SaveSettings();
}

void AwsGameKitIdentityLayoutDetails::SaveSettings()
{
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();

    // Save Settings
    featureResourceManager->SetFeatureVariable(FeatureType::Identity, GAMEKIT_IDENTITY_EMAIL_ENABLED, this->emailCheckbox->IsChecked() ? "true" : "false");
    featureResourceManager->SetFeatureVariable(FeatureType::Identity, GAMEKIT_IDENTITY_FACEBOOK_ENABLED, this->facebookCheckbox->IsChecked() ? "true" : "false");
    featureResourceManager->SetFeatureVariable(FeatureType::Identity, GAMEKIT_IDENTITY_FACEBOOK_APP_ID, this->facebookAppId->GetText().ToString());

    // Save Secret
    if (!this->facebookAppSecret->GetText().IsEmpty())
    {
        featureResourceManager->SaveSecret(GAMEKIT_IDENTITY_FACEBOOK_APP_SECRET, this->facebookAppSecret->GetText().ToString());
    }
}

void AwsGameKitIdentityLayoutDetails::SetFacebookEnabledControls()
{
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();

    if (!editorModule->GetEditorState()->GetCredentialState())
    {
        return;
    }

    auto featureVars = editorModule->GetFeatureResourceManager()->GetFeatureVariables(FeatureType::Identity);
    if (featureVars.Contains(GAMEKIT_IDENTITY_FACEBOOK_ENABLED) && !featureVars[GAMEKIT_IDENTITY_FACEBOOK_ENABLED].IsEmpty())
    {
        bool const isFacebookEnabled = featureVars[GAMEKIT_IDENTITY_FACEBOOK_ENABLED] == EditorState::TrueString;
        this->facebookCheckbox->SetIsChecked(isFacebookEnabled ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
        if (isFacebookEnabled)
        {
            this->DisplayFacebookSection(this->facebookCheckbox->IsChecked());
            auto appId = featureVars[GAMEKIT_IDENTITY_FACEBOOK_APP_ID];
            this->facebookAppId->SetText(FText::FromString(featureVars[GAMEKIT_IDENTITY_FACEBOOK_APP_ID]));

            if(featureResourceManager->CheckSecretExists(GAMEKIT_IDENTITY_FACEBOOK_APP_SECRET).Result == GameKit::GAMEKIT_SUCCESS)
            {
                this->facebookAppSecret->SetHintText(FText::FromString(GAMEKIT_IDENTITY_SECRET_SECURED));
            }
            else
            {
                this->facebookAppSecret->SetText(FText::GetEmpty());
                this->facebookAppSecret->SetHintText(FText::GetEmpty());
            }
        }
    }
    else
    {
        this->facebookCheckbox->SetIsChecked(ECheckBoxState::Unchecked);
        this->facebookAppId->SetText(FText::GetEmpty());
        this->facebookAppSecret->SetText(FText::GetEmpty());
        this->facebookAppSecret->SetHintText(FText::GetEmpty());
    }
}

void AwsGameKitIdentityLayoutDetails::SetEmailEnabledControls()
{
    this->emailCheckbox->SetIsChecked(true);
    this->emailCheckbox->SetEnabled(false);
}

TSharedRef<SWidget> AwsGameKitIdentityLayoutDetails::GenerateAWSCostWarning(const FSimpleDelegate& InOnCreate, const FSimpleDelegate& InOnCancel)
{
    const FMargin buttonPadding = FMargin(22.5f, 2.5f);
    const FVector2D buttonParentPadding = FVector2D(10.0f, 2.0f);

    const FText warningMessage = FText::Format(LOCTEXT("FeatureAwsServiceLabel",
        "When any AWS GameKit feature is deployed, you may begin incurring charges based on your usage. If you're using AWS Free Tier, most or all of these charges are waived during this limited period.\
        \n\nLearn more about managing AWS costs in the <a id=\"external_link\" href=\"{0}\" style=\"ModalHyperlink\">AWS GameKit Developer Guide.</> You can track your usage on the game feature dashboard."),
        FText::FromString(INTRO_COST_URL.c_str()));

    return SNew(SBorder)
           .BorderImage(AwsGameKitStyleSet::Style->GetBrush("BackgroundModalDialogBrush"))
           [
               SNew(SVerticalBox)
               + SVerticalBox::Slot()
               .Padding(10)
               .AutoHeight()
               [
                   SNew(SRichTextBlock)
                   .Text(warningMessage)
                   .AutoWrapText(true)
                   .TextStyle(AwsGameKitStyleSet::Style, "ModalDialogText")
                   .DecoratorStyleSet(AwsGameKitStyleSet::Style.Get())
                   + SRichTextBlock::HyperlinkDecorator(TEXT("external_link"), FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
                   const FString* url = Metadata.Find(TEXT("href"));
                           if (url)
                           {
                               AwsGameKitEditorUtils::OpenBrowser(*url);
                           }
                       }))
               ]
               + SVerticalBox::Slot()
               .Padding(6)
               [
                   SNew(SSpacer)
               ]
               + SVerticalBox::Slot()
               .HAlign(HAlign_Right)
               .AutoHeight()
               [
                   SNew(SHorizontalBox)
                   + SHorizontalBox::Slot()
                   .AutoWidth()
                   .Padding(buttonParentPadding)
                    [
                        SNew(SButton)
                        .Text(FText::FromString("Deploy"))
                        .HAlign(HAlign_Right)
                        .ContentPadding(buttonPadding)
                        .OnClicked_Lambda([InOnCreate]()
                        {
                            InOnCreate.ExecuteIfBound();
                            return FReply::Handled();
                        })
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    .Padding(buttonParentPadding)
                    [
                        SNew(SButton)
                        .Text(FText::FromString("Cancel"))
                        .HAlign(HAlign_Right)
                        .ContentPadding(buttonPadding)
                        .OnClicked_Lambda([InOnCancel]()
                        {
                            InOnCancel.ExecuteIfBound();
                            return FReply::Handled();
                        })
                     
                   ]
               ]
           ];
}

void AwsGameKitIdentityLayoutDetails::OpenFirstDeploymentDialog(EAppReturnType::Type* serviceCostWarningResponse)
{
    const FVector2D clientSizeVector = FVector2D(550.0f, 200.0f);

    TSharedRef<SWindow> awsCostsNotification = SNew(SWindow)
        .Title(FText::FromString("AWS Service costs"))
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(clientSizeVector)
        .AutoCenter(EAutoCenter::PreferredWorkArea);

    TWeakPtr<SWindow> ModalWindowPtr = awsCostsNotification;

    awsCostsNotification->SetContent(
        GenerateAWSCostWarning(
            FSimpleDelegate::CreateLambda([ModalWindowPtr, serviceCostWarningResponse]()
                {
                    *serviceCostWarningResponse = EAppReturnType::Ok;
                    ModalWindowPtr.Pin()->RequestDestroyWindow();
                }),
            FSimpleDelegate::CreateLambda([ModalWindowPtr, serviceCostWarningResponse]()
                {
                    *serviceCostWarningResponse = EAppReturnType::Cancel;
                    ModalWindowPtr.Pin()->RequestDestroyWindow();
                })
         ));

    GEditor->EditorAddModalWindow(awsCostsNotification);
}

FReply AwsGameKitIdentityLayoutDetails::DeployFeature()
{
    // Validate facebook credentials and return early if invalid
    if (!editorModule->GetFeatureControlCenter()->IsValidProviderCredentialsInput(this->facebookCheckbox, this->facebookAppId, this->facebookAppSecret, GAMEKIT_IDENTITY_FACEBOOK_APP_SECRET))
    {
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString("Error: AwsGameKitIdentityLayoutDetails::DeployIdentity() Failed to deploy Identity. Please provide Facebook App ID and Secret."));
        return FReply::Handled();
    }

    SaveSettings();

    const TSharedPtr<EditorState> editorState = editorModule->GetEditorState();
    const TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();

    // Warning on a games first deployment, since Identity must be deployed before any other feature Identities status is checked 
    if (featureResourceManager->GetResourcesStackStatus(FeatureType::Identity) == FeatureResourceManager::UNDEPLOYED_STATUS_TEXT)
    {
        EAppReturnType::Type serviceCostWarningResponse = EAppReturnType::No;

        OpenFirstDeploymentDialog(&serviceCostWarningResponse);

        if (serviceCostWarningResponse == EAppReturnType::No || serviceCostWarningResponse == EAppReturnType::Cancel)
        {
            return FReply::Handled();;
        }
    }


    return AwsGameKitFeatureLayoutDetails::DeployFeature();
}

void AwsGameKitIdentityLayoutDetails::CredentialsStateMessageHandler(const FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitIdentityLayoutDetails::CredentialsStateMessageHandler(); Message(%d)"), message.IsSubmitted);
    if (message.IsSubmitted)
    {
        SetFacebookEnabledControls();
        SetEmailEnabledControls();
    }
}

#undef LOCTEXT_NAMESPACE
