// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsGameKitDocumentationManager.h"
#include "AwsGameKitStyleSet.h"

// GameKit
#include "AwsGameKitCore.h"

// Unreal
#include "Interfaces/IPluginManager.h"

TSharedPtr<FConfigFile> AwsGameKitDocumentationManager::documentationConfig;

FString AwsGameKitDocumentationManager::GetDocumentString(const FString& section, const FString& key)
{
    if (!documentationConfig.IsValid())
    {
        documentationConfig = MakeShareable(new FConfigFile());
        FString pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
        const FString documentationPath = FPaths::Combine(pluginBaseDir, TEXT("Resources"), TEXT("documentation"), TEXT("documentation.ini"));
        documentationConfig->Read(documentationPath);
    }

    FString result;
    if (!documentationConfig->Contains(section))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitDocumentationManager::GetDocumentString() couldn't find section: %s"), *section)
        return "";
    }
    if (!documentationConfig->Find(section)->Contains(FName(key)))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitDocumentationManager::GetDocumentString() couldn't find key: %s"), *key)
        return "";
    }

    documentationConfig->GetString(*section, *key, result);
    return result;
}

TSharedRef<SBox> AwsGameKitDocumentationManager::BuildHelpButton(const FString& section, const FString& linkKey)
{
    FString link = GetDocumentString(section, linkKey);

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
                .ButtonStyle(&AwsGameKitStyleSet::Style->GetWidgetStyle<FButtonStyle>("HelpButtonStyle"))
            ]
        ]
    ];

    return helpButton;
}
