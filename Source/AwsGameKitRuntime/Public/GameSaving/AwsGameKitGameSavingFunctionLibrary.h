// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitRuntime/Public/Utils/Blueprints/UAwsGameKitFileUtils.h>
#include "GameSaving/AwsGameKitGameSavingWrapper.h"
#include "Common/AwsGameKitBlueprintCommon.h"
#include "Models/AwsGameKitGameSavingModels.h"

// Unreal
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"
#include "UObject/NoExportTypes.h"

#include "AwsGameKitGameSavingFunctionLibrary.generated.h" // Last include (Unreal requirement)

/**
 * @brief This class provides Blueprint APIs for storing game save files in the cloud and synchronizing them with local devices.
 *
 * See AwsGameKitGameSaving for details on initialization, class concepts, and terminology.
 */
UCLASS()
class AWSGAMEKITRUNTIME_API UAwsGameKitGameSavingFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Asynchronously load slot information for all of the player's local saves on the device.
     *
     * This should be the first method you call on the Game Saving library and you should call it exactly once during the runtime of your game.
     * Afterwards, you should call GetAllSlotSyncStatuses().
     *
     * This method loads the SaveInfo.json files that were created on the device during previous game sessions when calling SaveSlot() and LoadSlot().
     * This overwrites any cached slots in memory which have the same slot name as the slots loaded from the SaveInfo.json files.
     *
     * @param FilePaths File paths for all of the player's SaveInfo.json files on the device. These paths are chosen by you when calling SaveSlot() and LoadSlot().
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void AddLocalSlots(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FFilePaths& FilePaths,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Asynchronously get a complete and updated view of the player's save slots (both local and cloud).
     *
     * After calling this method, you should inspect the Results array and take the recommended syncing action according to each slot's Slot Sync Status.
     *
     * This is the second and final method you should call during the once-per-runtime initialization of the Game Saving library. AddLocalSlots() is the first method you should call.
     *
     * Also call this method any time you suspect the cloud saves may have been updated from another device.
     *
     * This method adds cached slots for all cloud saves not currently on the device, updates all cached slots with accurate cloud attributes,
     * and marks the Slot Sync Status member of all cached slots with the recommended syncing action you should take.
     *
     * @param Results All the cached slots with updated information, or an empty list if the API call failed.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AWS GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure", AutoCreateRefTerm = "OnPartialResults"))
    static void GetAllSlotSyncStatuses(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        TArray<FGameSavingSlot>& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Asynchronously get an updated view and recommended syncing action for the player's specific save slot.
     *
     * This method updates the specific save slot's cloud attributes and marks the Slot Sync Status member with the recommended syncing action you should take.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Results The updated slot and all the cached slots.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AWS GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME: The provided slot name is malformed. Check the logs to see what the required format is.
     * - GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND: The provided slot name was not found in the cached slots. This either means you have a typo in the slot name,
     *                                             or the slot only exists in the cloud and you need to call GetAllSlotSyncStatuses() first before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void GetSlotSyncStatus(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FGameSavingGetSlotSyncStatusRequest& Request,
        FGameSavingSlotActionResults& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Asynchronously delete the player's cloud save slot and remove it from the cached slots.
     *
     * No local files are deleted from the device. Data is only deleted from the cloud and from memory (the cached slot).
     *
     * After calling DeleteSlot(), you'll probably want to delete the local save file and corresponding SaveInfo.json file from the device.
     * If you keep the SaveInfo.json file, then next time the game boots up this library will recommend re-uploading the save file to the cloud when
     * you call GetAllSlotSyncStatuses() or GetSlotSyncStatus().
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Results The deleted slot and all the cached slots.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AWS GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME: The provided slot name is malformed. Check the logs to see what the required format is.
     * - GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND: The provided slot name was not found in the cached slots. This either means you have a typo in the slot name,
     *                                             or the slot only exists in the cloud and you need to call GetAllSlotSyncStatuses() first before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void DeleteSlot(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FGameSavingDeleteSlotRequest& Request,
        FGameSavingSlotActionResults& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Asynchronously upload a data buffer to the cloud, overwriting the player's cloud slot if it already exists.
     *
     * Also write the slot's information to a SaveInfo.json file on the device, and add the slot to the cached slots if it doesn't already exist.
     * This SaveInfo.json file should be passed into AddLocalSlots() when you initialize the Game Saving library in the future.
     *
     * @param Request A struct containing all required fields for saving local data to the cloud.
     * @param Results The uploaded slot and all the cached slots.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AWS GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME: The provided slot name is malformed. Check the logs to see what the required format is.
     * - GAMEKIT_ERROR_FILE_WRITE_FAILED: The SaveInfo.json file was unable to be written to the device. If using the default file I/O callbacks,
     *                                    check the logs to see the root cause. If the platform is not supported by the default file I/O callbacks,
     *                                    use SetFileActions() to provide your own callbacks. See SetFileActions() for more details.
     * - GAMEKIT_ERROR_GAME_SAVING_MAX_CLOUD_SLOTS_EXCEEDED: The upload was cancelled because it would have caused the player to exceed their "maximum cloud save slots limit". This limit
     *                                                       was configured when you deployed the Game Saving feature and can be changed by doing another deployment through the Plugin UI.
     * - GAMEKIT_ERROR_GAME_SAVING_EXCEEDED_MAX_SIZE: The Metadata member of your Request object is too large. Please see the documentation on FGameSavingSaveSlotRequest::Metadata for details.
     * - GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT: The upload was cancelled to prevent overwriting the player's progress. This most likely indicates the player has played on multiple
     *                                            devices without having their progress properly synced with the cloud at the start and end of their play sessions. We recommend you inform
     *                                            the player of this conflict and present them with a choice - keep the cloud save or keep the local save. Then call SaveSlot() or LoadSlot()
     *                                            with OverrideSync=true to override the cloud/local file.
     * - GAMEKIT_ERROR_GAME_SAVING_CLOUD_SLOT_IS_NEWER: The upload was cancelled because the cloud save file is newer than the file you attempted to upload. Treat this like a
     *                                                  GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT because the local and cloud save might have non-overlapping game progress.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void SaveSlot(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FGameSavingSaveSlotRequest& Request,
        FGameSavingSlotActionResults& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);
    
    /**
     * Asynchronously download the player's cloud slot into a local data buffer.
     *
     * Also write the slot's information to a SaveInfo.json file on the device.
     * This SaveInfo.json file should be passed into AddLocalSlots() when you initialize the Game Saving library in the future.
     *
     * @param Request A struct containing all required fields for loading data from the cloud.
     * @param Results The downloaded slot and all the cached slots.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AWS GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME: The provided slot name is malformed. Check the logs to see what the required format is.
     * - GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND: The provided slot name was not found in the cached slots. This either means you have a typo in the slot name,
     *                                             or the slot only exists in the cloud and you need to call GetAllSlotSyncStatuses() first before calling this method.
     * - GAMEKIT_ERROR_FILE_WRITE_FAILED: The SaveInfo.json file was unable to be written to the device. If using the default file I/O callbacks,
     *                                    check the logs to see the root cause. If the platform is not supported by the default file I/O callbacks,
     *                                    use SetFileActions() to provide your own callbacks. See SetFileActions() for more details.
     * - GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT: The download was cancelled to prevent overwriting the player's progress. This most likely indicates the player has played on multiple
     *                                            devices without having their progress properly synced with the cloud at the start and end of their play sessions. We recommend you inform
     *                                            the player of this conflict and present them with a choice - keep the cloud save or keep the local save. Then call SaveSlot() or LoadSlot()
     *                                            with OverrideSync=true to override the cloud/local file.
     * - GAMEKIT_ERROR_GAME_SAVING_LOCAL_SLOT_IS_NEWER: The download was cancelled because the local save file is newer than the cloud file you attempted to download. Treat this like a
     *                                                  GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT because the local and cloud save might have non-overlapping game progress.
     * - GAMEKIT_ERROR_GAME_SAVING_SLOT_UNKNOWN_SYNC_STATUS: The download was cancelled because the sync status could not be determined. Treat this like a GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT.
     * - GAMEKIT_ERROR_GAME_SAVING_MISSING_SHA: The S3 file is missing a SHA-256 metadata attribute and therefore the validity of the file could not be determined. This should not happen. If it does,
     *                                          this indicates there is a bug in the backend code.
     * - GAMEKIT_ERROR_GAME_SAVING_SLOT_TAMPERED: The SHA-256 hash of the downloaded file does not match the SHA-256 hash of the original file that was uploaded to S3. This indicates the downloaded
     *                                            file was corrupted or tampered with. You should try downloading again to rule out the possibility of random corruption.
     * - GAMEKIT_ERROR_GAME_SAVING_BUFFER_TOO_SMALL: The data buffer you provided in the Request object is not large enough to hold the downloaded S3 file. This likely means a newer version of the
     *                                               cloud file was uploaded from another device since the last time you called GetAllSlotSyncStatuses() or GetSlotSyncStatus() on this device. To resolve,
     *                                               call GetSlotSyncStatus() to get the up-to-date size of the cloud file.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void LoadSlot(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FGameSavingLoadSlotRequest& Request,
        FGameSavingDataResults& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);
    
    /**
     * Convert a millisecond epoch timestamp to a human readable date time.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving")
    static FString EpochToHumanReadable(int64 epochTimeMilliseconds);

    /**
     * Get the recommended file extension for SaveInfo JSON files.
     *
     * This extension can be appended to any filename and retain a clear meaning.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Game Saving")
    static FString GetSaveInfoFileExtension();
};