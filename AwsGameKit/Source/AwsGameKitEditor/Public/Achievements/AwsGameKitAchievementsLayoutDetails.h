// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitAchievementsAdmin.h"
#include "AwsGameKitFeatureLayoutDetails.h"
#include "ImageDownloader.h"

// GameKit forward declarations
class AwsGameKitAchievementUI;
class UAwsGameKitAchievementsAdminCallableWrapper;

// Unreal
#include "Containers/UnrealString.h"
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "Misc/FileHelper.h"
#include "Templates/SharedPointer.h"

// Unreal forward declarations
class FMessageEndpoint;
class SButton;
class SScrollBox;
class STextBlock;
class SVerticalBox;
class SWindow;

class AWSGAMEKITEDITOR_API AwsGameKitAchievementsLayoutDetails : public AwsGameKitFeatureLayoutDetails
{
private:
    TSharedPtr<SWindow> achievementsConfig;
    TSharedPtr<SBorder> syncError;
    UAwsGameKitAchievementsAdminCallableWrapper* gamekitAchievements = nullptr;
    FString achievementIconsBaseUrl;
    FString localStatePath;
    bool configWindowOpen = false;
    bool achievementsDeployed = false;

    // Achievements Data
    TSharedPtr<SButton> addAchievementButton;
    TSharedPtr<SButton> getLatestButton;
    TSharedPtr<SScrollBox> achievementsSection;
    TSharedPtr<STextBlock> saveButtonText;
    TSharedPtr<SButton> saveButton;
    static const FText SAVE_BUTTON_TEXT;

    FReply AddAchievement();
    FReply UploadAchievements();
    FReply GetJsonTemplate();
    FReply ImportJson();
    FReply ExportJson();
    FReply ConfigAchievements();
    FReply GetLatestWindow();

    FString GetLocalAchievementsStateDir();
    void RefreshAchievementIconBaseUrl();
    void ConfigureLocalStatePath();
    void ListAchievements();
    void LoadAchievementsFromJsonFile(const FString& fileName);
    void LoadAchievementsFromJsonFile()
    {
        LoadAchievementsFromJsonFile(this->localStatePath);
    }
    void SetCloudActionButtonState();
    bool IsSaveDataToCloudEnabled() const;
    virtual void CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context) override;

    void OnListAchievementsComplete(const IntResult& result, const TArray<AdminAchievement>& achievements);
    void OnAddAchievementsComplete(const IntResult& result);
    void OnDeleteAchievementsComplete(const IntResult& result);

    void ProcessAchievements(const TArray<AdminAchievement>& incomingAchievements, const bool fromCloud = false);
    void SaveStateToJsonFile(const FString& fileName);
    void SaveStateToJsonFile()
    {
        SaveStateToJsonFile(this->localStatePath);
    }

public:
    AwsGameKitAchievementsLayoutDetails(const FAwsGameKitEditorModule* editorModule);
    ~AwsGameKitAchievementsLayoutDetails();

    /**
    * @brief Creates a new instance.
    * @return A new property type customization.
    */
    static TSharedRef<IDetailCustomization> MakeInstance(const FAwsGameKitEditorModule* editorModule);

    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;

    TMap<FString, TSharedPtr<AwsGameKitAchievementUI>> achievements;
    TMap<FString, TSharedPtr<AwsGameKitAchievementUI>> cloudSyncedAchievements;
    IImageDownloaderPtr imageDownloader;
    TArray<FString> invalidIds;

    int newAchievementCounter = 1;

    void Repopulate();
    void SortAchievements();
};

