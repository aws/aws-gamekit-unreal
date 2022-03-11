// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitSettingsLayoutDetails.h" // First include (Unreal requirement)

// GameKit
#include "Achievements/AwsGameKitAchievementsLayoutDetails.h"
#include "AwsGameKitCredentialsLayoutDetails.h"
#include "GameSaving/AwsGameKitGameSavingLayoutDetails.h"
#include "Identity/AwsGameKitIdentityLayoutDetails.h"
#include "UserGameplayData/AwsGameKitUserGameplayDataLayoutDetails.h"

AwsGameKitSettingsLayoutDetails::AwsGameKitSettingsLayoutDetails()
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    this->credentialsLayout = AwsGameKitCredentialsLayoutDetails::MakeInstance(editorModule);
    this->achievementsLayout = AwsGameKitAchievementsLayoutDetails::MakeInstance(editorModule);
    this->gameSavingLayout = AwsGameKitGameSavingLayoutDetails::MakeInstance(editorModule);
    this->identityLayout = AwsGameKitIdentityLayoutDetails::MakeInstance(editorModule);
    this->userGameplayDataLayout = AwsGameKitUserGameplayDataLayoutDetails::MakeInstance(editorModule);
}

TSharedRef<IDetailCustomization> AwsGameKitSettingsLayoutDetails::MakeInstance()
{
    return MakeShareable(new AwsGameKitSettingsLayoutDetails());
}

void AwsGameKitSettingsLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
    // Layout order is preserved
    this->credentialsLayout->CustomizeDetails(DetailLayout);
    this->identityLayout->CustomizeDetails(DetailLayout);
    this->achievementsLayout->CustomizeDetails(DetailLayout);
    this->gameSavingLayout->CustomizeDetails(DetailLayout);
    this->userGameplayDataLayout->CustomizeDetails(DetailLayout);
}
