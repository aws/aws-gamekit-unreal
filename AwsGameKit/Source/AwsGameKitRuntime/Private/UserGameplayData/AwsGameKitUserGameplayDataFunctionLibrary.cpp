// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "UserGameplayData/AwsGameKitUserGameplayDataFunctionLibrary.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimeInternalHelpers.h"
#include "AwsGameKitUserGameplayData.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/Logging.h"

// Unreal
#include "LatentActions.h"
#if PLATFORM_ANDROID
#include "Android/AndroidPlatformFile.h"
#endif

UAwsGameKitUserGameplayDataFunctionLibrary::UAwsGameKitUserGameplayDataFunctionLibrary(const FObjectInitializer& Initializer)
    : UBlueprintFunctionLibrary(Initializer)
{}

void UAwsGameKitUserGameplayDataFunctionLibrary::SetClientSettings(const FUserGameplayDataClientSettings& clientSettings)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::SetClientSettings()"));

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

    UserGameplayDataClientSettings settings;
    settings.ClientTimeoutSeconds = clientSettings.ClientTimeoutSeconds;
    settings.RetryIntervalSeconds = clientSettings.RetryIntervalSeconds;
    settings.MaxRetryQueueSize = clientSettings.MaxRetryQueueSize;
    settings.MaxRetries = clientSettings.MaxRetries;
    settings.RetryStrategy = clientSettings.RetryStrategy;
    settings.MaxExponentialRetryThreshold = clientSettings.MaxExponentialRetryThreshold;
    settings.PaginationSize = clientSettings.PaginationSize;

    library.UserGameplayDataWrapper->GameKitSetUserGameplayDataClientSettings(library.UserGameplayDataInstanceHandle, settings);
}

void UAwsGameKitUserGameplayDataFunctionLibrary::AddBundle(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FUserGameplayDataBundle& userGameplayDataBundle,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FUserGameplayDataBundle& UnprocessedItems,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::AddBundle()"));

    TAwsGameKitInternalActionStatePtr<FUserGameplayDataBundle> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, userGameplayDataBundle, SuccessOrFailure, Error, UnprocessedItems))
    {
        Action->LaunchThreadedWork([userGameplayDataBundle, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
            
            int32 pairCount = userGameplayDataBundle.BundleMap.Num();
            IntResult result;

            if (pairCount == 0)
            {
                UE_LOG(LogAwsGameKit, Error, TEXT("UAwsGameKitUserGameplayDataCallableWrapper::AddUserGameplayData - The bundle is empty."));
                result.Result = GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_INVALID;
                result.ErrorMessage = "The bundle is empty";
            }
            else
            {
                // Preallocate with expected size to prevent pointers becoming invalid
                // when array expands and data is copied.
                TArray<std::string> bundleItemKeys;
                bundleItemKeys.SetNum(pairCount);
                TArray<const char*> bundleItemKeysChrArray;
                bundleItemKeysChrArray.SetNum(pairCount);

                TArray<std::string> bundleItemValues;
                bundleItemValues.SetNum(pairCount);
                TArray<const char*> bundleItemValuesChrArray;
                bundleItemValuesChrArray.SetNum(pairCount);

                int i = 0;
                for (auto it = userGameplayDataBundle.BundleMap.begin(); it != userGameplayDataBundle.BundleMap.end(); ++it)
                {
                    const FString& key = it.Key();
                    const FString& value = it.Value();

                    bundleItemKeys[i] = TCHAR_TO_UTF8(*key);
                    bundleItemValues[i] = TCHAR_TO_UTF8(*value);

                    bundleItemKeysChrArray[i] = bundleItemKeys[i].c_str();
                    bundleItemValuesChrArray[i] = bundleItemValues[i].c_str();

                    i++;
                }

                std::string bundleName = TCHAR_TO_UTF8(*userGameplayDataBundle.BundleName);
                State->Results.BundleName = bundleName.c_str();
                UserGameplayDataBundle wrapperArgs
                {
                    bundleName.c_str(),
                    (bundleItemKeysChrArray.GetData()),
                    (bundleItemValuesChrArray.GetData()),
                    size_t(pairCount)
                };

                result = IntResult(library.UserGameplayDataWrapper->GameKitAddUserGameplayData(library.UserGameplayDataInstanceHandle, State->Results.BundleMap, wrapperArgs));
            }

            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::ListBundles(UObject* WorldContextObject, FLatentActionInfo LatentInfo, TArray<FString>& Results, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::ListBundles()"));

    TAwsGameKitInternalActionStatePtr<TArray<FString>> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

            IntResult result(library.UserGameplayDataWrapper->GameKitListUserGameplayDataBundles(library.UserGameplayDataInstanceHandle, State->Results));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::GetBundle(UObject* WorldContextObject, FLatentActionInfo LatentInfo, const FString& userGameplayDataBundleName, FUserGameplayDataBundle& Result, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::GetBundle()"));

    TAwsGameKitInternalActionStatePtr<FUserGameplayDataBundle> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, userGameplayDataBundleName, SuccessOrFailure, Error, Result))
    {
        Action->LaunchThreadedWork([userGameplayDataBundleName, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

            State->Results.BundleName = userGameplayDataBundleName;
            IntResult result(library.UserGameplayDataWrapper->GameKitGetUserGameplayDataBundle(library.UserGameplayDataInstanceHandle, State->Results.BundleMap, TCHAR_TO_UTF8(*userGameplayDataBundleName)));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::GetBundleItem(UObject* WorldContextObject, FLatentActionInfo LatentInfo, const FUserGameplayDataBundleItem& userGameplayDataBundleItem, FUserGameplayDataBundleItemValue& Result, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::GetBundleItem()"));

    TAwsGameKitInternalActionStatePtr<FUserGameplayDataBundleItemValue> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, userGameplayDataBundleItem, SuccessOrFailure, Error, Result))
    {
        Action->LaunchThreadedWork([userGameplayDataBundleItem, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

            FAwsGameKitInternalTempStrings ConvertString;
            UserGameplayDataBundleItem wrapperArgs
            {
                ConvertString(*userGameplayDataBundleItem.BundleName),
                ConvertString(*userGameplayDataBundleItem.BundleItemKey)
            };

            State->Results.BundleName = userGameplayDataBundleItem.BundleName;
            State->Results.BundleItemKey = userGameplayDataBundleItem.BundleItemKey;
            IntResult result(library.UserGameplayDataWrapper->GameKitGetUserGameplayDataBundleItem(library.UserGameplayDataInstanceHandle, State->Results.BundleItemValue, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::UpdateItem(UObject* WorldContextObject, FLatentActionInfo LatentInfo, const FUserGameplayDataBundleItemValue& userGameplayDataBundleItemValue, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::UpdateItem()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, userGameplayDataBundleItemValue, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([userGameplayDataBundleItemValue, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

            FAwsGameKitInternalTempStrings ConvertString;
            UserGameplayDataBundleItemValue wrapperArgs
            {
                ConvertString(*userGameplayDataBundleItemValue.BundleName),
                ConvertString(*userGameplayDataBundleItemValue.BundleItemKey),
                ConvertString(*userGameplayDataBundleItemValue.BundleItemValue)
            };

            IntResult result(library.UserGameplayDataWrapper->GameKitUpdateUserGameplayDataBundleItem(library.UserGameplayDataInstanceHandle, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::DeleteAllData(UObject* WorldContextObject, FLatentActionInfo LatentInfo, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::DeleteAllData()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

            IntResult result(library.UserGameplayDataWrapper->GameKitDeleteAllUserGameplayData(library.UserGameplayDataInstanceHandle));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::DeleteBundle(UObject* WorldContextObject, FLatentActionInfo LatentInfo, const FString& userGameplayDataBundleName, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::DeleteBundle()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, userGameplayDataBundleName, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([userGameplayDataBundleName, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

            IntResult result(library.UserGameplayDataWrapper->GameKitDeleteUserGameplayDataBundle(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*userGameplayDataBundleName)));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::DeleteBundleItems(UObject* WorldContextObject, FLatentActionInfo LatentInfo, const FUserGameplayDataDeleteItemsRequest& userGameplayDataBundleItemsDeleteRequest, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::DeleteBundleItems()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, userGameplayDataBundleItemsDeleteRequest, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([userGameplayDataBundleItemsDeleteRequest, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
            IntResult result;

            if (userGameplayDataBundleItemsDeleteRequest.BundleItemKeys.Num() == 0 ||
                userGameplayDataBundleItemsDeleteRequest.BundleName.IsEmpty())
            {
                UE_LOG(LogAwsGameKit, Error, TEXT("UAwsGameKitUserGameplayDataCallableWrapper::DeleteUserGameplayDataBundleItems - The bundle is invalid."));
                result.Result = GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_INVALID;
                result.ErrorMessage = "The bundle is invalid";
            }
            else
            {
                // Preallocate with expected size to prevent pointers becoming invalid
                // when array expands and data is copied.
                int32 numKeys = userGameplayDataBundleItemsDeleteRequest.BundleItemKeys.Num();
                TArray<std::string> bundleItemKeys;
                bundleItemKeys.SetNum(numKeys);
                TArray<const char*> bundleItemKeysChrArray;
                bundleItemKeysChrArray.SetNum(numKeys);

                for (int i = 0; i < numKeys; i++)
                {
                    auto& itemKey = userGameplayDataBundleItemsDeleteRequest.BundleItemKeys[i];

                    bundleItemKeys[i] = TCHAR_TO_UTF8(*itemKey);

                    bundleItemKeysChrArray[i] = bundleItemKeys[i].c_str();
                }

                std::string bundleName = TCHAR_TO_UTF8(*userGameplayDataBundleItemsDeleteRequest.BundleName);
                UserGameplayDataDeleteItemsRequest wrapperArgs
                {
                    bundleName.c_str(),
                    (bundleItemKeysChrArray.GetData()),
                    size_t(numKeys)
                };

                result = library.UserGameplayDataWrapper->GameKitDeleteUserGameplayDataBundleItems(library.UserGameplayDataInstanceHandle, wrapperArgs);
            }

            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::SetNetworkChangeDelegate(const FNetworkStatusChangeDelegate& NetworkStatusChangeDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::SetNetworkChangeDelegate()"));
    
    if (NetworkStatusChangeDelegate.IsBound())
    {
        FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
        runtimeModule->SetNetworkChangeDelegate(NetworkStatusChangeDelegate);
        
        UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
        library.UserGameplayDataWrapper->GameKitUserGameplayDataSetNetworkChangeCallback(library.UserGameplayDataInstanceHandle, runtimeModule, &FAwsGameKitRuntimeModule::OnNetworkStatusChangeDispatcher::Dispatch);
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::SetCacheProcessedDelegate(const FCacheProcessedDelegate& CacheProcessedDelegate)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::SetCacheProcessedDelegate()"));

    if (CacheProcessedDelegate.IsBound())
    {
        FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
        UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
        library.UserGameplayDataStateHandler->SetCacheProcessedDelegate(CacheProcessedDelegate);

        auto cacheProcessedDelegateExecutor = [](const bool isCacheProcessed)
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
            library.UserGameplayDataStateHandler->onCacheProcessedDelegate.ExecuteIfBound(isCacheProcessed);
        };
        typedef LambdaDispatcher<decltype(cacheProcessedDelegateExecutor), void, const bool> CacheProcessedDelegateExecutor;

        library.UserGameplayDataWrapper->GameKitUserGameplayDataSetCacheProcessedCallback(library.UserGameplayDataInstanceHandle, &cacheProcessedDelegateExecutor, CacheProcessedDelegateExecutor::Dispatch);
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::StartRetryBackgroundThread()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::StartRetryBackgroundThread()"));
    
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
    library.UserGameplayDataWrapper->GameKitUserGameplayDataStartRetryBackgroundThread(library.UserGameplayDataInstanceHandle);
}

void UAwsGameKitUserGameplayDataFunctionLibrary::StopRetryBackgroundThread()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::UAwsGameKitUserGameplayDataFunctionLibrary()"));
    
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
    library.UserGameplayDataWrapper->GameKitUserGameplayDataStopRetryBackgroundThread(library.UserGameplayDataInstanceHandle);
}

void UAwsGameKitUserGameplayDataFunctionLibrary::DropAllCachedEvents()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::DropAllCachedEvents()"));

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();
    library.UserGameplayDataWrapper->GameKitUserGameplayDataDropAllCachedEvents(library.UserGameplayDataInstanceHandle);
}

void UAwsGameKitUserGameplayDataFunctionLibrary::PersistToCache(UObject* WorldContextObject, FLatentActionInfo LatentInfo, const FString& CacheFile, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::PersistToCache()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, CacheFile, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([CacheFile, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

#if PLATFORM_ANDROID
            // Convert to platform path
            FString androidCacheFilePath = IAndroidPlatformFile::GetPlatformPhysical().ConvertToAbsolutePathForExternalAppForWrite(*CacheFile);
            IntResult result(library.UserGameplayDataWrapper->GameKitUserGameplayDataPersistApiCallsToCache(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*androidCacheFilePath)));
#else
            IntResult result(library.UserGameplayDataWrapper->GameKitUserGameplayDataPersistApiCallsToCache(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*CacheFile)));
#endif
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitUserGameplayDataFunctionLibrary::LoadFromCache(UObject* WorldContextObject, FLatentActionInfo LatentInfo, const FString& CacheFile, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitUserGameplayDataFunctionLibrary::LoadFromCache()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, CacheFile, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([CacheFile, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            UserGameplayDataLibrary library = runtimeModule->GetUserGameplayDataLibrary();

#if PLATFORM_ANDROID
            // Convert to platform path
            FString androidCacheFilePath = IAndroidPlatformFile::GetPlatformPhysical().ConvertToAbsolutePathForExternalAppForRead(*CacheFile); 
            IntResult result(library.UserGameplayDataWrapper->GameKitUserGameplayDataLoadApiCallsFromCache(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*androidCacheFilePath)));
#else
            IntResult result(library.UserGameplayDataWrapper->GameKitUserGameplayDataLoadApiCallsFromCache(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*CacheFile)));
#endif
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}
