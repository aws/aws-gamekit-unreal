// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitRuntimePublicHelpers.h"
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>
#include "UserGameplayData/AwsGameKitUserGameplayDataWrapper.h"
#include "Models/AwsGameKitUserGameplayDataModels.h"

// Unreal
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "Templates/SharedPointer.h"
#include "UObject/NoExportTypes.h"

struct UserGameplayDataLibrary;
class FNetworkStatusChangeDelegate;
class FCacheProcessedDelegate;

/**
 * @brief This class provides APIs for maintaining player game data in the cloud, available when and where the player signs into the game.
 */
class AWSGAMEKITRUNTIME_API AwsGameKitUserGameplayData
{
private:
    static UserGameplayDataLibrary GetUserGameplayDataLibraryFromModule();

public:
    /**
     * @brief Creates a new bundle or updates BundleItems within a specific bundle for the calling user.
     *
     * @param userGameplayDataBundle Struct holding the bundle name, bundleItemKeys, bundleItemValues and number of keys in the bundle being added.
     * @param OnCompleteDelegate Delegate that processes the status code after the AddBundle operation has completed.
     * @param ResultDelegate Delegate that processes the status code and can handle any retry logic for unprocessed bundles on a failure.
     * The ::IntResult (part of the `ResultDelegate` parameter) is a GameKit status code and indicates the result of the API call.
     * The ::FUserGameplayDataBundle (part of the `ResultDelegate` parameter) is a struct that contains any items that were unprocessed on a failure, otherwise the map of the struct will remain empty.
     * The `ResultDelegate` parameter takes an ::IntResult which contains a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name of userGameplayDataBundle is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: At least one of the bundle keys of userGameplayData are malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason. 
    */
    static void AddBundle(const FUserGameplayDataBundle& userGameplayDataBundle, TAwsGameKitDelegateParam<const IntResult&, const FUserGameplayDataBundle&> ResultDelegate);

    /**
     * @brief Applies the settings to the User Gameplay Data Client. Should be called immediately after the instance has been created and before any other API calls.
     *
     * @param clientSettings Configuration for the Gameplay Data API client.
    */
    static void SetClientSettings(const FUserGameplayDataClientSettings& clientSettings);

    /**
     * @brief Lists the bundle name of every bundle that the calling user owns.
     *
     * @param ResultDelegate Delegate that processes the status code and returned list of bundles.
     * The ::IntResult (part of the `ResultDelegate` parameter) is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The response body from the backend could not be parsed successfully
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    static void ListBundles(TAwsGameKitDelegateParam<const IntResult&, const TArray<FString>&> ResultDelegate);

    /**
     * @brief Gets all items that are associated with a certain bundle for the calling user.
     *
     * @param UserGameplayDataBundleName The name of the bundle that is being retrieved.
     * @param ResultDelegate Delegate that processes the status code and returned bundle.
     * The ::IntResult (part of the `ResultDelegate` parameter) is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name of UserGameplayDataBundleName is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The response body from the backend could not be parsed successfully
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    static void GetBundle(const FString& UserGameplayDataBundleName, TAwsGameKitDelegateParam<const IntResult&, const FUserGameplayDataBundle&> ResultDelegate);

    /**
     * @brief Gets a single item that is associated with a certain bundle for a user.
     *
     * @param userGameplayDataBundleItem Struct holding the bundle name and bundle item that should be retrieved.
     * @param ResultDelegate Delegate that processes the status code and returned bundle item.
     * The ::IntResult (part of the `ResultDelegate` parameter) is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in userGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: The bundle key in userGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    static void GetBundleItem(const FUserGameplayDataBundleItem& userGameplayDataBundleItem, TAwsGameKitDelegateParam<const IntResult&, const FUserGameplayDataBundleItemValue&> ResultDelegate);

    /**
     * @brief Updates the value of an existing item inside a bundle with new item data.
     *
     * @param userGameplayDataBundleItemValue Struct holding the bundle name, bundle item, and new item data.
     * @param OnCompleteDelegate Delegate that processes the status code after the UpdateItem operation has completed.
     * The `OnCompleteDelegate` parameter takes an ::IntResult which contains a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in userGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: The bundle key in userGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    static void UpdateItem(const FUserGameplayDataBundleItemValue& userGameplayDataBundleItemValue, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Permanently deletes all bundles associated with a user.
     *
     * @param OnCompleteDelegate Delegate that processes the status code after the DeleteAllData operation has completed.
     * The `OnCompleteDelegate` parameter takes an ::IntResult which contains a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    static void DeleteAllData(FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Permanently deletes an entire bundle, along with all corresponding items, associated with a user.
     *
     * @param UserGameplayDataBundleName The name of the bundle that is being deleted.
     * @param OnCompleteDelegate Delegate that processes the status code after the DeleteBundle operation has completed.
     * The `OnCompleteDelegate` parameter takes an ::IntResult which contains a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in UserGameplayDataBundleName is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    static void DeleteBundle(const FString& UserGameplayDataBundleName, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Permanently deletes a list of items inside of a bundle associated with a user.
     *
     * @param userGameplayDataBundleItemsDeleteRequest Struct holding the bundle name and the bundle items that should be deleted.
     * @param OnCompleteDelegate Delegate that processes the status code after the DeleteBundleItems operation has completed.
     * The `OnCompleteDelegate` parameter takes an ::IntResult which contains a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in userGameplayDataBundleItemsDeleteRequest is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: The bundle key in userGameplayDataBundleItemsDeleteRequest is malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    static void DeleteBundleItems(const FUserGameplayDataDeleteItemsRequest& userGameplayDataBundleItemsDeleteRequest, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Start the Retry background thread.
    */
    static void StartRetryBackgroundThread();

    /**
     * @brief Stop the Retry background thread.
    */
    static void StopRetryBackgroundThread();

    /**
     * @brief Clear all pending events from the users cache.
    */
    static void DropAllCachedEvents();

    /**
     * @brief Set the callback to invoke when the network state changes.
     *
     * @param networkStatusChangeDelegate Reference to receiver that will be notified on a network status change
    */
    static void SetNetworkChangeDelegate(const FNetworkStatusChangeDelegate& networkStatusChangeDelegate);

    /**
     * @brief Set the callback to invoke when the offline cache finishes processing.
     *
     * @param cacheProcessedDelegate Reference to receiver that will be notified on when the offline cache is finished processing
    */
    static void SetCacheProcessedDelegate(const FCacheProcessedDelegate& cacheProcessedDelegate);

    /**
     * @brief Write the pending API calls to cache.
     * Pending API calls are requests that could not be sent due to network being offline or other failures.
     * The internal queue of pending calls is cleared. It is recommended to stop the background thread before calling this method.
     *
     * @param cacheFile path to the offline cache file.
     * @param OnCompleteDelegate Delegate that processes the status code after the PersistToCache operation has completed.
     * The `OnCompleteDelegate` parameter takes an ::IntResult which contains a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_WRITE_FAILED: There was an issue writing the queue to the offline cache file.
    */
    static void PersistToCache(const FString& cacheFile, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Read the pending API calls from cache.
     * The calls will be enqueued and retried as soon as the Retry background thread is started and network connectivity is up.
     * The contents of the cache are deleted.
     *
     * @param cacheFile path to the offline cache file.
     * @param OnCompleteDelegate Delegate that processes the status code after the LoadFromCache operation has completed.
     * The `OnCompleteDelegate` parameter takes an ::IntResult which contains a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_READ_FAILED: There was an issue loading the offline cache file to the queue.
    */
    static void LoadFromCache(const FString& cacheFile, FAwsGameKitStatusDelegateParam OnCompleteDelegate);
};
