// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "AwsGameKitConfigLayoutDetails.h" // First include (Unreal requirement)
#include "AwsGameKitControlCenterLayout.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"
#include "Utils/AwsGameKitProjectSettingsUtils.h"

// Unreal
#include "DetailLayoutBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitLoginSettings"

AwsGameKitConfigLayoutDetails::AwsGameKitConfigLayoutDetails(FAwsGameKitEditorModule* editorModule) : editorModule(editorModule)
{
}

AwsGameKitConfigLayoutDetails::~AwsGameKitConfigLayoutDetails()
{
}

TSharedRef<IDetailCustomization> AwsGameKitConfigLayoutDetails::MakeInstance(FAwsGameKitEditorModule* editorModule)
{
    TSharedRef<AwsGameKitConfigLayoutDetails> layoutDetails = MakeShareable(new AwsGameKitConfigLayoutDetails(editorModule));

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        AwsGameKitControlCenterLayout::ControlCenterHomeTabName,
        FOnSpawnTab::CreateRaw(
            editorModule->GetControlCenterLayout().Get(),
            &AwsGameKitControlCenterLayout::OnSpawnHomeTab))
        .SetDisplayName(FText::FromString("AWS Control Center: Home"))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    return layoutDetails;
}

void AwsGameKitConfigLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    static const FName BindingsCategory = TEXT("Current Configuration");
    configCategoryBuilder = &DetailLayout.EditCategory(BindingsCategory);

    TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter = editorModule->GetFeatureControlCenter();

    const TSharedPtr<EditorState> editorState = editorModule->GetEditorState();
    configCategoryBuilder->AddCustomRow(LOCTEXT("CurrentConfigurationRowFilter", "Current Configuration | Config | Environment | Env | Control Center | GameKit | Game Kit | AWS"))
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .Padding(0, 5)
        .AutoHeight()
        [
            PROJECT_SETTINGS_ROW(
                // Left - environment label
                SNew(STextBlock)
                .Text(LOCTEXT("EnvironmentLabel", "Environment:")),

                // Right - current environment
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(STextBlock)
                    .Visibility_Lambda([this, editorState] { return editorState->AreCredentialsValid() ? EVisibility::Visible : EVisibility::Collapsed; })
                    .Text_Lambda([this] { return FText::FromString(this->editorModule->GetFeatureResourceManager()->GetNavigationString()); })
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SRichTextBlock)
                    .Visibility_Lambda([this, editorState] { return editorState->AreCredentialsValid() ? EVisibility::Collapsed : EVisibility::Visible; })
                    .Text(LOCTEXT("UnknownEnvironment", "<img src=\"WarningIconInline\"/> Select an environment to work with AWS GameKit features."))
                    .TextStyle(AwsGameKitStyleSet::Style, "DescriptionBoldText")
                    + SRichTextBlock::ImageDecorator("img", AwsGameKitStyleSet::Style.Get())
                ]
            )
        ]
        + SVerticalBox::Slot()
        .AutoHeight()
        [
            PROJECT_SETTINGS_ROW(
                // Left - control center description
                SNew(STextBlock)
                .AutoWrapText(true)
                .MinDesiredWidth(200)
                .Text(LOCTEXT("HelperLabel", "Create/select current environment:")),

                // Right - open control center button
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .OnClicked(this, &AwsGameKitConfigLayoutDetails::OpenControlCenter)
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGreen"))
                    .ContentPadding(FMargin(10, 2))
                    .Text(LOCTEXT("OpenControlCenterButton", "Open Control Center"))
                    .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                ]
                + SHorizontalBox::Slot()
                .Padding(10, 5, 5, 5)
                .MaxWidth(15)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .MaxHeight(15)
                    [
                        SNew(SImage)
                        .Image(AwsGameKitStyleSet::Style->GetBrush("ExternalIcon"))
                    ]
                ]
            )
        ]
    ];
}

FReply AwsGameKitConfigLayoutDetails::OpenControlCenter()
{
    this->editorModule->OpenControlCenter();
    return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE