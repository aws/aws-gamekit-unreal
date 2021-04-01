// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
 * @brief Interface for the Game Saving low level C API.
 *
 * @details Most of this code is undocumented because it would be a duplicate of
 * what's already found in AwsGameKitGameSaving and AwsGameKitGameSavingModels.h.
 */

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryWrapper.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryUtils.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>

namespace GameKit
{
    namespace GameSaving
    {
        namespace Wrapper
        {
            static const unsigned int S3PreSignedUrlDefaultTimeToLiveSeconds = 120;

            /**
             * Use this value as the pageSize parameter for AwsGameKitGameSavingWrapper::GameKitGetAllSlotSyncStatuses() to
             * use the default page size that is defined inside the Game Saving DLL.
             */
            static const unsigned int GetAllSlotSyncStatusesDefaultPageSize = 0;

            /**
             * This is the recommended file extension for SaveInfo JSON files.
             *
             * This extension can be appended to any filename and retain a clear meaning.
             */
            static const char* SaveInfoFileExtension = ".SaveInfo.json";
        }
    }
}

/**
 * @brief Pointer to an instance of a GameSaving class created in the imported Game Saving C library.
 *
 * Most GameKit C APIs require an instance handle to be passed in.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit C libraries expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_GAME_SAVING_INSTANCE_HANDLE;

enum class SlotSyncStatus : uint8_t 
{
    UNKNOWN = 0,
    SYNCED = 1,
    SHOULD_DOWNLOAD_CLOUD = 2,
    SHOULD_UPLOAD_LOCAL = 3,
    IN_CONFLICT = 4
};

struct Slot 
{
    const char* slotName;
    const char* metadataLocal = "";
    const char* metadataCloud = "";
    int64_t sizeLocal = 0;
    int64_t sizeCloud = 0;
    int64_t lastModifiedLocal = 0;
    int64_t lastModifiedCloud = 0;
    int64_t lastSync = 0;
    SlotSyncStatus slotSyncStatus = SlotSyncStatus::UNKNOWN;
};

struct GameSavingModel
{
    const char* slotName;
    const char* metadata;
    int64_t epochTime = 0;
    bool overrideSync = false;
    uint8_t* data = nullptr;
    unsigned int dataSize = 0;
    const char* localSlotInformationFilePath = nullptr;
    unsigned int urlTimeToLive = GameKit::GameSaving::Wrapper::S3PreSignedUrlDefaultTimeToLiveSeconds;
    bool consistentRead = true;
};

typedef void(*FuncGameSavingResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const Slot* cachedSlots, unsigned int slotCount, bool complete, unsigned int callStatus);
typedef void(*FuncGameSavingSlotActionResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const Slot* cachedSlots, unsigned int slotCount, Slot slot, unsigned int callStatus);
typedef void(*FuncGameSavingDataResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const Slot* cachedSlots, unsigned int slotCount, Slot slot, const uint8_t* data, unsigned int dataSize, unsigned int callStatus);

/**
 * @brief Save a byte array to a file, overwriting the file if it already exists.
 *
 * @param dispatchReceiver The pointer stored in FileActions::fileWriteDispatchReceiver. The implementer of this function may use this pointer however they find suitable.
 * For example, to point to a class instance on which this callback function can invoke a file-writing instance method.
 * @param filePath The absolute or relative path of the file to write to.
 * @param data The data to write to the file.
 * @param size The length of the `data` array.
 * @return True if the data was successfully written to the file, false otherwise.
 */
typedef bool(*FileWriteCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath, const uint8_t* data, const unsigned int size);

/**
 * @brief Load a file into a byte array.
 *
 * @param dispatchReceiver The pointer stored in FileActions::fileReadDispatchReceiver. The implementer of this function may use this pointer however they find suitable.
 * For example, to point to a class instance on which this callback function can invoke a file-reading instance method.
 * @param filePath The absolute or relative path of the file to read from.
 * @param data The array to store the loaded data in. Must be pre-allocated with enough space to store the entire contents of the file. The caller of
 * this function must call `delete[] data` when finished with the data to prevent a memory leak.
 * @param size The length of the `data` array.
 * @return True if the data was successfully read from the file, false otherwise.
 */
typedef bool(*FileReadCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath, uint8_t* data, unsigned int size);

/**
 * @brief Return the size of the file in bytes, or 0 if the file does not exist.
 *
 * @param dispatchReceiver The pointer stored in FileActions::fileSizeDispatchReceiver. The implementer of this function may use this pointer however they find suitable.
 * For example, to point to a class instance on which this callback function can invoke a size-getting instance method.
 * @param filePath The absolute or relative path of the file to check.
 * @return The file size in bytes, or 0 if the file does not exist.
 */
typedef unsigned int(*FileGetSizeCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath);

/**
 * @brief A bundle of callback functions that provide file I/O for the Game Saving library.
 */
struct FileActions
{
    /**
     * @brief A callback function the Game Saving library will call when it needs to write to a file.
     */
    FileWriteCallback fileWriteCallback;

    /**
     * @brief A callback function the Game Saving library will call when it needs to load a file.
     */
    FileReadCallback fileReadCallback;

    /**
     * @brief A callback function the Game Saving library will call when it needs to get the size of a file.
     */
    FileGetSizeCallback fileSizeCallback;

    /**
     * @brief This pointer will be passed into FileActions::fileWriteCallback() whenever it is invoked.
     *
     * @details The implementer of FileActions::fileWriteCallback() may use this pointer however they find suitable.
     * For example, to point to an instance of a class on which to invoke a file writing instance method.
     */
    DISPATCH_RECEIVER_HANDLE fileWriteDispatchReceiver;

    /**
     * @brief This pointer will be passed into FileActions::fileReadCallback() whenever it is invoked.
     *
     * @details The implementer of FileActions::fileReadCallback() may use this pointer however they find suitable.
     * For example, to point to an instance of a class on which to invoke a file reading instance method.
     */
    DISPATCH_RECEIVER_HANDLE fileReadDispatchReceiver;

    /**
     * @brief This pointer will be passed into FileActions::fileSizeCallback() whenever it is invoked.
     *
     * @details The implementer of FileActions::fileSizeCallback() may use this pointer however they find suitable.
     * For example, to point to an instance of a class on which to invoke a file size-getting instance method.
     */
    DISPATCH_RECEIVER_HANDLE fileSizeDispatchReceiver;
};

/**
 * @brief This class provides the default file I/O methods used by the Game Saving library.
 *
 * @details These use Unreal-provided file I/O methods and may not work on all platforms.
 * Specifically, it uses the Unreal FFileHelper and IFileManager classes.
 *
 * @details You can call AwsGameKitGameSaving::SetFileActions() to provide your own file I/O methods which support the necessary platform(s).
 */
class DefaultFileActions : public FileActions
{
public:
    DefaultFileActions();

private:
    /**
     * @brief Save a byte array to a file, overwriting the file if it already exists.
     *
     * @details Uses the Unreal FFileHelper::SaveArrayToFile() method.
     *
     * @param filePath The absolute or relative path of the file to write to.
     * @param data The data to write to the file.
     * @return True if the data was successfully written to the file, false otherwise.
     */
    static bool writeDesktopFile(const FString& filePath, const TArray<uint8>& data);

    /**
     * @brief Load a file into a byte array, resizing the array as needed, with two uninitialized bytes at the end as padding.
     *
     * @details Uses the Unreal FFileHelper::LoadFileToArray() method.
     *
     * @param filePath The absolute or relative path of the file to read from.
     * @param data The array to store the data in. The array will be re-sized to hold the contents of the file, plus will have two uninitialized bytes at the end as padding.
     * @return True if the data was successfully read from the file, false otherwise.
     */
    static bool readDesktopFile(const FString& filePath, TArray<uint8>& data);

    /**
     * @brief Return the size of the file in bytes, or 0 if the file does not exist.
     *
     * @details Uses the Unreal methods IFileManager::FileExists() and IFileManager::FileSize().
     *
     * @param filePath The absolute or relative path of the file to check.
     * @return The file size in bytes, or 0 if the file does not exist.
     */
    static int64 getDesktopFileSize(const FString& filePath);

    /**
     * A callback function that meets the signature of FileWriteCallback. Internally calls DefaultFileActions::writeDesktopFile().
     */
    static bool writeFileCallback(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath, const uint8_t* data, const unsigned int size);

    /**
     * A callback function that meets the signature of FileReadCallback. Internally calls DefaultFileActions::readDesktopFile().
     */
    static bool readFileCallback(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath, uint8_t* data, unsigned int size);

    /**
     * A callback function that meets the signature of FileGetSizeCallback. Internally calls DefaultFileActions::getDesktopFileSize().
     */
    static unsigned int getFileSizeCallback(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* filePath);
};

/**
 * @brief Wrapper for the Game Saving low level C API.
 *
 * @details See AwsGameKitGameSaving for details. That class is a higher level version of this class and has full documentation.
 */
class AWSGAMEKITRUNTIME_API AwsGameKitGameSavingWrapper : public AwsGameKitLibraryWrapper
{
private:
    DEFINE_FUNC_HANDLE(GAMEKIT_GAME_SAVING_INSTANCE_HANDLE, GameKitGameSavingInstanceCreateWithSessionManager, (
        void* sessionManager, 
        FuncLogCallback logCb, 
        const char** localSlotInformationFilePaths, 
        unsigned int arraySize, 
        FileActions fileActions));
    DEFINE_FUNC_HANDLE(GAMEKIT_GAME_SAVING_INSTANCE_HANDLE, GameKitGameSavingInstanceRelease, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance));

    DEFINE_FUNC_HANDLE(void, GameKitAddLocalSlots, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, const char** localSlotInformationFilePaths, unsigned int arraySize));
    DEFINE_FUNC_HANDLE(void, GameKitSetFileActions, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, FileActions fileActions));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetAllSlotSyncStatuses, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, DISPATCH_RECEIVER_HANDLE receiver, FuncGameSavingResponseCallback resultCb, bool waitForAllPages, unsigned int pageSize));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetSlotSyncStatus, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, DISPATCH_RECEIVER_HANDLE receiver, FuncGameSavingSlotActionResponseCallback resultCb, const char* slotName));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitDeleteSlot, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, DISPATCH_RECEIVER_HANDLE receiver, FuncGameSavingSlotActionResponseCallback resultCb, const char* slotName));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitSaveSlot, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, DISPATCH_RECEIVER_HANDLE receiver, FuncGameSavingSlotActionResponseCallback resultCb, GameSavingModel model));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitLoadSlot, (GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, DISPATCH_RECEIVER_HANDLE receiver, FuncGameSavingDataResponseCallback resultCb, GameSavingModel model));

    virtual std::string getLibraryFilename() override
    {
        return "aws-gamekit-game-saving";
    }

    virtual void importFunctions(void* loadedDllHandle) override;

public:
    GAMEKIT_GAME_SAVING_INSTANCE_HANDLE GameKitGameSavingInstanceCreateWithSessionManager(
        void* sessionManager, 
        FuncLogCallback logCb, 
        const char** localSlotInformationFilePaths,
        const unsigned int arraySize, 
        const FileActions& fileActions);

    void GameKitGameSavingInstanceRelease(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance);

    void GameKitAddLocalSlots(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
        const char** localSlotInformationFilePaths,
        const unsigned int arraySize);

    void GameKitSetFileActions(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
        const FileActions& fileActions);

    unsigned int GameKitGetAllSlotSyncStatuses(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, 
        DISPATCH_RECEIVER_HANDLE receiver,
        FuncGameSavingResponseCallback resultCb,
        bool waitForAllPages, 
        unsigned int pageSize);

    unsigned int GameKitGetSlotSyncStatus(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance, 
        DISPATCH_RECEIVER_HANDLE receiver,
        FuncGameSavingSlotActionResponseCallback resultCb, 
        const char* slotName);

    unsigned int GameKitDeleteSlot(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
        DISPATCH_RECEIVER_HANDLE receiver,
        FuncGameSavingSlotActionResponseCallback resultCb,
        const char* slotName);

    unsigned int GameKitSaveSlot(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
        DISPATCH_RECEIVER_HANDLE receiver,
        FuncGameSavingSlotActionResponseCallback resultCb,
        GameSavingModel& model);

    unsigned int GameKitLoadSlot(
        GAMEKIT_GAME_SAVING_INSTANCE_HANDLE gameSavingInstance,
        DISPATCH_RECEIVER_HANDLE receiver,
        FuncGameSavingDataResponseCallback resultCb,
        GameSavingModel& model);
};
