// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "UserGameplayData/AwsGameKitUserGameplayData.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimeInternalHelpers.h"
#include "AwsGameKitRuntimePublicHelpers.h"
#include "Core/AwsGameKitErrors.h"

// Unreal
#include "Async/Async.h"
#include "Templates/Function.h"

UserGameplayDataLibrary AwsGameKitUserGameplayData::GetUserGameplayDataLibraryFromModule()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitUserGameplayData::GetUserGameplayDataLibraryFromModule()"));
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    return runtimeModule->GetUserGameplayDataLibrary();
}

void AwsGameKitUserGameplayData::AddBundle(const FUserGameplayDataBundle& userGameplayDataBundle, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=] 
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

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
            UserGameplayDataBundle wrapperArgs
            {
                bundleName.c_str(),
                (bundleItemKeysChrArray.GetData()),
                (bundleItemValuesChrArray.GetData()),
                (pairCount)
            };

            result = IntResult(library.UserGameplayDataWrapper->GameKitAddUserGameplayData(library.UserGameplayDataInstanceHandle, wrapperArgs, Logging::LogCallBack));
        }

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitUserGameplayData::SetClientSettings(const FUserGameplayDataClientSettings& clientSettings)
{
    UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();

    UserGameplayDataClientSettings settings;
    settings.ClientTimeoutSeconds = clientSettings.ClientTimeoutSeconds;
    settings.RetryIntervalSeconds = clientSettings.RetryIntervalSeconds;
    settings.MaxRetryQueueSize = clientSettings.MaxRetryQueueSize;
    settings.MaxRetries = clientSettings.MaxRetries;
    settings.RetryStrategy = clientSettings.RetryStrategy;
    settings.MaxExponentialRetryThreshold = clientSettings.MaxExponentialRetryThreshold;
    settings.PaginationSize = clientSettings.PaginationSize;

    library.UserGameplayDataWrapper->GameKitSetUserGameplayDataClientSettings(library.UserGameplayDataInstanceHandle, settings, Logging::LogCallBack);
}

void AwsGameKitUserGameplayData::ListBundles(const TAwsGameKitDelegateParam<const IntResult&, const TArray<FString>&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        TArray<FString> bundles;
        IntResult result(library.UserGameplayDataWrapper->GameKitListUserGameplayDataBundles(library.UserGameplayDataInstanceHandle, bundles, Logging::LogCallBack));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, bundles);
    });
}

void AwsGameKitUserGameplayData::GetBundle(const FString& UserGameplayDataBundleName, TAwsGameKitDelegateParam<const IntResult&, const FUserGameplayDataBundle&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=] 
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        FUserGameplayDataBundle bundle;
        bundle.BundleName = UserGameplayDataBundleName;
        IntResult result(library.UserGameplayDataWrapper->GameKitGetUserGameplayDataBundle(library.UserGameplayDataInstanceHandle, bundle.BundleMap, TCHAR_TO_UTF8(*UserGameplayDataBundleName), Logging::LogCallBack));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, bundle);
    });
}

void AwsGameKitUserGameplayData::GetBundleItem(const FUserGameplayDataBundleItem& userGameplayDataBundleItem, TAwsGameKitDelegateParam<const IntResult&, const FUserGameplayDataBundleItemValue&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        FUserGameplayDataBundleItemValue bundleItem;
        bundleItem.BundleName = userGameplayDataBundleItem.BundleName;
        bundleItem.BundleItemKey = userGameplayDataBundleItem.BundleItemKey;

        FAwsGameKitInternalTempStrings ConvertString;
        UserGameplayDataBundleItem wrapperArgs
        {
            ConvertString(userGameplayDataBundleItem.BundleName),
            ConvertString(userGameplayDataBundleItem.BundleItemKey)
        };

        IntResult result(library.UserGameplayDataWrapper->GameKitGetUserGameplayDataBundleItem(library.UserGameplayDataInstanceHandle, bundleItem.BundleItemValue, wrapperArgs, Logging::LogCallBack));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, bundleItem);
    });
}

void AwsGameKitUserGameplayData::UpdateItem(const FUserGameplayDataBundleItemValue& userGameplayDataBundleItemValue, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        FAwsGameKitInternalTempStrings ConvertString;
        UserGameplayDataBundleItemValue wrapperArgs
        {
            ConvertString(*userGameplayDataBundleItemValue.BundleName),
            ConvertString(*userGameplayDataBundleItemValue.BundleItemKey),
            ConvertString(*userGameplayDataBundleItemValue.BundleItemValue)
        };

        IntResult result(library.UserGameplayDataWrapper->GameKitUpdateUserGameplayDataBundleItem(library.UserGameplayDataInstanceHandle, wrapperArgs, Logging::LogCallBack));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitUserGameplayData::DeleteAllData(FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        IntResult result(library.UserGameplayDataWrapper->GameKitDeleteAllUserGameplayData(library.UserGameplayDataInstanceHandle, Logging::LogCallBack));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitUserGameplayData::DeleteBundle(const FString& UserGameplayDataBundleName, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        IntResult result(library.UserGameplayDataWrapper->GameKitDeleteUserGameplayDataBundle(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*UserGameplayDataBundleName), Logging::LogCallBack));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitUserGameplayData::DeleteBundleItems(const FUserGameplayDataDeleteItemsRequest& userGameplayDataBundleItemsDeleteRequest, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

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
                (numKeys)
            };

            result = library.UserGameplayDataWrapper->GameKitDeleteUserGameplayDataBundleItems(library.UserGameplayDataInstanceHandle, wrapperArgs, Logging::LogCallBack);
        }

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitUserGameplayData::StartRetryBackgroundThread()
{
    UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
    library.UserGameplayDataWrapper->GameKitUserGameplayDataStartRetryBackgroundThread(library.UserGameplayDataInstanceHandle);
}

void AwsGameKitUserGameplayData::StopRetryBackgroundThread()
{
    UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
    library.UserGameplayDataWrapper->GameKitUserGameplayDataStopRetryBackgroundThread(library.UserGameplayDataInstanceHandle);
}

void AwsGameKitUserGameplayData::SetNetworkChangeDelegate(const FNetworkStatusChangeDelegate& networkStatusChangeDelegate)
{
    if (networkStatusChangeDelegate.IsBound())
    {
        FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
        runtimeModule->SetNetworkChangeDelegate(networkStatusChangeDelegate);

        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        library.UserGameplayDataWrapper->GameKitUserGameplayDataSetNetworkChangeCallback(library.UserGameplayDataInstanceHandle, runtimeModule, &FAwsGameKitRuntimeModule::OnNetworkStatusChangeDispatcher::Dispatch);
    }
}

void AwsGameKitUserGameplayData::PersistToCache(const FString& cacheFile, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        IntResult result(library.UserGameplayDataWrapper->GameKitUserGameplayDataPersistApiCallsToCache(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*cacheFile)));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitUserGameplayData::LoadFromCache(const FString& cacheFile, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        IntResult result(library.UserGameplayDataWrapper->GameKitUserGameplayDataLoadApiCallsFromCache(library.UserGameplayDataInstanceHandle, TCHAR_TO_UTF8(*cacheFile)));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

//void AwsGameKitUserGameplayData::SetNetworkChangeDelegate(const FNetworkStatusChangeDelegate& networkStatusChangeDelegate)
//{
//}


/*
InternalAwsGameKitRunLambdaOnWorkThread([=]
{
    UserGameplayDataLibrary library = GetUserGameplayDataLibraryFromModule();
    FGraphEventRef OrderedWorkChain;

    IntResult result(library.UserGameplayDataWrapper->Foo(library.UserGameplayDataInstanceHandle));

    InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
});
*/
