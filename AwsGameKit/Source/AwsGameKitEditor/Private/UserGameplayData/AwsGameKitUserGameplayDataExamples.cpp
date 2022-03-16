// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "UserGameplayData/AwsGameKitUserGameplayDataExamples.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitRuntime.h"
#include "FeatureResourceManager.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/Logging.h"

void AAwsGameKitUserGameplayDataExamples::BeginDestroy()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitUserGameplayDataExamples::BeginDestroy()"));

    Super::BeginDestroy();
}

bool AAwsGameKitUserGameplayDataExamples::IsEditorOnly() const
{
    return true;
}

bool AAwsGameKitUserGameplayDataExamples::ReloadSettings() const
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    if (runtimeModule->AreFeatureSettingsLoaded(FeatureType::UserGameplayData))
    {
        return true;
    }

    /*************
    // In order to call ReloadConfigFile() outside of AwsGameKitEditor module, use the lines below:

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();
    sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
    return runtimeModule->AreFeatureSettingsLoaded(FeatureType::UserGameplayData);
    *************/

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    return runtimeModule->ReloadConfigFile(editorModule->GetFeatureResourceManager()->GetClientConfigSubdirectory());
}

bool AAwsGameKitUserGameplayDataExamples::ReloadIdentitySettings() const
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    if (runtimeModule->AreFeatureSettingsLoaded(FeatureType::Identity))
    {
        return true;
    }

    /*************
    // In order to call ReloadConfigFile() outside of AwsGameKitEditor module, use the lines below:

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();
    sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
    return runtimeModule->AreFeatureSettingsLoaded(FeatureType::Identity);
    *************/

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    return runtimeModule->ReloadConfigFile(editorModule->GetFeatureResourceManager()->GetClientConfigSubdirectory());
}

/**
 * Must be called before using any of the Identity APIs.
 *
 * Load the DLL and create a GameKitIdentity instance.
 */
bool AAwsGameKitUserGameplayDataExamples::InitializeIdentityLibrary()
{
    if (!ReloadIdentitySettings())
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("This example requires an AWS GameKit backend service for Identity/Authentication."
            " See Edit > Project Settings > Plugins > AWS GameKit to create the Identity/Authentication backend."));
        return false;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("GameKitIdentity successfully initialized."));

    return true;
}

/**
 * Must be called before using any of the User Gameplay Data APIs.
 *
 * Load the DLL and create a GameKitUserGameplayData instance.
 */
bool AAwsGameKitUserGameplayDataExamples::InitializeUserGameplayDataLibrary()
{
    /*
     * This check is only meant for the examples.
     * This is to ensure that the User Gameplay Data feature has been deployed before running any of the sample code.
     */
    if (!ReloadSettings())
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("This example requires an AWS GameKit backend service for User Gameplay Data."
            " See Edit > Project Settings > Plugins > AWS GameKit to create the User Gameplay Data backend."));
        return false;
    }

    return true;
}

void AAwsGameKitUserGameplayDataExamples::CallLoginApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallLoginApi() called with parameters: UserName=%s, Password=<password hidden>"), *LoginUserName);

    const FUserLoginRequest request
    {
        LoginUserName,
        LoginPassword,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnLoginApiComplete);
    AwsGameKitIdentity::Login(request, Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnLoginApiComplete(const IntResult& result)
{
    LoginReturnValue = GetResultMessage(result);
}

void AAwsGameKitUserGameplayDataExamples::CallAddDataApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallAddDataApi() called"));

    if (AddBundleItemKeys.Num() != AddBundleItemValues.Num())
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("CallAddDataApi() number of keys must match number of values"));
        AddBundleReturnValue = GameKit::StatusCodeToHexFStr(GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_INVALID);
        return;
    }

    TMap<FString, FString> pairs;
    const int32 count = AddBundleItemKeys.Num();
    for (int i = 0; i < count; ++i)
    {
        pairs.Add(AddBundleItemKeys[i], AddBundleItemValues[i]);
    }

    const FUserGameplayDataBundle request
    {
        AddBundleName,
        pairs
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnAddDataComplete);
    AwsGameKitUserGameplayData::AddBundle(request, Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnAddDataComplete(const IntResult& result, const FUserGameplayDataBundle& unprocessedItems)
{
    AddBundleReturnValue = GetResultMessage(result);

    // For your game, you may want to implement a retry logic here or store the items that are failing to retry later
    if (result.Result == GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_UNPROCESSED_ITEMS)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AAwsGameKitUserGameplayDataExamples::OnAddDataComplete(): Failed to process some or all items"));

        FString errorMessage = "";

        if (unprocessedItems.BundleMap.Num() == 0)
        {
            return;
        }

        for (TTuple<FString, FString> item : unprocessedItems.BundleMap)
        {
            if (!errorMessage.IsEmpty())
            {
                errorMessage.Append(", ");
            }

            errorMessage.Append("{ Key: ");
            errorMessage.Append(item.Key);
            errorMessage.Append(", ");
            errorMessage.Append("Value: ");
            errorMessage.Append(item.Value);
            errorMessage.Append(" }");
        }

        UE_LOG(LogAwsGameKit, Error, TEXT("AAwsGameKitUserGameplayDataExamples::OnAddDataComplete(): Unprocessed Items: %s"), *errorMessage);
    }
}

void AAwsGameKitUserGameplayDataExamples::CallListBundlesApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallListBundlesApi() called"));

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnListBundlesComplete);

    AwsGameKitUserGameplayData::ListBundles(Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnListBundlesComplete(const IntResult& result, const TArray<FString>& values)
{
    ListBundlesReturnValue = GetResultMessage(result);
    ListBundlesResponse = values;
}

void AAwsGameKitUserGameplayDataExamples::CallGetBundleApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallGetBundleApi() called"));

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnGetBundleComplete);

    AwsGameKitUserGameplayData::GetBundle(GetBundleName, Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnGetBundleComplete(const IntResult& result, const FUserGameplayDataBundle& values)
{
    GetBundleReturnValue = GetResultMessage(result);
    GetBundleResponse = values.BundleMap;
}

void AAwsGameKitUserGameplayDataExamples::CallGetBundleItemApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallGetBundleItemApi() called"));

    const FUserGameplayDataBundleItem request
    {
        GetBundleItemBundleName,
        GetBundleItemKey
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnGetBundleItemComplete);

    AwsGameKitUserGameplayData::GetBundleItem(request, Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnGetBundleItemComplete(const IntResult& result, const FUserGameplayDataBundleItemValue& value)
{
    GetBundleItemReturnValue = GetResultMessage(result);
    GetBundleItemResponse = value.BundleItemValue;
}

void AAwsGameKitUserGameplayDataExamples::CallUpdateItemApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallUpdateItemApi() called"));

    const FUserGameplayDataBundleItemValue request
    {
        UpdateBundleName,
        UpdateBundleItemKey,
        UpdateBundleItemValue
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnUpdateItemComplete);
    
    AwsGameKitUserGameplayData::UpdateItem(request, Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnUpdateItemComplete(const IntResult& result)
{
    UpdateReturnValue = GetResultMessage(result);
}

void AAwsGameKitUserGameplayDataExamples::CallDeleteAllDataApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallDeleteAllDataApi() called"));

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnDeleteAllDataComplete);

    AwsGameKitUserGameplayData::DeleteAllData(Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnDeleteAllDataComplete(const IntResult& result)
{
    DeleteAllReturnValue = GetResultMessage(result);
}

void AAwsGameKitUserGameplayDataExamples::CallDeleteBundleApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallDeleteBundleApi() called"));

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnDeleteBundleComplete);

    AwsGameKitUserGameplayData::DeleteBundle(DeleteBundleName, Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnDeleteBundleComplete(const IntResult& result)
{
    DeleteBundleReturnValue = GetResultMessage(result);
}

void AAwsGameKitUserGameplayDataExamples::CallDeleteBundleItemsApi()
{
    if (!InitializeUserGameplayDataLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallDeleteBundleItemsApi() called"));

    const FUserGameplayDataDeleteItemsRequest request
    {
        DeleteBundleItemBundleName,
        DeleteBundleItemKeys
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitUserGameplayDataExamples::OnDeleteBundleItemsComplete);

    AwsGameKitUserGameplayData::DeleteBundleItems(request, Delegate);
}

void AAwsGameKitUserGameplayDataExamples::OnDeleteBundleItemsComplete(const IntResult& result)
{
    DeleteBundleItemReturnValue = GetResultMessage(result);
}

/**
 * Convert the error code into a readable string.
 */
FString AAwsGameKitUserGameplayDataExamples::GetResultMessage(unsigned int errorCode)
{
    if (errorCode == GameKit::GAMEKIT_SUCCESS)
    {
        return "GAMEKIT_SUCCESS";
    }
    else
    {
        FString hexError = GameKit::StatusCodeToHexFStr(errorCode);
        return FString::Printf(TEXT("Error code: %s. Check output log."), *hexError);
    }
}

/**
 * Convert the error code into a readable string.
 */
FString AAwsGameKitUserGameplayDataExamples::GetResultMessage(const IntResult& result)
{
    return GetResultMessage(result.Result);
}
