// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Achievements/AwsGameKitAchievementsAdmin.h"

// GameKit
#include "AwsGameKitEditor.h"
#include "AwsGameKitFeatureControlCenter.h"
#include "AwsGameKitRuntime.h"
#include "EditorState.h"
#include "Achievements/AwsGameKitAchievements.h"
#include "AwsGameKitRuntime/Private/AwsGameKitRuntimeInternalHelpers.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/Logging.h"

// Standard library
#include <vector>

// Unreal
#include "Interfaces/IPluginManager.h"

TSharedPtr<AccountCredentialsCopy> AwsGameKitAchievementsAdmin::accountCredCopy = MakeShareable(new AccountCredentialsCopy());
TSharedPtr<AccountInfoCopy> AwsGameKitAchievementsAdmin::accountInfoCopy = MakeShareable(new AccountInfoCopy());
AchievementsAdminLibrary AwsGameKitAchievementsAdmin::achievementsAdminLibrary;

AchievementsAdminLibrary AwsGameKitAchievementsAdmin::GetAchievementsAdminLibrary()
{
    if (achievementsAdminLibrary.AchievementsAdminWrapper == nullptr)
    {
        const FString pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
        FString pluginRootPath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"));
        pluginRootPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*pluginRootPath);

        achievementsAdminLibrary.AchievementsAdminWrapper = MakeShareable(new AwsGameKitAchievementsAdminWrapper());
        achievementsAdminLibrary.AchievementsAdminWrapper->Initialize();

        FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
        achievementsAdminLibrary.AchievementsInstanceHandle = achievementsAdminLibrary.AchievementsAdminWrapper->GameKitAchievementsInstanceCreateWithSessionManager(runtimeModule->GetSessionManagerInstance(), TCHAR_TO_UTF8(*pluginRootPath), Logging::LogCallBack);
    }

    return achievementsAdminLibrary;
}

bool AwsGameKitAchievementsAdmin::UpdateCredentials()
{
    TSharedPtr<EditorState> editorState = AWSGAMEKIT_EDITOR_STATE();
    if (!editorState->AreCredentialsValid())
    {
        return false;
    }

    auto creds = editorState->GetCredentials();
    accountCredCopy->accessKey = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_ACCESS_KEY].GetCharArray().GetData());
    accountCredCopy->accessSecret = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_ACCESS_SECRET].GetCharArray().GetData());
    accountCredCopy->region = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_REGION].GetCharArray().GetData());

    accountInfoCopy->environment = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_SELECTED_ENVIRONMENT].GetCharArray().GetData());
    accountInfoCopy->accountId = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_ACCOUNT_ID].GetCharArray().GetData());
    accountInfoCopy->gameName = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_SHORT_GAME_NAME].GetCharArray().GetData());
    return true;
}

void AwsGameKitAchievementsAdmin::ListAchievementsForGame(
    const FListAchievementsRequest& ListAchievementsRequest,
    TAwsGameKitDelegateParam<const TArray<AdminAchievement>&> OnResultReceivedDelegate,
    FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    if (!UpdateCredentials())
    {
        return;
    }

    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();

        FGraphEventRef OrderedWorkChain;

        auto listAchievementsDispatcher = [&](const char* response)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievements::ListAchievementsForGame(): ListAchievementsDispatcher::Dispatch"));
            auto data = UTF8_TO_TCHAR(response);
            TArray<AdminAchievement> output;
            AwsGameKitAchievementsAdmin::GetListOfAdminAchievementsFromResponse(output, data);

            if (output.Num() > 0)
            {
                InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnResultReceivedDelegate, MoveTemp(output));
            }
        };
        typedef LambdaDispatcher<decltype(listAchievementsDispatcher), void, const char*> ListAchievementsDispatcher;

        const AccountCredentials ac = accountCredCopy->ToCharPtrView();
        const AccountInfo ai = accountInfoCopy->ToCharPtrView();

        IntResult result(achievementsLibrary.AchievementsAdminWrapper->GameKitAdminListAchievements(
            achievementsLibrary.AchievementsInstanceHandle,
            &ac,
            &ai,
            ListAchievementsRequest.PageSize,
            ListAchievementsRequest.WaitForAllPages,
            &listAchievementsDispatcher,
            ListAchievementsDispatcher::Dispatch));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitAchievementsAdmin::AddAchievementsForGame(const AddAchievementsRequest& AddAchievementsRequest, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    if (!UpdateCredentials())
    {
        return;
    }

    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();
        FGraphEventRef OrderedWorkChain;

        unsigned int numAchievements = AddAchievementsRequest.achievements.Num();

        if (numAchievements == 0)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievementsAdmin::AddAchievementsForGame() No achievements to add."))
            return;
        }

        std::vector<Achievement> achs;

        TArray<std::string> idBuffers;
        idBuffers.SetNum(numAchievements);

        TArray<std::string> titleBuffers;
        titleBuffers.SetNum(numAchievements);

        TArray<std::string> lockedDescBuffers;
        lockedDescBuffers.SetNum(numAchievements);

        TArray<std::string> unlockedDescBuffers;
        unlockedDescBuffers.SetNum(numAchievements);

        TArray<std::string> lockedIconBuffers;
        lockedIconBuffers.SetNum(numAchievements);

        TArray<std::string> unlockedIconBuffers;
        unlockedIconBuffers.SetNum(numAchievements);

        for (unsigned int i = 0; i < numAchievements; i++)
        {
            AdminAchievement targetAchievement = AddAchievementsRequest.achievements[i];

            idBuffers[i] = TCHAR_TO_UTF8(*targetAchievement.achievementId);
            titleBuffers[i] = TCHAR_TO_UTF8(*targetAchievement.title);
            lockedDescBuffers[i] = TCHAR_TO_UTF8(*targetAchievement.lockedDescription);
            unlockedDescBuffers[i] = TCHAR_TO_UTF8(*targetAchievement.unlockedDescription);
            lockedIconBuffers[i] = TCHAR_TO_UTF8(*targetAchievement.lockedIcon);
            unlockedIconBuffers[i] = TCHAR_TO_UTF8(*targetAchievement.unlockedIcon);

            auto requiredAmount = targetAchievement.requiredAmount;
            requiredAmount = requiredAmount <= 0 ? 1 : requiredAmount;

            Achievement a
            {
                idBuffers[i].c_str(),
                titleBuffers[i].c_str(),
                lockedDescBuffers[i].c_str(),
                unlockedDescBuffers[i].c_str(),
                lockedIconBuffers[i].c_str(),
                unlockedIconBuffers[i].c_str(),
                (unsigned int)requiredAmount,
                (unsigned int)targetAchievement.points,
                (unsigned int)targetAchievement.sortOrder,
                targetAchievement.isStateful,
                targetAchievement.isSecret,
                targetAchievement.isHidden
            };
            achs.push_back(a);
        }

        const AccountCredentials ac = accountCredCopy->ToCharPtrView();
        const AccountInfo ai = accountInfoCopy->ToCharPtrView();

        IntResult result(achievementsLibrary.AchievementsAdminWrapper->GameKitAdminAddAchievements(
            achievementsLibrary.AchievementsInstanceHandle,
            &ac,
            &ai,
            achs.data(),
            achs.size()
            ));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result);
    });
}

void AwsGameKitAchievementsAdmin::DeleteAchievementsForGame(const DeleteAchievementsRequest& DeleteAchievementsRequest, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    if (!UpdateCredentials())
    {
        return;
    }

    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();
        FGraphEventRef OrderedWorkChain;

        unsigned int numAchievements = DeleteAchievementsRequest.achievementIdentifiers.Num();

        if (numAchievements == 0)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievementsAdmin::DeleteAchievementsForGame() No achievements to delete."))
            return;
        }

        TArray<std::string> buffers;
        buffers.SetNum(numAchievements);
        TArray<const char*> bufferChrPtrs;
        bufferChrPtrs.SetNum(numAchievements);

        for (unsigned int i = 0; i < numAchievements; i++)
        {
            auto target = DeleteAchievementsRequest.achievementIdentifiers[i];
            buffers[i] = TCHAR_TO_UTF8(*target);
            bufferChrPtrs[i] = buffers[i].c_str();
        }
      
        const AccountCredentials ac = accountCredCopy->ToCharPtrView();
        const AccountInfo ai = accountInfoCopy->ToCharPtrView();

        IntResult result(achievementsLibrary.AchievementsAdminWrapper->GameKitAdminDeleteAchievements(
            achievementsLibrary.AchievementsInstanceHandle,
            &ac,
            &ai,
            bufferChrPtrs.GetData(),
            numAchievements
            ));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result);
    });
}

void AwsGameKitAchievementsAdmin::AddSampleData(TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    const FString pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
    const FString templatePath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"), TEXT("misc"), TEXT("achievements"), TEXT("achievements_template.json"));
    FString message;
    FFileHelper::LoadFileToString(message, *templatePath);

    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(message);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonParsed))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsSampleDataCallableWrapper::AddSampleData() didn't successfully parse json."))
        IntResult result(GameKit::GAMEKIT_ERROR_PARSE_JSON_FAILED);
        ResultDelegate.ExecuteIfBound(result);
        return;
    }

    TArray<TSharedPtr<FJsonValue>> messageAchievements;
    messageAchievements = JsonParsed->GetArrayField("achievements");

    AddAchievementsRequest addStruct;
    for (auto a : messageAchievements)
    {
        AdminAchievement ach{
            a->AsObject()->GetStringField("achievement_id"),
            a->AsObject()->GetStringField("title"),
            a->AsObject()->GetStringField("locked_description"),
            a->AsObject()->GetStringField("unlocked_description"),
            a->AsObject()->GetStringField("locked_icon"),
            a->AsObject()->GetStringField("unlocked_icon"),
            a->AsObject()->GetNumberField("max_value"),
            a->AsObject()->GetNumberField("points"),
            a->AsObject()->GetNumberField("order_number"),
            a->AsObject()->GetBoolField("is_stateful"),
            a->AsObject()->GetBoolField("is_secret"),
            a->AsObject()->GetBoolField("is_hidden"),
        };
        addStruct.achievements.Add(ach);
    }

    AwsGameKitAchievementsAdmin::AddAchievementsForGame(addStruct, ResultDelegate);
}

void AwsGameKitAchievementsAdmin::DeleteSampleData(TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    auto pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
    auto templatePath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"), TEXT("misc"), TEXT("achievements"), TEXT("achievements_template.json"));
    FString message;
    FFileHelper::LoadFileToString(message, *templatePath);

    TSharedPtr<FJsonObject> JsonParsed;
    TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(message);
    if (!FJsonSerializer::Deserialize(JsonReader, JsonParsed))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsSampleDataCallableWrapper::DeleteSampleData() didn't successfully parse json."))
        IntResult result(GameKit::GAMEKIT_ERROR_PARSE_JSON_FAILED);
        ResultDelegate.ExecuteIfBound(result);
        return;
    }

    TArray<TSharedPtr<FJsonValue>> messageAchievements;
    messageAchievements = JsonParsed->GetArrayField("achievements");

    DeleteAchievementsRequest deleteStruct;
    for (const TSharedPtr<FJsonValue>& a : messageAchievements)
    {
        auto id = a->AsObject()->GetStringField("achievement_id");
        deleteStruct.achievementIdentifiers.Add(id);
    }

    AwsGameKitAchievementsAdmin::DeleteAchievementsForGame(deleteStruct, ResultDelegate);
}
