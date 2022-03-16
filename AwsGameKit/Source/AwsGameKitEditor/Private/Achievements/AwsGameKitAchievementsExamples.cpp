// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Achievements/AwsGameKitAchievementsExamples.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitRuntime.h"
#include "FeatureResourceManager.h"
#include "Achievements/AwsGameKitAchievements.h"
#include "Achievements/AwsGameKitAchievementsAdmin.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/Logging.h"
#include "Identity/AwsGameKitIdentity.h"
#include "SessionManager/AwsGameKitSessionManager.h"

// Unreal
#include "Async/Async.h"
#include "HAL/FileManagerGeneric.h"

void AAwsGameKitAchievementsExamples::BeginDestroy()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitAchievementsExamples::BeginDestroy()"));
    Super::BeginDestroy();
}

bool AAwsGameKitAchievementsExamples::IsEditorOnly() const
{
    return true;
}

bool AAwsGameKitAchievementsExamples::ReloadSettings()
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    if (runtimeModule->AreFeatureSettingsLoaded(FeatureType::Achievements))
    {
        return true;
    }

    /*************
    // In order to call ReloadConfigFile() outside of AwsGameKitEditor module, use the lines below:

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();
    sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
    return runtimeModule->AreFeatureSettingsLoaded(FeatureType::Achievements);
    *************/

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    return runtimeModule->ReloadConfigFile(editorModule->GetFeatureResourceManager()->GetClientConfigSubdirectory());
}

bool AAwsGameKitAchievementsExamples::ReloadIdentitySettings()
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    if (runtimeModule->AreFeatureSettingsLoaded(FeatureType::Identity))
    {
        return true;
    }

    /*************
    // In order to call ReloadConfigFile() outside of AwsGameKitEditor module, use the lines below:

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();
    sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
    return runtimeModule->AreFeatureSettingsLoaded(FeatureType::Identity);
    *************/

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    return runtimeModule->ReloadConfigFile(editorModule->GetFeatureResourceManager()->GetClientConfigSubdirectory());
}

bool AAwsGameKitAchievementsExamples::InitializeIdentityLibrary()
{
    if (!ReloadIdentitySettings())
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("This example requires an AWS GameKit backend service for Identity/Authentication."
            " See Edit > Project Settings > Plugins > AWS GameKit to create the Identity/Authentication backend."));
        return false;
    }

    return true;
}

/**
 * Must be called before using any of the Achievements APIs.
 *
 * Load the DLL and create a GameKitAchievements instance.
 */
bool AAwsGameKitAchievementsExamples::InitializeAchievementsLibrary()
{
    /*
     * This check is only meant for the examples.
     * This is to ensure that the feature has been deployed before running any of the sample code.
     */
    if (!ReloadSettings())
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("This example requires an AWS GameKit backend service for Achievements."
            " See Edit > Project Settings > Plugins > AWS GameKit to create the Achievements backend."));
        return false;
    }

    if(BaseIconUrl.IsEmpty())
    {
        // Retrieve icon base url
        const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitAchievementsExamples::OnGetIconBaseUrlComplete);
        AwsGameKitAchievements::GetAchievementIconBaseUrl(Delegate);

        ImageDownloaderInstance = ImageDownloader::MakeInstance();
    }

    return true;
}

void AAwsGameKitAchievementsExamples::OnGetIconBaseUrlComplete(const IntResult& result, const FString& url)
{
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AAwsGameKitAchievementExamples::OnGetIconBaseUrlComplete(): Could not get base icon url because error: %s"), *result.ErrorMessage);
        BaseIconUrl = "";
        return;
    }
    BaseIconUrl = url;
}

void AAwsGameKitAchievementsExamples::CallLoginApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallLoginApi() called with parameters: UserName=%s, Password=<password hidden>"), *LoginUserName);

    const FUserLoginRequest request{
        LoginUserName,
        LoginPassword,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitAchievementsExamples::OnLoginComplete);
    AwsGameKitIdentity::Login(request, Delegate);
}

void AAwsGameKitAchievementsExamples::OnLoginComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitAchievementExamples::OnLoginComplete()"));
    LoginReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitAchievementsExamples::CallSetTokenApi()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("CallSetTokenApi() called with parameters: IdToken=%s"), *IdTokenValue);
    AwsGameKitSessionManager::SetToken(TokenType_E::IdToken, IdTokenValue);
}

void AAwsGameKitAchievementsExamples::AddSampleData()
{
    if (!InitializeAchievementsLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("AddSampleData() called."));
    AddDataReturnValue = "Adding Sample Data ...";
    const auto Delegate = TAwsGameKitDelegate<const IntResult&>::CreateLambda([this](const IntResult& result) {AddDataReturnValue = GetResultMessage(result.Result);});
    AwsGameKitAchievementsAdmin::AddSampleData(Delegate);
}

void AAwsGameKitAchievementsExamples::DeleteSampleData()
{
    if (!InitializeAchievementsLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("DeleteSampleData() called."));

    DeleteDataReturnValue = "Deleting Sample Data ...";
    const auto Delegate = TAwsGameKitDelegate<const IntResult&>::CreateLambda([this](const IntResult& result){DeleteDataReturnValue = GetResultMessage(result.Result);});
    AwsGameKitAchievementsAdmin::DeleteSampleData(Delegate);
}

void AAwsGameKitAchievementsExamples::CallListAchievementsForPlayerApi()
{
    if (!InitializeAchievementsLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallListAchievementsForPlayerApi() called."));

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitAchievementsExamples::OnListAchievementsComplete);
    AwsGameKitAchievements::ListAchievementsForPlayer(Delegate);
}

void AAwsGameKitAchievementsExamples::OnListAchievementsComplete(const IntResult& result, const TArray<FAchievement>& achievements)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitAchievementsExamples::OnListAchievementsComplete()"));
    
    ListPlayerAchievementsResponse = achievements;
    ListPlayerAchievementsReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitAchievementsExamples::CallGetAchievementForPlayerApi()
{
    if (!InitializeAchievementsLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallGetAchievementForPlayerApi() called with parameter: achievementId=%s"), *GetAchievementId);

    const FGetAchievementRequest id{
        GetAchievementId,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitAchievementsExamples::OnGetAchievementComplete);
    AwsGameKitAchievements::GetAchievementForPlayer(id, Delegate);
}
void AAwsGameKitAchievementsExamples::OnGetAchievementComplete(const IntResult& result, const FAchievement& achievement)
{
    GetAchievementReturnValue = GetResultMessage(result.Result);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AAwsGameKitAchievementExamples::OnGetAchievementComplete(): Could not get achievement because error: %s"), *result.ErrorMessage);
        return;
    }

    GetAchievementResponse = achievement;
}

void AAwsGameKitAchievementsExamples::CallUpdateAchievementForPlayerApi()
{
    if (!InitializeAchievementsLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallUpdateAchievementForPlayerApi() called with parameters: achievementId=%s, incrementBy=%s"), *UpdateAchievementId, *UpdateAchievementIncrement);

    const FUpdateAchievementRequest update{
        UpdateAchievementId,
        FCString::Atoi(*UpdateAchievementIncrement),
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitAchievementsExamples::OnUpdateAchievementComplete);
    AwsGameKitAchievements::UpdateAchievementForPlayer(update, Delegate);
}

void AAwsGameKitAchievementsExamples::OnUpdateAchievementComplete(const IntResult& result, const FAchievement& achievement)
{
    UpdateAchievementReturnValue = GetResultMessage(result.Result);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AAwsGameKitAchievementExamples::OnUpdateAchievementComplete(): Could not update achievement because error: %s"), *result.ErrorMessage);
        return;
    }

    UpdateAchievementResponse = achievement;

    if (!achievement.IsNewlyEarned)
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallUpdateAchievementForPlayerApi() Achievement Earned: %s on %s"), *achievement.Title, *achievement.EarnedAt);

    // When earned make a popup window, and display earned icon image.
    TSharedPtr<GameKitImage> icon;
    TSharedPtr<SButton> close;

    const int32 ACHIEVEMENT_ICON_SIZE = 50;

    TSharedRef<SWindow> earnedPopup = SNew(SWindow)
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(FVector2D(300.f, 115.f))
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .Padding(10)
            .AutoHeight()
            .VAlign(VAlign_Center)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .HAlign(HAlign_Fill)
                .FillWidth(2)
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("You earned achievement: " + achievement.AchievementId))
                    .AutoWrapText(true)
                ]
                + SHorizontalBox::Slot()
                .HAlign(HAlign_Right)
                .FillWidth(1)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .MaxHeight(ACHIEVEMENT_ICON_SIZE)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .MaxWidth(ACHIEVEMENT_ICON_SIZE)
                        [
                            SAssignNew(icon, GameKitImage)
                            .IsEnabled(false)
                        ]
                    ]
                ]
            ]
            + SVerticalBox::Slot()
            .Padding(75, 10, 75, 10)
            .AutoHeight()
            [
                SAssignNew(close, SButton)
                .Text(FText::FromString("Ok"))
                .HAlign(HAlign_Center)
            ]
        ];

    close->SetOnClicked(FOnClicked::CreateLambda([earnedPopup]{earnedPopup->RequestDestroyWindow(); return FReply::Handled();}));

    if (!BaseIconUrl.IsEmpty())
    {
        ImageDownloaderInstance->SetImageFromUrl(BaseIconUrl + achievement.UnlockedIcon, icon, 1);
    }

    FSlateApplication::Get().AddWindow(earnedPopup);
}

FString AAwsGameKitAchievementsExamples::GetResultMessage(unsigned int errorCode)
{
    return errorCode == GameKit::GAMEKIT_SUCCESS ? "GAMEKIT_SUCCESS" : FString("Error code: ") + GameKit::StatusCodeToHexFStr(errorCode) + " Check output log.";
}
