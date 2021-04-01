// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsGameKitDocumentationManager.h"

// GameKit
#include "AwsGameKitCore.h"

// Unreal
#include "Interfaces/IPluginManager.h"

AwsGameKitDocumentationManager::AwsGameKitDocumentationManager(const FString& targetSection)
{
    auto pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
    auto documentationPath = FPaths::Combine(pluginBaseDir, TEXT("resources"), TEXT("documentation"), TEXT("documentation.ini"));
    this->documentationConfig.Read(documentationPath);
    SetSection(targetSection);

    helpStyle = MakeShareable(new FButtonStyle());
    helpStyle->SetPressed(*FEditorStyle::GetBrush("HelpIcon.Pressed"));
    helpStyle->SetNormal(*FEditorStyle::GetBrush("HelpIcon"));
    helpStyle->SetHovered(*FEditorStyle::GetBrush("HelpIcon.Hovered"));
}

FString AwsGameKitDocumentationManager::GetDocumentString(const FString& key)
{
    FString result;
    if (!this->documentationConfig.Contains(section))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitDocumentationManager::GetDocumentString() couldn't find section: %s"), *section)
        return "";
    }
    if (!this->documentationConfig.Find(section)->Contains(FName(key)))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitDocumentationManager::GetDocumentString() couldn't find key: %s"), *key)
        return "";
    }

    this->documentationConfig.GetString(*section, *key, result);
    return result;
}

TSharedRef<SBox> AwsGameKitDocumentationManager::BuildHelpButton(const FString& linkKey)
{
    auto link = GetDocumentString(linkKey);

    TSharedRef<SBox> helpButton = 
    SNew(SBox)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot()
        .VAlign(VAlign_Center)
        .AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .HAlign(HAlign_Center)
            .AutoWidth()
            [
                SNew(SButton)
                .ButtonStyle(FCoreStyle::Get(), "NoBorder")
                .ToolTipText(FText::FromString("Click to open documentation"))
                .OnClicked_Lambda([link]
                {
                    FPlatformProcess::LaunchURL(*link, nullptr, nullptr);
                    return FReply::Handled();
                })
                .ButtonStyle(helpStyle.Get())
            ]
        ]
    ];

    return helpButton;
}
