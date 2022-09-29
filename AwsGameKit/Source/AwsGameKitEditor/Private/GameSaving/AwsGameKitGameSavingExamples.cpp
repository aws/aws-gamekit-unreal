// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "GameSaving/AwsGameKitGameSavingExamples.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitStyleSet.h"
#include "FeatureResourceManager.h"
#include "Core/AwsGameKitErrors.h"
#include "Identity/AwsGameKitIdentity.h"
#include "Models/AwsGameKitGameSavingModels.h"
#include "Utils/Blueprints/UAwsGameKitFileUtils.h"

// Unreal
#include "Modules/ModuleManager.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitGameSavingExamples"

#define result_row(label, text) \
    + SVerticalBox::Slot() \
    .AutoHeight() \
    [ \
        SNew(SHorizontalBox) \
        + SHorizontalBox::Slot() \
        .Padding(25, 5, 5, 5) \
        .FillWidth(1) \
        [ \
           SNew(STextBlock) \
           .Text(FText::FromString(label)) \
        ] \
        + SHorizontalBox::Slot() \
        .Padding(5) \
        .FillWidth(3) \
        [ \
           SNew(SEditableTextBox) \
           .IsReadOnly(true) \
           .Text(FText::FromString(text)) \
        ] \
    ] \

InitializationStatus AAwsGameKitGameSavingExamples::gameSavingInitializationStatus = InitializationStatus::NOT_STARTED;
FGameSavingSlots AAwsGameKitGameSavingExamples::cachedSlotsCopy = FGameSavingSlots();

/**
 * Set the default values for this actor's properties.
 */
AAwsGameKitGameSavingExamples::AAwsGameKitGameSavingExamples()
{
    savePopoutOpen = false;
    loadPopoutOpen = false;
    gameSavingInitializationStatus = InitializationStatus::NOT_STARTED;
}

void AAwsGameKitGameSavingExamples::Destroyed()
{
    if (savePopoutOpen)
    {
        SaveSlotWindow.Get()->RequestDestroyWindow();
    }

    if (loadPopoutOpen)
    {
        LoadSlotWindow.Get()->RequestDestroyWindow();
    }

    Super::Destroyed();
}

bool AAwsGameKitGameSavingExamples::IsEditorOnly() const
{
    return true;
}

bool AAwsGameKitGameSavingExamples::IsIdentityDeployed() const
{
    return IsFeatureDeployed(FeatureType::Identity, "Identity/Authentication");
}

bool AAwsGameKitGameSavingExamples::IsGameSavingDeployed() const
{
    return IsFeatureDeployed(FeatureType::GameStateCloudSaving, "Game Saving");
}

bool AAwsGameKitGameSavingExamples::IsFeatureDeployed(FeatureType featureType, FString featureName) const
{
    /*
     * This check is only meant for the examples.
     * This is to ensure the feature has been deployed before running any of the sample code.
     */
    if (!ReloadSettings(featureType))
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("This example requires an AWS GameKit backend service for " + featureName + "."
            " See Edit > Project Settings > Plugins > AWS GameKit to create the " + featureName + " backend."));
        return false;
    }

    return true;
}

bool AAwsGameKitGameSavingExamples::ReloadSettings(FeatureType featureType) const
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    if (runtimeModule->AreFeatureSettingsLoaded(featureType))
    {
        return true;
    }

    /*************
    // In order to call ReloadConfigFile() outside of AwsGameKitEditor module, use the lines below:

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();
    sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
    return runtimeModule->AreFeatureSettingsLoaded(FeatureType::GameStateCloudSaving);
    *************/

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    return runtimeModule->ReloadConfigFile(editorModule->GetFeatureResourceManager()->GetClientConfigSubdirectory());
}

// This initializer is called from the Details Panel APIs.
void AAwsGameKitGameSavingExamples::InitializeGameSavingLibrary(const OnButtonClickFromDetailsPanel postInitCallback, FString* postInitStatusCode)
{
    const TFunction<void()> postInitCallbackWrapper = [this, postInitCallback]()
    {
        (this->* (postInitCallback))();
    };

    InitializeGameSavingLibrary(postInitCallbackWrapper, postInitStatusCode);
}

// This initializer is called from the Popout Window APIs.
void AAwsGameKitGameSavingExamples::InitializeGameSavingLibrary(const OnButtonClickFromPopupWindow postInitCallback, FString* postInitStatusCode)
{
    const TFunction<void()> postInitCallbackWrapper = [this, postInitCallback]()
    {
        (this->* (postInitCallback))();
    };

    InitializeGameSavingLibrary(postInitCallbackWrapper, postInitStatusCode);
}

/**
 * This function must be called exactly once before using any of the Game Saving APIs in this example.
 *
 * The Game Saving library needs to be initialized exactly once by calling AddLocalSlots() followed by GetAllSlotSyncStatuses().
 * It's important to call these two APIs first (before any other Game Saving APIs) and to call them in this order.
 * It's also important to only call AddLocalSlots() once, regardless of how many separate Actors use Game Saving in a single game session.
 *
 * AddLocalSlots() ensures Game Saving knows about local saves on the device that exist from previous times the game was played.
 *
 * GetAllSlotSyncStatuses() ensures Game Saving has the latest information about the cloud saves, knows which local saves are synchronized
 * with the cloud, and which saves should be uploaded, downloaded, or need manual conflict resolution.
 *
 * @param postInitCallback The function to execute after the Game Saving library has finished initializing. This is the Game Saving API the user clicked.
 * @param postInitStatusCode The status code to report the initialization statuses to. This should correspond to the Game Saving API the user clicked.
 */
void AAwsGameKitGameSavingExamples::InitializeGameSavingLibrary(const TFunction<void()> postInitCallback, FString* postInitStatusCode)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::InitializeGameSavingLibrary()"));

    if (gameSavingInitializationStatus == InitializationStatus::SUCCESSFUL)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::InitializeGameSavingLibrary() Game Saving is already initialized. Exiting early. Game Saving should only be initialized once."));
        postInitCallback();
        return;
    }

    if (gameSavingInitializationStatus == InitializationStatus::IN_PROGRESS)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::Initialize() Game Saving is already being initialized. Exiting early."));
        *postInitStatusCode = "Try again after initialization is complete.";
        return;
    }

    gameSavingInitializationStatus = InitializationStatus::IN_PROGRESS;

    // Record which function the user originally clicked. We'll call it back after initialization is complete:
    postInitializationCallback = postInitCallback;
    postInitializationStatusCode = postInitStatusCode;

    // Let user know the Game Saving library is being initialized, because initialization is asynchronous & long-running:
    *postInitStatusCode = "Initializing Game Saving library - Adding local saves and syncing status with cloud ...";

    // Begin actual initialization:

    /*
     * Find all SaveInfo.json files on the device.
     *
     * In this example, the SaveInfo.json files are saved to UAwsGameKitFileUtils::GetFeatureSaveDirectory().
     *
     * In your own game, you should store each SaveInfo.json file alongside its corresponding save file.
     * This will help other developers and players to understand both files go together.
     */
    FFilePaths saveInfoFilePaths;
    const FString searchDirectory = UAwsGameKitFileUtils::GetFeatureSaveDirectory(FeatureType_E::GameStateCloudSaving);
    const FString fileExtension = AwsGameKitGameSaving::GetSaveInfoFileExtension();
    UAwsGameKitFileUtils::GetFilesInDirectory(saveInfoFilePaths, searchDirectory, fileExtension);

    // Call AddLocalSlots():
    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitGameSavingExamples::OnAddLocalSlotsComplete);
    AwsGameKitGameSaving::AddLocalSlots(saveInfoFilePaths, ResultDelegate);

    // GetAllSlotSyncStatuses() is called in the ResultDelegate for AddLocalSlots()
    // because it should be called *after* AddLocalSlots() has completed.
}

void AAwsGameKitGameSavingExamples::OnAddLocalSlotsComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::OnAddLocalSlotsComplete()"));

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        // Update UI with the error message:
        *postInitializationStatusCode = GetResultMessage(result.Result);

        gameSavingInitializationStatus = InitializationStatus::FAILED;
        return;
    }

    // Call GetAllSlotSyncStatuses():
    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitGameSavingExamples::OnGetAllSlotSyncStatusesForInitializationComplete);
    AwsGameKitGameSaving::GetAllSlotSyncStatuses(ResultDelegate);
}

void AAwsGameKitGameSavingExamples::OnGetAllSlotSyncStatusesForInitializationComplete(const IntResult& result, const TArray<FGameSavingSlot>& cachedSlots)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::OnGetAllSlotSyncStatusesForInitializationComplete()"));

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        // Update UI with the error message:
        *postInitializationStatusCode = GetResultMessage(result.Result);

        gameSavingInitializationStatus = InitializationStatus::FAILED;
        return;
    }

    gameSavingInitializationStatus = InitializationStatus::SUCCESSFUL;

    // Copy the cached slots:
    // This is an optimization for the LoadSlot() API. See OnLoadGameButtonClicked() for details.
    cachedSlotsCopy = FGameSavingSlots();
    cachedSlotsCopy.Slots = cachedSlots;

    /*
     * Check for sync conflicts:
     *
     * In your own game, when you call GetAllSlotSyncStatuses() at the game's startup,
     * you would want to loop through the "cachedSlots" parameter and look for any slots with SlotSyncStatus == IN_CONFLICT.
     *
     * If any slots are in conflict, you'd likely want to present this conflict to the player and let them decide which file to keep: the local save or the cloud save.
     * The Blueprint Game Saving example has a UI to demonstrate this.
     */

    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::OnGetAllSlotSyncStatusesForInitializationComplete() Game Saving library successfully initialized."));

    // Call the function the user originally clicked:
    postInitializationCallback();
}

void AAwsGameKitGameSavingExamples::CallLoginApi()
{
    if (!IsIdentityDeployed())
    {
        return;
    }

    // Log the user inputs:
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitIdentity::Login() called with parameters: UserName=%s, Password=<password hidden>"), *LoginUserName);

    // Create the request model:
    const FUserLoginRequest request{
        LoginUserName,
        LoginPassword,
    };

    // Call the API:
    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitGameSavingExamples::OnLoginComplete);
    AwsGameKitIdentity::Login(request, ResultDelegate);

    // Let user know the API was called, because it is an asynchronous & long-running API:
    LoginReturnValue = "Logging in ...";
}

void AAwsGameKitGameSavingExamples::OnLoginComplete(const IntResult& result)
{
    // Update UI with the call status:
    LoginReturnValue = GetResultMessage(result.Result);
}

TSharedPtr<SWindow> AAwsGameKitGameSavingExamples::GetPopoutWindow(const FString& action, bool metadataShown, TSharedPtr<SEditableTextBox>& slotName,
    TSharedPtr<SCheckBox>& overrideBox, TSharedPtr<SEditableTextBox>& filePath, bool savingFile, TSharedPtr<SVerticalBox>& slotSection,
    const FString* statusCode, std::function<FReply()> handler)
{
    const float windowWidth = 600.f;
    const float windowHeight = 650.f;
    const int bigPadding = 25;
    const int titlePadding = 10;
    const int rowPadding = 3;

    const static int LEFT_COL_WIDTH = 1;
    const static int RIGHT_COL_WIDTH = 3;

    const static int ALIGNMENT_FILLER = 1;
    const static int CENTER_ELEMENT_WIDTH = 2;

    filePath = SNew(SEditableTextBox).IsReadOnly(true).HintText(FText::FromString(R"(C:/Saves/SaveNameExample.dat)"));

    return SNew(SWindow)
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(FVector2D(windowWidth, windowHeight))
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        .Title(FText::FromString(action + " Game"))
        [
            // Grey Background (to hide black & grey stripes)
            SNew(SBorder)
            .Padding(titlePadding)
            .BorderImage(AwsGameKitStyleSet::Style->GetBrush("BackgroundGreyBrush"))
            [
                SNew(SScrollBox)
                + SScrollBox::Slot()
                [
                    SNew(SVerticalBox)

                    // Save Name
                    + SVerticalBox::Slot()
                    .Padding(rowPadding)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            [
                            SNew(STextBlock)
                            .Text(LOCTEXT("SaveNameLabel", "Save Name:"))
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SAssignNew(slotName, SEditableTextBox)
                            .HintText(LOCTEXT("SaveNameHintText", "SaveNameExample"))
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .Padding(3,0)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(SSpacer)
                        ]
                        +SHorizontalBox::Slot()
                        .AutoWidth()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .ColorAndOpacity(FLinearColor::Gray)
                            .Text(LOCTEXT("SaveSlotRequirments", "Save name must have 1-512 characters. Valid characters: a-z, A-Z, 0-9, _, - (hyphen)"))
                            .Font(AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8"))
                        ]
                    ]

                    // Metadata
                    + SVerticalBox::Slot()
                    .Padding(rowPadding)
                    .AutoHeight()
                    [

                        SNew(SBox)
                        .Visibility_Lambda([metadataShown]{ return metadataShown ? EVisibility::Visible : EVisibility::Collapsed;})
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(LEFT_COL_WIDTH)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .VAlign(VAlign_Center)
                                [
                                    SNew(STextBlock)
                                    .Text(LOCTEXT("MetadataLabel", "Metadata:"))
                                    .ToolTipText(LOCTEXT("MetaDataToolTipTextBlock", "Any textual data you'd like to associate with this game save."))
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(RIGHT_COL_WIDTH)
                            [
                                SAssignNew(SaveFromFileMetadata, SEditableTextBox)
                                .HintText(LOCTEXT("MetaDataHintText", R"({"DisplayName": "Save Name Example", "TotalPlaytime": "03:45:11"})"))
                                .ToolTipText(LOCTEXT("MetaDataToolTipEditableBox", "Any textual data you'd like to associate with this game save."))
                            ]
                        ]
                    ]

                    // Override
                    + SVerticalBox::Slot()
                    .Padding(rowPadding)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("OverrideLabel", "Override:"))
                                .ToolTipText(savingFile ?
                                    LOCTEXT("OverrideCloudToolTip", "If checked, will upload the local save file even if the cloud save file is newer.") :
                                    LOCTEXT("OverrideLocalToolTip", "If checked, will download the cloud save file even if the local save file is newer.")
                                )
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SAssignNew(overrideBox, SCheckBox)
                        ]
                    ]

                    // File Picker
                    + SVerticalBox::Slot()
                    .Padding(rowPadding)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot()
                            .VAlign(VAlign_Center)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("SaveFileLocation", "Save File Location:"))
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SButton)
                                .Text(LOCTEXT("Browse", "Browse"))
                                .OnClicked_Lambda([this, filePath, savingFile] {
                                    filePath->SetText(FText::FromString(UAwsGameKitFileUtils::PickFile(FString("Pick file."), FString("All Files|*"), savingFile)));
                                    return FReply::Handled();
                                })
                            ]
                            + SHorizontalBox::Slot()
                            .Padding(rowPadding, 0, 0, 0)
                            [
                                filePath.ToSharedRef()
                            ]
                        ]
                    ]

                    // Save/Load (Button)
                    + SVerticalBox::Slot()
                    .Padding(0, bigPadding, 0, 0)
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(ALIGNMENT_FILLER)
                        [
                            SNew(SSpacer)
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(CENTER_ELEMENT_WIDTH)
                        [

                            SNew(SButton)
                            .Text(FText::FromString(action + " Game"))
                            .HAlign(HAlign_Center)
                            .OnClicked_Lambda(handler)
                            .IsEnabled_Lambda([this, savingFile]
                            {
                                if (savingFile)
                                {
                                    return !SaveFromFilePath->GetText().IsEmpty() && !SaveFromFileSlotName->GetText().IsEmpty();
                                }
                                return !LoadToFilePath->GetText().IsEmpty() && !LoadToFileSlotName->GetText().IsEmpty();
                            })
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(ALIGNMENT_FILLER)
                        [
                            SNew(SSpacer)
                        ]
                    ]

                    // Result code
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(ALIGNMENT_FILLER)
                        [
                            SNew(SSpacer)
                        ]
                        + SHorizontalBox::Slot()
                        .Padding(titlePadding)
                        .FillWidth(CENTER_ELEMENT_WIDTH)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(ALIGNMENT_FILLER)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .VAlign(VAlign_Center)
                                [
                                    SNew(STextBlock)
                                    .Text(LOCTEXT("ReturnValue", "Return Value:"))
                                ]
                            ]
                            +SHorizontalBox::Slot()
                            .FillWidth(2)
                            [
                                SNew(SBorder)
                                .BorderImage(AwsGameKitStyleSet::Style->GetBrush("MediumGreyBrush"))
                                [
                                    SNew(STextBlock)
                                    .AutoWrapText(true)
                                    .ToolTipText_Lambda([this, statusCode] {return FText::FromString(*statusCode); })
                                    .Text_Lambda([this, statusCode] {return FText::FromString(*statusCode); })
                                ]
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(ALIGNMENT_FILLER)
                        [
                            SNew(SSpacer)
                        ]
                    ]

                    // Sync error banner
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    [
                        SNew(SBorder)
                        .BorderImage(AwsGameKitStyleSet::Style->GetBrush("ErrorRedBrush"))
                        .HAlign(HAlign_Center)
                        .Padding(titlePadding)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .VAlign(VAlign_Center)
                                [
                                    SNew(SImage)
                                    .Image(AwsGameKitStyleSet::Style->GetBrush("WarningIcon"))
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .Padding(titlePadding, rowPadding, rowPadding, rowPadding)
                            [
                                SNew(STextBlock)
                                .AutoWrapText(true)
                                .Justification(ETextJustify::Left)
                                .Text(savingFile ? 
                                        LOCTEXT("SyncConflictCloudRecent", "Cloud Sync Conflict: We've detected a more recent file already saved to the cloud. If you still want to replace that file with your local one, select the Override checkbox and re-submit.") : 
                                        LOCTEXT("SyncConflictLocalRecent", "Cloud Sync Conflict: We've detected a more recent file already saved locally. If you still want to replace that file with the version saved in the cloud, select the Override checkbox and re-submit.")
                                )
                                    
                            ]
                        ]
                        .Visibility_Lambda([this, statusCode]
                        {
                            const FString badSync = GetResultMessage(GameKit::GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT);
                            if (*statusCode == badSync)
                            {
                                return EVisibility::Visible;
                            }
                            return EVisibility::Collapsed;
                        })
                    ]
                ]

                // Saved/Loaded Slot info
                + SScrollBox::Slot()
                .Padding(rowPadding, bigPadding, rowPadding, rowPadding)
                [
                    SNew(SExpandableArea)
                    .HeaderContent()
                    [
                        SNew(SBox)
                        .Padding(titlePadding)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("ResponseDataGameSaveInfo", "Response data (Game Save Info)"))
                            .Font(AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold11"))
                        ]
                    ]
                    .BodyContent()
                    [
                        SNew(SBox)
                        .Padding(rowPadding)
                        [
                            SAssignNew(slotSection, SVerticalBox)
                        ]
                    ]
                ]
            ]
        ];
}

/**
 * Open popout window for SaveSlot API.
 */
void AAwsGameKitGameSavingExamples::CallSaveApi()
{
    if (savePopoutOpen)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SaveWindowOpenMessage", "You have an open Save Game window already."));
        return;
    }

    auto handler = [this] {
        return this->OnSaveGameButtonClicked();
    };

    SaveSlotWindow = GetPopoutWindow("Save", true, SaveFromFileSlotName, SaveFromFileOverride, SaveFromFilePath, true, SaveSlotSection, &SaveSlotReturnValue, handler);

    SaveSlotWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride::CreateLambda([this](const TSharedRef<SWindow>& window)
    {
        savePopoutOpen = false;
        FSlateApplicationBase::Get().RequestDestroyWindow(window);
    }));

    // Start with empty UI fields:
    SaveSlotReturnValue = "";
    SaveSlotSection->ClearChildren();

    // Adding as child to top level window keeps it from minimizing on dialog popups
    FSlateApplication::Get().AddWindowAsNativeChild(this->SaveSlotWindow.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef());
    savePopoutOpen = true;
}

/**
 * Call SaveSlot API with parameters provided in the UI.
 */
FReply AAwsGameKitGameSavingExamples::OnSaveGameButtonClicked()
{
    /*
     * This method demonstrates how to call the AwsGameKitGameSaving::SaveSlot() API.
     *
     * The SaveSlot() API uploads a byte array to the cloud.
     * The bytes can come from anywhere:
     *  - In this example, the bytes are loaded from the filesystem via the Unreal FFileHelper::LoadFileToArray() method.
     *  - The bytes could also be loaded from the filesystem via an OS-specific or platform dependent method.
     *  - Or the bytes could come directly from memory. They don't need to be loaded from the filesystem at all.
     */

    if (!IsGameSavingDeployed())
    {
        return FReply::Handled();
    }

    if (gameSavingInitializationStatus != InitializationStatus::SUCCESSFUL)
    {
        InitializeGameSavingLibrary(&AAwsGameKitGameSavingExamples::OnSaveGameButtonClicked, &SaveSlotReturnValue);
        return FReply::Handled();
    }

    // Collect user inputs from the UI:
    const FString slotName = SaveFromFileSlotName->GetText().ToString();
    const FString saveFileAbsolutePath = SaveFromFilePath->GetText().ToString();
    const FString userMetadata = SaveFromFileMetadata->GetText().ToString();
    const bool shouldOverrideCloudFile = SaveFromFileOverride->IsChecked();

    // Load the save file into a byte array:
    TArray<uint8> fileContentBytes;
    if (UAwsGameKitFileUtils::LoadFileIntoByteArray(*saveFileAbsolutePath, fileContentBytes) != GameKit::GAMEKIT_SUCCESS)
    {
        FString errorMessage = "ERROR: Unable to read file: " + saveFileAbsolutePath;
        SaveSlotReturnValue = errorMessage;
        UE_LOG(LogAwsGameKit, Error, TEXT("SaveSlot() %s"), *errorMessage);
        return FReply::Handled();
    }

    // Determine the save file's last modified timestamp since epoch:
    int64 lastModifiedEpochTimeMillis;
    if (!UAwsGameKitFileUtils::GetFileLastModifiedTimestamp(*saveFileAbsolutePath, lastModifiedEpochTimeMillis))
    {
        FString errorMessage = "ERROR: Unable to determine last modified timestamp of file: " + saveFileAbsolutePath;
        SaveSlotReturnValue = errorMessage;
        UE_LOG(LogAwsGameKit, Error, TEXT("SaveSlot() %s"), *errorMessage);
        return FReply::Handled();
    }

    // Determine where to save the SaveInfo.json file:
    const FString saveInfoFilepath = GetSaveInfoFilePath(slotName);

    // Create the request model:
    FGameSavingSaveSlotRequest request;
    request.SlotName = slotName;
    request.SaveInfoFilePath = saveInfoFilepath;
    request.Data = fileContentBytes;
    request.Metadata = userMetadata;
    request.EpochTime = lastModifiedEpochTimeMillis;
    request.OverrideSync = shouldOverrideCloudFile;

    // Log the request model:
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::SaveSlot() called with parameters: Request=%s"), *static_cast<FString>(request));

    // Call the API:
    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitGameSavingExamples::OnSaveGameComplete);
    AwsGameKitGameSaving::SaveSlot(request, ResultDelegate);

    // Let user know the API was called, because it is an asynchronous & long-running API:
    SaveSlotReturnValue = "Saving game ...";

    return FReply::Handled();
}

/**
 * Handle response from SaveSlot API by updating popout window with the results returned to this handler.
 */
void AAwsGameKitGameSavingExamples::OnSaveGameComplete(const IntResult& result, const FGameSavingSlotActionResults& slotActionResults)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::OnSaveGameComplete()"));

    // Copy the cached slots:
    cachedSlotsCopy = slotActionResults.Slots;

    // Update UI with the call status and other response data:
    SaveSlotReturnValue = GetResultMessage(result.Result);
    SaveSlotSection->ClearChildren();
    SaveSlotSection->AddSlot()[SlotToResultUI(slotActionResults.ActedOnSlot)];
}

void AAwsGameKitGameSavingExamples::CallLoadApi()
{
    if (loadPopoutOpen)
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("LoadWindowOpenMessage", "You have an open Load Game window already."));
        return;
    }

    auto clickHandler = [this] {
        return OnLoadGameButtonClicked();
    };

    LoadSlotWindow = GetPopoutWindow("Load", false, LoadToFileSlotName, LoadToFileOverride, LoadToFilePath, false, LoadSlotSection, &LoadSlotReturnValue, clickHandler);

    LoadSlotWindow->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride::CreateLambda([this](const TSharedRef<SWindow>& window)
    {
        loadPopoutOpen = false;
        FSlateApplicationBase::Get().RequestDestroyWindow(window);
    }));

    // Start with empty UI fields:
    LoadSlotReturnValue = "";
    LoadSlotSection->ClearChildren();

    // Adding as child to top level window keeps it from minimizing on dialog popups
    FSlateApplication::Get().AddWindowAsNativeChild(this->LoadSlotWindow.ToSharedRef(), FSlateApplication::Get().GetActiveTopLevelWindow().ToSharedRef());
    loadPopoutOpen = true;
}

FReply AAwsGameKitGameSavingExamples::OnLoadGameButtonClicked()
{
    if (!IsGameSavingDeployed())
    {
        return FReply::Handled();
    }

    if (gameSavingInitializationStatus != InitializationStatus::SUCCESSFUL)
    {
        InitializeGameSavingLibrary(&AAwsGameKitGameSavingExamples::OnLoadGameButtonClicked, &LoadSlotReturnValue);
        return FReply::Handled();
    }

    // Collect user inputs from the UI:
    const FString slotName = this->LoadToFileSlotName->GetText().ToString();
    const bool shouldOverrideLocalFile = LoadToFileOverride->IsChecked();

    /*
     * Before calling LoadSlot(), you need to pre-allocate enough bytes to hold the cloud save file.
     *
     * We recommend determining how many bytes are needed by caching the FGameSavingSlots object
     * from the most recent Game Saving API call before calling LoadSlot(). From this cached object, you
     * can get the SizeCloud of the slot you are going to download. Note: the SizeCloud will be incorrect
     * if the cloud save has been updated from another device since the last time this device cached the
     * FGameSavingSlots. In that case, call GetSlotSyncStatus() to get the accurate size.
     *
     * In this example, we cache the FGameSavingSlots object from *every* Game Saving API call because we
     * allow you to test out the Game Saving APIs in any order.
     *
     * Alternative to caching, you can call GetSlotSyncStatus(slotName) to get the size of the cloud file.
     * However, this has extra latency compared to caching the results of the previous Game Saving API call.
     */
    unsigned int cloudFileSize = 0;
    for (const FGameSavingSlot gameSavingSlot : cachedSlotsCopy.Slots)
    {
        if (gameSavingSlot.SlotName == slotName)
        {
            cloudFileSize = gameSavingSlot.SizeCloud;
            break;
        }
    }

    // Determine where to save the SaveInfo.json file:
    const FString saveInfoFilePath = GetSaveInfoFilePath(slotName);

    // Create the request model:
    FGameSavingLoadSlotRequest request;
    request.SlotName = slotName;
    request.SaveInfoFilePath = saveInfoFilePath;
    request.Data.SetNum(cloudFileSize);
    request.OverrideSync = shouldOverrideLocalFile;

    // Log the request model:
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::LoadSlot() called with parameters: Request=%s"), *static_cast<FString>(request));

    // Call the API:
    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitGameSavingExamples::OnLoadGameComplete);
    AwsGameKitGameSaving::LoadSlot(request, ResultDelegate);

    // Let user know the API was called, because it is an asynchronous & long-running API:
    LoadSlotReturnValue = "Loading game ...";

    return FReply::Handled();
}

void AAwsGameKitGameSavingExamples::OnLoadGameComplete(const IntResult& result, const FGameSavingDataResults& dataResults)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::OnLoadGameComplete()"));

    // Copy the cached slots:
    cachedSlotsCopy = dataResults.Slots;

    // Update UI with the call status and other response data:
    LoadSlotReturnValue = GetResultMessage(result.Result);
    LoadSlotSection->ClearChildren();
    LoadSlotSection->AddSlot()[SlotToResultUI(dataResults.ActedOnSlot)];

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        return;
    }

    // Write cloud data to a local file:
    const FString path = LoadToFilePath->GetText().ToString();
    TArray<uint8> bytes(dataResults.Data);
    const int32 writeStatus = UAwsGameKitFileUtils::SaveByteArrayToFile(path, bytes);

    if (writeStatus != GameKit::GAMEKIT_SUCCESS)
    {
        FString errorMessage = "ERROR: Failed to write downloaded file to local file: " + path + ". Check the output log for details.";
        LoadSlotReturnValue = errorMessage;
    }
}

TSharedRef<SVerticalBox> AAwsGameKitGameSavingExamples::SlotToResultUI(const FGameSavingSlot& slot) const
{
    UEnum* type = FindObject<UEnum>(ANY_PACKAGE, TEXT("SlotSyncStatus_E"), true);
    const FString status = type->GetNameStringByValue(static_cast<int64>(slot.SlotSyncStatus));

    TSharedRef<SVerticalBox> ui = SNew(SVerticalBox)
        result_row("Save Name:", slot.SlotName)
        result_row("Metadata Local:", slot.MetadataLocal)
        result_row("Metadata Cloud:", slot.MetadataCloud)
        result_row("Size Local:", FString::Printf(TEXT("%lld"), slot.SizeLocal))
        result_row("Size Cloud:", FString::Printf(TEXT("%lld"), slot.SizeCloud))
        result_row("Last Modified Local:", FString::Printf(TEXT("%lld"), slot.LastModifiedLocal))
        result_row("Last Modified Cloud:", FString::Printf(TEXT("%lld"), slot.LastModifiedCloud))
        result_row("Last Sync:", FString::Printf(TEXT("%lld"), slot.LastSync))
        result_row("Save Sync Status:", status);

    return ui;
}

void AAwsGameKitGameSavingExamples::CallGetAllGameSaveStatusesApi()
{
    if (!IsGameSavingDeployed())
    {
        return;
    }

    if (gameSavingInitializationStatus != InitializationStatus::SUCCESSFUL)
    {
        InitializeGameSavingLibrary(&AAwsGameKitGameSavingExamples::CallGetAllGameSaveStatusesApi, &GetAllSlotSyncStatusesReturnValue);
        return;
    }

    // Log the API call:
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::GetAllSlotSyncStatuses() called with parameters: <no parameters>."));

    // Call the API:
    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitGameSavingExamples::OnGetAllGameSaveStatusesComplete);
    AwsGameKitGameSaving::GetAllSlotSyncStatuses(ResultDelegate);

    // Let user know the API was called, because it is an asynchronous & long-running API:
    GetAllSlotSyncStatusesReturnValue = "Getting statuses ...";
}

void AAwsGameKitGameSavingExamples::OnGetAllGameSaveStatusesComplete(const IntResult& result, const TArray<FGameSavingSlot>& cachedSlots)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::OnGetAllGameSaveStatusesComplete()"));

    // Copy the cached slots:
    cachedSlotsCopy = FGameSavingSlots();
    cachedSlotsCopy.Slots = cachedSlots;

    // Update UI with the call status and other response data:
    GetAllSlotSyncStatusesResponseCachedSlots = cachedSlots;
    GetAllSlotSyncStatusesReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitGameSavingExamples::CallDeleteGameSaveApi()
{
    if (!IsGameSavingDeployed())
    {
        return;
    }

    if (gameSavingInitializationStatus != InitializationStatus::SUCCESSFUL)
    {
        InitializeGameSavingLibrary(&AAwsGameKitGameSavingExamples::CallDeleteGameSaveApi, &DeleteSlotReturnValue);
        return;
    }

    // Collect user inputs from the UI:
    const FString slotName = DeleteSlotSlotName;

    // Create the request model:
    const FGameSavingDeleteSlotRequest request{ DeleteSlotSlotName };

    // Log the request model:
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::DeleteSlot() called with parameters: Request=%s"), *static_cast<FString>(request));

    // Call the API:
    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitGameSavingExamples::OnDeleteGameSaveComplete);
    AwsGameKitGameSaving::DeleteSlot(request, ResultDelegate);

    // Let user know the API was called, because it is an asynchronous & long-running API:
    DeleteSlotReturnValue = "Deleting game ...";
}

void AAwsGameKitGameSavingExamples::OnDeleteGameSaveComplete(const IntResult& result, const FGameSavingSlotActionResults& slotActionResults)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitGameSavingExamples::OnDeleteGameSaveComplete()"));

    // Copy the cached slots:
    cachedSlotsCopy = slotActionResults.Slots;

    // Update UI with the call status and other response data:
    DeleteSlotReturnValue = GetResultMessage(result.Result);
    DeleteSlotResponseCachedSlots = slotActionResults.Slots.Slots;
    DeleteSlotResponseDeletedSlot = slotActionResults.ActedOnSlot;

    /*
     * Delete the local SaveInfo.json file corresponding to the deleted save slot:
     *
     * In your own game, you'll probably want to delete the local save file and corresponding SaveInfo.json file from the device after calling DeleteSlot().
     * If you keep the SaveInfo.json file, then next time the game boots up this library will recommend re-uploading the save file to the cloud when
     * you call GetAllSlotSyncStatuses() or GetSlotSyncStatus().
     *
     * Note: DeleteSlot() doesn't delete any local files from the device. It only deletes data from the cloud and from memory (i.e. the cached slot).
     */
    if (result.Result == GameKit::GAMEKIT_SUCCESS)
    {
        const FString saveInfoAbsolutePath = GetSaveInfoFilePath(slotActionResults.ActedOnSlot.SlotName);
        UAwsGameKitFileUtils::DeleteFile(saveInfoAbsolutePath);
    }
}

/**
 * Determine the absolute path for where to store the SaveInfo.json file corresponding to the provided save slot.
 *
 * In your own game, you should store each SaveInfo.json file alongside its corresponding save file.
 * This will help other developers and players to understand these files go together.
 *
 * Contrary to this advice, this example stores the SaveInfo.json files in a location that has no relation to the save files.
 * This is because the example lets you upload/download files from anywhere on your filesystem and doesn't keep track of those locations.
 */
FString AAwsGameKitGameSavingExamples::GetSaveInfoFilePath(const FString& slotName)
{
    const FString saveFolder = UAwsGameKitFileUtils::GetFeatureSaveDirectory(FeatureType_E::GameStateCloudSaving);
    const FString filename = slotName + AwsGameKitGameSaving::GetSaveInfoFileExtension();
    const FString absolutePath = FPaths::Combine(saveFolder, filename);
    return absolutePath;
}

/**
 * Convert the error code into a readable string.
 */
FString AAwsGameKitGameSavingExamples::GetResultMessage(unsigned int errorCode)
{
    switch(errorCode)
    {
        case GameKit::GAMEKIT_SUCCESS:
            return "GAMEKIT_SUCCESS";
        case GameKit::GAMEKIT_ERROR_NO_ID_TOKEN:
            return "No ID token in session. Please login a user with the Identity feature first.";
        case GameKit::GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME:
            return "The Save Name is malformed. Please check the output log for details.";
        case GameKit::GAMEKIT_ERROR_HTTP_REQUEST_FAILED:
            return "HTTP request failed. Check the output log for details.";
        case GameKit::GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND:
            return "No local cached save found for the SaveName. Please check to make sure the SaveName is spelled correctly.";
        case GameKit::GAMEKIT_ERROR_GAME_SAVING_MAX_CLOUD_SLOTS_EXCEEDED:
            return "Maximum cloud saves exceeded. Must delete a cloud save first.";
        case GameKit::GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT:
            return "Cloud Sync Conflict";
        case GameKit::GAMEKIT_ERROR_GAME_SAVING_CLOUD_SLOT_IS_NEWER:
            return "The cloud save is newer and should be downloaded with Call Load Api.";
        case GameKit::GAMEKIT_ERROR_GAME_SAVING_LOCAL_SLOT_IS_NEWER:
            return "The local save is newer and should be uploaded with Call Save Api.";
        default:
            const FString hexError = GameKit::StatusCodeToHexFStr(errorCode);
            return FString::Printf(TEXT("Error code: %s. Check the output log for details."), *hexError);
    }
}

#undef LOCTEXT_NAMESPACE
