// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Achievements/AwsGameKitAchievements.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimeInternalHelpers.h"

AchievementsLibrary AwsGameKitAchievements::GetAchievementsLibraryFromModule()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievements::GetAchievementsLibraryFromModule()"));
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    return runtimeModule->GetAchievementsLibrary();
}

void AwsGameKitAchievements::ListAchievementsForPlayer(
    const FListAchievementsRequest& ListAchievementsRequest,
    TAwsGameKitDelegateParam<const TArray<FAchievement>&> OnResultReceivedDelegate,
    FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsLibrary achievementsLibrary = GetAchievementsLibraryFromModule();

        FGraphEventRef OrderedWorkChain;

        auto listAchievementsDispatcher = [&](const char* response)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievements::ListAchievementsForPlayer(): ListAchievementsDispatcher::Dispatch"));
            FString data = UTF8_TO_TCHAR(response);
            TArray<FAchievement> output;
            AwsGamekitAchievementsResponseProcessor::GetListOfAchievementsFromResponse(output, data);
            if (output.Num() > 0)
            {
                InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnResultReceivedDelegate, MoveTemp(output));
            }
        };
        typedef LambdaDispatcher<decltype(listAchievementsDispatcher), void, const char*> ListAchievementsDispatcher;

        IntResult result(achievementsLibrary.AchievementsWrapper->GameKitListAchievements(achievementsLibrary.AchievementsInstanceHandle, ListAchievementsRequest.PageSize, ListAchievementsRequest.WaitForAllPages, &listAchievementsDispatcher, ListAchievementsDispatcher::Dispatch));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitAchievements::GetAchievementForPlayer(
    const FGetAchievementRequest& GetAchievementRequest,
    TAwsGameKitDelegateParam<const IntResult&, const FAchievement&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsLibrary achievementsLibrary = GetAchievementsLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        FAchievement ach;
        auto getAchievementDispatcher = [&](const char* response)
        {
            FString achievementStr = UTF8_TO_TCHAR(response);
            ach = AwsGamekitAchievementsResponseProcessor::GetAchievementFromJsonResponse(AwsGamekitAchievementsResponseProcessor::UnpackResponseAsJson(achievementStr));
        };
        typedef LambdaDispatcher<decltype(getAchievementDispatcher), void, const char*> GetAchievementDispatcher;

        IntResult result(achievementsLibrary.AchievementsWrapper->GameKitGetAchievement(achievementsLibrary.AchievementsInstanceHandle,
            TCHAR_TO_UTF8(*GetAchievementRequest.AchievementId), &getAchievementDispatcher, GetAchievementDispatcher::Dispatch));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, ach);
    });
}

void AwsGameKitAchievements::UpdateAchievementForPlayer(
    const FUpdateAchievementRequest& UpdateAchievementRequest,
    TAwsGameKitDelegateParam<const IntResult&, const FAchievement&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsLibrary achievementsLibrary = GetAchievementsLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        FAchievement ach;
        auto updateAchievementDispatcher = [&](const char* response)
        {
            FString achievementStr = UTF8_TO_TCHAR(response);
            ach = AwsGamekitAchievementsResponseProcessor::GetAchievementFromJsonResponse(AwsGamekitAchievementsResponseProcessor::UnpackResponseAsJson(achievementStr));
        };
        typedef LambdaDispatcher<decltype(updateAchievementDispatcher), void, const char*> UpdateAchievementDispatcher;

        IntResult result(achievementsLibrary.AchievementsWrapper->GameKitUpdateAchievement(achievementsLibrary.AchievementsInstanceHandle,
            TCHAR_TO_UTF8(*UpdateAchievementRequest.AchievementId), UpdateAchievementRequest.IncrementBy,
            &updateAchievementDispatcher, UpdateAchievementDispatcher::Dispatch));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, ach);
    });
}

void AwsGameKitAchievements::GetAchievementIconBaseUrl(
    TAwsGameKitDelegateParam<const IntResult&, const FString&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]() {
        AchievementsLibrary achievementsLibrary = GetAchievementsLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        FString url;
        auto getBaseUrlDispatcher = [&](const char* response)
        {
            url = UTF8_TO_TCHAR(response);
        };
        typedef LambdaDispatcher<decltype(getBaseUrlDispatcher), void, const char*> GetBaseUrlDispatcher;

        IntResult result(achievementsLibrary.AchievementsWrapper->GameKitGetAchievementIconsBaseUrl(achievementsLibrary.AchievementsInstanceHandle, &getBaseUrlDispatcher, GetBaseUrlDispatcher::Dispatch));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, url);
    });
}
