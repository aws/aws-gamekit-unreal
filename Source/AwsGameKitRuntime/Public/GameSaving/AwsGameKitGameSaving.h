// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimePublicHelpers.h"
#include "Core/AwsGameKitErrors.h"
#include "GameSaving/AwsGameKitGameSavingWrapper.h"
#include "Models/AwsGameKitGameSavingModels.h"
#include "Utils/Blueprints/UAwsGameKitFileUtils.h"

// Unreal
#include "CoreMinimal.h"

/**
 * @brief This class provides APIs for storing game save files in the cloud and synchronizing them with local devices.
 *
 * ## Initialization
 * The Game Saving library must be initialized exactly once by calling SetFileActions(), AddLocalSlots(), and GetAllSlotSyncStatuses() in that order.
 * This initialization must be done before calling any other Game Saving APIs, and should
 * only be done once in the lifetime of your program because the Game Saving library internally uses a singleton.
 *
 * - SetFileActions() is optional. It lets you provide custom file I/O in case the default I/O functions provided by Unreal don't support your target platform(s).
 * - AddLocalSlots() ensures Game Saving knows about local saves on the device that exist from previous times the game was played.
 * - GetAllSlotSyncStatuses() ensures Game Saving has the latest information about the cloud saves, knows which local saves are synchronized
 *   with the cloud, and which saves should be uploaded, downloaded, or need manual conflict resolution.
 *
 * ## Save Slots
 * Save files that are uploaded/downloaded/tracked through this API are each associated with a named "save slot" for the player.
 *
 * When you deploy the Game Saving feature, you can configure the maximum number of cloud saves slots to provide each player. This limit can
 * prevent malicious players from storing too much data in the cloud. You can change this limit by doing another deployment through the Plugin UI.
 *
 * ## Slot Information
 * The local and cloud attributes for a save slot are collectively known as "slot information" and are stored in the FGameSavingSlot struct.
 *
 * ## Cached Slots
 * This library maintains a cache of slot information for all slots it interacts with (both locally and in the cloud).
 * The cached slots are updated on every API call, and are also returned in the delegate of most API calls.
 *
 * ## SaveInfo.json Files
 * This library creates "SaveInfo.json" files on the device every time save files are uploaded/downloaded through the SaveSlot() and LoadSlot() APIs.
 *
 * The exact filenames and locations are provided by you. We highly recommended you store the SaveInfo.json files alongside their corresponding
 * save file to help developers and curious players to understand these files go together.
 *
 * The SaveInfo.json files are loaded during game startup by calling AddLocalSlots(). This informs the library about any save files that exist on the
 * device from previous game sessions.
 */
class AWSGAMEKITRUNTIME_API AwsGameKitGameSaving
{
private:
    static GameSavingLibrary GetGameSavingLibraryFromModule();

public:
    /**
     * @brief Asynchronously load slot information for all of the player's local saves on the device.
     *
     * @details This should be the first method you call on the Game Saving library (except optionally SetFileActions()) and
     * you should call it exactly once during the runtime of your game. Afterwards, you should call GetAllSlotSyncStatuses().
     * See the class level documentation for more details on initialization.
     *
     * @details This method loads the SaveInfo.json files that were created on the device during previous game sessions when calling SaveSlot() and LoadSlot().
     * This overwrites any cached slots in memory which have the same slot name as the slots loaded from the SaveInfo.json files.
     *
     * @param LocalSlotInformationFilePaths File paths for all of the player's SaveInfo.json files on the device. These paths are chosen by you when calling SaveSlot() and LoadSlot().
     * @param ResultDelegate The delegate to invoke and return data to when the method has finished. The ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    static void AddLocalSlots(const FFilePaths& LocalSlotInformationFilePaths, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate);

    /**
     * @brief Asynchronously change the file I/O callbacks used by this library.
     *
     * @details If you call this method, it should be the first method called on the library (even before AddLocalSlots()).
     *
     * @details By default, this library uses the DefaultFileActions documented in AwsGameKitGameSavingWrapper.h
     * These use Unreal-provided file I/O methods and may not work on all platforms.
     * Call this method to provide your own file I/O methods which support the necessary platform(s).
     *
     * @param FileActions A struct of callbacks defining how to perform file I/O actions for the running platform.
     * @param ResultDelegate The delegate to invoke and return data to when the method has finished. The ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    static void SetFileActions(const FileActions& FileActions, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate);

    /**
     * @brief Asynchronously get a complete and updated view of the player's save slots (both local and cloud).
     *
     * @details After calling this method, you should inspect the **TArray<FGameSavingSlot> parameter** of the ResultDelegate and
     * take the recommended syncing action according to each slot's FGameSavingSlot::SlotSyncStatus.
     *
     * @details Call this method during initialization (see class level documentation) and any time you suspect the cloud saves may have
     * been updated from another device.
     *
     * @details This method adds cached slots for all cloud saves not currently on the device, updates all cached slots with accurate cloud attributes,
     * and marks the FGameSavingSlot::SlotSyncStatus member of all cached slots with the recommended syncing action you should take.
     *
     * @param ResultDelegate The delegate to invoke and return data to when the method has finished. The **TArray<FGameSavingSlot>
     * parameter** contains all the cached slots if the API call was successful, otherwise it is empty. The ::IntResult parameter is a
     * GameKit status code and indicates the result of the API call. Status codes are defined in AwsGameKitErrors.h. This method's
     * possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    static void GetAllSlotSyncStatuses(TAwsGameKitDelegateParam<const IntResult&, const TArray<FGameSavingSlot>&> ResultDelegate);

    /**
     * @brief Asynchronously get an updated view and recommended syncing action for the player's specific save slot.
     *
     * @details This method updates the specific save slot's cloud attributes and marks the FGameSavingSlot::SlotSyncStatus member with the recommended syncing action you should take.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param ResultDelegate The delegate to invoke and return data to when the method has finished. The ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME: The provided slot name is malformed. Check the logs to see what the required format is.
     * - GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND: The provided slot name was not found in the cached slots. This either means you have a typo in the slot name,
     *                                             or the slot only exists in the cloud and you need to call GetAllSlotSyncStatuses() first before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    static void GetSlotSyncStatus(const FGameSavingGetSlotSyncStatusRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingSlotActionResults&> ResultDelegate);

    /**
     * @brief Asynchronously delete the player's cloud save slot and remove it from the cached slots.
     *
     * @details No local files are deleted from the device. Data is only deleted from the cloud and from memory (the cached slot).
     *
     * @details After calling DeleteSlot(), you'll probably want to delete the local save file and corresponding SaveInfo.json file from the device.
     * If you keep the SaveInfo.json file, then next time the game boots up this library will recommend re-uploading the save file to the cloud when
     * you call GetAllSlotSyncStatuses() or GetSlotSyncStatus().
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param ResultDelegate The delegate to invoke and return data to when the method has finished. The ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME: The provided slot name is malformed. Check the logs to see what the required format is.
     * - GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND: The provided slot name was not found in the cached slots. This either means you have a typo in the slot name,
     *                                             or the slot only exists in the cloud and you need to call GetAllSlotSyncStatuses() first before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    static void DeleteSlot(const FGameSavingDeleteSlotRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingSlotActionResults&> ResultDelegate);

    /**
     * @brief Asynchronously upload a data buffer to the cloud, overwriting the player's cloud slot if it already exists.
     *
     * @details Also write the slot's information to a SaveInfo.json file on the device, and add the slot to the cached slots if it doesn't already exist.
     * This SaveInfo.json file should be passed into AddLocalSlots() when you initialize the Game Saving library in the future.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param ResultDelegate The delegate to invoke and return data to when the method has finished. The ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
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
    static void SaveSlot(const FGameSavingSaveSlotRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingSlotActionResults&> ResultDelegate);

    /**
     * @brief Asynchronously download the player's cloud slot into a local data buffer.
     *
     * @details Also write the slot's information to a SaveInfo.json file on the device.
     * This SaveInfo.json file should be passed into AddLocalSlots() when you initialize the Game Saving library in the future.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param ResultDelegate The delegate to invoke and return data to when the method has finished. The ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
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
    static void LoadSlot(const FGameSavingLoadSlotRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FGameSavingDataResults&> ResultDelegate);

    /**
     * @brief Get the recommended file extension for SaveInfo JSON files.
     *
     * @details This extension can be appended to any filename and retain a clear meaning.
     */
    static FString GetSaveInfoFileExtension();
};
