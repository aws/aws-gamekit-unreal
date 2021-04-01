// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitControlCenterLayout.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"

// Standard library
#include <functional>

// Unreal
#include "Brushes/SlateColorBrush.h"
#include "DesktopPlatform/Public/DesktopPlatformModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "Framework/Docking/TabManager.h"
#include "IDetailGroup.h"
#include "Interfaces/IPluginManager.h"
#include "ISettingsModule.h"
#include "MessageEndpointBuilder.h"
#include "Misc/FileHelper.h"
#include "PropertyCustomizationHelpers.h"
#include "Runtime/Core/Public/Async/Async.h"
#include "Runtime/Core/Public/Internationalization/Regex.h"
#include "Runtime/SlateCore/Public/Brushes/SlateImageBrush.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SHyperLink.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitControlCenter"

static const FString CreateAccountUrl("https://aws.amazon.com");
static const FString LearnMoreUrl("https://aws.amazon.com/free/");
static const FString RecoverPasswordUrl("https://alpha-docs-aws.amazon.com/gamekit/latest/DevGuide/setting-up-credentials.html");

#define NEW_CUSTOM_ENV_KEY  ":::"

const FName AwsGameKitControlCenterLayout::ControlCenterHomeTabName("AwsGameKitControlCenterHomeTab");

static void OnRichTextLinkClicked(const FSlateHyperlinkRun::FMetadata& metadata)
{
    const FString* url = metadata.Find(TEXT("href"));

    if (url)
    {
        FString error;
        FPlatformProcess::LaunchURL(**url, nullptr, &error);

        if (error.Len() > 0)
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("%s"), *error);
        }
    }
}

static void OnOpenProjectSettingsClicked(const FSlateHyperlinkRun::FMetadata& metadata)
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();

    // Close the Control Center
    editorModule->CloseControlCenter();

    // Open the AWS GameKit Project Settings
    editorModule->OpenProjectSettings();
}

AwsGameKitControlCenterLayout::AwsGameKitControlCenterLayout()
{

}

AwsGameKitControlCenterLayout::~AwsGameKitControlCenterLayout()
{
    // Unregister tab for Control Center
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitControlCenterLayout::~AwsGameKitControlCenterLayout()"));
}

TSharedRef<AwsGameKitControlCenterLayout> AwsGameKitControlCenterLayout::MakeInstance()
{
    TSharedRef<AwsGameKitControlCenterLayout> layoutDetails = MakeShareable(new AwsGameKitControlCenterLayout());

    // Populate controls
    layoutDetails->PopulateEnvironments();
    layoutDetails->PopulateRegions();

    // Setup message bus
    layoutDetails->messageEndpoint = FMessageEndpoint::Builder(AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME).Build();

    // Initialize UI so feature details panels can use class
    layoutDetails->GetHomeTab();

    return layoutDetails;
}

void AwsGameKitControlCenterLayout::CustomizeDetails(IDetailLayoutBuilder& detailBuilder)
{
    // TODO::Make extend different class, or no class and get rid of this
}

TSharedRef<SDockTab> AwsGameKitControlCenterLayout::OnSpawnHomeTab(const FSpawnTabArgs& spawnTabArgs)
{
    TSharedRef<SDockTab> homeTab = this->GetHomeTab();
    homeTab->SetCanCloseTab(SDockTab::FCanCloseTab::CreateRaw(this, &AwsGameKitControlCenterLayout::GetCanCloseControlHomeTab));
    return homeTab;
}

bool AwsGameKitControlCenterLayout::GetCanCloseControlHomeTab()
{
    if (switchEnvironmentsButton->IsEnabled() == true || switchEnvironmentsNotification->GetVisibility() == EVisibility::Visible) 
    {
        return true;
    }

    TSharedPtr<EditorState> editorState = AWSGAMEKIT_EDITOR_STATE();
    if(!editorState->GetCredentialState())
    {
        FText MessageTitle(LOCTEXT("CloseHomeTab", "Are you sure?"));
        const auto reply = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("NoSubmittedCredentials", "You have not submitted any credentials. You will not be able to use some GameKit features until credentials are submitted. Do you want to leave without submitting?"), &MessageTitle);
        if (reply == EAppReturnType::Yes || reply == EAppReturnType::Continue)
        {
            LoadLastUsedEnvironment();
            LoadLastUsedRegion();
            return true;
        }

        return false;
    }

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    FString const lastUsedEnv = editorModule->GetFeatureResourceManager()->GetLastUsedEnvironment();
    const FString key = environmentMapping[lastUsedEnv];

    FText MessageTitle(LOCTEXT("CloseHomeTab", "Are you sure?"));
    const auto reply = FMessageDialog::Open(EAppMsgType::YesNo, FText::Format(LOCTEXT("NoSubmittedCredentials", "You have not submitted your credentials for the new environment. If you exit now your environment will return to {0}. Do you want to leave without without switching?"), FText::FromString(key)), &MessageTitle);
    if (reply == EAppReturnType::Yes || reply == EAppReturnType::Continue)
    {
        LoadLastUsedEnvironment();
        LoadLastUsedRegion();
        return true;
    }
  
    return false;
}

TSharedRef<SDockTab> AwsGameKitControlCenterLayout::GetHomeTab()
{
    auto textBlock = [](const FString& s, const FSlateFontInfo& font)
    {
        return SNew(STextBlock)
            .AutoWrapText(true)
            .Justification(ETextJustify::Left)
            .Font(font)
            .Text(FText::FromString(s));
    };

    auto dynamicTextBlock = [](std::function<FText(void)> textFunc, const FSlateFontInfo& font)
    {
        return SNew(STextBlock)
            .AutoWrapText(true)
            .Justification(ETextJustify::Left)
            .Font(font)
            .Text_Lambda(textFunc);
    };

    auto warningBox = [this, textBlock](TSharedPtr<SBorder>& boxBorder, const FString& message)
    {
        SAssignNew(boxBorder, SBorder)
        .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor("ErrorRed"))
        .BorderImage(AwsGameKitStyleSet::Style->GetBrush("ErrorRedBrush"))
        .Visibility(EVisibility::Collapsed) // warnings are hidden by default
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(2, 3, 2, 2)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .MaxHeight(10)
                [
                    SNew(SImage)
                    .Image(AwsGameKitStyleSet::Style->GetBrush("WarningIconSmall"))
                ]
            ]
            + SHorizontalBox::Slot()
            .Padding(2, 1, 0, 0)
            .VAlign(EVerticalAlignment::VAlign_Center)
            [
                textBlock(message, AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8"))
            ]
        ];

        return boxBorder.ToSharedRef();
    };

    auto dynamicWarningBox = [this, dynamicTextBlock](TSharedPtr<SBorder>& boxBorder, std::function<FText(void)> textFunc)
    {
        SAssignNew(boxBorder, SBorder)
        .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor("ErrorRed"))
        .BorderImage(AwsGameKitStyleSet::Style->GetBrush("ErrorRedBrush"))
        .Visibility(EVisibility::Collapsed) // warnings are hidden by default
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(2, 3, 2, 2)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .MaxHeight(10)
                [
                    SNew(SImage)
                    .Image(AwsGameKitStyleSet::Style->GetBrush("WarningIconSmall"))
                ]
            ]
            + SHorizontalBox::Slot()
            .Padding(2, 1, 0, 0)
            .VAlign(EVerticalAlignment::VAlign_Center)
            [
                dynamicTextBlock(textFunc, AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8"))
            ]
        ];

        return boxBorder.ToSharedRef();
    };

    auto dynamicInfoBox = [this, dynamicTextBlock](TSharedPtr<SBorder>& boxBorder, std::function<FText(void)> textFunc)
    {
        SAssignNew(boxBorder, SBorder)
        .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor(("InfoBlue")))
        .BorderImage(AwsGameKitStyleSet::Style->GetBrush("InfoBlueBrush"))
        .Visibility(EVisibility::Collapsed) // warnings are hidden by default
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(2, 3, 2, 2)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .MaxHeight(10)
                [
                    SNew(SImage)
                    .Image(AwsGameKitStyleSet::Style->GetBrush("WarningIcon"))
                ]
            ]
            + SHorizontalBox::Slot()
            .Padding(2, 1, 0, 0)
            .VAlign(EVerticalAlignment::VAlign_Center)
            [
                dynamicTextBlock(textFunc, AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8"))
            ]
        ];

        return boxBorder.ToSharedRef();
    };

    auto dynamicVisibilityInfoBox = [this, textBlock](TSharedPtr<SBorder>& boxBorder, const FString& message, std::function<EVisibility(void)> visibilityFunc)
    {
        SAssignNew(boxBorder, SBorder)
        .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor(("InfoBlue")))
        .BorderImage(AwsGameKitStyleSet::Style->GetBrush("InfoBlueBrush"))
        .Visibility_Lambda(visibilityFunc) // warnings are hidden by default
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(2, 3, 2, 2)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .MaxHeight(10)
                [
                    SNew(SImage)
                    .Image(AwsGameKitStyleSet::Style->GetBrush("WarningIcon"))
                ]
            ]
            + SHorizontalBox::Slot()
            .Padding(2, 1, 0, 0)
            .VAlign(EVerticalAlignment::VAlign_Center)
            [
                textBlock(message, AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8"))
            ]
        ];

        return boxBorder.ToSharedRef();
    };

    const FSlateFontInfo robotoBold10 = AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold10");
    const FSlateFontInfo robotoRegular10 = AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular10");
    const FSlateFontInfo robotoRegular8 = AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8");

    TSharedRef<SDockTab> homeTab = SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SBox)
            .Padding(10)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot() // Left pane (Control Center form)
                .HAlign(HAlign_Fill)
                .FillWidth(1)
                [   
                    SNew(SScrollBox) + SScrollBox::Slot() 
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot() // Wrap between scrollbar and content
                        .Padding(0, 10, 10, 0)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot() // Top - project name
                            .AutoHeight()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 10)
                                [
                                    textBlock("Configure GameKit settings for your game project", robotoBold10)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 2)
                                [
                                    SNew(SBox)
                                    .Visibility_Lambda([this]() -> EVisibility { return projectNameBox->GetVisibility() == EVisibility::Collapsed ? EVisibility::Visible : EVisibility::Collapsed; })
                                    [
                                        dynamicTextBlock([this]
                                            { return (FText::FromString("Game title: " + this->gameTitleText.ToString())); }, robotoRegular10)
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SAssignNew(projectNameBox, SVerticalBox)
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 0, 0, 2)
                                    [
                                        textBlock("Game title", robotoRegular10)
                                    ]
                                    + SVerticalBox::Slot()
                                     .AutoHeight()
                                     .Padding(0, 0, 0, 2)
                                    [
                                        textBlock("Provide a unique title for this GameKit configuration. It cannot be changed later", robotoRegular8)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SAssignNew(projectNameTextBox, SEditableTextBox)
                                        .IsEnabled(true)
                                        .OnTextChanged(this, &AwsGameKitControlCenterLayout::OnProjectNameTextChanged)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        warningBox(projectNameValidation, "Please provide a valid game title.")
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 2, 0, 0)
                                    [
                                        textBlock("Game title must contain 1-12 characters. Valid characters are lowercase a-z, 0-9.", robotoRegular8)
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .Padding(0, 10)
                                .AutoHeight()
                                .HAlign(HAlign_Right)
                                [
                                    SNew(SHyperlink)
                                    .Text(FText::FromString("Locate an existing GameKit configuration."))
                                    .OnNavigate(FSimpleDelegate::CreateLambda([this] { AwsGameKitControlCenterLayout::OnLoadCustomGameConfigFile(); }))
                                ]
                            ] // End Project name
                            + SVerticalBox::Slot() // Separator
                            .AutoHeight()
                            [
                                SNew(SSeparator)
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 10, 0, 0)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .HAlign(HAlign_Fill)
                                    .FillWidth(1)
                                    [
                                        SAssignNew(switchEnvironmentsButton, SButton)
                                        .HAlign(HAlign_Center)
                                        .Text(FText::FromString("Switch environments"))
                                        .IsEnabled_Lambda([this]() { return IsSwitchEnvironmentButtonEnabled(); })
                                        .OnClicked(this, &AwsGameKitControlCenterLayout::OnSwitchEnvironment)
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    dynamicVisibilityInfoBox(switchEnvironmentsNotification, "You can't switch environments while AWS resources are deploying or updating.", [this]() { return this->UpdateSwitchEnvironmentValidation(); })
                                ]
                            ]
                            + SVerticalBox::Slot() // Middle - Region and Environment
                            .AutoHeight()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 10)
                                [
                                    textBlock("Select an environment", robotoBold10)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 0, 0, 2)
                                [
                                    textBlock("Environment", robotoRegular10)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 0, 0, 0)
                                [
                                    SAssignNew(environmentComboBox, SComboBox<FComboBoxItem>)
                                    .OptionsSource(&environmentOptions)
                                    .OnGenerateWidget(this, &AwsGameKitControlCenterLayout::MakeWidget)
                                    .InitiallySelectedItem(currentEnvironment)
                                    .OnSelectionChanged(this, &AwsGameKitControlCenterLayout::OnEnvironmentSelectionChanged)
                                    [
                                        SNew(STextBlock)
                                        .Text(this, &AwsGameKitControlCenterLayout::GetEnvironmentItemLabel)
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(40, 10, 0, 0)
                                [
                                    SAssignNew(customEnvironmentBox, SVerticalBox)
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 0, 0, 2)
                                    [
                                        textBlock("Name", robotoRegular10)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SAssignNew(customEnvironmentNameTextBox, SEditableTextBox)
                                        .OnTextChanged(this, &AwsGameKitControlCenterLayout::OnCustomEnvironmentNameChanged)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        dynamicWarningBox(customEnvironmentNameValidation, [this]() { return this->environmentNameErrorText; })
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 2, 0, 0)
                                    [
                                        textBlock("Constraints: 1-16 alphanumeric characters and spaces.", robotoRegular8)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 10, 0, 2)
                                    [
                                        textBlock("Environment code", robotoRegular10)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SAssignNew(customEnvironmentCodeTextBox, SEditableTextBox)
                                        .OnTextChanged(this, &AwsGameKitControlCenterLayout::OnCustomEnvironmentCodeChanged)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        dynamicWarningBox(customEnvironmentCodeValidation, [this]() { return this->environmentCodeErrorText; })
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 2, 0, 0)
                                    [
                                        textBlock("Constraints: 2-3 lowercase alphanumeric characters.", robotoRegular8)
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 10, 0, 2)
                                [
                                    textBlock("Region", robotoRegular10)
                                ]
                                + SVerticalBox::Slot()
                                .Padding(20, 0, 0, 0)
                                .AutoHeight()
                                [
                                    SAssignNew(regionComboBox, SComboBox<FComboBoxItem>)
                                    .OptionsSource(&regionOptions)
                                    .OnGenerateWidget(this, &AwsGameKitControlCenterLayout::MakeWidget)
                                    .InitiallySelectedItem(currentRegion)
                                    .OnSelectionChanged(this, &AwsGameKitControlCenterLayout::OnRegionSelectionChanged)
                                    [
                                        SNew(STextBlock)
                                        .Text(this, &AwsGameKitControlCenterLayout::GetRegionItemLabel)
                                    ]
                                ]
                            ] // End Region-Environment
                            + SVerticalBox::Slot() // Bottom - AWS Credentials
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .Padding(0, 15, 0, 0)
                                .AutoHeight()
                                [
                                    textBlock("AWS account credentials", robotoBold10)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    dynamicInfoBox(newEnvironmentNotification, [this]()
                                    {
                                        if (this->GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY)
                                        {
                                            return FText::FromString("Set credentials for this environment. Use existing values (carried over from the previous environment) or enter new ones.");
                                        }
                                        return FText::FromString("Set credentials for " + this->GetEnvironmentItemLabel().ToString() + " environment. Use existing values (carried over from the previous environment) or enter new ones.");
                                    })
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    dynamicWarningBox(submitValidation, [this]() { return this->submitValidationText; })
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 10, 0, 2)
                                [
                                    textBlock("Access key ID", robotoRegular10)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 0, 0, 0)
                                [
                                    SAssignNew(accessKeyTextBox, SEditableTextBox)
                                    .IsEnabled(true)
                                    .OnTextChanged(this, &AwsGameKitControlCenterLayout::OnAccessKeyChanged)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 0, 0, 0)
                                [
                                    warningBox(accessKeyValidation, "Enter a valid access key ID.")
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 10, 0, 2)
                                [
                                    textBlock("Secret access key", robotoRegular10)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 0, 0, 0)
                                [
                                    SAssignNew(secretKeyTextBox, SEditableTextBox)
                                    .IsEnabled(true)
                                    .OnTextChanged(this, &AwsGameKitControlCenterLayout::OnSecretKeyChanged)
                                    .IsPassword(true)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 0, 0, 0)
                                [
                                    warningBox(secretKeyValidation, "Enter a valid secret access key.")
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 15, 0, 2)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    [
                                        dynamicTextBlock([this]
                                            { return (FText::FromString("AWS Account ID: " + this->accountIdText.ToString())); }, robotoRegular10)
                                    ]
                                    + SHorizontalBox::Slot()
                                    .AutoWidth()
                                    [
                                        SAssignNew(accountLoadingAnimationBox, SBox)
                                        .Padding(FMargin(5, 0, 0, 0))
                                        .WidthOverride(23)
                                        .HeightOverride(10)
                                        .HAlign(HAlign_Right)
                                        .VAlign(VAlign_Center)
                                        .Visibility(EVisibility::Collapsed)
                                        [
                                            SNew(SCircularThrobber)
                                            .Radius(10)
                                        ]
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 20, 0, 0)
                                [
                                    SNew(SHorizontalBox)
                                    +SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .HAlign(HAlign_Left)
                                    [
                                        SAssignNew(storeCredentialsCheckBox, SCheckBox)
                                        .IsChecked(ECheckBoxState::Checked) // Checked by design
                                    ]
                                    +SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .Padding(5, 0, 0, 0)
                                    .HAlign(HAlign_Left)
                                    [
                                        textBlock("Store my credentials", robotoRegular10)
                                    ]
                                ]
                                +SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 10, 0, 0)
                                [
                                    SNew(SHorizontalBox)
                                    +SHorizontalBox::Slot()
                                    .AutoWidth()
                                    .HAlign(HAlign_Left)
                                    [
                                        SAssignNew(submitButton, SButton)
                                        .HAlign(HAlign_Left)
                                        .Text(FText::FromString("Submit"))
                                        .OnClicked(this, &AwsGameKitControlCenterLayout::OnSubmit)
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .HAlign(HAlign_Left)
                                .Padding(0, 10, 0, 0)
                                [
                                    SNew(SHyperlink)
                                    .Text(FText::FromString("Help me get my user credentials"))
                                    .OnNavigate(FSimpleDelegate::CreateLambda([]() { AwsGameKitControlCenterLayout::OpenBrowser(RecoverPasswordUrl); }))
                                ]
                            ] // End AWS Credentials
                        ]
                    ]
                ] // End left pane
                + SHorizontalBox::Slot() // Middle (separator)
                .AutoWidth()
                [
                    SNew(SSeparator)
                    .Orientation(EOrientation::Orient_Vertical)
                ]
                + SHorizontalBox::Slot() // Right pane - AWS Text / Credentials Submitted / Empty
                .Padding(10, 10, 10, 10)
                .HAlign(HAlign_Left)
                [
                    SAssignNew(rightPane, SBox)
                ] // End right pane
            ]
        ];

    // Right pane - AWS Text
    SAssignNew(rightPaneAwsText, SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [
            textBlock("New to AWS?", robotoBold10)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [
            textBlock("If you want to get the full experience of what GameKit offers, create your "
            "AWS account and provide your credentials in the GameKit plugin. Your new AWS account "
            "comes with a slate of free usage benefits, including all of the AWS services that "
            "GameKit game features use. ", robotoRegular10)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 0, 0, 10)
        [
            textBlock("With an AWS account, you can get in-depth, hands-on experience with each "
            "GameKit game feature, all for free. You can work with the full GameKit plugin, customize "
            "each GameKit feature and add it to your game, create the necessary AWS cloud resources, "
            "and then test to see your new GameKit game features in action. Without an AWS account, "
            "you can view some areas of the GameKit plugin and explore the GameKit sample materials.", robotoRegular10)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .HAlign(HAlign_Left)
            [
                SNew(SButton)
                .Text(FText::FromString("Create an AWS account now"))
                .OnClicked_Lambda([]()->FReply { AwsGameKitControlCenterLayout::OpenBrowser(CreateAccountUrl); return FReply::Handled(); })
            ]
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            SNew(SRichTextBlock).Text(FText::FromString("<a id=\"external_link\" href=\"https://aws.amazon.com/free/\" style=\"Hyperlink\">Learn more</> about the AWS free tier."))
            + SRichTextBlock::HyperlinkDecorator(TEXT("external_link"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnRichTextLinkClicked))
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            SNew(SRichTextBlock).Text(FText::FromString("Ready to move beyond the free tier? <a id=\"external_link\" href=\"https://aws.amazon.com/getting-started/hands-on/control-your-costs-free-tier-budgets/\" style=\"Hyperlink\">Learn more</> about controlling costs with AWS."))
            + SRichTextBlock::HyperlinkDecorator(TEXT("external_link"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnRichTextLinkClicked))
        ];

    // Right pane - Credentials Submitted
    SAssignNew(rightPaneCredentialsSubmitted, SVerticalBox)
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0)
        [
            textBlock("Credentials successfully submitted", robotoBold10)
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(0, 10, 0, 0)
        [
            SNew(SRichTextBlock)
            .AutoWrapText(true)
            .Justification(ETextJustify::Center)
            .Text(FText::FromString("<a id=\"internal_link\" style=\"Hyperlink\">Continue to Project Settings &gt; AWS GameKit</>"))
            + SRichTextBlock::HyperlinkDecorator(TEXT("internal_link"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnOpenProjectSettingsClicked))
        ];

    // Initialize the right pane
    TSharedPtr<EditorState> const editorState = AWSGAMEKIT_EDITOR_STATE();
    if (!editorState->GetCredentialState())
    {
        // Show the "New to AWS?" text
        rightPane->SetContent(rightPaneAwsText.ToSharedRef());
        rightPane->SetVAlign(VAlign_Top);
    }

    return homeTab;
}

EVisibility AwsGameKitControlCenterLayout::UpdateSwitchEnvironmentValidation() const
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    return editorModule->GetFeatureControlCenter()->IsAnyFeatureUpdating() == true ? EVisibility::Visible : EVisibility::Collapsed;
}

bool AwsGameKitControlCenterLayout::IsSwitchEnvironmentButtonEnabled() const
{
    FAwsGameKitEditorModule* const editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    TSharedPtr<EditorState> const editorState = AWSGAMEKIT_EDITOR_STATE();
    return !editorModule->GetFeatureControlCenter()->IsAnyFeatureUpdating() && editorState->GetCredentialState() && !environmentComboBox->IsEnabled();
}

FReply AwsGameKitControlCenterLayout::OpenControlCenter()
{
    FGlobalTabmanager::Get()->TryInvokeTab(ControlCenterHomeTabName);

    SetInitialState();

    return FReply::Handled();
}

void AwsGameKitControlCenterLayout::SetInitialState()
{
    // Disable all but project name
    projectNameTextBox->SetEnabled(true);

    EnableInputBoxes(false);
    submitButton->SetEnabled(false);

    if (GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY)
    {
        customEnvironmentBox->SetVisibility(EVisibility::Visible);
    }
    else
    {
        customEnvironmentBox->SetVisibility(EVisibility::Collapsed);
    }

    // Try to load project name and credentials from config when a game name hasn't been entered
    if (gameTitleText.IsEmpty())
    {
        FString configFile;
        FString configGameName;
        if (TryFindConfigFile(configFile, configGameName))
        {
            gameTitleText = FText::FromString(configGameName);
            projectNameBox->SetVisibility(EVisibility::Collapsed);
            SetPartiallyCompleteState(configGameName);
            LoadLastUsedEnvironment();
            LoadLastUsedRegion();
        }
    }
    else if (IsGameNameValid(gameTitleText.ToString()))
    {
        projectNameBox->SetVisibility(EVisibility::Collapsed);
        LoadLastUsedEnvironment();
        LoadLastUsedRegion();
        if (TryLoadAwsCredentialsFromFile())
        {
            OnAwsCredentialsChanged(true);
        }
    }
}

void AwsGameKitControlCenterLayout::SetPartiallyCompleteState(const FString& gameName)
{
    EnableInputBoxes(true);

    submitButton->SetEnabled(false);

    PopulateCustomEnvironments(gameName);
    if (TryLoadAwsCredentialsFromFile())
    {
        OnAwsCredentialsChanged(true);
    }
}

bool AwsGameKitControlCenterLayout::TryFindConfigFile(FString& outConfigPath, FString& outGameName)
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();

    const FString& gamekitRoot = editorModule->GetFeatureResourceManager()->GetRootPath();
    TArray<FString> filesFound;
    IFileManager::Get().FindFilesRecursive(filesFound, *gamekitRoot, TEXT("saveInfo.yml"), true, false, true);

    if (filesFound.Num() > 0)
    {
        // Take the first result found
        outConfigPath = filesFound[0];
        UE_LOG(LogAwsGameKit, Log, TEXT("Found candidate config: %s"), *outConfigPath);
        return TryParseGameNameFromConfig(outConfigPath, outGameName);
    }

    return false;
}

bool AwsGameKitControlCenterLayout::TryParseGameNameFromConfig(const FString& configFilePath, FString& outGameName)
{
    FString configDir = FPaths::GetPathLeaf(FPaths::GetPath(configFilePath));
    outGameName = configDir;
    UE_LOG(LogAwsGameKit, Log, TEXT("Parsed game name from config: \"%s\""), *outGameName);

    return !outGameName.IsEmpty();
}

void AwsGameKitControlCenterLayout::PopulateCustomEnvironments(const FString& gameName)
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();

    editorModule->GetFeatureResourceManager()->SetGameName(gameName);
    TMap<FString, FString> envs = editorModule->GetFeatureResourceManager()->GetSettingsEnvironments();

    for (const auto& kvp : envs)
    {
        if (!this->environmentMapping.Contains(kvp.Key))
        {
            this->environmentMapping.Add(kvp.Key, kvp.Value);
            this->environmentOptions.Add(MakeShareable(new FString(kvp.Value)));
        }
    }
}

void AwsGameKitControlCenterLayout::LoadLastUsedEnvironment()
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    FString const lastUsedEnv = editorModule->GetFeatureResourceManager()->GetLastUsedEnvironment();

    const auto lastUsedEnvironmentList = environmentOptions.FindByPredicate([this, lastUsedEnv](const FComboBoxItem comboBox){ return (*comboBox.Get() == environmentMapping[lastUsedEnv]); });

    if (lastUsedEnvironmentList != nullptr)
    {
        // Find by predicate always returns an Array, in this case there is only one of each environment type
        if (currentEnvironment != lastUsedEnvironmentList[0]) 
        {
            environmentComboBox->SetSelectedItem(lastUsedEnvironmentList[0]);
        }
        return;
    }

    // If there is no last used environment default to dev
    environmentComboBox->SetSelectedItem(environmentOptions[0]);
}

void AwsGameKitControlCenterLayout::LoadLastUsedRegion()
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    FString const lastUsedRegion = editorModule->GetFeatureResourceManager()->GetLastUsedRegion();

    const auto lastUsedRegionList = regionOptions.FindByPredicate([this, lastUsedRegion](const FComboBoxItem comboBox) { return (*comboBox.Get() == regionMapping[lastUsedRegion]); });

    if (lastUsedRegionList != nullptr)
    {
        // Find by predicate always returns an Array, in this case there is only one of each region type
        if (currentRegion != lastUsedRegionList[0])
        {
            regionComboBox->SetSelectedItem(lastUsedRegionList[0]);
        }
        return;
    }

    // If there is no last used region default to us-east-1
    regionComboBox->SetSelectedItem(regionOptions[0]);
}


bool AwsGameKitControlCenterLayout::TryLoadAwsCredentialsFromFile()
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();

    FString accessKey;
    FString secretKey;

    FString envKey = GetSelectedEnvironmentKey();
    if (envKey == NEW_CUSTOM_ENV_KEY)
    {
        FString envName; // not used
        GetCustomEnvironment(envKey, envName);
    }

    auto credentialsManager = editorModule->GetCredentialsManager();
    credentialsManager->SetGameName(gameTitleText.ToString());
    credentialsManager->SetEnv(envKey);
    accessKey = credentialsManager->GetAccessKey();
    secretKey = credentialsManager->GetSecretKey();

    if (!accessKey.IsEmpty() && !secretKey.IsEmpty())
    {
        accessKeyTextBox->SetText(FText::FromString(accessKey));
        secretKeyTextBox->SetText(FText::FromString(secretKey));

        return true;
    }

    EnableInputBoxes(true);

    if (projectNameBox->GetVisibility() == EVisibility::Collapsed) {
        newEnvironmentNotification->SetVisibility(EVisibility::Visible);
    }

    return false;
}

bool AwsGameKitControlCenterLayout::IsGameNameValid(const FString& gameName)
{
    const FRegexPattern validGameNameRx("^[a-z0-9]+$");
    FRegexMatcher matcher(validGameNameRx, gameName);

    if (matcher.FindNext() && gameName.Len() <= 12)
    {
        return true;
    }

    return false;
}

bool AwsGameKitControlCenterLayout::IsEnvironmentNameValid(const FString& environmentName) const
{
    const FRegexPattern validEnvionmentNameRx("^[A-Za-z0-9 ]{1,16}$");
    FRegexMatcher matcher(validEnvionmentNameRx, environmentName);

    if (matcher.FindNext())
    {
        return true;
    }

    return false;
}

bool AwsGameKitControlCenterLayout::IsEnvironmentNameInUse(const FString& environmentName) const
{
    for (auto const& x : this->environmentMapping)
    {
        if(x.Value == environmentName)
        {
            return true;
        }
    }

    return false;
}

bool AwsGameKitControlCenterLayout::IsEnvironmentCodeValid(const FString& environmentCode) const
{
    const FRegexPattern validEnvionmentNameRx("^[a-z0-9]{2,3}$");
    FRegexMatcher matcher(validEnvionmentNameRx, environmentCode);

    if (matcher.FindNext())
    {
        return true;
    }

    return false;
}

bool AwsGameKitControlCenterLayout::IsEnvironmentCodeInUse(const FString& environmentCode) const
{
    if (this->environmentMapping.Contains(environmentCode))
    {
        return true;
    }

    return false;
}

void AwsGameKitControlCenterLayout::RetrieveAccountId(const FString& accessKey, const FString& secretKey)
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    auto featureResourceManager = editorModule->GetFeatureResourceManager();

    accountLoadingAnimationBox->SetVisibility(EVisibility::Visible);
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&, accessKey, secretKey, featureResourceManager]()
    {
        std::string access = TCHAR_TO_UTF8(*accessKey);
        std::string secret = TCHAR_TO_UTF8(*secretKey);

        FString accountId = featureResourceManager->GetAccountId(accessKey, secretKey);

        AsyncTask(ENamedThreads::GameThread, [&, accountId]() 
        {
            if (!accountId.IsEmpty())
            {
                accountIdText = FText::FromString(accountId);
                accessKeyValidation->SetVisibility(EVisibility::Collapsed);
                secretKeyValidation->SetVisibility(EVisibility::Collapsed);
            }
            else
            {
                accountIdText = FText::FromString("...");
                accessKeyValidation->SetVisibility(EVisibility::Visible);
                secretKeyValidation->SetVisibility(EVisibility::Visible);
            }

            if (environmentComboBox->IsEnabled())
            {
                submitButton->SetEnabled(true);
            }

            accountLoadingAnimationBox->SetVisibility(EVisibility::Collapsed);
        });
    });
}

void AwsGameKitControlCenterLayout::PopulateEnvironments()
{
    environmentMapping.Add("dev", "Development");
    environmentMapping.Add("qa", "QA");
    environmentMapping.Add("stg", "Staging");
    environmentMapping.Add("prd", "Production");
    environmentMapping.Add(NEW_CUSTOM_ENV_KEY, "Add new environment");

    for (auto& kvp : environmentMapping)
    {
        environmentOptions.Add(MakeShareable(new FString(kvp.Value)));
    }

    currentEnvironment = environmentOptions[0]; // Set to "dev" by design
}

void AwsGameKitControlCenterLayout::PopulateRegions()
{
    // List of all AWS regions (supported and unsupported)
    // All regions added here to also keep track of currently unsupported ones
    regionMapping.Add("us-east-1", "us-east-1: US East (N. Virginia)");
    regionMapping.Add("us-east-2", "us-east-2: US East (Ohio)");
    regionMapping.Add("us-west-1", "us-west-1: US West (N. California)");
    regionMapping.Add("us-west-2", "us-west-2: US West (Oregon)");
    regionMapping.Add("af-south-1", "af-south-1: Africa (Cape Town)");
    regionMapping.Add("ap-east-1", "ap-east-1: Asia Pacific (Hong Kong)");
    regionMapping.Add("ap-south-1", "ap-south-1: Asia Pacific (Mumbai)");
    regionMapping.Add("ap-northeast-3", "ap-northeast-3: Asia Pacific (Osaka)");
    regionMapping.Add("ap-northeast-2", "ap-northeast-2: Asia Pacific (Seoul)");
    regionMapping.Add("ap-southeast-1", "ap-southeast-1: Asia Pacific (Singapore)");
    regionMapping.Add("ap-southeast-2", "ap-southeast-2: Asia Pacific (Sydney)");
    regionMapping.Add("ap-northeast-1", "ap-northeast-1: Asia Pacific (Tokyo)");
    regionMapping.Add("ca-central-1", "ca-central-1: Canada (Central)");
    regionMapping.Add("eu-central-1", "eu-central-1: Europe (Frankfurt)");
    regionMapping.Add("eu-west-1", "eu-west-1: Europe (Ireland)");
    regionMapping.Add("eu-west-2", "eu-west-2: Europe (London)");
    regionMapping.Add("eu-south-1", "eu-south-1: Europe (Milan)");
    regionMapping.Add("eu-west-3", "eu-west-3: Europe (Paris)");
    regionMapping.Add("eu-north-1", "eu-north-1: Europe (Stockholm)");
    regionMapping.Add("me-south-1", "me-south-1: Middle East (Bahrain)");
    regionMapping.Add("sa-east-1", "sa-east-1: South America (Sao Paulo)");

    // List of all regions currently unsupported in GameKit
    // Remove from this list as these regions become supported
    unsupportedRegions.Add("af-south-1");
    unsupportedRegions.Add("ap-east-1");
    unsupportedRegions.Add("ap-northeast-3");
    unsupportedRegions.Add("eu-south-1");

    for (auto& kvp : regionMapping)
    {
        // Only add to regionOptions if not in unsupported regions array
        if (!unsupportedRegions.Contains(kvp.Key))
        {
            regionOptions.Add(MakeShareable(new FString(kvp.Value)));
        }
    }

    // Set to "us-west-2" by design
    static const int defaultRegionIndex = 3;
    currentRegion = regionOptions[defaultRegionIndex]; 
}

void AwsGameKitControlCenterLayout::OnRegionSelectionChanged(FComboBoxItem newValue, ESelectInfo::Type selectionType)
{
    currentRegion = newValue;
}

void AwsGameKitControlCenterLayout::OnEnvironmentSelectionChanged(FComboBoxItem newValue, ESelectInfo::Type selectionType)
{
    currentEnvironment = newValue;
    FString currentEnvironmentCode = GetSelectedEnvironmentKey();
    if (currentEnvironmentCode == NEW_CUSTOM_ENV_KEY)
    {
        newEnvironmentNotification->SetVisibility(EVisibility::Visible);
        customEnvironmentBox->SetVisibility(EVisibility::Visible);
        customEnvironmentNameTextBox->SetEnabled(true);
        customEnvironmentCodeTextBox->SetEnabled(true);

        if (customEnvironmentNameTextBox->GetText().IsEmpty() ||
            customEnvironmentCodeTextBox->GetText().IsEmpty())
        {
            submitButton->SetEnabled(false);
        }
    }
    else
    {
        newEnvironmentNotification->SetVisibility(EVisibility::Collapsed);
        customEnvironmentBox->SetVisibility(EVisibility::Collapsed);
        customEnvironmentNameTextBox->SetEnabled(false);
        customEnvironmentCodeTextBox->SetEnabled(false);

        TryLoadAwsCredentialsFromFile();
    }

    OnCheckFields();
}

void AwsGameKitControlCenterLayout::OnCustomEnvironmentNameChanged(const FText& fieldText)
{
    EVisibility fieldValidationVisibility = EVisibility::Collapsed;

    if (!IsEnvironmentNameValid(fieldText.ToString()))
    {
        this->environmentNameErrorText = FText::FromString("A valid environment code is required.");
        fieldValidationVisibility = EVisibility::Visible;
    }

    if (IsEnvironmentNameInUse(fieldText.ToString()))
    {
        this->environmentNameErrorText = FText::FromString("Environment name is already in use.");
        fieldValidationVisibility = EVisibility::Visible;
    }

    customEnvironmentNameValidation->SetVisibility(fieldValidationVisibility);

    OnCheckFields();
}

void AwsGameKitControlCenterLayout::OnCustomEnvironmentCodeChanged(const FText& fieldText)
{
    EVisibility fieldValidationVisibility = EVisibility::Collapsed;

    if (!IsEnvironmentCodeValid(fieldText.ToString()))
    {
        this->environmentCodeErrorText = FText::FromString("A valid environment code is required.");
        fieldValidationVisibility = EVisibility::Visible;
    }

    if (IsEnvironmentCodeInUse(fieldText.ToString()))
    {
        this->environmentCodeErrorText = FText::FromString("Environment code is already in use.");
        fieldValidationVisibility = EVisibility::Visible;
    }

    customEnvironmentCodeValidation->SetVisibility(fieldValidationVisibility);

    OnCheckFields();
}

void AwsGameKitControlCenterLayout::OnProjectNameTextChanged(const FText& fieldText)
{
    EVisibility fieldValidationVisibility = EVisibility::Collapsed;
    gameTitleText = fieldText;

    FString projectName = fieldText.ToString();

    if (IsGameNameValid(projectName))
    {
        SetPartiallyCompleteState(projectName);
    }
    else
    {
        SetInitialState();
        fieldValidationVisibility = EVisibility::Visible;
    }

    projectNameValidation->SetVisibility(fieldValidationVisibility);
}

void AwsGameKitControlCenterLayout::OnLoadCustomGameConfigFile()
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();

    FString file;
    FString gameName;
    FString rootPath = editorModule->GetFeatureResourceManager()->GetRootPath();

    if (TrySelectGameConfigFile(rootPath, file) && TryParseGameNameFromConfig(file, gameName))
    {
        gameTitleText = FText::FromString(gameName);
    }

    // Reinitialize settings
    editorModule->GetFeatureResourceManager()->InitializeSettings(true);
}

void AwsGameKitControlCenterLayout::OnAccessKeyChanged(const FText& fieldText)
{
    newEnvironmentNotification->SetVisibility(EVisibility::Collapsed);

    const FString accessKey = fieldText.ToString();
    const FRegexPattern validAccessKeyRx("^[A-Z0-9]+$");
    FRegexMatcher matcher(validAccessKeyRx, accessKey);

    // Length of access keys should be 16-128 characters, but not checking min length to avoid
    // showing the user a warning while they type the key.
    bool valid = matcher.FindNext() && accessKey.Len() <= 128;

    EVisibility fieldValidationVisibility = EVisibility::Collapsed;
    if (!valid)
    {
        fieldValidationVisibility = EVisibility::Visible;
    }

    accessKeyValidation->SetVisibility(fieldValidationVisibility);

    OnAwsCredentialsChanged(valid);
}

void AwsGameKitControlCenterLayout::OnSecretKeyChanged(const FText& fieldText)
{
    newEnvironmentNotification->SetVisibility(EVisibility::Collapsed);

    // Length of secret keys should be 40 characters, but only checking for length > 40 to avoid
    // showing the user a warning while they type the secret.
    FString secretKey = fieldText.ToString();
    bool valid = !fieldText.IsEmptyOrWhitespace() && secretKey.Len() <= 40;

    EVisibility fieldValidationVisibility = EVisibility::Collapsed;
    if (!valid)
    {
        fieldValidationVisibility = EVisibility::Visible;
    }

    secretKeyValidation->SetVisibility(fieldValidationVisibility);

    OnAwsCredentialsChanged(valid);
}

void AwsGameKitControlCenterLayout::OnAwsCredentialsChanged(bool areFieldsValid)
{
    submitButton->SetEnabled(false);
    submitValidation->SetVisibility(EVisibility::Collapsed);

    if (!areFieldsValid)
    {
        return;
    }

    FString accessKey = accessKeyTextBox->GetText().ToString();
    FString secretKey = secretKeyTextBox->GetText().ToString();
    FText accountId = accountIdText;

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    auto featureResourceManager = editorModule->GetFeatureResourceManager();
    TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    // Only proceed to auto-retrieve account Id if the fields have the required lengths.
    //TODO - Change this to only validate when we leave focus to avoid hard length checking 
    if (accessKey.Len() !=20  || secretKey.Len() != 40)
    {
        this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ false });
        return;
    }

    AccountDetails accountDetails = GetAccountDetails();

    if (editorState->AreCredentialsValid())
    {
        this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ true });
    }

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, featureResourceManager, accountDetails, accessKey, secretKey, editorState]()
    {
        const bool valid = featureResourceManager->IsAccountInfoValid(accountDetails);

        AsyncTask(ENamedThreads::GameThread, [this, valid, accessKey, secretKey, editorState]()
        {
            if (valid)
            {
                RetrieveAccountId(accessKey, secretKey);

                if (editorState->GetCredentialState())
                {
                    EnableInputBoxes(false);
                    submitButton->SetEnabled(false);

                    // Empty the right pane
                    rightPane->SetContent(SNew(SBox));
                }
                else
                {
                    EnableInputBoxes(true);
                }
            }
            else
            {
                EnableInputBoxes(true);

                submitValidationText = FText::FromString("The AWS credentials entered are not valid.");
                accountIdText = FText::FromString("...");
                submitValidation->SetVisibility(EVisibility::Visible);
            }
        });
    });
}

void AwsGameKitControlCenterLayout::OnCheckFields()
{
    // check fields and enable submit button if all are filled

    bool validEnvironment = GetSelectedEnvironmentKey() != NEW_CUSTOM_ENV_KEY ||
        (customEnvironmentNameValidation->GetVisibility() == EVisibility::Collapsed && customEnvironmentCodeValidation->GetVisibility() == EVisibility::Collapsed
            && !customEnvironmentCodeTextBox->GetText().IsEmpty() && !customEnvironmentNameTextBox->GetText().IsEmpty());

    if (!validEnvironment)
    {
        submitValidationText = FText::FromString("Please enter a valid environment.");
        accessKeyTextBox->SetEnabled(false);
        secretKeyTextBox->SetEnabled(false);
        submitButton->SetEnabled(false);
        return;
    }

    accessKeyTextBox->SetEnabled(true);
    secretKeyTextBox->SetEnabled(true);

    if (!gameTitleText.IsEmpty() &&
        validEnvironment &&
        !accessKeyTextBox->GetText().IsEmpty() &&
        !secretKeyTextBox->GetText().IsEmpty() &&
        !accountIdText.EqualTo(FText::FromString("...")))
    {
        submitButton->SetEnabled(true);
    }
    else
    {
        submitValidationText = FText::FromString("The AWS credentials entered are not valid.");
        submitButton->SetEnabled(false);
    }
}

FReply AwsGameKitControlCenterLayout::OnSwitchEnvironment ()
{
    FText MessageTitle(LOCTEXT("SwitchEnvironments", "Switch Environments"));
    const auto reply = FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("ChangeEnvAndCreds", "You can switch to another environment, change the AWS Region for deployments, or enter new AWS credentials. After changing settings, you must choose Submit. Are you sure that you want to change environment settings?  \
        \n\nNOTE: After submitting new environment settings, you must restart Unreal Editor."), &MessageTitle);
    if (reply == EAppReturnType::No || reply == EAppReturnType::Cancel)
    {
        return FReply::Handled();;
    }

    submitButton->SetEnabled(true);

    // Empty the right pane
    rightPane->SetContent(SNew(SBox));

    if (GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY) {
        environmentComboBox->SetSelectedItem(environmentOptions.Last());

        customEnvironmentNameTextBox->SetText(FText::FromString(""));
        customEnvironmentCodeTextBox->SetText(FText::FromString(""));

        customEnvironmentNameValidation->SetVisibility(EVisibility::Collapsed);
        customEnvironmentCodeValidation->SetVisibility(EVisibility::Collapsed);
    }

    EnableInputBoxes(true);
    submitButton->SetEnabled(true);

    TSharedPtr<EditorState> editorState = AWSGAMEKIT_EDITOR_STATE();

    return FReply::Handled();
}

FReply AwsGameKitControlCenterLayout::OnSubmit()
{
    newEnvironmentNotification->SetVisibility(EVisibility::Collapsed);

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    auto featureResourceManager = editorModule->GetFeatureResourceManager();
    auto credentialsManager = editorModule->GetCredentialsManager();
    auto featureControlCenter = editorModule->GetFeatureControlCenter();

    EVisibility fieldValidationVisibility = EVisibility::Collapsed;
    submitValidation->SetVisibility(fieldValidationVisibility);

    bool customEnv = GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY;
    AccountDetails accountDetails = GetAccountDetails();
    SetAccountDetails();

    gameTitleText = FText::FromString(accountDetails.gameName);
    projectNameBox->SetVisibility(EVisibility::Collapsed);

    // block all controls
    EnableInputBoxes(false);

    BulkSetEnabled(
        {
            projectNameTextBox.Get(),
            submitButton.Get()
        }
    , false);

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&, customEnv, accountDetails, featureResourceManager, credentialsManager, featureControlCenter]()
    {
        IntResult result = featureResourceManager->BootstrapAccount();

        AsyncTask(ENamedThreads::GameThread, [&, result, customEnv, accountDetails, featureResourceManager, credentialsManager, featureControlCenter]()
        {
            if (result.Result == GameKit::GAMEKIT_SUCCESS)
            {
                fieldValidationVisibility = EVisibility::Collapsed;

                // save credentials
                if (storeCredentialsCheckBox->IsChecked())
                {
                    credentialsManager->SetGameName(accountDetails.gameName);
                    credentialsManager->SetEnv(accountDetails.environment);
                    credentialsManager->SetAccessKey(accountDetails.accessKey);
                    credentialsManager->SetSecretKey(accountDetails.accessSecret);
                    credentialsManager->SaveCredentials();
                }

                featureResourceManager->SaveSettings();

                // save custom environment
                if (customEnv)
                {
                    FString envCode;
                    FString envName;
                    GetCustomEnvironment(envCode, envName);
                    featureResourceManager->SaveCustomEnvironment(envCode, envName);
                    PopulateCustomEnvironments(accountDetails.gameName);
                }

                // Get editor module and save editor state
                TSharedPtr<EditorState> editorState = AWSGAMEKIT_EDITOR_STATE();
                editorState->SetCredentials(accountDetails);
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                runtimeModule->ReloadConfigFile(featureResourceManager->GetClientConfigSubdirectory());

                // TODO::Find better way to trigger redraw
                // Set state so details panels can know when to update.
                this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ true });

                // Reset our feature statuses so we don't incorrectly
                //  show statuses from a previously selected stack
                featureControlCenter->ResetFeatureStatuses();
                featureControlCenter->RefreshFeatureStatuses();

                // If the environment dropdown is left open when the user submits it will stay active, this allows makes sure its closed
                environmentComboBox->SetIsOpen(false);

                // Inform user the credentials were valid and provide a link back to the Project Settings
                rightPane->SetContent(rightPaneCredentialsSubmitted.ToSharedRef());
                rightPane->SetVAlign(VAlign_Center);
            }
            else
            {
                EnableInputBoxes(true);
                submitButton->SetEnabled(true);

                fieldValidationVisibility = EVisibility::Visible;
                FString hexError = FString(GameKit::StatusCodeToHexStr(result.Result).c_str());
                submitValidationText = FText::FromString("The user credentials you provided cannot be validated.\nPlease enter a valid access key pair or create a new one using AWS IAM.");
                UE_LOG(LogAwsGameKit, Error, TEXT("The user credentials you provided cannot be validated: error %s"), *hexError);

                submitValidation->SetVisibility(fieldValidationVisibility);
            }
        });
    });
    return FReply::Handled();
}

TSharedRef<SWidget> AwsGameKitControlCenterLayout::MakeWidget(FComboBoxItem item)
{
    return SNew(STextBlock).Text(FText::FromString(*item));
}

FText AwsGameKitControlCenterLayout::GetRegionItemLabel() const
{
    return FText::FromString(*currentRegion.Get());
}

FText AwsGameKitControlCenterLayout::GetEnvironmentItemLabel() const
{
    return FText::FromString(*currentEnvironment.Get());
}

bool AwsGameKitControlCenterLayout::TrySelectGameConfigFile(const FString& rootPath, FString& outFilePath)
{
    IDesktopPlatform* platform = FDesktopPlatformModule::Get();
    void* handle = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
    TArray<FString> files;
    bool dialogResult = platform->OpenFileDialog(handle, "Open GameKit configuration file", rootPath, "saveInfo.yml", "YAML|*.yml", 0, files);
    if (dialogResult && files.Num() > 0)
    {
        outFilePath = files[0];
        return true;
    }

    UE_LOG(LogAwsGameKit, Error, TEXT("Configuration file wasn't selected."));

    return false;
}

FReply AwsGameKitControlCenterLayout::OpenBrowser(const FString& url)
{
    FString error;
    FPlatformProcess::LaunchURL(*url, nullptr, &error);

    if (error.Len() > 0)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("%s"), *error);
    }

    return FReply::Handled();
}

FString AwsGameKitControlCenterLayout::GetSelectedEnvironmentKey() const
{
    const FString* key = environmentMapping.FindKey(*currentEnvironment);

    return key != nullptr ? *key : "";
}

FString AwsGameKitControlCenterLayout::GetCurrentRegionKey() const
{
    const FString* key = regionMapping.FindKey(*currentRegion);

    return key != nullptr ? *key : "";
}

void AwsGameKitControlCenterLayout::GetCustomEnvironment(FString& outKey, FString& outName) const
{
    if (GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY)
    {
        outKey = customEnvironmentCodeTextBox->GetText().ToString();
        outName = customEnvironmentNameTextBox->GetText().ToString();
    }
}

AccountDetails AwsGameKitControlCenterLayout::GetAccountDetails() const
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();

    FString envCode = GetSelectedEnvironmentKey();
    if (envCode == NEW_CUSTOM_ENV_KEY)
    {
        envCode = this->customEnvironmentCodeTextBox->GetText().ToString();
    }

    AccountDetails accountDetails;
    accountDetails.environment = envCode;
    accountDetails.accountId = accountIdText.ToString();
    accountDetails.gameName = this->gameTitleText.ToString();
    accountDetails.region = GetCurrentRegionKey();
    accountDetails.accessKey = accessKeyTextBox->GetText().ToString();
    accountDetails.accessSecret = secretKeyTextBox->GetText().ToString();

    return accountDetails;
}

void AwsGameKitControlCenterLayout::SetAccountDetails() const
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    editorModule->GetFeatureResourceManager()->SetAccountDetails(GetAccountDetails());
}

void AwsGameKitControlCenterLayout::EnableInputBoxes(bool isEnabled)
{
    BulkSetEnabled(
        {
            environmentComboBox.Get(),
            customEnvironmentNameTextBox.Get(),
            customEnvironmentCodeTextBox.Get(),
            regionComboBox.Get(),
            accessKeyTextBox.Get(),
            secretKeyTextBox.Get(),
            storeCredentialsCheckBox.Get(),
        }, isEnabled);
}

void AwsGameKitControlCenterLayout::BulkSetEnabled(const TArray<SWidget*> widgets, bool enabled)
{
    for (auto& widget : widgets)
    {
        widget->SetEnabled(enabled);
    }
}

void AwsGameKitControlCenterLayout::BulkSetVisibility(const TArray<SWidget*> widgets, EVisibility visibility)
{
    for (auto& widget : widgets)
    {
        widget->SetVisibility(visibility);
    }
}

#undef LOCTEXT_NAMESPACE