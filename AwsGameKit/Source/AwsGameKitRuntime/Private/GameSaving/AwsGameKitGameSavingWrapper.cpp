// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "GameSaving/AwsGameKitGameSavingWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"
#include "Misc/FileHelper.h"

void AwsGameKitGameSavingWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitGameSavingWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitGameSavingInstanceCreateWithSessionManager, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGameSavingInstanceRelease, loadedDllHandle);

    LOAD_PLUGIN_FUNC(GameKitAddLocalSlots, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSetFileActions, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetAllSlotSyncStatuses, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetSlotSyncStatus, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitDeleteSlot, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSaveSlot, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitLoadSlot, loadedDllHandle);
}

GAMEKIT_GAME_SAVING_INSTANCE_HANDLE AwsGameKitGameSavingWrapper::GameKitGameSavingInstanceCreateWithSessionManager(
    void* sessionManager,
    FuncLogCallback logCb,
    const char** localSlotInformationFilePaths,
    const unsigned int arraySize,
    const FileActions& fileActions)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(GameSaving, GameKitGameSavingInstanceCreateWithSessionManager, nullptr);

    return INVOKE_FUNC(GameKitGameSavingInstanceCreateWithSessionManager, sessionManager, logCb, localSlotInformationFilePaths, arraySize, fileActions);
}

void AwsGameKitGameSavingWrapper::GameKitGameSavingInstanceRelease(GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(GameSaving, GameKitGameSavingInstanceRelease);

    INVOKE_FUNC(GameKitGameSavingInstanceRelease, gameSavingInstance);
}

void AwsGameKitGameSavingWrapper::GameKitAddLocalSlots(GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, const char** localSlotInformationFilePaths, const unsigned int arraySize)
{
    if (funcGameKitAddLocalSlots == nullptr)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AWS GameKit GameKitAddLocalSlots Plugin is null"));
        return;
    }

    INVOKE_FUNC(GameKitAddLocalSlots, gameSavingInstance, localSlotInformationFilePaths, arraySize);
}

void AwsGameKitGameSavingWrapper::GameKitSetFileActions(GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, const FileActions& fileActions)
{
    if (funcGameKitSetFileActions == nullptr)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AWS GameKit GameKitSetFileActions Plugin is null"));
        return;
    }

    INVOKE_FUNC(GameKitSetFileActions, gameSavingInstance, fileActions);
}

unsigned int AwsGameKitGameSavingWrapper::GameKitGetAllSlotSyncStatuses(
    GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
    DISPATCH_RECEIVER_HANDLE receiver,
    FuncGameSavingResponseCallback resultCb,
    bool waitForAllPages,
    unsigned int pageSize)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(GameSaving, GameKitGetAllSlotSyncStatuses, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitGetAllSlotSyncStatuses, gameSavingInstance, receiver, resultCb, waitForAllPages, pageSize);
}

unsigned int AwsGameKitGameSavingWrapper::GameKitGetSlotSyncStatus(
    GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
    DISPATCH_RECEIVER_HANDLE receiver,
    FuncGameSavingSlotActionResponseCallback resultCb,
    const char* slotName)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(GameSaving, GameKitGetSlotSyncStatus, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitGetSlotSyncStatus, gameSavingInstance, receiver, resultCb, slotName);
}

unsigned int AwsGameKitGameSavingWrapper::GameKitDeleteSlot(
    GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
    DISPATCH_RECEIVER_HANDLE receiver,
    FuncGameSavingSlotActionResponseCallback resultCb,
    const char* slotName)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(GameSaving, GameKitDeleteSlot, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitDeleteSlot, gameSavingInstance, receiver, resultCb, slotName);
}

unsigned int AwsGameKitGameSavingWrapper::GameKitSaveSlot(
    GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
    DISPATCH_RECEIVER_HANDLE receiver,
    FuncGameSavingSlotActionResponseCallback resultCb,
    GameSavingModel& model)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(GameSaving, GameKitSaveSlot, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitSaveSlot, gameSavingInstance, receiver, resultCb, model);
}

unsigned int AwsGameKitGameSavingWrapper::GameKitLoadSlot(
    GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
    DISPATCH_RECEIVER_HANDLE receiver,
    FuncGameSavingDataResponseCallback resultCb,
    GameSavingModel& model)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(GameSaving, GameKitLoadSlot, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitLoadSlot, gameSavingInstance, receiver, resultCb, model);
}

DefaultFileActions::DefaultFileActions()
{
    fileWriteCallback = writeFileCallback;
    fileReadCallback = readFileCallback;
    fileSizeCallback = getFileSizeCallback;

    // These dispatch receivers are not used because the callbacks above don't require stateful information.
    // Dispatch receivers are used when a callback needs to invoke an instance method on a stateful object.
    fileWriteDispatchReceiver = nullptr;
    fileReadDispatchReceiver = nullptr;
    fileSizeDispatchReceiver = nullptr;
}

bool DefaultFileActions::writeDesktopFile(const FString& filePath, const TArray<uint8>& data)
{
    if (filePath.IsEmpty())
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("DesktopWriteFile() ERROR: No file path given to load data to."));
        return false;
    }

    if (!FFileHelper::SaveArrayToFile(data, *filePath))
    {
        FString errorMessage = "ERROR: Unable to load data to file: " + filePath;
        UE_LOG(LogAwsGameKit, Error, TEXT("DesktopWriteFile(): %s"), *errorMessage);
        return false;
    }
    return true;
}

bool DefaultFileActions::readDesktopFile(const FString& filePath, TArray<uint8>& data)
{
    if (filePath.IsEmpty())
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("DesktopReadFile() ERROR: No file path given to load data to."));
        return false;
    }

    if (!FFileHelper::LoadFileToArray(data, *filePath))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("DesktopReadFile() ERROR: Unable to read file: %s"), *filePath);
        return false;
    }

    return true;
}

int64 DefaultFileActions::getDesktopFileSize(const FString& filePath)
{
    if (!IFileManager::Get().FileExists(ToCStr(filePath)))
    {
        return 0;
    }
    return IFileManager::Get().FileSize(ToCStr(filePath));
}

bool DefaultFileActions::writeFileCallback(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath, const uint8_t* data, const unsigned int size)
{
    FString filePathFString(UTF8_TO_TCHAR(filePath));
    return writeDesktopFile(filePathFString, TArray<uint8>(data, size));
}

bool DefaultFileActions::readFileCallback(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath, uint8_t* data, unsigned int size)
{
    TArray<uint8> buffer;
    FString filePathFString(UTF8_TO_TCHAR(filePath));
    bool result = readDesktopFile(filePathFString, buffer);
    if (!result || size < (unsigned int)buffer.Num())
    {
        return false;
    }

    memcpy(data, buffer.GetData(), buffer.Num());

    return true;
}

unsigned int DefaultFileActions::getFileSizeCallback(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath)
{
    FString filePathFString(UTF8_TO_TCHAR(filePath));
    return getDesktopFileSize(filePathFString);
}
