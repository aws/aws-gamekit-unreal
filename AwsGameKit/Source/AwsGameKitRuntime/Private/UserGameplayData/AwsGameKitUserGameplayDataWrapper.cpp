// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "UserGameplayData/AwsGameKitUserGameplayDataWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"

void AwsGameKitUserGameplayDataWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitUserGameplayDataWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataInstanceCreateWithSessionManager, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitSetUserGameplayDataClientSettings, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitAddUserGameplayData, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitListUserGameplayDataBundles, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetUserGameplayDataBundle, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetUserGameplayDataBundleItem, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUpdateUserGameplayDataBundleItem, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitDeleteAllUserGameplayData, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitDeleteUserGameplayDataBundle, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitDeleteUserGameplayDataBundleItems, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataStartRetryBackgroundThread, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataStopRetryBackgroundThread, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataSetNetworkChangeCallback, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataSetCacheProcessedCallback, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataDropAllCachedEvents, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataPersistApiCallsToCache, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitUserGameplayDataLoadApiCallsFromCache, loadedDllHandle);
}

GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataInstanceCreateWithSessionManager(void* sessionManager, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataInstanceCreateWithSessionManager, nullptr);
    return INVOKE_FUNC(GameKitUserGameplayDataInstanceCreateWithSessionManager, sessionManager, logCb);
}

void AwsGameKitUserGameplayDataWrapper::GameKitSetUserGameplayDataClientSettings(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataClientSettings settings)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitSetUserGameplayDataClientSettings);
    INVOKE_FUNC(GameKitSetUserGameplayDataClientSettings, userGameplayDataInstance, settings);
}

void AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataInstanceRelease(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataInstanceRelease);
    INVOKE_FUNC(GameKitUserGameplayDataInstanceRelease, userGameplayDataInstance);
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitAddUserGameplayData(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, TMap<FString, FString>& inOutUnprocessedItems, UserGameplayDataBundle userGameplayDataBundle)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitAddUserGameplayData, GameKit::GAMEKIT_ERROR_GENERAL);

    inOutUnprocessedItems.Empty();
    auto unprocessedItemsSetter = [&inOutUnprocessedItems](const char* key, const char* value)
    {
        inOutUnprocessedItems.Add(key, value);
    };
    typedef LambdaDispatcher<decltype(unprocessedItemsSetter), void, const char*, const char*> UnprocessedItemsSetter;

    const IntResult result = INVOKE_FUNC(GameKitAddUserGameplayData, userGameplayDataInstance, userGameplayDataBundle, (void*)&unprocessedItemsSetter, UnprocessedItemsSetter::Dispatch);

    return result.Result;
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitListUserGameplayDataBundles(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, TArray<FString>& inOutData)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitListUserGameplayDataBundles, GameKit::GAMEKIT_ERROR_GENERAL);

    inOutData.Empty();
    auto userDataSetter = [&inOutData](const char* bundle)
    {
        inOutData.Add(bundle);
    };
    typedef LambdaDispatcher<decltype(userDataSetter), void, const char*> BundleSetter;

    IntResult result = INVOKE_FUNC(GameKitListUserGameplayDataBundles, userGameplayDataInstance, (void*)&userDataSetter, BundleSetter::Dispatch);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: AwsGameKitUserGameplayDataWrapper::GameKitListUserGameplayDataBundles() Failed to retrieve data.");
        const FString error = GameKit::StatusCodeToHexFStr(result.Result);
        const FString message = result.ErrorMessage + " : " + error;
        UE_LOG(LogAwsGameKit, Error, TEXT("%s"), *message);
        inOutData.Reset();
        return GameKit::GAMEKIT_ERROR_GENERAL;
    }

    return result.Result;
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitGetUserGameplayDataBundle(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, TMap<FString, FString>& inOutData, char* bundleName)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitGetUserGameplayDataBundle, GameKit::GAMEKIT_ERROR_GENERAL);

    inOutData.Empty();
    auto bundleSetter = [&inOutData](const char* key, const char* value)
    {
        inOutData.Add(key, value);
    };
    typedef LambdaDispatcher<decltype(bundleSetter), void, const char*, const char*> BundleSetter;

    IntResult result = INVOKE_FUNC(GameKitGetUserGameplayDataBundle, userGameplayDataInstance, bundleName, (void*)&bundleSetter, BundleSetter::Dispatch);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: AwsGameKitUserGameplayDataWrapper::GameKitGetUserGameplayDataBundle() Failed to retrieve data.");
        const FString error = GameKit::StatusCodeToHexFStr(result.Result);
        const FString message = result.ErrorMessage + " : " + error;
        UE_LOG(LogAwsGameKit, Error, TEXT("%s"), *message);
        inOutData.Reset();
        return GameKit::GAMEKIT_ERROR_GENERAL;
    }

    return result.Result;
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitGetUserGameplayDataBundleItem(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, FString& inOutData, UserGameplayDataBundleItem userGameplayDataBundleItem)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitGetUserGameplayDataBundleItem, GameKit::GAMEKIT_ERROR_GENERAL);

    auto bundleItemSetter = [&inOutData](const char* retrievedBundleItem)
    {
        inOutData = retrievedBundleItem;
    };
    typedef LambdaDispatcher<decltype(bundleItemSetter), void, const char*> BundleItemSetter;

    IntResult result = INVOKE_FUNC(GameKitGetUserGameplayDataBundleItem, userGameplayDataInstance, userGameplayDataBundleItem, (void*)&bundleItemSetter, BundleItemSetter::Dispatch);

    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        result.ErrorMessage = FString("Error: AwsGameKitUserGameplayDataWrapper::GameKitGetUserGameplayDataBundleItem() Failed to retrieve data.");
        const FString error = GameKit::StatusCodeToHexFStr(result.Result);
        const FString message = result.ErrorMessage + " : " + error;
        UE_LOG(LogAwsGameKit, Error, TEXT("%s"), *message);
        inOutData = "";
        return GameKit::GAMEKIT_ERROR_GENERAL;
    }

    return result.Result;
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitUpdateUserGameplayDataBundleItem(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataBundleItemValue userGameplayDataBundleItemValue)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUpdateUserGameplayDataBundleItem, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitUpdateUserGameplayDataBundleItem, userGameplayDataInstance, userGameplayDataBundleItemValue);
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitDeleteAllUserGameplayData(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitDeleteAllUserGameplayData, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitDeleteAllUserGameplayData, userGameplayDataInstance);
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitDeleteUserGameplayDataBundle(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, char* bundleName)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitDeleteUserGameplayDataBundle, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitDeleteUserGameplayDataBundle, userGameplayDataInstance, bundleName);
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitDeleteUserGameplayDataBundleItems(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, UserGameplayDataDeleteItemsRequest deleteItemsRequest)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitDeleteUserGameplayDataBundleItems, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitDeleteUserGameplayDataBundleItems, userGameplayDataInstance, deleteItemsRequest);
}

void AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataStartRetryBackgroundThread(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataStartRetryBackgroundThread);
    INVOKE_FUNC(GameKitUserGameplayDataStartRetryBackgroundThread, userGameplayDataInstance);
}

void AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataStopRetryBackgroundThread(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataStopRetryBackgroundThread);
    INVOKE_FUNC(GameKitUserGameplayDataStopRetryBackgroundThread, userGameplayDataInstance);
}

void AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataSetNetworkChangeCallback(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, DISPATCH_RECEIVER_HANDLE receiverHandle, NetworkStatusChangeCallback statusChangeCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataSetNetworkChangeCallback);
    INVOKE_FUNC(GameKitUserGameplayDataSetNetworkChangeCallback, userGameplayDataInstance, receiverHandle, statusChangeCallback);
}

void AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataSetCacheProcessedCallback(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, DISPATCH_RECEIVER_HANDLE receiverHandle, CacheProcessedCallback cacheProcessedCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataSetCacheProcessedCallback);
    INVOKE_FUNC(GameKitUserGameplayDataSetCacheProcessedCallback, userGameplayDataInstance, receiverHandle, cacheProcessedCallback);
}

void AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataDropAllCachedEvents(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataDropAllCachedEvents);
    INVOKE_FUNC(GameKitUserGameplayDataDropAllCachedEvents, userGameplayDataInstance);
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataPersistApiCallsToCache(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, const char* offlineCacheFile)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataPersistApiCallsToCache, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitUserGameplayDataPersistApiCallsToCache, userGameplayDataInstance, offlineCacheFile);
}

unsigned int AwsGameKitUserGameplayDataWrapper::GameKitUserGameplayDataLoadApiCallsFromCache(GAMEKIT_USER_GAMEPLAY_DATA_INSTANCE_HANDLE userGameplayDataInstance, const char* offlineCacheFile)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(UserGameplayData, GameKitUserGameplayDataLoadApiCallsFromCache, GameKit::GAMEKIT_ERROR_GENERAL);
    return INVOKE_FUNC(GameKitUserGameplayDataLoadApiCallsFromCache, userGameplayDataInstance, offlineCacheFile);
}

#undef LOCTEXT_NAMESPACE