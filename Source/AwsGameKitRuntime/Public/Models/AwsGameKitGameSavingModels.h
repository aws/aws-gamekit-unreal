// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

//#include "AwsGameKitCommonModels.h"
#include "GameSaving/AwsGameKitGameSavingWrapper.h"

#include "AwsGameKitGameSavingModels.generated.h"  // Last include (Unreal requirement)

/**
 * The recommended action your game should take in order to keep the local and cloud save file in sync.
 */
UENUM(BlueprintType)
enum class SlotSyncStatus_E : uint8
{
    /**
     * This status should not be possible.
     */
    UNKNOWN = 0 UMETA(DisplayName = "Unknown"),

    /**
     * No action needed.
     *
     * The cloud file and local file are the same. They both have the same last modified timestamp.
     */
    SYNCED = 1 UMETA(DisplayName = "Synced"),

    /**
     * You should call LoadSlot() to download a newer version of this save from the cloud.
     *
     * Either the save file does not exist locally, or
     * the save file exists locally, the cloud file is newer, and the local file has previously been uploaded from this device.
     */
    SHOULD_DOWNLOAD_CLOUD = 2 UMETA(DisplayName = "Should Download from Cloud"),

    /**
     * You should call SaveSlot() to upload the local save file to the cloud.
     *
     * Either the save slot does not exist in the cloud, or
     * the save slot exists in the cloud, the local file is newer, and the last time the cloud save was updated was from this device.
     */
    SHOULD_UPLOAD_LOCAL = 3 UMETA(DisplayName = "Should Upload from Local"),

    /**
     * You should ask the player to select which file they want to keep: the local file or the cloud file.
     *
     * The local file and the cloud file are different, and based on their last modified timestamps it is not clear which file should be kept.
     *
     * This may happen when a player plays on multiple devices, and especially when played in offline mode across multiple devices.
     */
    IN_CONFLICT = 4 UMETA(DisplayName = "Sync Conflict")
};

/**
 * Contains local and cloud information about a cached slot.
 *
 * This is also the data that gets written to the SaveInfo.json files.
 */
USTRUCT(BlueprintType)
struct FGameSavingSlot
{
    GENERATED_BODY()

    /**
     * The slot name matching one of the cached slots.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    FString SlotName;

    /**
     * An arbitrary string you have associated with this save file locally.
     *
     * For example, this could be used to store information you want to display in the UI before you download the save file from the cloud,
     * such as a friendly display name, a user provided description, the total playtime, the percentage of the game completed, etc.
     *
     * The string can be in any format (ex: JSON), fully supporting UTF-8 compliant characters. It is limited to 1410 bytes.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    FString MetadataLocal;

    /**
     * An arbitrary string you have associated with the cloud save file.
     *
     * See FGameSavingSlot::MetadataLocal for details.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    FString MetadataCloud;

    /**
     * The size of the local save file in bytes.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    int64 SizeLocal;

    /**
     * The size of the cloud save file in bytes.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    int64 SizeCloud;

    /**
     * The last time the local save file was modified in epoch milliseconds.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    int64 LastModifiedLocal;

    /**
     * The last time the cloud save file was modified in epoch milliseconds.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    int64 LastModifiedCloud;

    /**
     * The last time the local save file was uploaded from this device or downloaded to this device.
     *
     * This time will be equal to LastModifiedLocal after calling SaveSlot(), and equal to LastModifiedCloud after calling LoadSlot().
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    int64 LastSync;

    /**
     * The recommended action your game should take in order to keep the local and cloud save file in sync.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    SlotSyncStatus_E SlotSyncStatus;

    /**
     * Create an Unreal-friendly FGameSavingSlot from the plain C++ Slot.
     *
     * This method is used internally and you probably won't need to use this.
     */
    static FGameSavingSlot From(const Slot& slot)
    {
        return FGameSavingSlot
        {
            UTF8_TO_TCHAR(slot.slotName),
            UTF8_TO_TCHAR(slot.metadataLocal),
            UTF8_TO_TCHAR(slot.metadataCloud),
            slot.sizeLocal,
            slot.sizeCloud,
            slot.lastModifiedLocal,
            slot.lastModifiedCloud,
            slot.lastSync,
            static_cast<SlotSyncStatus_E>(slot.slotSyncStatus)
        };
    }

    /**
     * Create an Unreal-friendly TArray of FGameSavingSlot from the plain C++ Slot array.
     *
     * This method is used internally and you probably won't need to use this.
     */
    static TArray<FGameSavingSlot> ToArray(const Slot* cachedSlots, unsigned int slotCount)
    {
        TArray<FGameSavingSlot> slots;
        for (unsigned int i = 0; i < slotCount; ++i)
        {
            slots.Add(FGameSavingSlot::From(cachedSlots[i]));
        }

        return slots;
    }

    /**
     * Convert this struct into a human-readable string for the purpose of logging.
     */
    operator FString() const
    {
        const FString formatString = FString(TEXT("FGameSavingSlot(slotName={0}, metadataLocal={1}, metadataCloud={2}, sizeLocal={3}, sizeCloud={4}, lastModifiedLocal={5}, lastModifiedCloud={6}, lastSync={7})"));
        return FString::Format(*formatString, { SlotName, MetadataLocal, MetadataCloud, SizeLocal, SizeCloud, LastModifiedLocal, LastModifiedCloud, LastSync });
    }
};

/**
 * An array of slots, usually returned to your delegate by the Game Saving APIs.
 */
USTRUCT(BlueprintType)
struct FGameSavingSlots
{
    GENERATED_BODY()

    /**
     * An array of slots, usually returned to your delegate by the Game Saving APIs.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    TArray<FGameSavingSlot> Slots;
};

/**
 * This is a response object returned in the delegate of Game Saving APIs which act on a single slot.
 *
 * It contains information about the individual slot that was acted on by the API, the current set of cached slots,
 * and the result of the API call (success or a specific reason for failure).
 */
USTRUCT(BlueprintType)
struct FGameSavingSlotActionResults
{
    GENERATED_BODY()

    /**
     * A copy of the current set of cached slots.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    FGameSavingSlots Slots;

    /**
     * If the API call was successful, then this contains the cached slot that was acted on during the API call. Otherwise it is empty (i.e. a default FGameSavingSlot) and
     * should not be used. This slot may not be valid once this object leaves scope (i.e. when your provided delegate returns).
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    FGameSavingSlot ActedOnSlot;

    /**
     * A GameKit status code which indicates the result of the Game Saving API call.
     *
     * GAMEKIT_SUCCESS on success, else a non-zero value. Consult the API's documentation for details, or consult the AwsGameKitCore/Public/Core/AwsGameKitErrors.h file.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving")
    int32 CallStatus = 0;
};

/**
 * This is a response object returned in the delegate of the LoadSlot() API.
 *
 * It contains the data that was downloaded, information about the downloaded save slot,
 * the current set of cached slots, and the result of the API call (success or a specific reason for failure).
 */
USTRUCT(BlueprintType)
struct FGameSavingDataResults
{
    GENERATED_BODY()

    /**
     * A copy of the current set of cached slots.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    FGameSavingSlots Slots;

    /**
     * If the API call was successful, then this contains the cached slot that was downloaded. Otherwise it is empty (i.e. a default FGameSavingSlot) and
     * should not be used. This slot may not be valid once this object leaves scope (i.e. when your provided delegate returns).
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    FGameSavingSlot ActedOnSlot;

    /**
     * If the API call was successful, then this contains an array of unsigned bytes containing the downloaded save file. Otherwise it is empty.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    TArray<uint8> Data;

    /**
     * A GameKit status code which indicates the result of the Game Saving API call.
     *
     * GAMEKIT_SUCCESS on success, else a non-zero value. Consult the API's documentation for details, or consult the AwsGameKitCore/Public/Core/AwsGameKitErrors.h file.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    int32 CallStatus = 0;
};

#pragma region Request Objects

/**
 * The request object for:
 * - Blueprints: AWS GameKit > Game Saving > Get Slot Sync Status
 * - C++: AwsGameKitGameSaving::GetSlotSyncStatus()
 */
USTRUCT(BlueprintType)
struct FGameSavingGetSlotSyncStatusRequest
{
    GENERATED_BODY()

    /**
     * The name of a cached slot to update.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | GetSlotSyncStatus")
    FString SlotName;

    /**
     * Convert this struct into a human-readable string for the purpose of logging.
     */
    operator FString() const
    {
        const FString formatString = FString(TEXT("FGameSavingGetSlotSyncStatusRequest(SlotName={0})"));
        return FString::Format(*formatString, { SlotName });
    }
};

/**
 * The request object for:
 * - Blueprints: AWS GameKit > Game Saving > Delete Slot
 * - C++: AwsGameKitGameSaving::DeleteSlot()
 */
USTRUCT(BlueprintType)
struct FGameSavingDeleteSlotRequest
{
    GENERATED_BODY()

    /**
     * The name of the cached slot to delete.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | DeleteSlot")
    FString SlotName;

    /**
     * Convert this struct into a human-readable string for the purpose of logging.
     */
    operator FString() const
    {
        const FString formatString = FString(TEXT("FGameSavingDeleteSlotRequest(SlotName={0})"));
        return FString::Format(*formatString, { SlotName });
    }
};

/**
 * The request object for:
 * - Blueprints: AWS GameKit > Game Saving > Save Slot
 * - C++: AwsGameKitGameSaving::SaveSlot()
 */
USTRUCT(BlueprintType)
struct FGameSavingSaveSlotRequest
{
    GENERATED_BODY()

    /**
     * The name of the save slot to upload to the cloud. The name may be new, it does not have to exist in the cached slots.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | SaveSlot")
    FString SlotName;

    /**
     * The absolute path and filename for where to save the SaveInfo.json file.
     *
     * We recommend naming the file with the file extension provided by:
     * - Blueprints: AWS GameKit > Game Saving > Get Save Info File Extension
     * - C++: AwsGameKitGameSaving::GetSaveInfoFileExtension()
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | SaveSlot")
    FString SaveInfoFilePath;

    /**
     * The save file to upload, formatted as an array of unsigned bytes.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving  | SaveSlot")
    TArray<uint8> Data;

    /**
     * (Optional) An arbitrary string you want to associate with the save file.
     *
     * For example, this could be used to store information you want to display in the UI before you download the save file from the cloud,
     * such as a friendly display name, a user provided description, the total playtime, the percentage of the game completed, etc.
     *
     * The string can be in any format (ex: JSON), fully supporting UTF-8 compliant characters. It is limited to 1410 bytes.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | SaveSlot")
    FString Metadata;

    /**
     * (Optional) The millisecond epoch time of when the local save file was last modified in UTC.
     *
     * Defaults to 0. If 0, will use the system's current timestamp as the EpochTime. The default is useful for
     * save files which only exist in memory (i.e. they aren't persisted on the device).
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | SaveSlot")
    int64 EpochTime = 0;

    /**
     * (Optional) If set to true, this method will ignore the SlotSyncStatus and override the cloud/local data.
     *
     * Set this to true when you are resolving a sync conflict.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | SaveSlot")
    bool OverrideSync = false;

    /**
     * Convert this struct into a human-readable string for the purpose of logging.
     */
    operator FString() const
    {
        const FString formatString = FString(TEXT("FGameSavingSaveSlotRequest(SlotName={0}, SaveInfoFilePath={1}, Data=<bytes>, Metadata={2}, EpochTime={3}, OverrideSync={4})"));
        const FString overrideSyncString = OverrideSync ? "true" : "false";
        return FString::Format(*formatString, { SlotName, SaveInfoFilePath, Metadata, EpochTime, overrideSyncString });
    }
};

/**
 * The request object for Game Saving Load Slot (AwsGameKitGameSaving::LoadSlot()).
 */
USTRUCT(BlueprintType)
struct FGameSavingLoadSlotRequest
{
    GENERATED_BODY()

    /**
     * The name of the save slot to download from the cloud. The name must exist in the cached slots.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    FString SlotName;

    /**
     * The absolute path and filename for where to save the SaveInfo.json file.
     *
     * We recommend naming the file with the file extension provided by:
     * - Blueprints: AWS GameKit > Game Saving > Get Save Info File Extension
     * - C++: AwsGameKitGameSaving::GetSaveInfoFileExtension()
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    FString SaveInfoFilePath;

    /**
     * An array of unsigned bytes large enough to contain the save file after downloading from the cloud.
     *
     * We recommend determining how many bytes are needed by caching the FGameSavingSlots object
     * from the most recent Game Saving API call before calling LoadSlot(). From this cached object, you
     * can get the FGameSavingSlot::SizeCloud of the slot you are going to download. Note: the SizeCloud
     * will be incorrect if the cloud save has been updated from another device since the last time this
     * device cached the FGameSavingSlots. In that case, call GetSlotSyncStatus() to get the accurate size.
     *
     * Alternative to caching, you can call GetSlotSyncStatus(slotName) to get the size of the cloud file.
     * However, this has extra latency compared to caching the results of the previous Game Saving API call.
     *
     * In Blueprints, you can allocate bytes in the array by calling the Resize Array node.
     *
     * In C++, you can allocate bytes in the array by calling myGameSavingModel.Data.SetNum(cloudFileSize).
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    TArray<uint8> Data;

    /**
     * (Optional) If set to true, this method will ignore the SlotSyncStatus and override the cloud/local data.
     *
     * Set this to true when you are resolving a sync conflict.
     */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Game Saving | LoadSlot")
    bool OverrideSync = false;

    /**
     * Convert this struct into a human-readable string for the purpose of logging.
     */
    operator FString() const
    {
        const FString formatString = FString(TEXT("FGameSavingLoadSlotRequest(SlotName={0}, SaveInfoFilePath={1}, Data=<bytes>, OverrideSync={3})"));
        const FString overrideSyncString = OverrideSync ? "true" : "false";
        return FString::Format(*formatString, { SlotName, SaveInfoFilePath, overrideSyncString });
    }
};
#pragma endregion

/**
 * Pointers to file I/O methods. Used to pass custom file I/O methods to the Game Saving library via SetFileActions().
 */
struct FGameSavingFileIOCallback
{
    typedef bool(*FileWriteDispatcher)(const FString& filePath, const TArray<uint8>& data);
    typedef bool(*FileReadDispatcher)(const FString& filePath, TArray<uint8>& data);
    typedef int64(*FileSizeDispatcher)(const FString& filePath);

    FileWriteDispatcher FileWrite;
    FileReadDispatcher FileRead;
    FileSizeDispatcher FileSize;
};

/**
 * @brief Used for storing strings while they are being used by the Game Saving low level C API, preventing them from going out scope or being un/re-assigned.
 */
class ModelCache
{
private:
    const std::string slotName;
    const std::string saveInfoFilePath;
    const std::string metadata = "";

    const int64 epochTime = 0;
    const bool overrideSync = false;
    const TArray<uint8> data;

public:
    ModelCache(const FGameSavingSaveSlotRequest& request) :
        slotName(TCHAR_TO_UTF8(ToCStr(request.SlotName))),
        saveInfoFilePath(TCHAR_TO_UTF8(ToCStr(request.SaveInfoFilePath))),
        metadata(TCHAR_TO_UTF8(ToCStr(request.Metadata))),
        epochTime(request.EpochTime),
        overrideSync(request.OverrideSync),
        data(request.Data) {}

    ModelCache(const FGameSavingLoadSlotRequest& request) :
        slotName(TCHAR_TO_UTF8(ToCStr(request.SlotName))),
        saveInfoFilePath(TCHAR_TO_UTF8(ToCStr(request.SaveInfoFilePath))),
        overrideSync(request.OverrideSync),
        data(request.Data) {}

    operator GameSavingModel() const
    {
        return
        {
            slotName.c_str(),
            metadata.c_str(),
            epochTime,
            overrideSync,
            (uint8_t*)data.GetData(),
            (unsigned int)data.Num(),
            saveInfoFilePath.c_str(),
        };
    }
};
