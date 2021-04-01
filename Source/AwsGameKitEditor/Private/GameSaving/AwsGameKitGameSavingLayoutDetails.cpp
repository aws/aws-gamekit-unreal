// Copyriht Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "GameSaving/AwsGameKitGameSavingLayoutDetails.h"

// GameKit
#include "AwsGameKitControlCenterLayout.h"
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "EditorState.h"
#include "Utils/AwsGameKitProjectSettingsUtils.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "MessageEndpointBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitLoginSettings"

const FString AwsGameKitGameSavingLayoutDetails::MAX_SAVE_SLOTS_PER_PLAYER = "max_save_slots_per_player";

AwsGameKitGameSavingLayoutDetails::AwsGameKitGameSavingLayoutDetails(const FAwsGameKitEditorModule* editorModule) : AwsGameKitFeatureLayoutDetails(FeatureType::GameStateCloudSaving, editorModule)
{
}

TSharedRef<IDetailCustomization> AwsGameKitGameSavingLayoutDetails::MakeInstance(const FAwsGameKitEditorModule* editorModule)
{
    // WHEN THIS IS CALLED FOR FIRST TIME FEATURE IS IMPLICITLY ACTIVATED
    // ADD NECESSARY INFO TO saveInfo.yml

    TSharedRef<AwsGameKitGameSavingLayoutDetails> layoutDetails = MakeShareable(new AwsGameKitGameSavingLayoutDetails(editorModule));
    TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    // Get editor module and update feature status
    if (editorState->AreCredentialsValid())
    {
        // Set Account details
        editorModule->GetFeatureResourceManager()->SetAccountDetails(editorState->GetCredentials());

        // Update feature status
        editorModule->GetFeatureControlCenter()->GetFeatureStatusAsync(FeatureType::GameStateCloudSaving);
    }

    return layoutDetails;
}

void AwsGameKitGameSavingLayoutDetails::CustomizeDetails(class IDetailLayoutBuilder& DetailBuilder)
{
    static const FName BindingsCategory = TEXT("Game State Cloud Saving");
    featureCategoryBuilder = &DetailBuilder.EditCategory(BindingsCategory);

    featureCategoryBuilder->AddCustomRow(LOCTEXT("GameSavingDescriptionRowFilter", "GameStateCloudSaving | GameSaving | Game State Cloud Saving | Description | GameKit | Game Kit | AWS"))
    [
        GetFeatureHeader(LOCTEXT("GameSavingDescription", "Enable players to store game save files and seamlessly resume play at a later time or on other devices."))
    ];

    // Add configuration options
    featureCategoryBuilder->AddCustomRow(LOCTEXT("GameSavingConfigRowFilter", "GameStateCloudSaving | GameSaving | Game State Cloud Saving | Config | GameKit | Game Kit | AWS"))
    [
        SNew(SVerticalBox)
        .IsEnabled_Raw(this, &AwsGameKitGameSavingLayoutDetails::CanEditConfiguration)
        + SVerticalBox::Slot()
        .Padding(0, 5)
        [
            PROJECT_SETTINGS_ROW(
                // Left - max save slots label
                SNew(STextBlock)
                .Text(FText::FromString("Maximum save slots")),

                // Right - max save slots slider
                SAssignNew(this->maximumCloudSaveSlotsPerPlayer, SSpinBox<int>)
                    .MinSliderValue(SLIDER_MINIMUM_MAX_SAVE_SLOTS_PER_PLAYER)
                    .MaxSliderValue(SLIDER_MAXIMUM_MAX_SAVE_SLOTS_PER_PLAYER)
                    .MinValue(SLIDER_MINIMUM_MAX_SAVE_SLOTS_PER_PLAYER)
                    .MaxValue(SLIDER_MANUALLY_ENTERED_MAXIMUM_MAX_SAVE_SLOTS_PER_PLAYER)
                    .OnEndSliderMovement(this, &AwsGameKitGameSavingLayoutDetails::SliderValueChanged)
            )
        ]
    ];

    this->LoadFeatureVars();

    // Add feature deployment controls
    featureCategoryBuilder->AddCustomRow(LOCTEXT("GameSavingDeployRowFilter", "GameStateCloudSaving | GameSaving | Game State Cloud Saving | Deploy | GameKit | Game Kit | AWS"))
    [
        this->GetDeployControls()
    ];
}

void AwsGameKitGameSavingLayoutDetails::SliderValueChanged(const int newValue)
{
    // Save Settings
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    featureResourceManager->SetFeatureVariable(FeatureType::GameStateCloudSaving, MAX_SAVE_SLOTS_PER_PLAYER, FString::FromInt(newValue));
}

void AwsGameKitGameSavingLayoutDetails::LoadFeatureVars()
{
    // Set default values:
    this->maximumCloudSaveSlotsPerPlayer->SetValue(DEFAULT_MAX_SAVE_SLOTS_PER_PLAYER);

    if (!editorModule->GetEditorState()->GetCredentialState())
    {
        return;
    }

    // Load values from saveInfo.yml file:
    auto featureVars = editorModule->GetFeatureResourceManager()->GetFeatureVariables(FeatureType::GameStateCloudSaving);
    if (featureVars.Contains(MAX_SAVE_SLOTS_PER_PLAYER) && !featureVars[MAX_SAVE_SLOTS_PER_PLAYER].IsEmpty())
    {
        this->maximumCloudSaveSlotsPerPlayer->SetValue(FCString::Atoi(*featureVars[MAX_SAVE_SLOTS_PER_PLAYER]));
    }
}

FReply AwsGameKitGameSavingLayoutDetails::DeployFeature()
{
    // Save Settings
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    featureResourceManager->SetFeatureVariable(FeatureType::GameStateCloudSaving, MAX_SAVE_SLOTS_PER_PLAYER, FString::FromInt(maximumCloudSaveSlotsPerPlayer->GetValue()));

    // Deploy
    return editorModule->GetFeatureControlCenter()->CreateOrUpdateResources(FeatureType::GameStateCloudSaving);
}

/**
 * Once the game developer's AWS credentials are submitted, then populate the Details Panel UI with the feature variables stored in saveInfo.yml.
 */
void AwsGameKitGameSavingLayoutDetails::CredentialsStateMessageHandler(const FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSavingLayoutDetails::CredentialsStateMessageHandler(); Message(%d)"), message.IsSubmitted);
    if (message.IsSubmitted)
    {
        this->LoadFeatureVars();
    }
}

#undef LOCTEXT_NAMESPACE
