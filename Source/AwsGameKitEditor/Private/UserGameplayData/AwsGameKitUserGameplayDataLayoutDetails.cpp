// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "UserGameplayData/AwsGameKitUserGameplayDataLayoutDetails.h" // First include (Unreal requirement)
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
    // WHEN THIS IS CALLED FOR FIRST TIME FEATURE IS IMPLICITLY ACTIVATED
    // ADD NECESSARY INFO TO saveInfo.yml

    TSharedRef<AwsGameKitUserGameplayDataLayoutDetails> layoutDetails = MakeShareable(new AwsGameKitUserGameplayDataLayoutDetails(editorModule));
    TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    // Get editor module and update feature status
    if (editorState->AreCredentialsValid())
    {
        editorModule->GetFeatureResourceManager()->SetAccountDetails(editorState->GetCredentials());

        // Update feature status
        editorModule->GetFeatureControlCenter()->GetFeatureStatusAsync(FeatureType::UserGameplayData);
    }

    return layoutDetails;
}

void AwsGameKitUserGameplayDataLayoutDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
    // Add User Gamplay Data UI 
    static const FName BindingsCategory = TEXT("User Gameplay Data");
    featureCategoryBuilder = &DetailBuilder.EditCategory(BindingsCategory);

    featureCategoryBuilder->AddCustomRow(LOCTEXT("UserGameplayDataDescriptionRowFilter", "UserGameplayData | User | Gameplay | Data | Description | GameKit | Game Kit | AWS"))
    [
        GetFeatureHeader(LOCTEXT("UserGameplayDataDescription", "Maintain player game data in the cloud, available when and where the player signs into the game."))
    ];

    featureCategoryBuilder->AddCustomRow(LOCTEXT("UserGameplayDataDeployRowFilter", "UserGameplayData | User | Gameplay | Data | Deploy | GameKit | Game Kit | AWS"))
    [
        this->GetDeployControls(false)
    ];
}

#undef LOCTEXT_NAMESPACE
