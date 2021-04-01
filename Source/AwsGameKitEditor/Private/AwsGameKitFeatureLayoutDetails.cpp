// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

AwsGameKitFeatureLayoutDetails::AwsGameKitFeatureLayoutDetails(const FeatureType featureType, const FAwsGameKitEditorModule* editorModule) : editorModule(editorModule), featureType(featureType)
{
    this->messageEndpoint = FMessageEndpoint::Builder(AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME)
        .Handling<FMsgCredentialsState>(this, &AwsGameKitFeatureLayoutDetails::CredentialsStateMessageHandler)
        .Build();
    this->messageEndpoint->Subscribe<FMsgCredentialsState>();
}

TSharedRef<SVerticalBox> AwsGameKitFeatureLayoutDetails::GetFeatureHeader(const FText& featureDescription)
{
    const FText description = FText::Format(LOCTEXT("FeatureAwsServiceLabel",
        "{0} Uses AWS services: {1} <a id=\"external_link\" href=\"{2}\" style=\"Hyperlink\">Learn more</>"),
        featureDescription, 
        FText::FromString(AwsGameKitEnumConverter::FeatureResourcesUIString(this->featureType)),
        FText::FromString(AwsGameKitEnumConverter::FeatureToDocumentationUrl(this->featureType)));

    return SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(0, 5, 0, 0)
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
                .Visibility_Lambda([this, featureControlCenter] { return featureControlCenter->GetIconStyle(this->featureType).Compare("") == 0 ? EVisibility::Collapsed : EVisibility::Visible; })
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
                .ToolTipText(FText::FromString("Get current deployment status."))
                .IsEnabled_Lambda([this, editorState] { return editorState->AreCredentialsValid(); })
                .OnClicked_Lambda([this, featureControlCenter]
                {
                    featureControlCenter->RefreshFeatureStatuses();
                    return FReply::Handled();
                })
                .ForegroundColor(FSlateColor::UseForeground())
                [
                    SNew(SImage)
                    .Image(AwsGameKitStyleSet::Style->GetBrush("refreshIcon"))
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
                            if (featureControlCenter->IsCreateEnabled(this->featureType, false))
                            {
                                return EVisibility::Visible;
                            }
                            return EVisibility::Collapsed;
                        })
                    .Text(LOCTEXT("Deploy", "Deploy"))
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
                            if (featureControlCenter->IsCreateEnabled(this->featureType, false))
                            {
                                return EVisibility::Collapsed;
                            }
                            return EVisibility::Visible;
                        })
                    .Text(LOCTEXT("Deploy", "Deploy"))
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
                .Text(LOCTEXT("Update", "Update"))
                .OnClicked(this, &AwsGameKitFeatureLayoutDetails::DeployFeature)
                .ToolTipText_Lambda([this, featureControlCenter]()
                {
                    if (!featureControlCenter->CanCreateOrUpdateDependentFeature(this->featureType))
                    {
                        return FText::FromString("The following resource(s) must be deployed before updating this resource:\n" + featureControlCenter->createOrUpdateOverrideTooltips[this->featureType]);
                    }

                    return FText::FromString(""); 
                })
                .IsEnabled_Lambda([this, featureControlCenter] {return featureControlCenter->IsRedeployEnabled(this->featureType, false); })
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
                .IsEnabled_Lambda([this, featureControlCenter] { return featureControlCenter->IsDeleteEnabled(this->featureType, false); })
            ]
            + SHorizontalBox::Slot()
            [
                SNew(SHorizontalBox)
                .Visibility_Lambda([this, featureControlCenter]()
                    { 
                        return featureControlCenter->GetStatus(this->featureType).ToString() != FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str() 
                            ? EVisibility::Visible 
                            : EVisibility::Collapsed;
                    })
                + SHorizontalBox::Slot()
                .Padding(2)
                // Fill the remaining space - use a large scalar to effectively tell Slate to stretch the slot as far as possible
                .FillWidth(GameKit::PROJECT_SETTINGS_FILL_REMAINING)
                .HAlign(HAlign_Right)
                .VAlign(VAlign_Center)
                [
                    SNew(SHyperlink)
                    .Text(LOCTEXT("OpenDashboard", "Open Dashboard"))
                    .OnNavigate(FSimpleDelegate::CreateLambda([this, featureResourceManager]()
                    {
                        const FString url = featureResourceManager->GetDashboardURL(FeatureToUIString(this->featureType));
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
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .MaxHeight(15)
                    [
                        SNew(SImage)
                        .Image(AwsGameKitStyleSet::Style->GetBrush("ExternalIcon"))
                    ]
                ]
            ]
        )
    ];
}

void AwsGameKitFeatureLayoutDetails::CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    // This page left blank on purpose.
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