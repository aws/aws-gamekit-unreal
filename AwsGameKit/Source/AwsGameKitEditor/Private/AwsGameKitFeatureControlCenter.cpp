// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitFeatureControlCenter.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitEnumConverter.h"
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
#include "Widgets/Text/SRichTextBlock.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitFeatureControlCenter"

AwsGameKitFeatureControlCenter::AwsGameKitFeatureControlCenter(FAwsGameKitEditorModule* editorModule) : editorModule(editorModule)
{
    this->credentialsMessageEndpoint = FMessageEndpoint::Builder(AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME)
        .Handling<FMsgCredentialsState>(this, &AwsGameKitFeatureControlCenter::CredentialsStateMessageHandler)
        .Build();
    this->credentialsMessageEndpoint->Subscribe<FMsgCredentialsState>();
}

AwsGameKitFeatureControlCenter::~AwsGameKitFeatureControlCenter()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitFeatureControlCenter::~AwsGameKitFeatureControlCenter()"));
}

void AwsGameKitFeatureControlCenter::ResetFeatureStatuses()
{
    this->featureStatusMessage.Empty();
}

bool AwsGameKitFeatureControlCenter::FeatureAvailable(FeatureType feature)
{
    const FString* fs = featureStatusMessage.Find(feature);
    if (fs == nullptr || (*fs).Compare("") == 0 || !credentialsSubmitted)
    {
        return false;
    }

    return true;
}

FText AwsGameKitFeatureControlCenter::GetStatus(FeatureType feature)
{
    const FString* fs = featureStatusMessage.Find(feature);
    if (fs == nullptr || (*fs).Compare("") == 0)
    {
        return LOCTEXT("NoFeatureStatus", "No environment selected");
    }

    if (!credentialsSubmitted)
    {
        return LOCTEXT("UnsubmittedCredentialsStatus", "To enable the actions below, submit Environment and Credentials");
    }

    return FText::FromString(*fs);
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
    valuesMap.Add(AwsGameKitFeatureLayoutDetails::GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED, "true");
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
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();

    // Ensure if we are deploying before configuring this feature,
    //  we have the default values set in featureResourceManager
    for (TPair<FString, FString> defaultValues : DefaultValuesForFeature(feature))
    {
        featureResourceManager->SetFeatureVariableIfUnset(feature, defaultValues.Key, defaultValues.Value);
    }

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask,
        [feature, featureResourceManager, this]()
        {
            // Create/Update Main Stack
            ConditionallyCreateOrUpdateFeatureResources(featureResourceManager, FeatureType::Main, feature);

            // Create/Update feature
            ConditionallyCreateOrUpdateFeatureResources(featureResourceManager, feature);

            // Refresh feature button state and reenable
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
    const std::string stackStatus = featureResourceManager->GetResourcesStackStatus(feature);

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
        // Only missing instance files will be created; existing instance files will be reused
        result = featureResourceManager->GenerateFeatureInstanceFiles(feature);
        if (result.Result != GameKit::GAMEKIT_SUCCESS)
        {
            featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
            AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
            return;
        }
    }
    else
    {
        // Ensure we have valid parameters for our features
        result = featureResourceManager->ValidateFeatureParameters(feature);
        if (result.Result != GameKit::GAMEKIT_SUCCESS)
        {
            featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
            AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
            return;
        }
    }

    if (!featureResourceManager->IsFeatureCloudFormationInstanceTemplatePresent(feature))
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("CloudFormation instance template not found for %s. Getting deployed template..."), *AwsGameKitEnumConverter::FeatureToUIString(feature));
        IntResult writeResult = featureResourceManager->SaveDeployedFeatureTemplate(feature);
        if (writeResult.Result != GameKit::GAMEKIT_SUCCESS)
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("Unable to retrieve deployed CloudFormation template for %s."), *AwsGameKitEnumConverter::FeatureToUIString(feature));
            featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
            AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
            return;
        }
    }

    featureStatusMessage.FindOrAdd(featureTypeStatusOverride) = FeatureResourceManager::UPLOADING_DASHBOARDS_STATUS_TEXT.c_str();
    result = featureResourceManager->UploadDashboards(feature);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::UPLOADING_LAYERS_STATUS_TEXT.c_str();
    result = featureResourceManager->UploadLayers(feature);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::UPLOADING_FUNCTIONS_STATUS_TEXT.c_str();
    result = featureResourceManager->UploadFunctions(feature);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::DEPLOYING_STATUS_TEXT.c_str();
    result = featureResourceManager->CreateOrUpdateFeatureResources(feature);
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
        AwsGameKitEditorUtils::ShowMessageDialogAsync(EAppMsgType::Ok, FText::FromString(result.ErrorMessage));
        return;
    }

    if (feature != FeatureType::Main)
    {
        featureStatusMessage[featureTypeStatusOverride] = FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str();
    }
}

FReply AwsGameKitFeatureControlCenter::DeleteResources(FeatureType feature)
{
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();

    executeDeleteButton->SetEnabled(false);
    featureStatusMessage.FindOrAdd(feature) = FeatureResourceManager::DELETING_RESOURCES_STATUS_TEXT.c_str();

    AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&, feature, featureResourceManager]()
        {
            IntResult result = featureResourceManager->DeleteFeatureResources(feature);

            if (result.Result != GameKit::GAMEKIT_SUCCESS)
            {
                featureStatusMessage[feature] = FeatureResourceManager::ERROR_STATUS_TEXT.c_str();
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
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();

    FString featureStr = AwsGameKitEnumConverter::FeatureToUIString(this->featureToDelete);
    FeatureType feature = this->featureToDelete;
    TSharedPtr<STextBlock> textBlock;

    const FSlateFontInfo robotoRegular10 = AwsGameKitStyleSet::Style->GetFontStyle(("RobotoRegular10"));
    const FSlateFontInfo robotoRegular12 = AwsGameKitStyleSet::Style->GetFontStyle(("RobotoRegular12"));
    const FSlateFontInfo robotoBold12 = AwsGameKitStyleSet::Style->GetFontStyle(("RobotBold12"));

    const FText instanceFilesDeleteDescription = FText::Format(LOCTEXT("InstanceFilesDelete",
        "{0} <a id=\"external_link\" href=\"{1}\" style=\"Hyperlink\">{2}</>"),
        LOCTEXT("InstanceFilesDisclaimer", "This action retains your locally stored GameKit code and configuration files for this feature, which will be used in future deployments. To return this feature to the default GameKit experience, you must delete these files manually."),
        AwsGameKitDocumentationManager::GetDocumentText("dev_guide_url", "delete_instance_reference"),
        LOCTEXT("LearnMore", "Learn more"));

    deleteResourcesWindow = SNew(SWindow)
    .SizingRule(ESizingRule::UserSized)
    .ClientSize(FVector2D(850.f, 550.f))
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
                            .OnNavigate(FSimpleDelegate::CreateLambda([]() { AwsGameKitEditorUtils::OpenBrowser(AwsGameKitDocumentationManager::GetDocumentString("url", "backup_dynamo_reference")); }))
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
                        .Padding(10, 0)
                        .HAlign(HAlign_Left)
                        [
                            SNew(SRichTextBlock)
                            .AutoWrapText(true)
                            .TextStyle(AwsGameKitStyleSet::Style, "DescriptionText")
                            .Text(instanceFilesDeleteDescription)
                            .DecoratorStyleSet(AwsGameKitStyleSet::Style.Get())
                            + SRichTextBlock::HyperlinkDecorator(TEXT("external_link"), FSlateHyperlinkRun::FOnClick::CreateLambda([this](const FSlateHyperlinkRun::FMetadata& Metadata) {
                                const FString* url = Metadata.Find(TEXT("href"));
                                if (url)
                                {
                                    AwsGameKitEditorUtils::OpenBrowser(*url);
                                }
                            }))
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

            FText resourceText = FText::FromString(FString::Join(resourcesInfo, TEXT("\n")));
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
    for (const FeatureType& featureType : availableFeatures)
    {
        if (IsFeatureUpdating(featureType))
        {
            return true;
        }
    }
    return false;
}

bool AwsGameKitFeatureControlCenter::IsFeatureUpdating(const FeatureType feature)
{
    const FString* featureStatus = featureStatusMessage.Find(feature);
    
    return featureStatus != nullptr &&
        !featureStatus->Equals(FeatureResourceManager::DEPLOYED_STATUS_TEXT.c_str()) &&
        !featureStatus->Equals(FeatureResourceManager::UNDEPLOYED_STATUS_TEXT.c_str()) &&
        !featureStatus->Equals(FeatureResourceManager::ERROR_STATUS_TEXT.c_str()) &&
        !featureStatus->Equals(FeatureResourceManager::ROLLBACK_COMPLETE_STATUS_TEXT.c_str()) &&
        !featureStatus->IsEmpty();
}

bool AwsGameKitFeatureControlCenter::IsRefreshAvailable()
{
    const TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    const TSharedPtr<EditorState> editorState = editorModule->GetEditorState();

    if (!editorState->AreCredentialsValid())
    {
        return false;
    }

    for (const FeatureType& featureType : availableFeatures)
    {
        if (!this->FeatureAvailable(featureType) || featureResourceManager->IsTaskInProgress(featureType))
        {
            return false;
        }
    }
    return true;
}

bool AwsGameKitFeatureControlCenter::CheckDependentFeatureStatus(FeatureType feature, const char* status, TArray<FString>& dependentFeatures)
{
    if (featureStatusMessage.Contains(feature) && featureStatusMessage[feature] != status)
    {
        dependentFeatures.Add("- " + AwsGameKitEnumConverter::FeatureToUIString(feature));
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

bool AwsGameKitFeatureControlCenter::IsCreateEnabled(FeatureType feature)
{
    FString featureStatus;
    if (!IsFeatureInteractable(feature, featureStatus)) {
        return false;
    }

    if (!CanCreateOrUpdateDependentFeature(feature))
    {
        return false;
    }

    return createEnabledStatuses.Contains(*featureStatus);
}

bool AwsGameKitFeatureControlCenter::IsRedeployEnabled(FeatureType feature)
{
    FString featureStatus;
    if (!IsFeatureInteractable(feature, featureStatus)) {
        return false;
    }

    if (!CanCreateOrUpdateDependentFeature(feature))
    {
        return false;
    }

    return redeployEnabledStatuses.Contains(featureStatus);
}

bool AwsGameKitFeatureControlCenter::IsDeleteEnabled(FeatureType feature)
{
    FString featureStatus;
    if (!IsFeatureInteractable(feature, featureStatus)) {
        return false;
    }

    if (!CanDeleteDependentFeature(feature))
    {
        return false;
    }

    return deleteEnabledStatuses.Contains(featureStatus);
}

bool AwsGameKitFeatureControlCenter::IsFeatureInteractable(FeatureType feature, FString& outFeatureStatus)
{
    if (!credentialsSubmitted)
    {
        return false;
    }

    // Don't allow interaction with features of unknown status
    const FString* status = featureStatusMessage.Find(feature);
    if (status == nullptr) {
        return false;
    }

    // Only allow one deployment at a time
    if (IsAnyFeatureUpdating()) {
        return false;
    }

    outFeatureStatus = *status;

    return true;
}

void AwsGameKitFeatureControlCenter::GetFeatureStatusAsync(FeatureType feature)
{
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    AsyncTask(ENamedThreads::GameThread, [&, feature, featureResourceManager]()
        {
            {
                FScopeLock lock(&this->featureStatusMessageMutex);
                const std::string stackStatus = featureResourceManager->GetResourcesStackStatus(feature);
                featureStatusMessage.FindOrAdd(feature) = FString(stackStatus.c_str());
            }
        });
}

void AwsGameKitFeatureControlCenter::CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    this->credentialsSubmitted = message.IsSubmitted;
}

bool AwsGameKitFeatureControlCenter::IsValidProviderCredentialsInput(
    TSharedPtr<SCheckBox> providerCheckbox,
    TSharedPtr<SEditableTextBox> providerAppId,
    TSharedPtr<SEditableTextBox> providerAppSecret,
    FString secretId
    ) const
{
    // If provider checkbox is not checked, then credentials are unnecessary, so valid input
    if (!providerCheckbox->IsChecked())
    {
        return true;
    }

    // If provider checkbox is checked and appId is not provided, invalid input.
    if (providerAppId->GetText().IsEmptyOrWhitespace())
    {
        return false;
    }

    // If provider checkbox is checked and appId and appSecret also provided, then valid input.
    if (!providerAppId->GetText().IsEmptyOrWhitespace() && !providerAppSecret->GetText().IsEmptyOrWhitespace())
    {
        return true;
    }

    // Check for saved secret if checkbox checked, appId provided and secret not provided.
    TSharedPtr<FeatureResourceManager> featureResourceManager = editorModule->GetFeatureResourceManager();
    bool const secretStored = featureResourceManager->CheckSecretExists(secretId).Result == GameKit::GAMEKIT_SUCCESS;

    return secretStored;
}

#undef LOCTEXT_NAMESPACE
