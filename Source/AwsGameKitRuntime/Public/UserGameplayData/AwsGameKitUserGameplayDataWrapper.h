// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
 * @brief Interface for the User Gameplay Data low level C API.
 */

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryWrapper.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryUtils.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>

// Standard library
#include <string>

/**
 * Structs/Enums
 */

 /**
 *@struct UserGameplayDataBundle
 *@brief Struct that stores information about a bundle
 *
 *@var bundleName The name of the bundle
 *@var bundleItemKeys The item keys associated with this bundle
 *@var bundleItemValues The values corresponding to each item key
 * BundleItemValues can be converted back to any data type once they are retrieved from DynamoDB
 *@var numKeys The number of keys that are being referenced in this bundle.
 */
struct UserGameplayDataBundle
{
    const char* bundleName;
    const char** bundleItemKeys;
    const char** bundleItemValues;
    size_t numKeys;
};

/**
 *@struct UserGameplayDataBundleItem
 *@brief Struct that stores information needed to reference a single item contained in a bundle
 *
 *@var bundleName The name of the bundle being referenced
 *@var bundleItemKey The key of the item being referenced within the bundle
*/
struct UserGameplayDataBundleItem
{
    const char* bundleName;
    const char* bundleItemKey;
};

/**
 *@struct UserGameplayDataBundleItemValue
 *@brief Struct that stores information needed to update a single item contained in a bundle
 *
 *@var bundleName The name of the bundle that contains the item being updated
 *@var bundleItemKey The key of the item being updated
 *@var bundleItemValue The new char* value that should be associated with the bundleItemId for the calling user
*/
struct UserGameplayDataBundleItemValue
{
    const char* bundleName;
    const char* bundleItemKey;
    const char* bundleItemValue;
};

/**
 *@struct UserGamplayDataDeleteItemsRequest
 *@brief Struct that stores information needed to update a single item contained in a bundle
 *
 *@var bundleName The name of the bundle that contains the item being updated
 *@var bundleItemKey The key of the items being deleted
*/
struct UserGameplayDataDeleteItemsRequest
{
    const char* bundleName;
    const char** bundleItemKeys;
    size_t numKeys;
};

/**
 * @brief Settings for the User Gameplay Data API client.
 *
 */
struct UserGameplayDataClientSettings
{
    /**
     * @brief Connection timeout in seconds for the internal HTTP client. Default is 3. Uses default if set to 0.
     */
    unsigned int ClientTimeoutSeconds;

    /**
     * @brief Seconds to wait between retries. Default is 5. Uses default value if set to 0.
     */
    unsigned int RetryIntervalSeconds;

    /**
     * @brief Maximum length of custom http client request queue size. Once the queue is full, new requests will be dropped. Default is 256. Uses default if set to 0.
     */
    unsigned int MaxRetryQueueSize;

    /**
     * @brief Maximum number of times to retry a request before dropping it. Default is 32. Uses default if set to 0.
     */
    unsigned int MaxRetries;

    /**
     * @brief Retry strategy to use. Use 0 for Exponential Backoff, 1 for Constant Interval. Default is 0.
     */
    unsigned int RetryStrategy;

    /**
     * @brief Maximum retry threshold for Exponential Backoff. Forces a retry even if exponential backoff is set to a greater value. Default is 32. Uses default if set to 0.
     */
    unsigned int MaxExponentialRetryThreshold;

    /**
     * @brief Number of items to retrieve when executing paginated calls such as Get All Data. Default is 100. Uses default if set to 0.
     */
    unsigned int PaginationSize;
};

/**
 * @brief Pointer to an instance of a UserGameplayData class created in the imported User Gameplay Data C library.
 *
 * Most GameKit C APIs require an instance handle to be passed in.
 *
 * Instance handles are stored as a void* (instead of a class type) because the GameKit C libraries expose a C-level interface (not a C++ interface).
 */
typedef void* GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE;

/**
 * Callback pointer declarations
 */
typedef void(*FuncAllBundlesUserGameplayDataResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* responseBundleName);
typedef void(*FuncBundleResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* responseKey, const char* responseValue);
typedef void(*FuncBundleItemResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const char* responseValue);
typedef void(*NetworkStatusChangeCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, bool isConnectionOk, const char* connectionClient);

/**
 * This class exposes the GameKit UserGameplayData APIs and loads the underlying DLL into memory.
 *
 * This is a barebones wrapper over the DLL's C-level interface. It uses C data types instead of Unreal data types (ex: char* instead of FString).
 */
class AWSGAMEKITRUNTIME_API AwsGameKitUserGameplayDataWrapper : public AwsGameKitLibraryWrapper
{

private:
    /**
    * Function pointer handles and declarations
    */
    DEFINE_FUNC_HANDLE(void*, GameKitUserGameplayDataInstanceCreateWithSessionManager, (void* sessionManager, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitSetUserGameplayDataClientSettings, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataClientSettings settings, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitAddUserGameplayData, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataBundle userGameplayDataBundle, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitListUserGameplayDataBundles, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, DISPATCH_RECEIVER_HANDLE receiver, FuncAllBundlesUserGameplayDataResponseCallback responseCb, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetUserGameplayDataBundle, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, char* bundleName, DISPATCH_RECEIVER_HANDLE receiver, FuncBundleResponseCallback responseCb, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetUserGameplayDataBundleItem, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataBundleItem userGameplayDataBundleItem, DISPATCH_RECEIVER_HANDLE receiver, FuncBundleItemResponseCallback responseCb, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitUpdateUserGameplayDataBundleItem, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataBundleItemValue userGameplayDataBundleItemValue, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitDeleteAllUserGameplayData, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitDeleteUserGameplayDataBundle, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, char* bundleName, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitDeleteUserGameplayDataBundleItems, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataDeleteItemsRequest deleteItemsRequest, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(void, GameKitUserGameplayDataInstanceRelease, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance));
    DEFINE_FUNC_HANDLE(void, GameKitUserGameplayDataStartRetryBackgroundThread, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance));
    DEFINE_FUNC_HANDLE(void, GameKitUserGameplayDataStopRetryBackgroundThread, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance));
    DEFINE_FUNC_HANDLE(void, GameKitUserGameplayDataSetNetworkChangeCallback, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, DISPATCH_RECEIVER_HANDLE receiver, NetworkStatusChangeCallback statusChangeCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitUserGameplayDataPersistApiCallsToCache, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, const char* offlineCacheFile));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitUserGameplayDataLoadApiCallsFromCache, (GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, const char* offlineCacheFile));

protected:
    virtual std::string getLibraryFilename() override
    {
        return "aws-gamekit-user-gameplay-data";
    }

    virtual void importFunctions(void* loadedDllHandle) override;

public:
    /**
     * @brief Creates an UserGameplayData instance, which can be used to access the UserGameplayData API.
     *
     * @details Make sure to call GameKitUserGameplayDataInstanceRelease to destroy the returned object when finished with it.
     *
     * @param sessionManager Pointer to a session manager object which manages tokens and configuration.
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new UserGameplayData instance.
    */
    virtual GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE GameKitUserGameplayDataInstanceCreateWithSessionManager(void* sessionManager, FuncLogCallback logCb);

    /**
     * @brief Creates an UserGameplayDatainstance, which can be used to access the UserGameplayData API.
     *
     * @details Make sure to call GameKitUserGameplayDataInstanceRelease to destroy the returned object when finished with it.
     *
     * @param sessionManager Pointer to a session manager object which manages tokens and configuration.
     * @param settings Configuration for the Gameplay Data API client.
     * @param logCb Callback function for logging information and errors.
     * @return Pointer to the new UserGameplayData instance.
    */
    virtual void GameKitSetUserGameplayDataClientSettings(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataClientSettings settings, FuncLogCallback logCb);

    /**
     * @brief Destroys the passed in user UserGameplayData instance.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
    */
    virtual void GameKitUserGameplayDataInstanceRelease(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance);

    /**
     * @brief Creates or updates BundleItems within a specific bundle for the calling user.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager().
     * @param userGameplayDataBundle Struct holding the bundle name, bundleItemKeys, bundleItemValues and number of keys in the bundle being added.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
    */
    virtual unsigned int GameKitAddUserGameplayData(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataBundle userGameplayDataBundle, FuncLogCallback logCb);

    /**
     * @brief Gets user all bundles that a player currently owns
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
     * @param inOutData Pointer to the array that will contain the names of all bundles that a player currently has
     * @param logCb Callback function for logging information and errors.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitListUserGameplayDataBundles(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, TArray<FString>& inOutData, FuncLogCallback logCb);

    /**
     * @brief Gets user gameplay data stored for the calling user from a specific bundle.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager().
     * @param inOutData Pointer to the map the will store all key value pairs retrieved from DynamoDB for the requested bundle.
     * @param bundleName The name of the bundle that should be referenced in DyanmoDB.
     * @param logCb Callback function for logging information and errors.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitGetUserGameplayDataBundle(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, TMap<FString, FString>& inOutData, char* bundleName, FuncLogCallback logCb);

    /**
     * @brief Gets a single stored item from a specific bundle for the calling user.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager().
     * @param inOutData Pointer to the value that will store the value retrieved from DynamoDB
     * @param userGameplayDataBundleItem Struct holding the bundle name and bundle item that should be retrieved.
     * @param logCb Callback function for logging information and errors.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitGetUserGameplayDataBundleItem(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, FString& inOutData, UserGameplayDataBundleItem userGameplayDataBundleItem, FuncLogCallback logCb);

    /**
     * @brief Updates a single item inside of a bundle for the calling user.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager().
     * @param userGameplayDataBundleItemValue Struct holding the bundle name, bundle item, and item data that will replace the existing data.
     * @param logCb Callback function for logging information and errors.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitUpdateUserGameplayDataBundleItem(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataBundleItemValue userGameplayDataBundleItemValue, FuncLogCallback logCb);

    /**
     * @brief Deletes all user gameplay data stored for the calling user.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager().
     * @param logCb Callback function for logging information and errors.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitDeleteAllUserGameplayData(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, FuncLogCallback logCb);

    /**
     * @brief Deletes all user gameplay data stored within a specific bundle for the calling user.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager().
     * @param bundleName The name of the bundle to be deleted.
     * @param logCb Callback function for logging information and errors.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitDeleteUserGameplayDataBundle(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, char* bundleName, FuncLogCallback logCb);

    /**
     * @brief Deletes a single user gameplay data item for the calling user.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
     * @param deleteItemsRequest Struct holding the bundle name and the bundle item that should be deleted.
     * @param logCb Callback function for logging information and errors.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitDeleteUserGameplayDataBundleItems(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataDeleteItemsRequest deleteItemsRequest, FuncLogCallback logCb);

    /**
    * @brief Start the Retry background thread.
    *
    * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
    */
    virtual void GameKitUserGameplayDataStartRetryBackgroundThread(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance);

    /**
     * @brief Stop the Retry background thread.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
     */
    virtual void GameKitUserGameplayDataStopRetryBackgroundThread(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance);

    /**
     * @brief Set the callback to invoke when the network state changes.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
     * @param receiverHandle A pointer to an instance of a class to notify when the network state changes.
     * @param statusChangeCallback Calback function for notifying network state changes.
     */
    virtual void GameKitUserGameplayDataSetNetworkChangeCallback(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, DISPATCH_RECEIVER_HANDLE receiver, NetworkStatusChangeCallback statusChangeCallback);

    /**
     * @brief Write the pending UserGameplayData API calls to cache.
     * Pending API calls are requests that could not be sent due to network being offline or other failures.
     * The internal queue of pending calls is cleared. It is recommended to stop the background thread before calling this method.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
     * @param offlineCacheFile path to the offline cache file.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitUserGameplayDataPersistApiCallsToCache(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, const char* offlineCacheFile);

    /**
     * @brief Read the pending API calls from cache.
     * The calls will be enqueued and retried as soon as the Retry background thread is started and network connectivity is up.
     * The contents of the cache are deleted.
     *
     * @param userGameplayDataInstance Pointer to GameKitUserGameplayData instance created with GameKitUserGameplayDataInstanceCreateWithSessionManager()
     * @param offlineCacheFile path to the offline cache file.
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult AwsGameKitErrors.h file for details.
     */
    virtual unsigned int GameKitUserGameplayDataLoadApiCallsFromCache(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, const char* offlineCacheFile);
};
