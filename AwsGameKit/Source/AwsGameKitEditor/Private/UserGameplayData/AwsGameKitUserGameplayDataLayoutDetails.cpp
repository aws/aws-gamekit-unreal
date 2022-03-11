// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "UserGameplayData/AwsGameKitUserGameplayDataLayoutDetails.h"

// GameKit
#include "AwsGameKitEditor.h"
#include "AwsGameKitFeatureControlCenter.h"
#include "EditorState.h"
#include "FeatureResourceManager.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "MessageEndpointBuilder.h"
#include "PropertyCustomizationHelpers.h"

#define LOCTEXT_NAMESPACE "AwsGameKitLoginSettings"

AwsGameKitUserGameplayDataLayoutDetails::AwsGameKitUserGameplayDataLayoutDetails(const FAwsGameKitEditorModule* editorModule) : AwsGameKitFeatureLayoutDetails(FeatureType::UserGameplayData, editorModule)
{
}

TSharedRef<IDetailCustomization> AwsGameKitUserGameplayDataLayoutDetails::MakeInstance(const FAwsGameKitEditorModule* editorModule)
{
    return MakeShareable(new AwsGameKitUserGameplayDataLayoutDetails(editorModule));
}

void AwsGameKitUserGameplayDataLayoutDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
    // Add User Gamplay Data UI 
    static const FName BindingsCategory = TEXT("User Gameplay Data");
    featureCategoryBuilder = &DetailBuilder.EditCategory(BindingsCategory);

    featureCategoryBuilder->AddCustomRow(LOCTEXT("UserGameplayDataDeployRowFilter", "UserGameplayData | User | Gameplay | Data | Deploy | GameKit | Game Kit | AWS"))
    [
        this->GetDeployControls(false)
    ];

    featureCategoryBuilder->AddCustomRow(LOCTEXT("UserGameplayDataDescriptionRowFilter", "UserGameplayData | User | Gameplay | Data | Description | GameKit | Game Kit | AWS"))
    [
        GetFeatureFooter(LOCTEXT("UserGameplayDataDescription", "Maintain player game data in the cloud, available when and where the player signs into the game."))
    ];
}

#undef LOCTEXT_NAMESPACE
