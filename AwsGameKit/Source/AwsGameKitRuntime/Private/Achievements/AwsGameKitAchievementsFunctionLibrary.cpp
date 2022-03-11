// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Achievements//AwsGameKitAchievementsFunctionLibrary.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "Core/AwsGameKitErrors.h"

// Unreal
#include "LatentActions.h"

UAwsGameKitAchievementsFunctionLibrary::UAwsGameKitAchievementsFunctionLibrary(const FObjectInitializer& Initializer)
    : UBlueprintFunctionLibrary(Initializer)
{}

void UAwsGameKitAchievementsFunctionLibrary::GetAchievementIconsBaseUrl(
    UObject* WorldContextObject,
    struct FLatentActionInfo LatentInfo,
    FString& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::GetAchievementIconsBaseUrl()"));

    TAwsGameKitInternalActionStatePtr<FString> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            AchievementsLibrary achievementsLibrary = runtimeModule->GetAchievementsLibrary();

            auto getUrlDispatcher = [&](const char* response)
            {
                UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::GetAchievementIconsBaseUrl() GetUrlDispatcher::Dispatch"));
                State->Results = UTF8_TO_TCHAR(response);
            };

            typedef LambdaDispatcher<decltype(getUrlDispatcher), void, const char*> GetUrlDispatcher;
            IntResult result = IntResult(achievementsLibrary.AchievementsWrapper->GameKitGetAchievementIconsBaseUrl(achievementsLibrary.AchievementsInstanceHandle,
                &getUrlDispatcher,
                GetUrlDispatcher::Dispatch));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitAchievementsFunctionLibrary::ListAchievementsForPlayer(
    UObject* WorldContextObject,
    struct FLatentActionInfo LatentInfo,
    const FListAchievementsRequest& ListAchievementsRequest,
    const FDelegateOnListAchievementsResultReceived OnPartialResults,
    TArray<FAchievement>& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::ListAchievementsForPlayer()"));

    TAwsGameKitInternalActionStatePtr<TArray<FAchievement>> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, ListAchievementsRequest, SuccessOrFailure, Error, Results, OnPartialResults))
    {
        Action->LaunchThreadedWork([ListAchievementsRequest, State]
        {
            TArray<FAchievement> CompletedResult;
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            AchievementsLibrary achievementsLibrary = runtimeModule->GetAchievementsLibrary();

            FAwsGameKitOperationResult operationResult;

            auto listAchievementsDispatcher = [&](const char* response)
            {
                UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::ListAchievementsForPlayer(): ListAchievementsDispatcher::Dispatch"));
                FString data(UTF8_TO_TCHAR(response));
                TArray<FAchievement> output;
                AwsGamekitAchievementsResponseProcessor::GetListOfAchievementsFromResponse(output, data);
                if (output.Num() > 0)
                {
                    if (State->PartialResultsQueue)
                    {
                        State->PartialResultsQueue->Enqueue(output);
                    }
                    CompletedResult.Append(MoveTemp(output));
                }                
            };
            typedef LambdaDispatcher<decltype(listAchievementsDispatcher), void, const char*> ListAchievementsDispatcher;

            IntResult result = IntResult(achievementsLibrary.AchievementsWrapper->GameKitListAchievements(achievementsLibrary.AchievementsInstanceHandle,
                ListAchievementsRequest.PageSize,
                ListAchievementsRequest.WaitForAllPages,
                &listAchievementsDispatcher,
                ListAchievementsDispatcher::Dispatch));

            State->Results = MoveTemp(CompletedResult);
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitAchievementsFunctionLibrary::UpdateAchievementForPlayer(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FUpdateAchievementRequest& UpdateAchievementsRequest,
    FAchievement& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::UpdateAchievementForPlayer()"));

    TAwsGameKitInternalActionStatePtr<FAchievement> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([UpdateAchievementsRequest, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            AchievementsLibrary achievementsLibrary = runtimeModule->GetAchievementsLibrary();

            auto updatedAchievementDispatcher = [&, UpdateAchievementsRequest](const char* response)
            {
                FString updatedAchievementResponse(UTF8_TO_TCHAR(response));
                UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::UpdateAchievementForPlayer() GetUrlDispatcher::Dispatch"));
                State->Results = AwsGamekitAchievementsResponseProcessor::GetAchievementFromJsonResponse(AwsGamekitAchievementsResponseProcessor::UnpackResponseAsJson(updatedAchievementResponse));
            };

            typedef LambdaDispatcher<decltype(updatedAchievementDispatcher), void, const char*> UpdatedAchievementDispatcher;
            IntResult result = IntResult(achievementsLibrary.AchievementsWrapper->GameKitUpdateAchievement(achievementsLibrary.AchievementsInstanceHandle,
                TCHAR_TO_UTF8(*UpdateAchievementsRequest.AchievementId),
                UpdateAchievementsRequest.IncrementBy,
                &updatedAchievementDispatcher,
                UpdatedAchievementDispatcher::Dispatch));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitAchievementsFunctionLibrary::GetAchievementForPlayer(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FString& AchievementId,
    FAchievement& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::GetAchievementForPlayer()"));

    TAwsGameKitInternalActionStatePtr<FAchievement> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([AchievementId, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            AchievementsLibrary achievementsLibrary = runtimeModule->GetAchievementsLibrary();

            auto getAchievementDispatcher = [&](const char* response)
            {
                FString achievementResponse(UTF8_TO_TCHAR(response));
                UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitAchievementsFunctionLibrary::GetAchievementForPlayer() GetAchievementDispatcher::Dispatch"));
                State->Results = AwsGamekitAchievementsResponseProcessor::GetAchievementFromJsonResponse(AwsGamekitAchievementsResponseProcessor::UnpackResponseAsJson(achievementResponse));
            };

            typedef LambdaDispatcher<decltype(getAchievementDispatcher), void, const char*> GetAchievementDispatcher;
            IntResult result = IntResult(achievementsLibrary.AchievementsWrapper->GameKitGetAchievement(achievementsLibrary.AchievementsInstanceHandle,
                TCHAR_TO_UTF8(*AchievementId),
                &getAchievementDispatcher,
                GetAchievementDispatcher::Dispatch));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}
