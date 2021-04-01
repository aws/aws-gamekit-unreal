// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "GameSaving/AwsGameKitGameSaving.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimeInternalHelpers.h"
#include "AwsGameKitRuntimePublicHelpers.h"

GameSavingLibrary AwsGameKitGameSaving::GetGameSavingLibraryFromModule()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::GetGameSavingLibraryFromModule()"));
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    return runtimeModule->GetGameSavingLibrary();
}

void AwsGameKitGameSaving::AddLocalSlots(const FFilePaths& LocalSlotInformationFilePaths, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::AddLocalSlots()"));

    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        GameSavingLibrary gameSavingLibrary = GetGameSavingLibraryFromModule();

        // Transform local slot information file paths into const char**
        const unsigned int arraySize = LocalSlotInformationFilePaths.FilePaths.Num();
        TArray<std::string> strFilePaths;
        TArray<const char*> rawFilePaths;
        strFilePaths.SetNum(arraySize);
        rawFilePaths.SetNum(arraySize);
        for (unsigned int i = 0; i < arraySize; ++i)
        {
            strFilePaths[i] = TCHAR_TO_UTF8(ToCStr(LocalSlotInformationFilePaths.FilePaths[i]));
            rawFilePaths[i] = strFilePaths[i].c_str();
        }

        gameSavingLibrary.GameSavingWrapper->GameKitAddLocalSlots(gameSavingLibrary.GameSavingInstanceHandle, rawFilePaths.GetData(), arraySize);
        const IntResult result = IntResult(GameKit::GAMEKIT_SUCCESS);

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result);
    });
}

void AwsGameKitGameSaving::SetFileActions(const FileActions& FileActions, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::SetFileActions()"));

    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        GameSavingLibrary gameSavingLibrary = GetGameSavingLibraryFromModule();

        gameSavingLibrary.GameSavingWrapper->GameKitSetFileActions(gameSavingLibrary.GameSavingInstanceHandle, FileActions);
        const IntResult result = IntResult(GameKit::GAMEKIT_SUCCESS);

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result);
    });
}

void AwsGameKitGameSaving::GetAllSlotSyncStatuses( TAwsGameKitDelegateParam<const IntResult&, const TArray<FGameSavingSlot>&> ResultDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::GetAllSlotSyncStatuses()"));

    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        GameSavingLibrary gameSavingLibrary = GetGameSavingLibraryFromModule();

        auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, bool complete, unsigned int callStatus)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::GetAllSlotSyncStatuses() GetAllSlotSyncStatuses::Dispatch"));

            TArray<FGameSavingSlot> results = FGameSavingSlot::ToArray(cachedSlots, slotCount);

            InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, IntResult(callStatus), results);
        };
        typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, bool, unsigned int> Dispatcher;

        const bool shouldWaitForAllPages = true;
        const unsigned int defaultPageSize = GameKit::GameSaving::Wrapper::GetAllSlotSyncStatusesDefaultPageSize;
        gameSavingLibrary.GameSavingWrapper->GameKitGetAllSlotSyncStatuses(gameSavingLibrary.GameSavingInstanceHandle, &dispatcher, Dispatcher::Dispatch, shouldWaitForAllPages, defaultPageSize);
    });
}

void AwsGameKitGameSaving::GetSlotSyncStatus(const FGameSavingGetSlotSyncStatusRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingSlotActionResults&> ResultDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::GetSlotSyncStatus()"));

    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        GameSavingLibrary gameSavingLibrary = GetGameSavingLibraryFromModule();

        auto getSlotSyncStatusDispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot actedOnSlot, unsigned int callStatus)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::GetSlotSyncStatus() GetSlotSyncStatus::Dispatch"));

            FGameSavingSlotActionResults results;
            results.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
            results.ActedOnSlot = FGameSavingSlot::From(actedOnSlot);
            results.CallStatus = callStatus;

            InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, IntResult(callStatus), results);
        };
        typedef LambdaDispatcher<decltype(getSlotSyncStatusDispatcher), void, const Slot*, unsigned int, Slot, unsigned int> GetSlotSyncStatusDispatcher;

        gameSavingLibrary.GameSavingWrapper->GameKitGetSlotSyncStatus(gameSavingLibrary.GameSavingInstanceHandle, &getSlotSyncStatusDispatcher, GetSlotSyncStatusDispatcher::Dispatch, TCHAR_TO_UTF8(*Request.SlotName));
    });
}

void AwsGameKitGameSaving::DeleteSlot(const FGameSavingDeleteSlotRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingSlotActionResults&> ResultDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::DeleteSlot()"));

    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        GameSavingLibrary gameSavingLibrary = GetGameSavingLibraryFromModule();

        auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot actedOnSlot, unsigned int callStatus)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::DeleteSlot() DeleteSlot::Dispatch"));

            FGameSavingSlotActionResults results;
            results.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
            results.ActedOnSlot = FGameSavingSlot::From(actedOnSlot);
            results.CallStatus = callStatus;

            InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, IntResult(callStatus), results);
        };
        typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, Slot, unsigned int> Dispatcher;

        gameSavingLibrary.GameSavingWrapper->GameKitDeleteSlot(gameSavingLibrary.GameSavingInstanceHandle, &dispatcher, Dispatcher::Dispatch, TCHAR_TO_UTF8(*Request.SlotName));
    });
}

void AwsGameKitGameSaving::SaveSlot(const FGameSavingSaveSlotRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingSlotActionResults&> ResultDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::SaveSlot()"));

    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        GameSavingLibrary gameSavingLibrary = GetGameSavingLibraryFromModule();

        auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot actedOnSlot, unsigned int callStatus)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::SaveSlot() SaveSlot::Dispatch"));

            FGameSavingSlotActionResults results;
            results.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
            results.ActedOnSlot = FGameSavingSlot::From(actedOnSlot);
            results.CallStatus = callStatus;

            InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, IntResult(callStatus), results);
        };
        typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, Slot, unsigned int> Dispatcher;

        ModelCache modelCache(Request);
        GameSavingModel gameSavingModel = modelCache;
        gameSavingLibrary.GameSavingWrapper->GameKitSaveSlot(gameSavingLibrary.GameSavingInstanceHandle, &dispatcher, Dispatcher::Dispatch, gameSavingModel);
    });
}

void AwsGameKitGameSaving::LoadSlot(const FGameSavingLoadSlotRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingDataResults&> ResultDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::LoadSlot()"));

    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        GameSavingLibrary gameSavingLibrary = GetGameSavingLibraryFromModule();

        auto dispatcher = [&](const Slot* cachedSlots, unsigned int slotCount, Slot actedOnSlot, const uint8_t* data, unsigned int dataSize, unsigned int callStatus)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSaving::LoadSlot() LoadSlot::Dispatch"));

            FGameSavingDataResults results;
            results.Slots.Slots = FGameSavingSlot::ToArray(cachedSlots, slotCount);
            results.ActedOnSlot = FGameSavingSlot::From(actedOnSlot);
            results.Data.AddUninitialized(dataSize);
            FMemory::Memcpy(results.Data.GetData(), (uint8*)data, dataSize);
            results.CallStatus = callStatus;

            InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, IntResult(callStatus), results);
        };
        typedef LambdaDispatcher<decltype(dispatcher), void, const Slot*, unsigned int, Slot, const uint8_t*, unsigned int, unsigned int> Dispatcher;

        ModelCache modelCache(Request);
        GameSavingModel gameSavingModel = modelCache;
        gameSavingLibrary.GameSavingWrapper->GameKitLoadSlot(gameSavingLibrary.GameSavingInstanceHandle, &dispatcher, Dispatcher::Dispatch, gameSavingModel);
    });
}

FString AwsGameKitGameSaving::GetSaveInfoFileExtension()
{
    return GameKit::GameSaving::Wrapper::SaveInfoFileExtension;
}
