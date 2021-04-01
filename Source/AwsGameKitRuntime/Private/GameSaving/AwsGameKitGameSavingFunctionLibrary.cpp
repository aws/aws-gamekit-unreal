// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "GameSaving/AwsGameKitGameSavingFunctionLibrary.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "Core/AwsGameKitDispatcher.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/Logging.h"

// Standard library
#include <vector>

// Unreal
#include "Async/Async.h"

/**
 * Macro for dispatcher parameters
 */
#define DISPATCHER &dispatcher, &Dispatcher::Dispatch

void UAwsGameKitGameSavingFunctionLibrary::AddLocalSlots(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FFilePaths& FilePaths,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::GetAllSlotSyncStatuses()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, FilePaths, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([FilePaths, State]
            {
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                GameSavingLibrary gameSavingLibrary = runtimeModule->GetGameSavingLibrary();

                // Transform local slot information file paths into const char**
                const unsigned int arraySize = FilePaths.FilePaths.Num();
                TArray<std::string> strFilePaths;
                TArray<const char*> rawFilePaths;
                strFilePaths.SetNum(arraySize);
                rawFilePaths.SetNum(arraySize);
                for (unsigned int i = 0; i < arraySize; ++i)
                {
                    strFilePaths[i] = TCHAR_TO_UTF8(*FilePaths.FilePaths[i]);
                    rawFilePaths[i] = strFilePaths[i].c_str();
                }
                
                gameSavingLibrary.GameSavingWrapper->GameKitAddLocalSlots(gameSavingLibrary.GameSavingInstanceHandle, rawFilePaths.GetData(), arraySize);
                IntResult result = IntResult(GameKit::GAMEKIT_SUCCESS);
                State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
            });
    }
}

void UAwsGameKitGameSavingFunctionLibrary::GetAllSlotSyncStatuses(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    TArray<FGameSavingSlot>& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::GetAllSlotSyncStatuses()"));

    TAwsGameKitInternalActionStatePtr<TArray<FGameSavingSlot>> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State]
            {
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                GameSavingLibrary gameSavingLibrary = runtimeModule->GetGameSavingLibrary();

                auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, bool complete, unsigned int callStatus)
                {
                    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::GetAllSlotSyncStatuses(): GetAllSlotSyncStatuses::Dispatch"));

                    TArray<FGameSavingSlot> gameSavingResults = FGameSavingSlot::ToArray(cachedSlots, slotCount);

                    State->Results = MoveTemp(gameSavingResults);
                };
                typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, bool, unsigned int> Dispatcher;

                const bool shouldWaitForAllPages = true;
                const unsigned int defaultPageSize = GameKit::GameSaving::Wrapper::GetAllSlotSyncStatusesDefaultPageSize;
                IntResult result = IntResult(gameSavingLibrary.GameSavingWrapper->GameKitGetAllSlotSyncStatuses(gameSavingLibrary.GameSavingInstanceHandle, DISPATCHER, shouldWaitForAllPages, defaultPageSize));
                State->Err = FAwsGameKitOperationResult { static_cast<int>(result.Result), result.ErrorMessage };
            });
    }
}

void UAwsGameKitGameSavingFunctionLibrary::GetSlotSyncStatus(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FGameSavingGetSlotSyncStatusRequest& Request,
    FGameSavingSlotActionResults& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::GetSlotSyncStatus()"));

    TAwsGameKitInternalActionStatePtr<FGameSavingSlotActionResults> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State, Request]
            {
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                GameSavingLibrary gameSavingLibrary = runtimeModule->GetGameSavingLibrary();

                auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot slot, unsigned int callStatus)
                {
                    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::GetSlotSyncStatus() GetSlotSyncStatus::Dispatch"));

                    FGameSavingSlotActionResults gameSavingResults;
                    gameSavingResults.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
                    gameSavingResults.ActedOnSlot = FGameSavingSlot::From(slot);

                    State->Results = gameSavingResults;
                };
                typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, Slot, unsigned int> Dispatcher;

                IntResult result = IntResult(gameSavingLibrary.GameSavingWrapper->GameKitGetSlotSyncStatus(gameSavingLibrary.GameSavingInstanceHandle, DISPATCHER, TCHAR_TO_UTF8(ToCStr(Request.SlotName))));
                State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
            });
    }
}

void UAwsGameKitGameSavingFunctionLibrary::DeleteSlot(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FGameSavingDeleteSlotRequest& Request,
    FGameSavingSlotActionResults& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::DeleteSlot()"));

    TAwsGameKitInternalActionStatePtr<FGameSavingSlotActionResults> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State, Request]
            {
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                GameSavingLibrary gameSavingLibrary = runtimeModule->GetGameSavingLibrary();

                auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot slot, unsigned int callStatus)
                {
                    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::DeleteSlot() DeleteSlot::Dispatch"));

                    FGameSavingSlotActionResults gameSavingResults;
                    gameSavingResults.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
                    gameSavingResults.ActedOnSlot = FGameSavingSlot::From(slot);

                    State->Results = gameSavingResults;
                };
                typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, Slot, unsigned int> Dispatcher;

                IntResult result = IntResult(gameSavingLibrary.GameSavingWrapper->GameKitDeleteSlot(gameSavingLibrary.GameSavingInstanceHandle, DISPATCHER, TCHAR_TO_UTF8(ToCStr(Request.SlotName))));
                State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
            });
    }
}

void UAwsGameKitGameSavingFunctionLibrary::SaveSlot(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FGameSavingSaveSlotRequest& Request,
    FGameSavingSlotActionResults& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::SaveSlot()"));

    TAwsGameKitInternalActionStatePtr<FGameSavingSlotActionResults> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State, Request]
            {
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                GameSavingLibrary gameSavingLibrary = runtimeModule->GetGameSavingLibrary();

                auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot slot, unsigned int callStatus)
                {
                    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::SaveSlot() SaveSlot::Dispatch"));

                    FGameSavingSlotActionResults gameSavingResults;
                    gameSavingResults.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
                    gameSavingResults.ActedOnSlot = FGameSavingSlot::From(slot);

                    State->Results = gameSavingResults;
                };
                typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, Slot, unsigned int> Dispatcher;

                ModelCache modelCache(Request);
                GameSavingModel gameSavingModel = modelCache;

                IntResult result = IntResult(gameSavingLibrary.GameSavingWrapper->GameKitSaveSlot(gameSavingLibrary.GameSavingInstanceHandle, DISPATCHER, gameSavingModel));
                State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
            });
    }
}

void UAwsGameKitGameSavingFunctionLibrary::LoadSlot(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FGameSavingLoadSlotRequest& Request,
    FGameSavingDataResults& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::LoadSlot()"));

    TAwsGameKitInternalActionStatePtr<FGameSavingDataResults> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State, Request]
            {
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                GameSavingLibrary gameSavingLibrary = runtimeModule->GetGameSavingLibrary();

                auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot slot, const uint8_t* data, unsigned int dataSize, unsigned int callStatus) 
                { 
                    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitGameSavingBlueprintFunctionLibrary::LoadSlot() LoadSlot::Dispatch"));

                    FGameSavingDataResults gameSavingResults;
                    gameSavingResults.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
                    gameSavingResults.ActedOnSlot = FGameSavingSlot::From(slot);
                    gameSavingResults.Data.AddUninitialized(dataSize);
                    FMemory::Memcpy(gameSavingResults.Data.GetData(), (uint8*)data, dataSize);

                    State->Results = gameSavingResults;
                }; 
                typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, Slot, const uint8_t*, unsigned int, unsigned int> Dispatcher;

                ModelCache modelCache(Request);
                GameSavingModel gameSavingModel = modelCache;

                IntResult result = IntResult(gameSavingLibrary.GameSavingWrapper->GameKitLoadSlot(gameSavingLibrary.GameSavingInstanceHandle, DISPATCHER, gameSavingModel));
                State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
            });
    }
}

FString UAwsGameKitGameSavingFunctionLibrary::EpochToHumanReadable(int64 epochTime)
{
    // time for a save slot is in epoch milliseconds, FDateTime is expecting seconds
    int64 epochSeconds = epochTime / 1000;

    return FDateTime::FromUnixTimestamp(epochSeconds).ToHttpDate();
}

FString UAwsGameKitGameSavingFunctionLibrary::GetSaveInfoFileExtension()
{
    return GameKit::GameSaving::Wrapper::SaveInfoFileExtension;
}

