// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Standard Library
#include <functional>

// GameKit
#include "GameSaving/AwsGameKitGameSaving.h"
#include "Models/AwsGameKitGameSavingModels.h"

// Unreal
#include "Containers/UnrealString.h"
#include "GameFramework/Actor.h"
#include "Widgets/Layout/SScrollBox.h"

// Unreal forward declarations
class SCheckBox;
class SEditableTextBox;
class SMultiLineEditableTextBox;

#include "AwsGameKitGameSavingExamples.generated.h" // Last include (Unreal requirement)

enum class InitializationStatus
{
    NOT_STARTED,
    IN_PROGRESS,
    FAILED,
    SUCCESSFUL
};

DECLARE_DELEGATE(ZeroParamDelegate);

/**
* This class demonstrates how to call the GameKit Game Saving APIs through C++ code.
*
* This Actor's Details Panel let's you call the GameKit Game Saving APIs against your deployed AWS resources (see AWS Command Center).
*
* The Output Log shows any errors that may occur (example: calling SaveGame() with a SlotName that's malformed).
* Enable the Output Log in Unreal by selecting: Window -> Developer Tools -> Output Log
*
* Copy/paste snippets of the code into your own game in order to integrate your game with GameKit Game Saving.
*/
UCLASS(DisplayName = "AWS GameKit Game State Cloud Saving Examples")
class AWSGAMEKITEDITOR_API AAwsGameKitGameSavingExamples : public AActor
{
    GENERATED_BODY()

public:
    AAwsGameKitGameSavingExamples();
    virtual void Destroyed() override;
    virtual bool IsEditorOnly() const override;

private:
    /**
     * The initialization status of the Game Saving library.
     *
     * This is static because the Game Saving library should only be initialized once in the lifetime of a running game.
     *
     * For example, it should only be initialized once, no matter how many AAwsGameKitGameSavingExamples Actors are created or destroyed.
     */
    static InitializationStatus gameSavingInitializationStatus;

    /**
     * The most recent set of cached slots returned by any Game Saving API.
     *
     * This is an optimization for the LoadSlot() API. See OnLoadGameButtonClicked() for details.
     */
    static FGameSavingSlots cachedSlotsCopy;

    // Initialization
    typedef void (AAwsGameKitGameSavingExamples::* OnButtonClickFromDetailsPanel)(void);
    typedef FReply(AAwsGameKitGameSavingExamples::* OnButtonClickFromPopupWindow)(void);

    void InitializeGameSavingLibrary(const OnButtonClickFromDetailsPanel postInitCallback, FString* postInitStatusCode);
    void InitializeGameSavingLibrary(const OnButtonClickFromPopupWindow postInitCallback, FString* postInitStatusCode);
    void InitializeGameSavingLibrary(const TFunction<void()> postInitCallback, FString* postInitStatusCode);
    void OnAddLocalSlotsComplete(const IntResult& result);
    void OnGetAllSlotSyncStatusesForInitializationComplete(const IntResult& result, const TArray<FGameSavingSlot>& cachedSlots);

    TFunction<void()> postInitializationCallback;
    FString* postInitializationStatusCode;

    bool IsIdentityDeployed() const;
    bool IsGameSavingDeployed() const;
    bool IsFeatureDeployed(FeatureType featureType, FString featureName) const;
    bool ReloadSettings(FeatureType featureType) const;

    // Utility Functions
    static FString GetSaveInfoFilePath(const FString& slotName);
    static FString GetResultMessage(unsigned int errorCode);

    // Popout Window UI
    TSharedRef<SVerticalBox> SlotToResultUI(const FGameSavingSlot& slot) const;
    TSharedPtr<SWindow> GetPopoutWindow(const FString& action, bool metadataShown, TSharedPtr<SEditableTextBox>& slotName,
        TSharedPtr<SCheckBox>& overrideBox, TSharedPtr<SEditableTextBox>& filePath, bool savingFile, TSharedPtr<SVerticalBox>& slotSection,
        const FString* statusCode, std::function<FReply()> handler);

    /*
     * GameKit APIs:
     */

    // Identity Login API
    UFUNCTION(CallInEditor, Category = "1. Login")
    void CallLoginApi();
    void OnLoginComplete(const IntResult& result);

    // Inputs
    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Login", DisplayName = "User Name:")
    FString LoginUserName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Login", DisplayName = "Password:")
    FString LoginPassword;

    // Outputs
    UPROPERTY(Transient, VisibleInstanceOnly, Category = "1. Login", DisplayName = "Return Value:")
    FString LoginReturnValue;

    // GetAllSlotSyncStatuses API
    UFUNCTION(CallInEditor, Category = "2. Get All Game Save Statuses")
    void CallGetAllGameSaveStatusesApi();
    void OnGetAllGameSaveStatusesComplete(const IntResult& result, const TArray<FGameSavingSlot>& cachedSlots);

    // Inputs
    // (none)

    // Outputs
    UPROPERTY(Transient, VisibleInstanceOnly, Category = "2. Get All Game Save Statuses", DisplayName = "Return Value:")
    FString GetAllSlotSyncStatusesReturnValue;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "2. Get All Game Save Statuses", DisplayName = "Response (Cached Saves):")
    TArray<FGameSavingSlot> GetAllSlotSyncStatusesResponseCachedSlots;


    // SaveSlot API
    UFUNCTION(CallInEditor, Category = "3. Save Game")
    void CallSaveApi();
    FReply OnSaveGameButtonClicked();
    void OnSaveGameComplete(const IntResult& result, const FGameSavingSlotActionResults& slotActionResults);

    // Popout Window
    TSharedPtr<SWindow> SaveSlotWindow;
    bool savePopoutOpen;

    // Inputs
    TSharedPtr<SEditableTextBox> SaveFromFileSlotName;
    TSharedPtr<SEditableTextBox> SaveFromFileMetadata;
    TSharedPtr<SCheckBox> SaveFromFileOverride;
    TSharedPtr<SEditableTextBox> SaveFromFilePath;

    // Outputs
    TSharedPtr<SVerticalBox> SaveSlotSection;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "3. Save Game", DisplayName = "Return Value:")
    FString SaveSlotReturnValue;


    // LoadSlot API
    UFUNCTION(CallInEditor, Category = "4. Load Game")
    void CallLoadApi();
    FReply OnLoadGameButtonClicked();
    void OnLoadGameComplete(const IntResult& result, const FGameSavingDataResults& dataResults);

    // Popout Window
    TSharedPtr<SWindow> LoadSlotWindow;
    bool loadPopoutOpen;

    // Inputs
    TSharedPtr<SEditableTextBox> LoadToFileSlotName;
    TSharedPtr<SCheckBox> LoadToFileOverride;
    TSharedPtr<SEditableTextBox> LoadToFilePath;

    // Outputs
    TSharedPtr<SVerticalBox> LoadSlotSection;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "4. Load Game", DisplayName = "Return Value:")
    FString LoadSlotReturnValue;


    // DeleteSlot API
    UFUNCTION(CallInEditor, Category = "5. Delete Game Save")
    void CallDeleteGameSaveApi();
    void OnDeleteGameSaveComplete(const IntResult& result, const FGameSavingSlotActionResults& slotActionResults);

    // Inputs
    UPROPERTY(Transient, EditInstanceOnly, Category = "5. Delete Game Save", DisplayName = "Save Name:")
    FString DeleteSlotSlotName;

    // Outputs
    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. Delete Game Save", DisplayName = "Return Value:")
    FString DeleteSlotReturnValue;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. Delete Game Save", DisplayName = "Response (Deleted Save):")
    FGameSavingSlot DeleteSlotResponseDeletedSlot;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. Delete Game Save", DisplayName = "Response (Cached Saves):")
    TArray<FGameSavingSlot> DeleteSlotResponseCachedSlots;
};
