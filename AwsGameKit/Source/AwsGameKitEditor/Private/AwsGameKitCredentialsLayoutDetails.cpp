// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitCredentialsLayoutDetails.h" // First include (Unreal requirement)

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitDocumentationManager.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitProjectSettingsUtils.h"
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
#include "Runtime/SlateCore/Private/Application/ActiveTimerHandle.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Images/SThrobber.h"
#include "Widgets/Input/SHyperLink.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/SRichTextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitCredentialsSettings"

// All keywords cannnot be used in the games title or environment code
static const TArray<FString> RESERVED_KEYWORDS = { "aws", "amazon", "cognito" };

static const FText GameKitIntroduction = LOCTEXT("GameKitIntroduction", "If you want to get the full experience of what GameKit offers, go to AWS to "
                                        "create an account, then provide your credentials in the GameKit plugin. Your new AWS account "
                                        "comes with a slate of free usage benefits, including all of the AWS services that "
                                        "GameKit game features use. ");

static const FText AwsIntroduction = LOCTEXT("AwsIntroduction", "With an AWS account, you can get in-depth, hands-on experience with each "
                                    "GameKit game feature, all for free. You can work with the full GameKit plugin, customize "
                                    "each GameKit feature and add it to your game, create the necessary AWS cloud resources, "
                                    "and then test to see your new GameKit game features in action. Without an AWS account, "
                                    "you can view some areas of the GameKit plugin and explore the GameKit sample materials.");

static const FText ChangeEnvironmentWarning = LOCTEXT("ChangeEnvAndCreds", "You can switch to another environment, change the AWS Region for deployments, or enter new AWS credentials. "
                                            "After changing settings, you must choose Submit. Are you sure that you want to change environment settings?"
                                            "\n\nNOTE: After submitting new environment settings, you must restart Unreal Editor.");

static const FString CreateAccountUrl = AwsGameKitDocumentationManager::GetDocumentString("url", "create_account");
static const FString GetCredentialsUrl = AwsGameKitDocumentationManager::GetDocumentString("dev_guide_url", "setting_up_credentials");

static const FString ClientConfigFile = "awsGameKitClientConfig.yml";

// This string is shown in place of the user's AWS Account ID when there is an error determining the Account ID or when either the access or secret keys are not syntactically correct.
const FString AwsGameKitCredentialsLayoutDetails::AWS_ACCOUNT_ID_EMPTY = "...";

static const FString NEW_CUSTOM_ENV_KEY = ":::";

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

    // Open the AWS GameKit Project Settings
    editorModule->OpenProjectSettings();
}

AwsGameKitCredentialsLayoutDetails::AwsGameKitCredentialsLayoutDetails(FAwsGameKitEditorModule* editorModule) : editorModule(editorModule)
{
}

AwsGameKitCredentialsLayoutDetails::~AwsGameKitCredentialsLayoutDetails()
{
    // Unregister tab for Control Center
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitCredentialsLayoutDetails::~AwsGameKitCredentialsLayoutDetails()"));

    // Unregister timer callback
    projectNameTextBox->UnRegisterActiveTimer(projectNameTimerHandle.ToSharedRef());
}

TSharedRef<IDetailCustomization> AwsGameKitCredentialsLayoutDetails::MakeInstance(FAwsGameKitEditorModule* editorModule)
{
    TSharedRef<AwsGameKitCredentialsLayoutDetails> layoutDetails = MakeShareable(new AwsGameKitCredentialsLayoutDetails(editorModule));

    // Populate controls
    layoutDetails->PopulateEnvironments();
    layoutDetails->PopulateRegions();

    // Setup message bus
    layoutDetails->messageEndpoint = FMessageEndpoint::Builder(AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME).Build();

    return layoutDetails;
}

void AwsGameKitCredentialsLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    static const FName BindingsCategory = TEXT("Environment and Credentials");
    configCategoryBuilder = &DetailLayout.EditCategory(BindingsCategory);

    TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter = editorModule->GetFeatureControlCenter();

    const TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    auto textBlock = [](const FText& text, const FSlateFontInfo& font)
    {
        return SNew(STextBlock)
            .AutoWrapText(true)
            .Justification(ETextJustify::Left)
            .Font(font)
            .Text(text);
    };

    auto dynamicTextBlock = [](std::function<FText(void)> textFunc, const FSlateFontInfo& font)
    {
        return SNew(STextBlock)
            .AutoWrapText(true)
            .Justification(ETextJustify::Left)
            .Font(font)
            .Text_Lambda(textFunc);
    };

    auto warningBox = [this, textBlock](TSharedPtr<SBorder>& boxBorder, const FText& message)
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

    auto dynamicVisibilityInfoBox = [this, textBlock](TSharedPtr<SBorder>& boxBorder, const FText& message, std::function<EVisibility(void)> visibilityFunc)
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

    const FSlateFontInfo robotoBold10 = AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold10");
    const FSlateFontInfo robotoRegular10 = AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular10");
    const FSlateFontInfo robotoRegular8 = AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8");

    configCategoryBuilder->AddCustomRow(LOCTEXT("CurrentConfigurationRowFilter", "Current Configuration | Config | Environment | Env | Control Center | GameKit | Game Kit | AWS"))
    [
        SNew(SBox)
        .Padding(0)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot() // Environment and Credentials
            .HAlign(HAlign_Fill)
            .FillWidth(1)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot() // Wrap between and content
                .Padding(0, 10, 10, 0)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot() // Top - project name
                    .AutoHeight()
                    [
                        SNew(SExpandableArea)
                        .InitiallyCollapsed(true)
                        .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor("LightGrey"))
                        .AreaTitlePadding(10)
                        .Padding(0)
                        .AreaTitle(LOCTEXT("NewToAwsText", "New to AWS?"))
                        .AreaTitleFont(robotoBold10)
                        .BodyContent()
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(15,5,0,15)
                            [
                                textBlock(GameKitIntroduction, robotoRegular10)
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(15, 0, 15, 15)
                            [
                                    textBlock(AwsIntroduction, robotoRegular10)
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(15, 0, 15, 5)
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .HAlign(HAlign_Left)
                                [
                                    SNew(SButton)
                                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGreen"))
                                    .Text(FText::FromString("Create an account"))
                                    .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                                    .OnClicked_Lambda([]()->FReply { AwsGameKitCredentialsLayoutDetails::OpenBrowser(CreateAccountUrl); return FReply::Handled(); })
                                ]
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(15, 5, 15, 0)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                .AutoWidth()
                                .VAlign(VAlign_Center)
                                [ 
                                    SNew(SRichTextBlock).Text(FText::FromString("<a id=\"external_link\" href=\"" +
                                        AwsGameKitDocumentationManager::GetDocumentString("url", "free_tier_intro") +
                                        "\" style=\"Hyperlink\">Learn more about the AWS free tier.</>"))
                                    + SRichTextBlock::HyperlinkDecorator(TEXT("external_link"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnRichTextLinkClicked))
                                ]
                                + SHorizontalBox::Slot()
                                .Padding(10, 2, 0, 2)
                                .AutoWidth()
                                .MaxWidth(15)
                                .HAlign(HAlign_Left)
                                .VAlign(VAlign_Center)
                                [
                                    EXTERNAL_ICON_BOX()
                                ]
                            ]
                            + SVerticalBox::Slot()
                            .AutoHeight()
                            .Padding(15, 10, 15, 15)
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .VAlign(VAlign_Center)
                                [
                                    SNew(SRichTextBlock).Text(FText::FromString("Ready to move beyond the free tier? <a id=\"external_link\" href=\"" +
                                        AwsGameKitDocumentationManager::GetDocumentString("url", "free_tier_reference") +
                                        "\" style=\"Hyperlink\">Learn more about controlling costs with AWS.</>"))
                                    + SRichTextBlock::HyperlinkDecorator(TEXT("external_link"), FSlateHyperlinkRun::FOnClick::CreateStatic(&OnRichTextLinkClicked))
                                ]
                                + SHorizontalBox::Slot()
                                .Padding(10, 2, 0, 2)
                                .AutoWidth()
                                .MaxWidth(15)
                                .HAlign(HAlign_Left)
                                .VAlign(VAlign_Center)
                                [
                                    EXTERNAL_ICON_BOX()
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot() // Top - project name
                    .AutoHeight()
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 10)
                        [
                            textBlock(LOCTEXT("ProjectCreationInstructions", "Create your project"), robotoBold10)
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(20, 0, 0, 2)
                        [
                            SNew(SBox)
                            .Visibility_Lambda([this]() -> EVisibility { return projectNameBox->GetVisibility() == EVisibility::Collapsed ? EVisibility::Visible : EVisibility::Collapsed; })
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .FillWidth(1)
                                [
                                    textBlock(LOCTEXT("GameTitleSet", "Game title: "), robotoRegular10)
                                ]
                                + SHorizontalBox::Slot()
                                .FillWidth(4)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .HAlign(HAlign_Left)
                                    .VAlign(VAlign_Center)
                                    [
                                        dynamicTextBlock([this]
                                            { return (FText::FromString( this->gameTitleText.ToString())); }, robotoRegular10)
                                    ]
                                ]
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(20, 0,0 ,0)
                        [
                            SAssignNew(projectNameBox, SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 2)
                                [
                                    textBlock(LOCTEXT("GameTitleNotSet", "Game title"), robotoRegular10)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 2, 0, 0)
                                [
                                    textBlock(LOCTEXT("UniqueAlias", "(This will create a unique alias for your project, and cannot be changed later)"), robotoRegular8)
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(4)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .Padding(0,0,10,0)
                                    [
                                        SNew(SVerticalBox)
                                        +SVerticalBox::Slot()
                                        [
                                            SAssignNew(projectNameTextBox, SEditableTextBox)
                                            .IsEnabled(true)
                                            .OnTextChanged(this, &AwsGameKitCredentialsLayoutDetails::OnProjectNameTextChanged)
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        [
                                            dynamicWarningBox(projectNameValidation, [this]() { return this->projectNameValidationErrorText; })
                                        ]
                                        + SVerticalBox::Slot()
                                        .AutoHeight()
                                        .Padding(0, 2, 0, 0)
                                        [
                                            textBlock(LOCTEXT("GameTitleRestriction", "The game title must have 1-12 characters. Valid characters: a-z, 0-9."), robotoRegular8)
                                        ]
                                    ]
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0, 10, 0, 0)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .HAlign(HAlign_Fill)
                        .FillWidth(1)
                        [
                            SNew(SSpacer)
                        ]   
                        + SHorizontalBox::Slot()
                        .HAlign(HAlign_Left)
                        .VAlign(VAlign_Center)
                        .Padding(20,0,0,0)
                        .FillWidth(4)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            [
                                SAssignNew(switchEnvironmentsButton, SButton)
                                .ContentPadding(FMargin(10, 2))
                                .HAlign(HAlign_Center)
                                .Text(LOCTEXT("ChangeEnvironmentAndCreds", "Change environment + credentials"))
                                .IsEnabled_Lambda([this]() { return IsSwitchEnvironmentButtonEnabled(); })
                                .Visibility_Lambda([this]() { return IsSwitchEnvironmentButtonVisible(); })
                                .ToolTipText_Lambda([this]()
                                {
                                    if (this->editorModule->GetFeatureControlCenter()->IsAnyFeatureUpdating())
                                    {
                                        return LOCTEXT("ChangeEnvironmentAndToolTip", "You can't switch environments while AWS resources are deploying or updating");
                                    }

                                    return FText::FromString("");
                                })
                                .OnClicked(this, &AwsGameKitCredentialsLayoutDetails::OnChangeEnvironmentAndCredentials)
                            ]
                            + SVerticalBox::Slot()
                            [
                                SAssignNew(cancelEnvironmentSwitchButton, SButton)
                                .ContentPadding(FMargin(10, 2))
                                .HAlign(HAlign_Center)
                                .Text(LOCTEXT("CancelEnvironmentAndCredsSwitch", "Cancel environment + credentials change"))
                                .Visibility(EVisibility::Collapsed)
                                .OnClicked(this, &AwsGameKitCredentialsLayoutDetails::OnCancelEnvironmentAndCredentialsChange)
                            ]
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
                            textBlock(LOCTEXT("SelectAnEnvironment", "Select an environment"), robotoBold10)
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(20, 0, 0, 2)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1)
                            [
                                textBlock(LOCTEXT("Environment", "Environment"), robotoRegular10)
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(4)
                            [
                                SNew(SVerticalBox)
                                +SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(0, 0, 0, 0)
                                [
                                    SAssignNew(environmentComboBox, SComboBox<FComboBoxItem>)
                                    .OptionsSource(&environmentOptions)
                                    .OnGenerateWidget(this, &AwsGameKitCredentialsLayoutDetails::MakeWidget)
                                    .InitiallySelectedItem(currentEnvironment)
                                    .OnSelectionChanged(this, &AwsGameKitCredentialsLayoutDetails::OnEnvironmentSelectionChanged)
                                    [
                                        SNew(STextBlock)
                                        .Text(this, &AwsGameKitCredentialsLayoutDetails::GetEnvironmentItemLabel)
                                    ]
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .Padding(20, 10, 0, 0)
                                [
                                    SAssignNew(customEnvironmentBox, SVerticalBox)
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 0, 0, 2)
                                    [
                                        textBlock(LOCTEXT("EnvironmentName", "Environment name"), robotoRegular10)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SAssignNew(customEnvironmentNameTextBox, SEditableTextBox)
                                        .OnTextChanged(this, &AwsGameKitCredentialsLayoutDetails::OnCustomEnvironmentNameChanged)
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
                                        textBlock(LOCTEXT("EnvironmentNameRestriction", "The environment name must have 1-16 characters. Valid characters: A-Z, a-z, 0-9, space."), robotoRegular8)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    .Padding(0, 10, 0, 2)
                                    [
                                        textBlock(LOCTEXT("EnvironmentCode", "Environment code"), robotoRegular10)
                                    ]
                                    + SVerticalBox::Slot()
                                    .AutoHeight()
                                    [
                                        SAssignNew(customEnvironmentCodeTextBox, SEditableTextBox)
                                        .OnTextChanged(this, &AwsGameKitCredentialsLayoutDetails::OnCustomEnvironmentCodeChanged)
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
                                        textBlock(LOCTEXT("EnvironmentCodeRestriction", "The environment code must have 2-3 characters. Valid characters: a-z, 0-9."), robotoRegular8)
                                    ]
                                ]
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(20, 10, 0, 2)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1)
                            [
                                textBlock(LOCTEXT("Region", "Region"), robotoRegular10)
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(4)
                            [
                                SAssignNew(regionComboBox, SComboBox<FComboBoxItem>)
                                .OptionsSource(&regionOptions)
                                .OnGenerateWidget(this, &AwsGameKitCredentialsLayoutDetails::MakeWidget)
                                .InitiallySelectedItem(currentRegion)
                                .OnSelectionChanged(this, &AwsGameKitCredentialsLayoutDetails::OnRegionSelectionChanged)
                                [
                                    SNew(STextBlock)
                                    .Text(this, &AwsGameKitCredentialsLayoutDetails::GetRegionItemLabel)
                                ]
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
                            textBlock(LOCTEXT("AWSAccountCredentials","AWS account credentials"), robotoBold10)
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            dynamicInfoBox(newEnvironmentNotification, [this]()
                            {
                                if (this->GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY)
                                {
                                    return LOCTEXT("CredentialsAutoFilledWarning", "Set credentials for this environment. Use existing values (carried over from the previous environment) or enter new ones.");
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
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1)
                            [
                                textBlock(LOCTEXT("AccessKeyID", "Access key ID"), robotoRegular10)
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(4)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                [
                                    SAssignNew(accessKeyTextBox, SEditableTextBox)
                                    .IsEnabled(true)
                                    .OnTextChanged(this, &AwsGameKitCredentialsLayoutDetails::OnAccessKeyChanged)
                                ]
                                + SVerticalBox::Slot()
                                 .AutoHeight()
                                [
                                    warningBox(accessKeyValidation, LOCTEXT("ValidAccessKeyIDWarning", "Enter a valid access key ID."))
                                ]
                             
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(20, 10, 0, 2)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1)
                            [
                                textBlock(LOCTEXT("SecretAccessKey", "Secret access key"), robotoRegular10)
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(4)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                [
                                    SAssignNew(secretKeyTextBox, SEditableTextBox)
                                    .IsEnabled(true)
                                    .OnTextChanged(this, &AwsGameKitCredentialsLayoutDetails::OnSecretKeyChanged)
                                    .IsPassword(true)
                                ]
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    warningBox(secretKeyValidation, LOCTEXT("ValidSecretAccessKeyWarning", "Enter a valid secret access key."))
                                ]
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(20, 10, 0, 2)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1)
                            [
                                textBlock(LOCTEXT("AWSAccountId","AWS Account ID: "), robotoBold10)
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(4)
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .HAlign(HAlign_Left)
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
                                + SHorizontalBox::Slot()
                                [
                                    SNew(SBox)
                                    .Visibility_Lambda([this]() -> EVisibility { return accountLoadingAnimationBox->GetVisibility() == EVisibility::Collapsed ? EVisibility::Visible : EVisibility::Collapsed; })
                                    [
                                        dynamicTextBlock([this]
                                            { return (FText::FromString(this->accountIdText.ToString())); }, robotoRegular10)
                                    ]
                                ]
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0, 20, 0, 10)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(1)
                            [
                                SNew(SHorizontalBox)
                                +SHorizontalBox::Slot()
                                .AutoWidth()
                                .VAlign(VAlign_Center)
                                .HAlign(HAlign_Left)
                                [
                                    SAssignNew(storeCredentialsCheckBox, SCheckBox)
                                    .IsChecked(ECheckBoxState::Checked) // Checked by design
                                ]
                                + SHorizontalBox::Slot()
                                .AutoWidth()
                                .Padding(5, 0, 0, 0)
                                .VAlign(VAlign_Center)
                                .HAlign(HAlign_Left)
                                [
                                    textBlock(LOCTEXT("StoreMyCredentials", "Store my credentials"), robotoRegular10)
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(4)
                            [
                                SNew(SHorizontalBox)
                                + SHorizontalBox::Slot()
                                .Padding(20, 0, 0, 0)
                                .VAlign(VAlign_Center)
                                .HAlign(HAlign_Left)
                                [
                                    SAssignNew(submitButton, SButton)
                                    .ContentPadding(FMargin(10,2))
                                    .HAlign(HAlign_Left)
                                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGreen"))
                                    .Text(LOCTEXT("SubmitCredentials", "Submit"))
                                    .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                                    .OnClicked(this, &AwsGameKitCredentialsLayoutDetails::OnSubmit)
                                ]
                                + SHorizontalBox::Slot()
                                [
                                    SNew (SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .HAlign(HAlign_Right)
                                    .VAlign(VAlign_Center)
                                    [
                                        SNew(SHyperlink)
                                        .Text(LOCTEXT("HelpGetUserCredentials", "Help me get my user credentials"))
                                        .OnNavigate(FSimpleDelegate::CreateLambda([]() { AwsGameKitCredentialsLayoutDetails::OpenBrowser(GetCredentialsUrl); }))
                                    ]
                                    + SHorizontalBox::Slot()
                                    .Padding(10, 2, 0, 2)
                                    .AutoWidth()
                                    .MaxWidth(15)
                                    .HAlign(HAlign_Right)
                                    .VAlign(VAlign_Center)
                                    [
                                        EXTERNAL_ICON_BOX()
                                    ]
                                ]
                            ]
                        ]
                    ] // End AWS Credentials
                ]
            ]
        ]
    ];

    // Bind project name state transition callback
    projectNameTimerHandle = projectNameTextBox->RegisterActiveTimer(configFileCheckDelay, FWidgetActiveTimerDelegate::CreateRaw(this, &AwsGameKitCredentialsLayoutDetails::ProjectNameStateTransitionCallback));

    SetInitialState();
}

bool AwsGameKitCredentialsLayoutDetails::IsSwitchEnvironmentButtonEnabled() const
{
    return !editorModule->GetFeatureControlCenter()->IsAnyFeatureUpdating() && editorModule->GetEditorState()->GetCredentialState() && !environmentComboBox->IsEnabled();
}

EVisibility AwsGameKitCredentialsLayoutDetails::IsSwitchEnvironmentButtonVisible() const
{
    return editorModule->GetEditorState()->GetCredentialState() && projectNameBox->GetVisibility() == EVisibility::Collapsed ? EVisibility::Visible : EVisibility::Collapsed;
}

void AwsGameKitCredentialsLayoutDetails::SetInitialState()
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
            PopulateCustomEnvironments(configGameName);
            LoadLastUsedEnvironment();
            LoadLastUsedRegion();
            SetPartiallyCompleteState();

            // If the loaded credentials are no longer valid, IsSubmitted will be set to false when the AWS AccountID is checked
            this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ true });
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

void AwsGameKitCredentialsLayoutDetails::SetPartiallyCompleteState()
{
    EnableInputBoxes(true);

    submitButton->SetEnabled(false);

    if (TryLoadAwsCredentialsFromFile())
    {
        OnAwsCredentialsChanged(true);
    }
    else if (IsGameNameValid(projectNameTextBox->GetText().ToString()) && !accountIdText.EqualTo(FText::FromString(AWS_ACCOUNT_ID_EMPTY)))
    {
        submitButton->SetEnabled(true);
    }
}

bool AwsGameKitCredentialsLayoutDetails::TryFindConfigFile(FString& outConfigPath, FString& outGameName)
{
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

bool AwsGameKitCredentialsLayoutDetails::ConfigFileExists(const FString& subfolder)
{
    const FString clientConfigPath = FPaths::ConvertRelativePathToFull(FPaths::GameSourceDir().Replace(TEXT("source/"), ToCStr(subfolder + ClientConfigFile), ESearchCase::IgnoreCase));
    return FPaths::FileExists(clientConfigPath);
}

bool AwsGameKitCredentialsLayoutDetails::TryParseGameNameFromConfig(const FString& configFilePath, FString& outGameName)
{
    FString configDir = FPaths::GetPathLeaf(FPaths::GetPath(configFilePath));
    outGameName = configDir;
    UE_LOG(LogAwsGameKit, Log, TEXT("Parsed game name from config: \"%s\""), *outGameName);

    return !outGameName.IsEmpty();
}

void AwsGameKitCredentialsLayoutDetails::PopulateCustomEnvironments(const FString& gameName)
{
    editorModule->GetFeatureResourceManager()->SetGameName(gameName);
    TMap<FString, FString> envs = editorModule->GetFeatureResourceManager()->GetSettingsEnvironments();

    for (const TTuple<FString, FString>& kvp : envs)
    {
        if (!this->environmentMapping.Contains(kvp.Key))
        {
            this->environmentMapping.Add(kvp.Key, kvp.Value);
            this->environmentOptions.Add(MakeShareable(new FString(kvp.Value)));
        }
    }
}

void AwsGameKitCredentialsLayoutDetails::LoadLastUsedEnvironment()
{
    FString const lastUsedEnv = editorModule->GetFeatureResourceManager()->GetLastUsedEnvironment();

    const auto lastUsedEnvironmentList = environmentOptions.FindByPredicate([this, lastUsedEnv](const FComboBoxItem comboBox) { return (*comboBox.Get() == environmentMapping[lastUsedEnv]); });

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

void AwsGameKitCredentialsLayoutDetails::LoadLastUsedRegion()
{
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

bool AwsGameKitCredentialsLayoutDetails::TryLoadAwsCredentialsFromFile()
{
    isLoadingEnvironmentFromFile = true;

    FString envKey = GetSelectedEnvironmentKey();
    if (envKey == NEW_CUSTOM_ENV_KEY)
    {
        FString envName; // not used
        GetCustomEnvironment(envKey, envName);
    }

    TSharedPtr<AwsCredentialsManager> credentialsManager = editorModule->GetCredentialsManager();
    credentialsManager->SetGameName(gameTitleText.ToString());
    credentialsManager->SetEnv(envKey);
    FString accessKey = credentialsManager->GetAccessKey();
    FString secretKey = credentialsManager->GetSecretKey();

    if (!accessKey.IsEmpty() && !secretKey.IsEmpty())
    {
        accessKeyTextBox->SetText(FText::FromString(accessKey));
        secretKeyTextBox->SetText(FText::FromString(secretKey));
        isLoadingEnvironmentFromFile = false;

        return true;
    }

    EnableInputBoxes(true);

    if (projectNameBox->GetVisibility() == EVisibility::Collapsed) {
        newEnvironmentNotification->SetVisibility(EVisibility::Visible);
    }
    isLoadingEnvironmentFromFile = false;

    return false;
}

TArray<FString> AwsGameKitCredentialsLayoutDetails::GetInvalidRegexCharacters(const FRegexPattern& regexPattern, const FString& input)
{
    TArray<FString> invalidCharacters;
    FRegexMatcher matcher(regexPattern, input);
    for (int32 i = 0; i < input.Len(); i++)
    {
        FString currentInputChar = input.Mid(i, 1);
        matcher.SetLimits(i, i + 1);
        if (!matcher.FindNext())
        {
            invalidCharacters.Add(currentInputChar);
        }
    }

    return invalidCharacters;
}

bool AwsGameKitCredentialsLayoutDetails::InputContainsReservedKeyword(const FString& input, FString& reservedKeywordOutput)
{
    for (FString reservedKeyword : RESERVED_KEYWORDS)
    {
        if (input.Contains(reservedKeyword))
        {
            reservedKeywordOutput = reservedKeyword;
            return true;
        }
    }

    return false;
}

bool AwsGameKitCredentialsLayoutDetails::IsGameNameValid(const FString& gameName)
{
    if (gameName.Len() > 12)
    {
        this->projectNameValidationErrorText = LOCTEXT("GameTitleLengthInvalid", "The game title must have 1 - 12 characters");
        return false;
    }

    FString reservedKeywordOutput = "";
    if (InputContainsReservedKeyword(gameName, reservedKeywordOutput))
    {
        this->projectNameValidationErrorText = FText::Format(LOCTEXT("GameNameReservedString", "The game title cannot contain the substring '{0}'."), FText::FromString(*reservedKeywordOutput));
        return false;
    }

    const FRegexPattern validGameNameRx("^[a-z0-9]+$");
    const TArray<FString> invalidCharacters = GetInvalidRegexCharacters(validGameNameRx, gameName);

    if (invalidCharacters.Num() != 0)
    {
        const FString invalidCharactersMessage = FString::Join(invalidCharacters, TEXT(", "));

        this->projectNameValidationErrorText = FText::Format(LOCTEXT("GameNameInvalidCharacters", "Invalid characters: {0}."), FText::FromString(invalidCharactersMessage));

        return false;
    }

    return true;
}

bool AwsGameKitCredentialsLayoutDetails::IsEnvironmentNameValid(const FString& environmentName)
{
    if (environmentName.Len() < 1 || environmentName.Len() > 16)
    {
        this->environmentNameErrorText = LOCTEXT("EnvironmentNameLengthInvalid", "The environment name must have 1-16 characters");
        return false;
    }

    const FRegexPattern validEnvironmentNameRx("^[A-Za-z0-9]+$");
    const TArray<FString> invalidCharacters = GetInvalidRegexCharacters(validEnvironmentNameRx, environmentName);;

    if (invalidCharacters.Num() != 0)
    {
        const FString invalidCharactersMessage = FString::Join(invalidCharacters, TEXT(", "));

        this->environmentNameErrorText = FText::Format(LOCTEXT("EnvNameInvalidCharacters", "Invalid characters: {0}."), FText::FromString(invalidCharactersMessage));

        return false;
    }

    return true;
}

bool AwsGameKitCredentialsLayoutDetails::IsEnvironmentNameInUse(const FString& environmentName) const
{
    for (TTuple<FString, FString> const& environmentItem : this->environmentMapping)
    {
        if (environmentItem.Value == environmentName)
        {
            return true;
        }
    }

    return false;
}

bool AwsGameKitCredentialsLayoutDetails::IsEnvironmentCodeValid(const FString& environmentCode)
{
    FString reservedKeywordOutput = "";
    if (InputContainsReservedKeyword(environmentCode, reservedKeywordOutput))
    {
        this->environmentCodeErrorText = FText::Format(LOCTEXT("EnvCodeReservedString", "The environment code cannot contain the substring '{0}'."), FText::FromString(*reservedKeywordOutput));
        return false;
    }

    if (environmentCode.Len() < 2 || environmentCode.Len() > 3)
    {
        this->environmentCodeErrorText = LOCTEXT("EnvironmentCodeLengthInvalid", "The environment code must have 2-3 characters");
        return false;
    }

    const FRegexPattern validEnvironmentCodeRx("^[a-z0-9]+$");
    const TArray<FString> invalidCharacters = GetInvalidRegexCharacters(validEnvironmentCodeRx, environmentCode);;;

    if (invalidCharacters.Num() != 0)
    {
        const FString invalidCharactersMessage = FString::Join(invalidCharacters, TEXT(", "));

        this->environmentCodeErrorText = FText::Format(LOCTEXT("EnvCodeInvalidCharacters", "Invalid characters: {0}."), FText::FromString(invalidCharactersMessage));

        return false;
    }

    return true;
}

bool AwsGameKitCredentialsLayoutDetails::IsEnvironmentCodeInUse(const FString& environmentCode)
{
    if (this->environmentMapping.Contains(environmentCode))
    {
        this->environmentCodeErrorText = LOCTEXT("EnvironmentCodeInUse", "Environment code is already in use.");
        return true;
    }

    return false;
}

void AwsGameKitCredentialsLayoutDetails::RetrieveAccountId(const FString& accessKey, const FString& secretKey)
{
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();

    accountLoadingAnimationBox->SetVisibility(EVisibility::Visible);
    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&, accessKey, secretKey, featureResourceManager]()
    {
        const FString accountId = featureResourceManager->GetAccountId(TCHAR_TO_UTF8(*accessKey), TCHAR_TO_UTF8(*secretKey));

        AsyncTask(ENamedThreads::GameThread, [&, accountId]()
            {
                if (!accountId.IsEmpty())
                {
                    accountIdText = FText::FromString(accountId);
                    submitValidation->SetVisibility(EVisibility::Collapsed);
                    accessKeyValidation->SetVisibility(EVisibility::Collapsed);
                    secretKeyValidation->SetVisibility(EVisibility::Collapsed);
                }
                else
                {
                    accountIdText = FText::FromString(AWS_ACCOUNT_ID_EMPTY);
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

void AwsGameKitCredentialsLayoutDetails::PopulateEnvironments()
{
    environmentMapping.Add("dev", "Development");
    environmentMapping.Add("qa", "QA");
    environmentMapping.Add("stg", "Staging");
    environmentMapping.Add("prd", "Production");
    environmentMapping.Add(NEW_CUSTOM_ENV_KEY, "Add new environment");

    for (TTuple<FString, FString>& kvp : environmentMapping)
    {
        environmentOptions.Add(MakeShareable(new FString(kvp.Value)));
    }

    currentEnvironment = environmentOptions[0]; // Set to "dev" by design
}

void AwsGameKitCredentialsLayoutDetails::PopulateRegions()
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

    for (TTuple<FString, FString>& kvp : regionMapping)
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

void AwsGameKitCredentialsLayoutDetails::OnRegionSelectionChanged(FComboBoxItem newValue, ESelectInfo::Type selectionType)
{
    currentRegion = newValue;
}

void AwsGameKitCredentialsLayoutDetails::OnEnvironmentSelectionChanged(FComboBoxItem newValue, ESelectInfo::Type selectionType)
{
    currentEnvironment = newValue;
    FString currentEnvironmentCode = GetSelectedEnvironmentKey();
    if (currentEnvironmentCode == NEW_CUSTOM_ENV_KEY)
    {
        customEnvironmentBox->SetVisibility(EVisibility::Visible);
        customEnvironmentNameTextBox->SetEnabled(true);
        customEnvironmentCodeTextBox->SetEnabled(true);

        if (!this->secretKeyTextBox->GetText().IsEmpty() && !this->accessKeyTextBox->GetText().IsEmpty())
        {
            newEnvironmentNotification->SetVisibility(EVisibility::Visible);
        }

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

void AwsGameKitCredentialsLayoutDetails::OnCustomEnvironmentNameChanged(const FText& fieldText)
{
    EVisibility fieldValidationVisibility = EVisibility::Collapsed;

    if (!IsEnvironmentNameValid(fieldText.ToString()))
    {
        fieldValidationVisibility = EVisibility::Visible;
    }

    if (IsEnvironmentNameInUse(fieldText.ToString()))
    {
        this->environmentNameErrorText = LOCTEXT("EnvironmentNameInUse", "Environment name is already in use.");
        fieldValidationVisibility = EVisibility::Visible;
    }

    customEnvironmentNameValidation->SetVisibility(fieldValidationVisibility);

    OnCheckFields();
}

void AwsGameKitCredentialsLayoutDetails::OnCustomEnvironmentCodeChanged(const FText& fieldText)
{
    EVisibility fieldValidationVisibility = EVisibility::Collapsed;

    if (!IsEnvironmentCodeValid(fieldText.ToString()) || IsEnvironmentCodeInUse(fieldText.ToString()))
    {
        fieldValidationVisibility = EVisibility::Visible;
    }

    customEnvironmentCodeValidation->SetVisibility(fieldValidationVisibility);

    OnCheckFields();
}

void AwsGameKitCredentialsLayoutDetails::OnProjectNameTextChanged(const FText& fieldText)
{
    EVisibility fieldValidationVisibility = EVisibility::Collapsed;
    gameTitleText = fieldText;

    FString projectName = fieldText.ToString();

    if (IsGameNameValid(projectName))
    {
        // Delay the config file check to allow game name changes without excessive logging
        configFileFieldChangedValid = true;
        nextConfigFileCheckTimestamp = FPlatformTime::Seconds() + configFileCheckDelay;
    }
    else
    {
        submitButton->SetEnabled(false);
        configFileFieldChangedValid = false;
        fieldValidationVisibility = EVisibility::Visible;
    }

    projectNameValidation->SetVisibility(fieldValidationVisibility);
}

EActiveTimerReturnType AwsGameKitCredentialsLayoutDetails::ProjectNameStateTransitionCallback(double inCurrentTime, float inDeltaTime)
{
    if (!configFileFieldChangedValid)
    {
        return EActiveTimerReturnType::Continue;
    }

    if (nextConfigFileCheckTimestamp <= inCurrentTime)
    {
        UE_LOG(LogAwsGameKit, Verbose, TEXT("%f Project name check"), inCurrentTime);
        configFileFieldChangedValid = false;

        FString projectName = projectNameTextBox->GetText().ToString();
        if (IsGameNameValid(projectName))
        {
            PopulateCustomEnvironments(projectName);
            SetPartiallyCompleteState();
        }
        else
        {
            SetInitialState();
        }
    }
    else
    {
        UE_LOG(LogAwsGameKit, Verbose, TEXT("%f Skipping project name check, not enough time has passed"), inCurrentTime);
    }

    return EActiveTimerReturnType::Continue;
}

void AwsGameKitCredentialsLayoutDetails::OnLoadCustomGameConfigFile()
{
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

bool AwsGameKitCredentialsLayoutDetails::IsAccessKeyFieldValid(const FText& accessKeyFieldText)
{
    const FString accessKey = accessKeyFieldText.ToString();
    const FRegexPattern validAccessKeyRx("^[A-Z0-9]+$");
    FRegexMatcher matcher(validAccessKeyRx, accessKey);

    // Length of access keys should be 16-128 characters, but not checking min length to avoid
    // showing the user a warning while they type the key.
    return matcher.FindNext() && accessKey.Len() <= 128;
}

bool AwsGameKitCredentialsLayoutDetails::IsSecretKeyFieldValid(const FText& secretKeyFieldText)
{
    const FString secretKey = secretKeyFieldText.ToString();
    return  !secretKeyFieldText.IsEmptyOrWhitespace() && secretKey.Len() <= 40;
}

void AwsGameKitCredentialsLayoutDetails::OnAccessKeyChanged(const FText& fieldText)
{
    if (!isLoadingEnvironmentFromFile)
    {
        newEnvironmentNotification->SetVisibility(EVisibility::Collapsed);

        const bool valid = IsAccessKeyFieldValid(fieldText);

        EVisibility fieldValidationVisibility = EVisibility::Collapsed;
        if (!valid)
        {
            fieldValidationVisibility = EVisibility::Visible;
        }

        accessKeyValidation->SetVisibility(fieldValidationVisibility);

        OnAwsCredentialsChanged(valid);
    }
}

void AwsGameKitCredentialsLayoutDetails::OnSecretKeyChanged(const FText& fieldText)
{
    // Changing the secret key should always be done after the access key when loading from file

    newEnvironmentNotification->SetVisibility(EVisibility::Collapsed);

    const bool valid = IsSecretKeyFieldValid(fieldText);

    EVisibility fieldValidationVisibility = EVisibility::Collapsed;
    if (!valid)
    {
        fieldValidationVisibility = EVisibility::Visible;
    }

    secretKeyValidation->SetVisibility(fieldValidationVisibility);

    OnAwsCredentialsChanged(valid);
}

void AwsGameKitCredentialsLayoutDetails::OnAwsCredentialsChanged(bool areFieldsValid)
{
    submitButton->SetEnabled(false);
    submitValidation->SetVisibility(EVisibility::Collapsed);

    if (!areFieldsValid)
    {
        accountIdText = FText::FromString(AWS_ACCOUNT_ID_EMPTY);
        return;
    }

    FString accessKey = accessKeyTextBox->GetText().ToString();
    FString secretKey = secretKeyTextBox->GetText().ToString();
    FText accountId = accountIdText;

    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    // Only proceed to auto-retrieve account Id if the fields have the required lengths.
    //TODO - Change this to only validate when we leave focus to avoid hard length checking 
    if (accessKey.Len() != 20 || secretKey.Len() != 40)
    {
        if (!accessKey.IsEmpty() && !secretKey.IsEmpty())
        {
            submitValidationText = LOCTEXT("AWSCredentialsNotValid", "The AWS credentials entered are not valid.");
            accountIdText = FText::FromString(AWS_ACCOUNT_ID_EMPTY);
            submitValidation->SetVisibility(EVisibility::Visible);
        }

        this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ false });
        return;
    }

    AccountDetails accountDetails = GetAccountDetails();

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
                        }
                        else
                        {
                            EnableInputBoxes(true);
                        }
                    }
                    else
                    {
                        EnableInputBoxes(true);
                        
                        submitButton->SetEnabled(false);

                        submitValidationText = LOCTEXT("AWSCredentialsNotValid", "The AWS credentials entered are not valid.");
                        accountIdText = FText::FromString(AWS_ACCOUNT_ID_EMPTY);
                        submitValidation->SetVisibility(EVisibility::Visible);
                    }
                });
        });
}

void AwsGameKitCredentialsLayoutDetails::OnCheckFields()
{
    // check fields and enable submit button if all are filled
    bool validEnvironment = GetSelectedEnvironmentKey() != NEW_CUSTOM_ENV_KEY ||
        (customEnvironmentNameValidation->GetVisibility() == EVisibility::Collapsed && customEnvironmentCodeValidation->GetVisibility() == EVisibility::Collapsed
            && !customEnvironmentCodeTextBox->GetText().IsEmpty() && !customEnvironmentNameTextBox->GetText().IsEmpty());

    if (!validEnvironment)
    {
        submitValidationText = LOCTEXT("PleaseEnterValidEnvironment", "Please enter a valid environment.");
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
        !accountIdText.EqualTo(FText::FromString(AWS_ACCOUNT_ID_EMPTY)))
    {
        submitButton->SetEnabled(true);
    }
    else
    {
        submitValidationText = LOCTEXT("AWSCredentialsNotValid", "The AWS credentials entered are not valid.");
        submitButton->SetEnabled(false);
    }
}

FReply AwsGameKitCredentialsLayoutDetails::OnChangeEnvironmentAndCredentials()
{
    FText MessageTitle(LOCTEXT("ChangeEnvironment", "Change Environment"));
    const EAppReturnType::Type reply = FMessageDialog::Open(EAppMsgType::YesNo, ChangeEnvironmentWarning, &MessageTitle);
    if (reply == EAppReturnType::No || reply == EAppReturnType::Cancel)
    {
        return FReply::Handled();;
    }

    this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ false });
    submitButton->SetEnabled(true);

    if (GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY) {
        environmentComboBox->SetSelectedItem(environmentOptions.Last());

        customEnvironmentNameTextBox->SetText(FText::FromString(""));
        customEnvironmentCodeTextBox->SetText(FText::FromString(""));

        customEnvironmentNameValidation->SetVisibility(EVisibility::Collapsed);
        customEnvironmentCodeValidation->SetVisibility(EVisibility::Collapsed);
    }

    EnableInputBoxes(true);
    submitButton->SetEnabled(true);
    cancelEnvironmentSwitchButton->SetVisibility(EVisibility::Visible);

    return FReply::Handled();
}

FReply AwsGameKitCredentialsLayoutDetails::OnCancelEnvironmentAndCredentialsChange()
{
    LoadLastUsedEnvironment();
    LoadLastUsedRegion();
    if (TryLoadAwsCredentialsFromFile())
    {
        OnAwsCredentialsChanged(true);

        this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ true });

        EnableInputBoxes(false);
        submitButton->SetEnabled(false);
        cancelEnvironmentSwitchButton->SetVisibility(EVisibility::Collapsed);
    }

    return FReply::Handled();
}

FReply AwsGameKitCredentialsLayoutDetails::OnSubmit()
{
    newEnvironmentNotification->SetVisibility(EVisibility::Collapsed);

    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    TSharedPtr<AwsCredentialsManager> credentialsManager = editorModule->GetCredentialsManager();
    TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter = editorModule->GetFeatureControlCenter();

    EVisibility fieldValidationVisibility = EVisibility::Collapsed;
    submitValidation->SetVisibility(fieldValidationVisibility);

    bool customEnv = GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY;
    AccountDetails accountDetails = GetAccountDetails();
    SetAccountDetails();

    gameTitleText = FText::FromString(accountDetails.gameName);
    projectNameBox->SetVisibility(EVisibility::Collapsed);
    cancelEnvironmentSwitchButton->SetVisibility(EVisibility::Collapsed);


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
                        editorModule->GetEditorState()->SetCredentials(accountDetails);
                        FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

                        const FString clientConfigSubdirectory = featureResourceManager->GetClientConfigSubdirectory();

                        if (!ConfigFileExists(clientConfigSubdirectory))
                        {
                            UE_LOG(LogAwsGameKit, Display, TEXT("Creating default config file, this is expected for new game projects."));           
                            featureResourceManager->CreateEmptyClientConfigFile();
                        }

                        runtimeModule->ReloadConfigFile(clientConfigSubdirectory);

                        // TODO::Find better way to trigger redraw
                        // Set state so details panels can know when to update.
                        this->messageEndpoint->Publish<FMsgCredentialsState>(new FMsgCredentialsState{ true });

                        // Reset our feature statuses so we don't incorrectly
                        //  show statuses from a previously selected stack
                        featureControlCenter->ResetFeatureStatuses();
                        featureControlCenter->RefreshFeatureStatuses();

                        // If the environment dropdown is left open when the user submits it will stay active, this allows makes sure its closed
                        environmentComboBox->SetIsOpen(false);
                    }
                    else
                    {
                        EnableInputBoxes(true);
                        submitButton->SetEnabled(true);

                        fieldValidationVisibility = EVisibility::Visible;
                        FString hexError = FString(GameKit::StatusCodeToHexStr(result.Result).c_str());
                        submitValidationText = LOCTEXT("CredentialsNotValidated", "The user credentials you provided cannot be validated.\nPlease enter a valid access key pair or create a new one using AWS IAM.");
                        UE_LOG(LogAwsGameKit, Error, TEXT("The user credentials you provided cannot be validated: error %s"), *hexError);

                        submitValidation->SetVisibility(fieldValidationVisibility);
                    }
                });
        });
    return FReply::Handled();
}

TSharedRef<SWidget> AwsGameKitCredentialsLayoutDetails::MakeWidget(FComboBoxItem item)
{
    return SNew(STextBlock).Text(FText::FromString(*item));
}

FText AwsGameKitCredentialsLayoutDetails::GetRegionItemLabel() const
{
    return FText::FromString(*currentRegion.Get());
}

FText AwsGameKitCredentialsLayoutDetails::GetEnvironmentItemLabel() const
{
    return FText::FromString(*currentEnvironment.Get());
}

bool AwsGameKitCredentialsLayoutDetails::TrySelectGameConfigFile(const FString& rootPath, FString& outFilePath)
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

FReply AwsGameKitCredentialsLayoutDetails::OpenBrowser(const FString& url)
{
    FString error;
    FPlatformProcess::LaunchURL(*url, nullptr, &error);

    if (error.Len() > 0)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("%s"), *error);
    }

    return FReply::Handled();
}

FString AwsGameKitCredentialsLayoutDetails::GetSelectedEnvironmentKey() const
{
    const FString* key = environmentMapping.FindKey(*currentEnvironment);

    return key != nullptr ? *key : "";
}

FString AwsGameKitCredentialsLayoutDetails::GetCurrentRegionKey() const
{
    const FString* key = regionMapping.FindKey(*currentRegion);

    return key != nullptr ? *key : "";
}

void AwsGameKitCredentialsLayoutDetails::GetCustomEnvironment(FString& outKey, FString& outName) const
{
    if (GetSelectedEnvironmentKey() == NEW_CUSTOM_ENV_KEY)
    {
        outKey = customEnvironmentCodeTextBox->GetText().ToString();
        outName = customEnvironmentNameTextBox->GetText().ToString();
    }
}

AccountDetails AwsGameKitCredentialsLayoutDetails::GetAccountDetails() const
{
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

void AwsGameKitCredentialsLayoutDetails::SetAccountDetails() const
{
    editorModule->GetFeatureResourceManager()->SetAccountDetails(GetAccountDetails());
}

void AwsGameKitCredentialsLayoutDetails::EnableInputBoxes(bool isEnabled)
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

void AwsGameKitCredentialsLayoutDetails::BulkSetEnabled(const TArray<SWidget*> widgets, bool enabled)
{
    for (SWidget* const& widget : widgets)
    {
        widget->SetEnabled(enabled);
    }
}

void AwsGameKitCredentialsLayoutDetails::BulkSetVisibility(const TArray<SWidget*> widgets, EVisibility visibility)
{
    for (SWidget* const& widget : widgets)
    {
        widget->SetVisibility(visibility);
    }
}

#undef LOCTEXT_NAMESPACE
