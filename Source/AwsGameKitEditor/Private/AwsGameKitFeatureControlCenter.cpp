// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitFeatureControlCenter.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"
#include "FeatureResourceManager.h"
#include "GameSaving/AwsGameKitGameSavingLayoutDetails.h"
#include "Identity/AwsGameKitIdentityLayoutDetails.h"
#include "Utils/AwsGameKitEditorUtils.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "MessageEndpointBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/Core/Public/Async/Async.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Types/SlateEnums.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitFeatureControlCenter"

static const FName FeatureDeleteTabName("DeleteFeature");

AwsGameKitFeatureControlCenter::AwsGameKitFeatureControlCenter()
{
    this->messageEndpoint = FMessageEndpoint::Builder(AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME)
        .Handling<FMsgDeploymentState>(this, &AwsGameKitFeatureControlCenter::DeploymentStateMessageHandler)
        .Build();
    this->messageEndpoint->Subscribe<FMsgDeploymentState>();
    this->messageEndpoint->Publish<FMsgDeploymentState>(new FMsgDeploymentState{ false });

    availableFeatures = TArray<FeatureType>(
        {
            FeatureType::Identity,
            FeatureType::Achievements,
            FeatureType::GameStateCloudSaving,
            FeatureType::UserGameplayData
        });
}

AwsGameKitFeatureControlCenter::~AwsGameKitFeatureControlCenter()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitFeatureControlCenter::~AwsGameKitFeatureControlCenter()"));
}

void AwsGameKitFeatureControlCenter::ResetFeatureStatuses()
{
    this->featureStatusMessage.Empty();
}

FText AwsGameKitFeatureControlCenter::GetStatus(FeatureType feature)
{
    auto fs = featureStatusMessage.Find(feature);
    if (fs == nullptr || (*fs).Compare("") == 0)
    {
        return LOCTEXT("NoFeatureStatus", "No environment selected");
    }
    else
    {
        return FText::FromString(*fs);
    }
}

FName AwsGameKitFeatureControlCenter::GetIconStyle(FeatureType feature)
{
    FName icon;
    const FString status = this->featureStatusMessage.FindOrAdd(feature);
    if (status.IsEmpty() || status.Compare(FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str()) == 0)
    {
        icon = "";
    }
    else if (status.Compare(FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str()) == 0)
    {
        icon = "DeployedIcon";
    }
    else if (status.Compare(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()) == 0 ||
             status.Compare(FeatureResourceManager::ROLLBACK_COMPLETE_STATUS_TEXT.c_str()) == 0)
    {
        icon = "ErrorIcon";
    }
    else
    {
        icon = "ProgressIcon";
    }

    return icon;
}

TMap<FString, FString> DefaultValuesForFeature(FeatureType feature)
{
    TMap<FString, FString> valuesMap;
    switch (feature)
    {
    case FeatureType::Identity:
        valuesMap.Add(AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_EMAIL_ENABLED, "true");
        valuesMap.Add(AwsGameKitIdentityLayoutDetails::GAMEKIT_IDENTITY_FACEBOOK_ENABLED, "false");
        break;
    case FeatureType::GameStateCloudSaving:
        valuesMap.Add(AwsGameKitGameSavingLayoutDetails::MAX_SAVE_SLOTS_PER_PLAYER, "10");
        break;
    default:
        break;
    }
    return valuesMap;
}

FReply AwsGameKitFeatureControlCenter::CreateOrUpdateResources(FeatureType feature)
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    auto featureResourceManager = editorModule->GetFeatureResourceManager();

    for (const FeatureType& featureToDisable : availableFeatures)
    {
        EnableFeatureButtons(featureToDisable, false, false, false);
    }

    // Ensure if we are deploying before configuring this feature,
    //  we have the default values set in featureResourceManager
    for (TPair<FString, FString> defaultValues : DefaultValuesForFeature(feature))
    {
        featureResourceManager->SetFeatureVariableIfUnset(feature, defaultValues.Key, defaultValues.Value);
    }

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
        [feature, featureResourceManager, this]()
        {
            // Notify that the current feature is being deployed
            messageEndpoint->Publish<FMsgDeploymentState>(new FMsgDeploymentState{ true, feature });

            // Create/Update Main Stack
            ConditionallyCreateOrUpdateFeatureResources(featureResourceManager, FeatureType::Main, feature);

            // Create/Update feature
            ConditionallyCreateOrUpdateFeatureResources(featureResourceManager, feature);

            // Refresh feature button state and reenable
            messageEndpoint->Publish<FMsgDeploymentState>(new FMsgDeploymentState{ false, feature });
            RefreshFeatureStatuses();
        }
    );

    return FReply::Handled();
}

void AwsGameKitFeatureControlCenter::ConditionallyCreateOrUpdateFeatureResources(TSharedPtr<FeatureResourceManager> featureResourceManager, FeatureType feature)
{
    ConditionallyCreateOrUpdateFeatureResources(featureResourceManager, feature, feature);
}

void AwsGameKitFeatureControlCenter::ConditionallyCreateOrUpdateFeatureResources(TSharedPtr<FeatureResourceManager> featureResourceManager, FeatureType feature, FeatureType featureTypeStatusOverride)
{
    IntResult result;

    // Check if the feature's been deployed. If not, copy the base templates to the instance
    auto stackStatus = featureResourceManager->GetResourcesStackStatus(feature);

    // Check if stack is in a running state from another dev
    if (stackStatus == FeatureResourceManager::WORKING_STATUS_TEXT)
    {
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString("The AWS resources for this game feature are currently being updated by another user."));
        featureStatusMessage.FindOrAdd(featureTypeStatusOverride) = FString(stackStatus.c_str());
        return;
    }

    featureStatusMessage.FindOrAdd(featureTypeStatusOverride) = FString(FeatureResourceManager::GENERATING_TEMPLATES_STATUS_TEXT.c_str());
    if (stackStatus == FeatureResourceManager::UNDEPLOYED_STATUS_TEXT)
    {
        featureStatusMessage.FindOrAdd(featureTypeStatusOverride) = FString(FeatureResourceManager::GENERATING_TEMPLATES_STATUS_TEXT.c_str());
        result = featureResourceManager->GenerateFeatureTemplates(feature);
        if (result.Result != GameKit::GAMEKIT_SUCCESS)
        {
            featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
            EnableFeatureButtonsAsync(feature, true, false, true);
            AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
            return;
        }
    }
    else
    {
        result = featureResourceManager->UpdateFeatureParameters(feature);
        if (result.Result != GameKit::GAMEKIT_SUCCESS)
        {
            featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
            EnableFeatureButtonsAsync(feature, true, false, true);
            AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
            return;
        }
    }

    if (!featureResourceManager->IsFeatureCloudFormationInstanceTemplatePresent(feature))
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("CloudFormation instance template not found for %s. Getting deployed template..."), *FeatureToUIString(feature));
        IntResult writeResult = featureResourceManager->SaveDeployedFeatureTemplate(feature);
        if (writeResult.Result != GameKit::GAMEKIT_SUCCESS)
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("Unable to retrieve deployed CloudFormation template for %s."), *FeatureToUIString(feature));
            featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
            EnableFeatureButtonsAsync(feature, true, false, true);
            AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
            return;
        }
    }

    featureStatusMessage.FindOrAdd(featureTypeStatusOverride) = FeatureResourceManager::UPLOADING_DASHBOARDS_STATUS_TEXT.c_str();
    result = featureResourceManager->UploadDashboards(feature);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        EnableFeatureButtonsAsync(feature, true, false, true);
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::UPLOADING_LAYERS_STATUS_TEXT.c_str();
    result = featureResourceManager->UploadLayers(feature);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        EnableFeatureButtonsAsync(feature, true, false, true);
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::UPLOADING_FUNCTIONS_STATUS_TEXT.c_str();
    result = featureResourceManager->UploadFunctions(feature);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        // enable all buttons to match GetFeatureStatusAsync error handling
        EnableFeatureButtonsAsync(feature, true, true, true);
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::DEPLOYING_STATUS_TEXT.c_str();
    result = featureResourceManager->CreateOrUpdateFeatureResources(feature);
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        // enable all buttons to match GetFeatureStatusAsync error handling
        EnableFeatureButtonsAsync(feature, true, true, true);
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    if (feature != FeatureType::Main) 
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str();
        EnableFeatureButtonsAsync(feature, false, true, true);
    }
}

FReply AwsGameKitFeatureControlCenter::DeleteResources(FeatureType feature)
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    auto featureResourceManager = editorModule->GetFeatureResourceManager();

    EnableFeatureButtons(feature, false, false, false);
    executeDeleteButton->SetEnabled(false);
    featureStatusMessage.FindOrAdd(feature) = FeatureResourceManager::DELETING_RESOURCES_STATUS_TEXT.c_str();

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&, feature, featureResourceManager]()
        {
            messageEndpoint->Publish<FMsgDeploymentState>(new FMsgDeploymentState{ true, feature });
            IntResult result = featureResourceManager->DeleteFeatureResources(feature);
            messageEndpoint->Publish<FMsgDeploymentState>(new FMsgDeploymentState{ false, feature });

            if (result.Result != GameKit::GAMEKIT_SUCCESS)
            {
                featureStatusMessage[feature] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
                EnableFeatureButtonsAsync(feature, false, true, true);
                EnableDeleteButtonAsync(true);
                AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
            }

            RefreshFeatureStatuses();
        });

    if (deleteResourcesWindow.IsValid())
    {
        deleteResourcesWindow->RequestDestroyWindow();
    }

    return FReply::Handled();
}

void AwsGameKitFeatureControlCenter::OpenDeleteDialog()
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    auto featureResourceManager = editorModule->GetFeatureResourceManager();

    FString featureStr = FeatureToUIString(this->featureToDelete);
    FeatureType feature = this->featureToDelete;
    TSharedPtr<STextBlock> textBlock;

    const auto robotoRegular10 = AwsGameKitStyleSet::Style->GetFontStyle(("RobotoRegular10"));
    const auto robotoRegular12 = AwsGameKitStyleSet::Style->GetFontStyle(("RobotoRegular12"));
    const auto robotoBold12 = AwsGameKitStyleSet::Style->GetFontStyle(("RobotBold12"));

    deleteResourcesWindow = SNew(SWindow)
    .SizingRule(ESizingRule::UserSized)
    .ClientSize(FVector2D(850.f, 525.f))
    .AutoCenter(EAutoCenter::PreferredWorkArea)
    .CreateTitleBar(false)
    [
        SNew(SBox)
        [
            // Header
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .Padding(10, 5, 10, 0)
                .HAlign(HAlign_Center)
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Font(robotoBold12)
                    .Text_Lambda([this]()->FText { return FText::FromString(stackCanBeDeleted ? "Confirm resource deletion" : "Cannot delete resources"); })
                ]

                + SVerticalBox::Slot()
                .Padding(0, 5, 0, 5)
                .AutoHeight()
                [
                    SNew(SBorder)
                    .BorderImage(FEditorStyle::GetBrush("Sequencer.Section.BackgroundTint"))
                    .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor("BackgroundGrey"))
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .Padding(10, 15, 10, 5)
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .AutoWrapText(true)
                            .Font(robotoRegular12)
                            .Text_Lambda([featureStr, this]()->FText { return FText::FromString(stackCanBeDeleted ?
                                "The following resources will be deleted for the '" + featureStr + "' feature:" :
                                "Cannot delete " + featureStr + " please inspect logs below:"); })
                        ]

                        // Resource info
                        + SVerticalBox::Slot()
                        .Padding(10, 5, 10, 5)
                        .MaxHeight(300)
                        [
                            SNew(SBorder)
                            .BorderImage(FEditorStyle::GetBrush("Sequencer.Section.BackgroundTint"))
                            .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor("MediumGrey"))
                            .Padding(FMargin(3, 3, 3, 3))
                            [
                                SNew(SBorder)
                                .BorderImage(FEditorStyle::GetBrush("Sequencer.Section.BackgroundTint"))
                                .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor("DarkGrey"))
                                .Padding(FMargin(3, 3, 3, 3))
                                [
                                    SNew(SScrollBox)
                                    + SScrollBox::Slot()
                                    .Padding(5, 5, 5, 5)
                                    [
                                        SAssignNew(textBlock, STextBlock)
                                        .AutoWrapText(true)
                                        .Font(robotoRegular10)
                                        .Text(FText::FromString("Loading...")).ColorAndOpacity(FLinearColor::Gray)
                                        .ColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("LightGrey"))
                                        .Text_Lambda([feature, this]()->FText { return *featureDeleteDetailsContent.FindOrAdd(feature); })
                                    ]
                                ]
                            ]
                        ]

                        // Backup hint
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Right)
                        .Padding(10, 1, 10, 5)
                        [
                            SNew(SHyperlink)
                            .Text(LOCTEXT("DynamoBackup", "How can I back up a DynamoDB table to Amazon S3?"))
                            .OnNavigate(FSimpleDelegate::CreateLambda([]() { AwsGameKitEditorUtils::OpenBrowser("https://aws.amazon.com/premiumsupport/knowledge-center/back-up-dynamodb-s3/"); }))
                        ]

                        // Confirm deletion
                        + SVerticalBox::Slot()
                        .Padding(10, 10, 10, 0)
                        .AutoHeight()
                        [
                            SNew(STextBlock)
                            .AutoWrapText(true)
                            .Font(robotoRegular12)
                            .Visibility_Lambda([this]() -> EVisibility { return stackCanBeDeleted ? EVisibility::Visible : EVisibility::Collapsed; })
                            .Text(FText::FromString("To confirm deletion, type 'Yes' below."))
                        ]

                        + SVerticalBox::Slot()
                        .Padding(10, 5, 10, 10)
                        .AutoHeight()
                        [
                            SAssignNew(deleteConfirmText, SEditableTextBox)
                            .BackgroundColor(FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f)))
                            .MinDesiredWidth(50)
                            .IsEnabled(true)
                            .IsReadOnly(false)
                            .Visibility_Lambda([this]() -> EVisibility { return stackCanBeDeleted ? EVisibility::Visible : EVisibility::Collapsed; })
                            .OnTextChanged(FOnTextChanged::CreateRaw(this, &AwsGameKitFeatureControlCenter::OnConfirmDeleteChanged))
                        ]

                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(10, 10, 10, 20)
                        .HAlign(HAlign_Right)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .Padding(0, 0, 15, 0)
                            .VAlign(VAlign_Center)
                            [
                                SNew(SBox)
                                .HeightOverride(25)
                                .WidthOverride(100)
                                [
                                    SAssignNew(executeCancelButton, SButton)
                                    .HAlign(HAlign_Center)
                                    .VAlign(VAlign_Center)
                                    .OnClicked(FOnClicked::CreateRaw(this, &AwsGameKitFeatureControlCenter::OnCancelDeleteChanged))
                                    .Text_Lambda([this]()->FText { return FText::FromString(stackCanBeDeleted ? "Cancel" : "OK"); })
                                ]
                            ]

                            + SHorizontalBox::Slot()
                            .FillWidth(50)
                            .VAlign(VAlign_Center)
                            [
                                SNew(SBox)
                                .HeightOverride(25)
                                .WidthOverride(100)
                                [
                                    SAssignNew(executeDeleteButton, SButton)
                                    .Text(LOCTEXT("DeleteLabel", "Confirm"))
                                    .HAlign(HAlign_Center)
                                    .VAlign(VAlign_Center)
                                    .Visibility_Lambda([this]() -> EVisibility { return stackCanBeDeleted ? EVisibility::Visible : EVisibility::Collapsed; })
                                    .OnClicked(FOnClicked::CreateRaw(this, &AwsGameKitFeatureControlCenter::DeleteResources, feature))
                                    .IsEnabled(false)
                                    ]
                                ]
                            ]
                        ]
                    ]
                ]
            ]
        ];

    TSharedPtr<FText> empty(new FText());
    featureDeleteDetailsContent.FindOrAdd(feature) = empty;

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&, feature, featureResourceManager]()
        {
            TArray<FString> resourcesInfo;
            IntResult result = featureResourceManager->DescribeFeatureResources(feature, resourcesInfo);

            if (result.Result != GameKit::GAMEKIT_SUCCESS)
            {
                resourcesInfo.Reset();
                resourcesInfo.Add(FString("Could not retrieve feature resources."));
                resourcesInfo.Add(result.ErrorMessage+"\n Logs:");
                resourcesInfo.Add(featureResourceManager->GetLog());
                stackCanBeDeleted = false;
            }
            else
            {
                stackCanBeDeleted = true;
            }

            static const TCHAR* newLine = L"\n";
            FText resourceText = FText::FromString(FString::Join(resourcesInfo, newLine));
            *featureDeleteDetailsContent.FindOrAdd(feature) = resourceText;
        });

    GEditor->EditorAddModalWindow(deleteResourcesWindow.ToSharedRef());
}

void AwsGameKitFeatureControlCenter::OnConfirmDeleteChanged(const FText& text)
{
    FString* currentFeatureStatus = featureStatusMessage.Find(this->featureToDelete);
    bool userConfirmedDeletion = text.CompareToCaseIgnored(FText::FromString("yes")) == 0 &&
        (currentFeatureStatus->Equals(FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str()) ||
         currentFeatureStatus->Equals(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()) ||
         currentFeatureStatus->Equals(FeatureResourceManager::ROLLBACK_COMPLETE_STATUS_TEXT.c_str()));
    executeDeleteButton->SetEnabled(userConfirmedDeletion);
}

FReply AwsGameKitFeatureControlCenter::OnCancelDeleteChanged()
{
    if (deleteResourcesWindow.IsValid())
    {
        deleteResourcesWindow->RequestDestroyWindow();
    }
    return FReply::Handled();
}

void AwsGameKitFeatureControlCenter::RefreshFeatureStatuses()
{
    for (const FeatureType& feature : availableFeatures)
    {
        GetFeatureStatusAsync(feature);
    }
}

FReply AwsGameKitFeatureControlCenter::PrepareDeleteResources(FeatureType feature)
{
    this->featureToDelete = feature;

    OpenDeleteDialog();

    return FReply::Handled();
}

bool AwsGameKitFeatureControlCenter::IsAnyFeatureUpdating()
{
    for (auto& featureStatus : featureStatusMessage)
    {
        if (featureStatus.Value != FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str() &&
            featureStatus.Value != FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str() &&
            featureStatus.Value != FeatureResourceManager::ERROR_STATUS_TEXT.c_str() &&
            featureStatus.Value != FeatureResourceManager::ROLLBACK_COMPLETE_STATUS_TEXT.c_str() &&
            !featureStatus.Value.IsEmpty())
        {
            return true;
        }
    }
    return false;
}

bool AwsGameKitFeatureControlCenter::IsFeatureUpdating(const FeatureType feature)
{
    return anyDeploymentRunning && currentlyDeployingFeature == feature;
}

bool AwsGameKitFeatureControlCenter::CheckDependentFeatureStatus(FeatureType feature, const char* status, TArray<FString>& dependentFeatures)
{
    if (featureStatusMessage.Contains(feature) && featureStatusMessage[feature] != status)
    {
        dependentFeatures.Add(FeatureToUIString(feature));
        return false;
    }
    return true;
}

bool AwsGameKitFeatureControlCenter::CanCreateOrUpdateDependentFeature(FeatureType feature)
{
    TArray<FString> dependentFeatures;
    switch (feature)
    {
    case FeatureType::Achievements:
        CheckDependentFeatureStatus(FeatureType::Identity, FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str(), dependentFeatures);
        break;
    case FeatureType::GameStateCloudSaving:
        CheckDependentFeatureStatus(FeatureType::Identity, FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str(), dependentFeatures);
        break;
    case FeatureType::UserGameplayData:
        CheckDependentFeatureStatus(FeatureType::Identity, FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str(), dependentFeatures);
        break;
    default:
        return true;
    }

    createOrUpdateOverrideTooltips.Add(feature, FString::Join(dependentFeatures, TEXT("\n")));
    return createOrUpdateOverrideTooltips[feature].IsEmpty() ? true : false;
}

bool AwsGameKitFeatureControlCenter::CanDeleteDependentFeature(FeatureType feature)
{
    TArray<FString> dependentFeatures;

    switch (feature)
    {
    case FeatureType::Identity:
        CheckDependentFeatureStatus(FeatureType::Achievements, FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str(), dependentFeatures);
        CheckDependentFeatureStatus(FeatureType::GameStateCloudSaving, FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str(), dependentFeatures);
        CheckDependentFeatureStatus(FeatureType::UserGameplayData, FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str(), dependentFeatures);
        break;
    default:
        return true;
    }

    deleteOverrideTooltips.Add(feature, FString::Join(dependentFeatures, TEXT("\n")));
    return deleteOverrideTooltips[feature].IsEmpty() ? true : false;
}

bool AwsGameKitFeatureControlCenter::IsCreateEnabled(FeatureType feature, bool defaultValue)
{
    if (!featureCreateButtonsState.Contains(feature))
    {
        featureCreateButtonsState.Add(feature, defaultValue);
    }

    if(!featureCreateOverrideButtonsState.Contains(feature))
    {
        featureCreateOverrideButtonsState.Add(feature, CanCreateOrUpdateDependentFeature(feature));
    }

    if (featureCreateOverrideButtonsState[feature] != CanCreateOrUpdateDependentFeature(feature))
    {
        featureCreateOverrideButtonsState[feature] = CanCreateOrUpdateDependentFeature(feature);
    }

    if (!featureCreateOverrideButtonsState[feature])
    {
        return featureCreateOverrideButtonsState[feature];
    }
   
    return featureCreateButtonsState[feature];
}

bool AwsGameKitFeatureControlCenter::IsRedeployEnabled(FeatureType feature, bool defaultValue)
{
    if (!featureRedeployButtonsState.Contains(feature))
    {
        featureRedeployButtonsState.Add(feature, defaultValue);
    }

    if (!featureRedeployOverrideButtonsState.Contains(feature))
    {
        featureRedeployOverrideButtonsState.Add(feature, CanCreateOrUpdateDependentFeature(feature));
    }

    if (featureRedeployOverrideButtonsState[feature] != CanCreateOrUpdateDependentFeature(feature))
    {
        featureRedeployOverrideButtonsState[feature] = CanCreateOrUpdateDependentFeature(feature);
    }

    if (!featureRedeployOverrideButtonsState[feature])
    {
        return featureRedeployOverrideButtonsState[feature];
    }

    return featureRedeployButtonsState[feature];
}

bool AwsGameKitFeatureControlCenter::IsDeleteEnabled(FeatureType feature, bool defaultValue)
{
    if (!featureDeleteButtonsState.Contains(feature))
    {
        featureDeleteButtonsState.Add(feature, defaultValue);
    }

    if (!featureDeleteOverrideButtonsState.Contains(feature))
    {
        featureDeleteOverrideButtonsState.Add(feature, CanDeleteDependentFeature(feature));
    }

    if (featureDeleteOverrideButtonsState[feature] != CanDeleteDependentFeature(feature))
    {
        featureDeleteOverrideButtonsState[feature] = CanDeleteDependentFeature(feature);
    }

    if (!featureDeleteOverrideButtonsState[feature])
    {
        return featureDeleteOverrideButtonsState[feature];
    }

    return featureDeleteButtonsState[feature];
}

void AwsGameKitFeatureControlCenter::EnableFeatureButtons(FeatureType feature, bool createEnabled, bool redeployEnabled, bool deleteEnabled)
{
    if (anyDeploymentRunning)
    {
        createEnabled = false;
        redeployEnabled = false;
        deleteEnabled = false;
    }
    featureCreateButtonsState.FindOrAdd(feature) = createEnabled;
    featureRedeployButtonsState.FindOrAdd(feature) = redeployEnabled;
    featureDeleteButtonsState.FindOrAdd(feature) = deleteEnabled;
}

void AwsGameKitFeatureControlCenter::EnableFeatureButtonsAsync(FeatureType feature, bool createEnabled, bool redeployEnabled, bool deleteEnabled)
{
    AsyncTask(ENamedThreads::GameThread, [this, feature, createEnabled, redeployEnabled, deleteEnabled]()
        {
            {
                FScopeLock scopeLock(&featureButtonsMutex);
                this->EnableFeatureButtons(feature, createEnabled, redeployEnabled, deleteEnabled);
            }
        });
}

void AwsGameKitFeatureControlCenter::EnableDeleteButtonAsync(bool enabled)
{
    AsyncTask(ENamedThreads::GameThread, [this, enabled]()
        {
            this->executeDeleteButton->SetEnabled(enabled);
        });
}

void AwsGameKitFeatureControlCenter::GetFeatureStatusAsync(FeatureType feature)
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    auto featureResourceManager = editorModule->GetFeatureResourceManager();
    AsyncTask(ENamedThreads::GameThread, [&, feature, featureResourceManager]()
        {
            std::string stackStatus;
            FString tmpLocalStatus;

            {
                FScopeLock lock(&this->featureStatusMessageMutex);
                stackStatus = featureResourceManager->GetResourcesStackStatus(feature);
                tmpLocalStatus = featureStatusMessage.FindOrAdd(feature);
                if (stackStatus == FeatureResourceManager::WORKING_STATUS_TEXT)
                {
                    messageEndpoint->Publish<FMsgDeploymentState>(new FMsgDeploymentState{ true, feature });
                }
            }

            if (tmpLocalStatus.IsEmpty())
            {
                // first time retrieving info and not deployed
                if (stackStatus == FeatureResourceManager::UNDEPLOYED_STATUS_TEXT)
                {
                    // enable create
                    this->EnableFeatureButtonsAsync(feature, true, false, false);
                    featureStatusMessage.FindOrAdd(feature) = FString(stackStatus.c_str());

                    return;
                }
                else
                {
                    // set temp local status to stack status
                    tmpLocalStatus = FString(stackStatus.c_str());
                    featureStatusMessage.FindOrAdd(feature) = FString(stackStatus.c_str());
                }
            }

            if (featureResourceManager->IsMainStackInProgress() || featureResourceManager->IsTaskInProgress(feature))
            {
                // disable all buttons, don't change status message while task is in progress
                messageEndpoint->Publish<FMsgDeploymentState>(new FMsgDeploymentState{ true, feature });
            }
            else if (stackStatus == FeatureResourceManager::DEPLOYED_STATUS_TEXT)
            {
                // enable redeploy, delete options
                this->EnableFeatureButtonsAsync(feature, false, true, true);
                featureStatusMessage[feature] = FString(stackStatus.c_str());
            }
            else if (stackStatus == FeatureResourceManager::UNDEPLOYED_STATUS_TEXT && 
                tmpLocalStatus != FString(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()))
            {
                // enable create
                this->EnableFeatureButtonsAsync(feature, true, false, false);
                featureStatusMessage[feature] = FString(stackStatus.c_str());
            }
            else
            {
                // error or drifted, enable all options
                this->EnableFeatureButtonsAsync(feature, true, true, true);
                if (tmpLocalStatus == FString(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()))
                {
                    featureStatusMessage[feature] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
                }
                else
                {
                    featureStatusMessage[feature] = FString(stackStatus.c_str());
                }
            }
        });
}

void AwsGameKitFeatureControlCenter::DeploymentStateMessageHandler(const struct FMsgDeploymentState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    {
        FScopeLock scopeLock(&anyDeploymentRunningMutex);
        if (message.IsRunning)
        {
            anyDeploymentRunning = true;
            currentlyDeployingFeature = message.FeatureType;
            for (const FeatureType& featureToDisable : availableFeatures)
            {
                EnableFeatureButtonsAsync(featureToDisable, false, false, false);
            }
        }
        else
        {
            anyDeploymentRunning = false;
        }
    }
}

bool AwsGameKitFeatureControlCenter::IsValidProviderCredentialsInput(
    TSharedPtr<SCheckBox> providerCheckbox,
    TSharedPtr<SEditableTextBox> providerAppId,
    TSharedPtr<SEditableTextBox> providerAppSecret,
    FString secretId
    ) const
{
    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    bool const secretStored = featureResourceManager->CheckSecretExists(secretId).Result == GameKit::GAMEKIT_SUCCESS;

    return !providerCheckbox->IsChecked() ||
        (providerCheckbox->IsChecked() && !providerAppId->GetText().IsEmptyOrWhitespace() && (!providerAppSecret->GetText().IsEmptyOrWhitespace() || secretStored));
}

#undef LOCTEXT_NAMESPACE
