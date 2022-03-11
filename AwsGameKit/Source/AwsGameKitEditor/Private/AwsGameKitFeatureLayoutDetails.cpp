// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "AwsGameKitFeatureLayoutDetails.h" // First import (Unreal required)
#include "AwsGameKitEditor.h"
#include "AwsGameKitFeatureControlCenter.h"
#include <AwsGameKitRuntime/Public/Models/AwsGameKitEnumConverter.h>
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"
#include "FeatureResourceManager.h"
#include "Utils/AwsGameKitEditorUtils.h"
#include "Utils/AwsGameKitProjectSettingsUtils.h"

// Unreal
#include "MessageEndpointBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitFeatureLayoutDetails"

const FString AwsGameKitFeatureLayoutDetails::GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED = "cloudwatch_dashboard_enabled";

AwsGameKitFeatureLayoutDetails::AwsGameKitFeatureLayoutDetails(const FeatureType featureType, const FAwsGameKitEditorModule* editorModule) : editorModule(editorModule), featureType(featureType)
{
    this->messageEndpoint = FMessageEndpoint::Builder(AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME)
        .Handling<FMsgCredentialsState>(this, &AwsGameKitFeatureLayoutDetails::CredentialsStateMessageHandler)
        .Build();
    this->messageEndpoint->Subscribe<FMsgCredentialsState>();
}

TSharedRef<SVerticalBox> AwsGameKitFeatureLayoutDetails::GetFeatureFooter(const FText& featureDescription)
{
    const FText description = FText::Format(LOCTEXT("FeatureAwsServiceLabel",
        "{0} Uses AWS services: {1} <a id=\"external_link\" href=\"{2}\" style=\"Hyperlink\">Learn more</>"),
        featureDescription, 
        FText::FromString(AwsGameKitEditorUtils::FeatureResourcesUIString(this->featureType)),
        FText::FromString(AwsGameKitEditorUtils::FeatureToDocumentationUrl(this->featureType)));

    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(0, 5, 0, 10)
        .MaxHeight(65)
        .AutoHeight()
        [
            SNew(SRichTextBlock)
            .Text(description)
            .AutoWrapText(true)
            .TextStyle(AwsGameKitStyleSet::Style, "DescriptionText")
            .DecoratorStyleSet(AwsGameKitStyleSet::Style.Get())
            + SRichTextBlock::HyperlinkDecorator(TEXT("external_link"), FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
                const FString* url = Metadata.Find(TEXT("href"));
                if (url)
                {
                    AwsGameKitEditorUtils::OpenBrowser(*url);
                }
            }))
        ];
}

bool AwsGameKitFeatureLayoutDetails::CanEditConfiguration() const
{
    const TSharedPtr<EditorState> editorState = editorModule->GetEditorState();
    const TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter = editorModule->GetFeatureControlCenter();

    return editorState->AreCredentialsValid() && !featureControlCenter->IsFeatureUpdating(this->featureType);
}

TSharedRef<SVerticalBox> AwsGameKitFeatureLayoutDetails::GetDeployControls(const bool addSeparator)
{
    const TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter = editorModule->GetFeatureControlCenter();
    const TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    const TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    return SNew(SVerticalBox)
    + SVerticalBox::Slot()
    .Padding(addSeparator ? 10 : 0)
    [
        SNew(SSeparator)
        .ColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("Black"))
        .Thickness(3)
        .Visibility(addSeparator ? EVisibility::Visible : EVisibility::Hidden)
    ]

    + SVerticalBox::Slot()
    .AutoHeight()
    [
        PROJECT_SETTINGS_ROW(
            // Left - Dashboard status label
            SNew(STextBlock)
            .Text(LOCTEXT("DashboardStatus", "Dashboard status")),

            // Right - Dashboard status
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 10, 0)
            .AutoWidth()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                [
                    SNew(STextBlock)
                    .Visibility_Lambda([this, featureControlCenter, editorState]()
                    {
                        if (!editorState->GetCredentialState())
                        {
                            return EVisibility::Visible;
                        }

                        return featureControlCenter->GetStatus(this->featureType).ToString().Equals(FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str()) ? EVisibility::Visible : EVisibility::Collapsed;
                    })
                    .Text_Lambda([this, featureResourceManager, editorState]()
                    {
                        if (!editorState->GetCredentialState())
                        {
                            return LOCTEXT("DashboardStatusInvalidEnviornment", "Enter valid environment and credentials to see dashboard status.");
                        }

                        TMap<FString, FString> featureVariables = featureResourceManager->GetFeatureVariables(this->featureType);
                        if (featureVariables.Contains(GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED) && featureVariables[GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED].Equals("false"))
                        {
                            return LOCTEXT("DashboardStatusUndeployedInactive", "Dashboard will not be active upon deployment");
                        }

                        return LOCTEXT("DashboardStatusUndeployedActive", "Dashboard will be active upon deployment.");
                    })
                ]
                + SHorizontalBox::Slot()
                [
                    SNew(SHorizontalBox)
                    .Visibility_Lambda([this, featureControlCenter, featureResourceManager, editorState]()
                    {
                        if (!editorState->GetCredentialState())
                        {
                            return EVisibility::Collapsed;
                        }

                        return ShowDashboardLink(featureControlCenter, featureResourceManager) ? EVisibility::Visible : EVisibility::Collapsed;
                    })
                    + SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    .Padding(0, 0, 10, 0)
                    .AutoWidth()
                    [
                        SNew(SImage)
                        .Image(AwsGameKitStyleSet::Style->GetBrush("DeployedIcon"))
                    ]
                    +SHorizontalBox::Slot()
                    .VAlign(VAlign_Center)
                    [
                        SNew(SHyperlink)
                        .Text(LOCTEXT("OpenDashboard", "Open Dashboard"))
                        .OnNavigate(FSimpleDelegate::CreateLambda([this, featureResourceManager]()
                        {
                            const FString url = featureResourceManager->GetDashboardURL(AwsGameKitEnumConverter::FeatureToUIString(this->featureType));
                            AwsGameKitEditorUtils::OpenBrowser(url);
                        }))
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
                + SHorizontalBox::Slot()
                [
                    SNew(STextBlock)
                    .Visibility_Lambda([this, featureControlCenter, featureResourceManager, editorState]()
                    {
                        if (!editorState->GetCredentialState() || featureControlCenter->GetStatus(this->featureType).ToString().Equals(FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str()))
                        {
                            return EVisibility::Collapsed;
                        }

                        return ShowDashboardLink(featureControlCenter, featureResourceManager) ? EVisibility::Collapsed : EVisibility::Visible;
                    })
                    .Text(LOCTEXT("CloudWatchDashboardInactive","Inactive"))
                ]
            ]
            + SHorizontalBox::Slot()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .Padding(2)
                // Fill the remaining space - use a large scalar to effectively tell Slate to stretch the slot as far as possible
                .FillWidth(GameKit::PROJECT_SETTINGS_FILL_REMAINING)
                .HAlign(HAlign_Right)
                .VAlign(VAlign_Center)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    [
                        SNew(SHyperlink)
                        .Text(LOCTEXT("LearnAboutDashboard", "Learn more about dashboards"))
                        .OnNavigate(FSimpleDelegate::CreateLambda([]() { AwsGameKitEditorUtils::OpenBrowser(AwsGameKitDocumentationManager::GetDocumentString("url", "cloudwatch_dashboards_reference")); }))
                    ]
                ]
                + SHorizontalBox::Slot()
                .Padding(10, 0, 10, 0)
                .AutoWidth()
                .MaxWidth(15)
                .HAlign(HAlign_Right)
                .VAlign(VAlign_Center)
                [
                    EXTERNAL_ICON_BOX()
                ]
            ]
        )
    ]

     + SVerticalBox::Slot()
    .AutoHeight()
    [
        PROJECT_SETTINGS_ROW(
            // Left - Dashboard actions label
            SNew(STextBlock)
            .Text(LOCTEXT("DashboardAction", "Dashboard action")),

            // Right - Dashboard actions
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 2, 2, 2)
            [
                // There isn't a TextStyle_Lambda binding available to use.
                // Rather than rolling a custom button that will update its text style based on a function result,
                // create one button with each style, and toggle between the two with a Visibility_Lambda.
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    .Visibility_Lambda([this, featureResourceManager, editorState]
                    {
                        if (editorState->GetCredentialState())
                        {
                            TMap<FString, FString> featureVariables = featureResourceManager->GetFeatureVariables(this->featureType);
                            if (featureVariables.Contains(GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED) && featureVariables[GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED].Equals("false"))
                            {
                                return EVisibility::Visible;
                            }
                        }

                        return EVisibility::Collapsed;
                    })
                    .Text(LOCTEXT("ActivateDashboard", "Activate"))
                    .ToolTipText_Lambda([this, featureControlCenter]()
                    {
                        if (featureControlCenter->IsAnyFeatureUpdating())
                        {
                            return LOCTEXT("DashboardStatusFeatureUpdating", "Deploying feature must be completed before changing a dashboard's status.");
                        }
                        return FText::FromString("");
                    })
                    .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGreen"))
                    .OnClicked_Lambda([this, featureControlCenter, featureResourceManager] {
                        featureResourceManager->SetFeatureVariable(this->featureType, GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED, "true");

                        if (featureControlCenter->IsRedeployEnabled(this->featureType))
                        {
                            DeployFeature();
                        }
                        return FReply::Handled();
                    })
                    .ContentPadding(FMargin(10, 2))
                    .IsEnabled_Lambda([this, featureControlCenter, editorState]{ return  editorState->GetCredentialState() && !featureControlCenter->IsAnyFeatureUpdating(); })
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    .Visibility_Lambda([this, featureResourceManager, editorState]
                    {
                        if (editorState->GetCredentialState())
                        {
                            TMap<FString, FString> featureVariables = featureResourceManager->GetFeatureVariables(this->featureType);
                            if (featureVariables.Contains(GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED) && featureVariables[GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED].Equals("false"))
                            {
                                return EVisibility::Collapsed;
                            }
                        }

                        return EVisibility::Visible;
                    })
                    .Text(LOCTEXT("DeactivateDsahboard", "Deactivate"))
                    .ToolTipText_Lambda([this, featureControlCenter]()
                    {
                        if (featureControlCenter->IsAnyFeatureUpdating())
                        {
                            return LOCTEXT("DashboardStatusFeatureUpdating", "Deploying feature must be completed before changing a dashboard's status.");
                        }
                        return FText::FromString("");
                    })
                    .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonRed"))
                    .OnClicked_Lambda([this, featureControlCenter, featureResourceManager] {
                        featureResourceManager->SetFeatureVariable(this->featureType, GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED, "false");

                        if (featureControlCenter->IsRedeployEnabled(this->featureType))
                        {
                            DeployFeature();
                        }
                        return FReply::Handled();
                    })
                    .ContentPadding(FMargin(10, 2))
                    .IsEnabled_Lambda([this, featureControlCenter, editorState]{ return editorState->GetCredentialState() && !featureControlCenter->IsAnyFeatureUpdating(); })
                ]
            ]
        )
    ]

    + SVerticalBox::Slot()
    .Padding(10)
    [
        SNew(SSeparator)
        .ColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("Black"))
        .Thickness(3)
    ]

    + SVerticalBox::Slot()
    .AutoHeight()
    [
        PROJECT_SETTINGS_ROW(
            // Left - Deployment status label
            SNew(STextBlock)
            .Text(LOCTEXT("DeploymentStatus", "Deployment status")),

            // Right - Deployment status
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .VAlign(VAlign_Center)
            .Padding(0, 0, 10, 0)
            .AutoWidth()
            [
                SNew(SImage)
                .Visibility_Lambda([this, featureControlCenter] { return !featureControlCenter->FeatureAvailable(this->featureType) || featureControlCenter->GetIconStyle(this->featureType).Compare("") == 0 ? EVisibility::Collapsed : EVisibility::Visible; })
                .Image_Lambda([this, featureControlCenter]()->const FSlateBrush*
                {
                    return AwsGameKitStyleSet::Style->GetBrush(featureControlCenter->GetIconStyle(this->featureType));
                })
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .HAlign(HAlign_Left)
            [
                SNew(STextBlock)
                .Text_Lambda([this, featureControlCenter]()->FText
                {
                    return featureControlCenter->GetStatus(this->featureType);
                })
            ]
            + SHorizontalBox::Slot()
            .HAlign(HAlign_Left)
            .VAlign(VAlign_Center)
            .Padding(5, 1, 0, 0)
            [
                SNew(SButton)
                .ButtonStyle(FCoreStyle::Get(), "NoBorder")
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .Cursor(EMouseCursor::Hand)
                .ToolTipText_Lambda([this, featureControlCenter, editorState]()
                {
                    if (!featureControlCenter->FeatureAvailable(this->featureType) || !editorState->AreCredentialsValid())
                    {
                        return LOCTEXT("RefreshDisabledNonValidEnv", "Enter valid environment or credentials to refresh deployment status.");
                    }

                    if (!featureControlCenter->IsRefreshAvailable())
                    {
                        return LOCTEXT("RefreshDisabledFeatureUpdating", "Status refresh is disabled while any feature is being updated.");
                    }

                    return LOCTEXT("RefreshDeploymentStatus", "Get current deployment status.");
                })
                .IsEnabled_Lambda([this, featureControlCenter] { return featureControlCenter->IsRefreshAvailable(); })
                .OnClicked_Lambda([this, featureControlCenter]
                {
                    featureControlCenter->RefreshFeatureStatuses();
                    return FReply::Handled();
                })
                .ForegroundColor(FSlateColor::UseForeground())
                [
                    SNew(SImage)
                    .Image_Lambda([this, featureControlCenter]()->const FSlateBrush*
                    {
                        if (featureControlCenter->IsRefreshAvailable())
                        {
                            return AwsGameKitStyleSet::Style->GetBrush("RefreshIcon");
                        }
                        
                        return AwsGameKitStyleSet::Style->GetBrush("WaitingIcon");
                    })
                ]
            ]
        )
    ]

    // Status and deploy buttons
    + SVerticalBox::Slot()
    .AutoHeight()
    [
        PROJECT_SETTINGS_ROW(
            // Left - resource actions label
            SNew(STextBlock)
            .Text(LOCTEXT("AWSResourceActions", "AWS resource actions")),

            // Right - deploy buttons
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(0, 2, 2, 2)
            [
                // There isn't a TextStyle_Lambda binding available to use.
                // Rather than rolling a custom button that will update its text style based on a function result,
                // create one button with each style, and toggle between the two with a Visibility_Lambda.
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    .Visibility_Lambda([this, featureControlCenter] 
                    {
                        if (featureControlCenter->IsCreateEnabled(this->featureType))
                        {
                            return EVisibility::Visible;
                        }
                        return EVisibility::Collapsed;
                    })
                    .Text(LOCTEXT("Create", "Create"))
                    .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGreen"))
                    .ContentPadding(FMargin(10, 2))
                    .OnClicked(this, &AwsGameKitFeatureLayoutDetails::DeployFeature)
                    .IsEnabled(true)
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .VAlign(VAlign_Center)
                    .HAlign(HAlign_Center)
                    .Visibility_Lambda([this, featureControlCenter] 
                    { 
                        if (featureControlCenter->IsCreateEnabled(this->featureType))
                        {
                            return EVisibility::Collapsed;
                        }
                        return EVisibility::Visible;
                    })
                    .Text(LOCTEXT("Create", "Create"))
                    .TextStyle(AwsGameKitStyleSet::Style, "Button.NormalText")
                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGrey"))
                    .ContentPadding(FMargin(10, 2))
                    .OnClicked(this, &AwsGameKitFeatureLayoutDetails::DeployFeature)
                    .ToolTipText_Lambda([this, featureControlCenter]()
                    {
                        if (!featureControlCenter->CanCreateOrUpdateDependentFeature(this->featureType))
                        {
                            return FText::FromString("The following resource(s) must be deployed before deploying this resource:\n" + featureControlCenter->createOrUpdateOverrideTooltips[this->featureType]);
                        }

                        return FText::FromString("");
                    })
                    .IsEnabled(false)
                ]
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2)
            [
                SNew(SButton)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGrey"))
                .ContentPadding(FMargin(10, 2))
                .Text(LOCTEXT("Redeploy", "Redeploy"))
                .OnClicked(this, &AwsGameKitFeatureLayoutDetails::DeployFeature)
                .ToolTipText_Lambda([this, featureControlCenter]()
                {
                    if (!featureControlCenter->CanCreateOrUpdateDependentFeature(this->featureType))
                    {
                        return FText::FromString("The following resource(s) must be deployed before updating this resource:\n" + featureControlCenter->createOrUpdateOverrideTooltips[this->featureType]);
                    }

                    return FText::FromString(""); 
                })
                .IsEnabled_Lambda([this, featureControlCenter] {return featureControlCenter->IsRedeployEnabled(this->featureType); })
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2)
            [
                SNew(SButton)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonRed"))
                .ContentPadding(FMargin(10, 2))
                .Text(LOCTEXT("Delete", "Delete"))
                .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                .OnClicked(this, &AwsGameKitFeatureLayoutDetails::DeleteFeature)
                .Visibility_Lambda([this, featureControlCenter]
                {
                    if (featureControlCenter->IsDeleteEnabled(this->featureType))
                    {
                        return EVisibility::Visible;
                    }
                    return EVisibility::Collapsed;
                })
                .IsEnabled(true)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(2)
            [
                SNew(SButton)
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Center)
                .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGrey"))
                .ContentPadding(FMargin(10, 2))
                .Text(LOCTEXT("Delete", "Delete"))
                .OnClicked(this, &AwsGameKitFeatureLayoutDetails::DeleteFeature)
                .ToolTipText_Lambda([this, featureControlCenter]()
                {
                    if (!featureControlCenter->CanDeleteDependentFeature(this->featureType))
                    {
                        return FText::FromString("The following resource(s) must be deleted before deleting this resource:\n" + featureControlCenter->deleteOverrideTooltips[this->featureType]);
                    }

                    return FText::FromString("");
                })
                .Visibility_Lambda([this, featureControlCenter]
                {
                    if (featureControlCenter->IsDeleteEnabled(this->featureType))
                    {
                        return EVisibility::Collapsed;
                    }
                    return EVisibility::Visible;
                })
                .IsEnabled(false)
            ]
        )
    ];
}

bool AwsGameKitFeatureLayoutDetails::ShowDashboardLink(const TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter, const TSharedPtr<FeatureResourceManager> featureResourceManager)
{
    TMap<FString, FString> featureVariables = featureResourceManager->GetFeatureVariables(this->featureType);

    if (!featureControlCenter->GetStatus(this->featureType).ToString().Equals(FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str()) &&
        featureVariables.Contains(GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED) && featureVariables[GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED].Equals("true"))
    {
        return true;
    }

    return  false;
}


void AwsGameKitFeatureLayoutDetails::CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    // This page intentionally left blank.
    // Override functionality in child class.
}

FReply AwsGameKitFeatureLayoutDetails::DeployFeature()
{
    return editorModule->GetFeatureControlCenter()->CreateOrUpdateResources(this->featureType);
}

FReply AwsGameKitFeatureLayoutDetails::DeleteFeature()
{
    return editorModule->GetFeatureControlCenter()->PrepareDeleteResources(this->featureType);
}

#undef LOCTEXT_NAMESPACE