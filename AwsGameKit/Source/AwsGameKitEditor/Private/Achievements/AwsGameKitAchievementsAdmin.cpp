// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Achievements/AwsGameKitAchievementsAdmin.h"

// GameKit
#include "AwsGameKitEditor.h"
#include "AwsGameKitRuntime.h"
#include "Achievements/AwsGameKitAchievements.h"
#include "AwsGameKitRuntime/Private/AwsGameKitRuntimeInternalHelpers.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/Logging.h"

// Standard library
#include <vector>

// Unreal
#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"

AchievementsAdminLibrary AwsGameKitAchievementsAdmin::achievementsAdminLibrary;
TSharedPtr<EditorState> AwsGameKitAchievementsAdmin::editorState;

AchievementsAdminLibrary AwsGameKitAchievementsAdmin::GetAchievementsAdminLibrary()
{
    if (achievementsAdminLibrary.AchievementsAdminWrapper == nullptr)
    {
        achievementsAdminLibrary.AchievementsAdminWrapper = MakeShareable(new AwsGameKitAchievementsAdminWrapper());
        achievementsAdminLibrary.AchievementsAdminWrapper->Initialize();
    }

    if (CredentialsValid(false) && achievementsAdminLibrary.AchievementsInstanceHandle == nullptr)
    {
        const FString pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
        FString cloudResourcesPath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"), TEXT("cloudResources"));
        cloudResourcesPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*cloudResourcesPath);

        FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

        AccountCredentialsCopy accountCreds;
        AccountInfoCopy accountInfo;
        GetCredentialsAndInfo(accountCreds, accountInfo);

        achievementsAdminLibrary.AchievementsInstanceHandle = achievementsAdminLibrary.AchievementsAdminWrapper->GameKitAdminAchievementsInstanceCreateWithSessionManager(
            runtimeModule->GetSessionManagerInstance(),
            TCHAR_TO_UTF8(*cloudResourcesPath),
            GAMEKIT_ACCOUNTCREDENTIALS_CHAR_PTR_VIEW(accountCreds),
            GAMEKIT_ACCOUNTINFO_CHAR_PTR_VIEW(accountInfo),
            FGameKitLogging::LogCallBack);
    }

    return achievementsAdminLibrary;
}

TSharedPtr<EditorState> AwsGameKitAchievementsAdmin::GetEditorState()
{
    if (!editorState.IsValid())
    {
        editorState = AWSGAMEKIT_EDITOR_STATE();
    }
    return editorState;
}

bool AwsGameKitAchievementsAdmin::CredentialsValid(bool loggingEnabled)
{
    if (!GetEditorState()->AreCredentialsValid())
    {
        if (loggingEnabled)
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsAdmin::CredentialsValid(): No valid Aws credentials configured, unable to use admin achievement methods."));
        }
        return false;
    }
    return true;
}

void AwsGameKitAchievementsAdmin::GetCredentialsAndInfo(AccountCredentialsCopy& ac, AccountInfoCopy& ai)
{
    TMap<FString, FString> creds = GetEditorState()->GetCredentials();
    ac.accessKey = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_ACCESS_KEY].GetCharArray().GetData());
    ac.accessSecret = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_ACCESS_SECRET].GetCharArray().GetData());
    ac.region = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_REGION].GetCharArray().GetData());
    ac.accountId = "";

    FString env = creds[EditorState::EDITOR_STATE_SELECTED_ENVIRONMENT];

    ai.environment = GameKit::ResourceEnvironment(env.IsEmpty() ? "dev" : TCHAR_TO_UTF8(*env));
    ai.accountId = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_ACCOUNT_ID].GetCharArray().GetData());
    ai.gameName = TCHAR_TO_UTF8(creds[EditorState::EDITOR_STATE_SHORT_GAME_NAME].GetCharArray().GetData());
    ai.companyName = "";
}

void AwsGameKitAchievementsAdmin::ChangeCredentials()
{
    if (!CredentialsValid())
    {
        return;
    }

    AccountCredentialsCopy accountCreds;
    AccountInfoCopy accountInfo;
    GetCredentialsAndInfo(accountCreds, accountInfo);

    AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();
    achievementsLibrary.AchievementsAdminWrapper->GameKitAdminCredentialsChanged(
        achievementsLibrary.AchievementsInstanceHandle,
        GAMEKIT_ACCOUNTCREDENTIALS_CHAR_PTR_VIEW(accountCreds),
        GAMEKIT_ACCOUNTINFO_CHAR_PTR_VIEW(accountInfo));
}

bool AwsGameKitAchievementsAdmin::IsAchievementIdValid(const FText& achievementId)
{
    AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();
    const char* id = TCHAR_TO_UTF8(*achievementId.ToString());
    return achievementsLibrary.AchievementsAdminWrapper->GameKitIsAchievementIdValid(id);
}

void AwsGameKitAchievementsAdmin::ListAchievementsForGame(
    const FListAchievementsRequest& ListAchievementsRequest,
    TAwsGameKitDelegateParam<const TArray<AdminAchievement>&> OnResultReceivedDelegate,
    FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    if (!CredentialsValid())
    {
        return;
    }

    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();

        FGraphEventRef OrderedWorkChain;

        auto listAchievementsDispatcher = [&](const char* response)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievements::ListAchievementsForGame(): ListAchievementsDispatcher::Dispatch"));
            const FString data = UTF8_TO_TCHAR(response);
            TArray<AdminAchievement> output;
            AwsGameKitAchievementsAdmin::GetListOfAdminAchievementsFromResponse(output, data);

            if (output.Num() > 0)
            {
                InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnResultReceivedDelegate, MoveTemp(output));
            }
        };
        typedef LambdaDispatcher<decltype(listAchievementsDispatcher), void, const char*> ListAchievementsDispatcher;

        IntResult result(achievementsLibrary.AchievementsAdminWrapper->GameKitAdminListAchievements(
            achievementsLibrary.AchievementsInstanceHandle,
            ListAchievementsRequest.PageSize,
            ListAchievementsRequest.WaitForAllPages,
            &listAchievementsDispatcher,
            ListAchievementsDispatcher::Dispatch));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitAchievementsAdmin::AddAchievementsForGame(const AddAchievementsRequest& AddAchievementsRequest, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    if (!CredentialsValid())
    {
        return;
    }

    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();
        FGraphEventRef OrderedWorkChain;

        unsigned int numAchievements = AddAchievementsRequest.achievements.Num();

        std::vector<GameKit::Achievement> achs;

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

            int32 requiredAmount = targetAchievement.requiredAmount;
            requiredAmount = requiredAmount <= 0 ? 1 : requiredAmount;

            GameKit::Achievement a
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

        IntResult result(achievementsLibrary.AchievementsAdminWrapper->GameKitAdminAddAchievements(
            achievementsLibrary.AchievementsInstanceHandle,
            achs.data(),
            achs.size()
            ));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result);
    });
}

void AwsGameKitAchievementsAdmin::DeleteAchievementsForGame(const DeleteAchievementsRequest& DeleteAchievementsRequest, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    if (!CredentialsValid())
    {
        return;
    }

    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsAdminLibrary achievementsLibrary = GetAchievementsAdminLibrary();
        FGraphEventRef OrderedWorkChain;

        const unsigned int numAchievements = DeleteAchievementsRequest.achievementIdentifiers.Num();

        TArray<std::string> buffers;
        buffers.SetNum(numAchievements);
        TArray<const char*> bufferChrPtrs;
        bufferChrPtrs.SetNum(numAchievements);

        for (unsigned int i = 0; i < numAchievements; i++)
        {
            FString target = DeleteAchievementsRequest.achievementIdentifiers[i];
            buffers[i] = TCHAR_TO_UTF8(*target);
            bufferChrPtrs[i] = buffers[i].c_str();
        }

        IntResult result(achievementsLibrary.AchievementsAdminWrapper->GameKitAdminDeleteAchievements(
            achievementsLibrary.AchievementsInstanceHandle,
            bufferChrPtrs.GetData(),
            numAchievements
            ));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result);
    });
}

void AwsGameKitAchievementsAdmin::AddSampleData(TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    const FString pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
    const FString templatePath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"), TEXT("cloudResources"), TEXT("misc"), TEXT("achievements"), TEXT("achievements_template.json"));
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
    for (TSharedPtr<FJsonValue> a : messageAchievements)
    {
        AdminAchievement ach = AwsGameKitAchievementsAdmin::GetAdminAchievementFromJsonResponse(a->AsObject());
        addStruct.achievements.Add(ach);
    }

    AwsGameKitAchievementsAdmin::AddAchievementsForGame(addStruct, ResultDelegate);
}

void AwsGameKitAchievementsAdmin::DeleteSampleData(TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    FString pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
    FString templatePath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"), TEXT("cloudResources"), TEXT("misc"), TEXT("achievements"), TEXT("achievements_template.json"));
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
        FString id = a->AsObject()->GetStringField("achievement_id");
        deleteStruct.achievementIdentifiers.Add(id);
    }

    AwsGameKitAchievementsAdmin::DeleteAchievementsForGame(deleteStruct, ResultDelegate);
}
