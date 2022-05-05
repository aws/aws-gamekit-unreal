// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>
#include "Common/AwsGameKitBlueprintCommon.h"
#include "UserGameplayData/AwsGameKitUserGameplayDataWrapper.h"
#include "Models/AwsGameKitUserGameplayDataModels.h"

// Unreal
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "Templates/SharedPointer.h"
#include "UObject/NoExportTypes.h"

#include "AwsGameKitUserGameplayDataFunctionLibrary.generated.h"

class FNetworkStatusChangeDelegate;
class FCacheProcessedDelegate;

/**
 * @brief This class provides Blueprint APIs for maintaining player game data in the cloud, available when and where the player signs into the game.
 */
UCLASS()
class AWSGAMEKITRUNTIME_API UAwsGameKitUserGameplayDataFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_UCLASS_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data")
    /**
     * Applies the settings to the User Gameplay Data Client. Should be called immediately after the instance has been created and before any other API calls.
     *
     * @param clientSettings Configuration for the Gameplay Data API client.
    */
    static void SetClientSettings(
        const FUserGameplayDataClientSettings& clientSettings);

    /**
     * Creates a new bundle or updates BundleItems within a specific bundle for the calling user.
     *
     * @param UserGameplayDataBundle Struct holding the bundle name, bundleItemKeys, bundleItemValues and number of keys in the bundle being added.
     * @param UnprocessedItems Struct holding all information about items that are unprocessed, if there has not been any processing error, the map in this struct will remain empty. 
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_INVALID: An empty bundle was passed in to the method.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name of UserGameplayDataBundle is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: At least one of the bundle keys of UserGameplayData are malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason. 
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void AddBundle(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FUserGameplayDataBundle& UserGameplayDataBundle,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FUserGameplayDataBundle& UnprocessedItems,
        FAwsGameKitOperationResult& Error);

    /**
     * Lists the bundle name of every bundle that the calling user owns.
     *
     * @param Results Contains a list of all bundle names the calling user currently has.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The response body from the backend could not be parsed successfully
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void ListBundles(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        TArray<FString>& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Gets all items that are associated with a certain bundle for the calling user.
     *
     * @param UserGameplayDataBundleName The name of the bundle that is being retrieved.
     * @param Results UStruct containing the bundle name and a map with all key value pairs in the bundle for the calling playing.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name of UserGameplayDataBundleName is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The response body from the backend could not be parsed successfully
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void GetBundle(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FString& UserGameplayDataBundleName,
        FUserGameplayDataBundle& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Gets a single item that is associated with a certain bundle for a user.
     *
     * @param UserGameplayDataBundleItem Struct holding the bundle name and bundle item that should be retrieved.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in UserGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: The bundle key in UserGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void GetBundleItem(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FUserGameplayDataBundleItem& UserGameplayDataBundleItem,
        FUserGameplayDataBundleItemValue& Result,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Updates the value of an existing item inside a bundle with new item data.
     *
     * @param UserGameplayDataBundleItemValue Struct holding the bundle name, bundle item, and new item data.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in UserGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: The bundle key in UserGameplayDataBundleItem is malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void UpdateItem(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FUserGameplayDataBundleItemValue& UserGameplayDataBundleItemValue,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Permanently deletes all bundles associated with a user.
     *
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void DeleteAllData(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Permanently deletes an entire bundle, along with all corresponding items, associated with a user.
     *
     * @param UserGameplayDataBundleName The name of the bundle that is being deleted.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in UserGameplayDataBundleName is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void DeleteBundle(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FString& UserGameplayDataBundleName,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Permanently deletes a list of items inside of a bundle associated with a user.
     *
     * @param UserGameplayDataBundleItemsDeleteRequest Struct holding the bundle name and the bundle items that should be deleted.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_INVALID: An empty bundle was passed in to the method.
     * - GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED: The session manager does not have settings loaded in for the User Gameplay data feature.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME: The bundle name in UserGameplayDataBundleItemsDeleteRequest is malformed. If this error is received, Check the output log for more details on requirements.
     * - GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY: The bundle key in UserGameplayDataBundleItemsDeleteRequest is malformed. If this error is received, Check the output log for more details on which item keys are not valid.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_TOO_LARGE: The payload in the request is too large. If this error is received, check the output log for more details on payload length constraints.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED: The call made to the backend service has failed.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED: The call made to the backend service has been dropped.
     * - GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED: The call made to the backend service has been enqueued as connection may be unhealthy and will automatically be retried.
     * - GAMEKIT_ERROR_GENERAL: The request has failed unknown reason.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void DeleteBundleItems(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FUserGameplayDataDeleteItemsRequest& UserGameplayDataBundleItemsDeleteRequest,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Start the Retry background thread.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data")
    static void StartRetryBackgroundThread();

    /**
     * Stop the Retry background thread.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data")
    static void StopRetryBackgroundThread();

    /**
     * Clear all pending events from the users cache.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data")
        static void DropAllCachedEvents();

    /**
     * Set the callback to invoke when the network state changes.
     *
     * @param NetworkStatusChangeDelegate Reference to receiver that will be notified on a network status change
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data")
    static void SetNetworkChangeDelegate(
        const FNetworkStatusChangeDelegate& NetworkStatusChangeDelegate);

    /**
     * Set the callback to invoke when the offline cache is finished processing.
     *
     * @param CacheProcessedDelegate Reference to receiver that will be notified when the cache is finished processing
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data")
        static void SetCacheProcessedDelegate(
            const FCacheProcessedDelegate& CacheProcessedDelegate);

    /**
     * Write the pending API calls to cache.
     * Pending API calls are requests that could not be sent due to network being offline or other failures.
     * The internal queue of pending calls is cleared. It is recommended to stop the background thread before calling this method.
     *
     * @param CacheFile path to the offline cache file.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_WRITE_FAILED: There was an issue writing the queue to the offline cache file.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void PersistToCache(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FString& CacheFile,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Read the pending API calls from cache.
     * The calls will be enqueued and retried as soon as the Retry background thread is started and network connectivity is up.
     * The contents of the cache are deleted.
     *
     * @param CacheFile path to the offline cache file.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_READ_FAILED: There was an issue loading the offline cache file to the queue.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void LoadFromCache(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FString& CacheFile,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);
};