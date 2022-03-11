// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsGameKitCommonModels.h"
#include "UserGameplayData/AwsGameKitUserGameplayDataWrapper.h"

// Unreal
#include "Containers/UnrealString.h"

#include "AwsGameKitUserGameplayDataModels.generated.h"

/**
 *@struct FUserGameplayDataBundleItem
 *@brief Struct that stores information needed to reference a single item contained in a bundle
 */
USTRUCT(BlueprintType)
struct FUserGameplayDataBundleItem
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | BundleItem")
    FString BundleName;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | BundleItem")
    FString BundleItemKey;
};

/**
 *@struct FUserGameplayDataBundleItemValue
 *@brief Struct that stores information needed to update a single item contained in a bundle
 */
USTRUCT(BlueprintType)
struct FUserGameplayDataBundleItemValue
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | BundleItemValue")
    FString BundleName;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | BundleItemValue")
    FString BundleItemKey;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | BundleItemValue")
    FString BundleItemValue;
};

/**
 *@struct FUserGameplayDataDeleteItemsRequest
 *@brief Struct that stores information needed to update a single item contained in a bundle
 */
USTRUCT(BlueprintType)
struct FUserGameplayDataDeleteItemsRequest
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | DeleteItemsRequest")
    FString BundleName;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | DeleteItemsRequest")
    TArray<FString> BundleItemKeys;
};

/**
 *@struct FUserGameplayDataBundle
 *@brief Struct that stores information about a bundle
 */
USTRUCT(BlueprintType)
struct FUserGameplayDataBundle
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Bundle")
    FString BundleName;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Bundle")
    TMap<FString, FString> BundleMap;
};

/**
 *@struct FUserGameplayDataClientSettings
 *@brief Struct that stores the User Gameplay Data API Client settings
 */
USTRUCT(BlueprintType)
struct FUserGameplayDataClientSettings
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Settings")
    int32 ClientTimeoutSeconds = 4;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Settings")
    int32 RetryIntervalSeconds = 5;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Settings")
    int32 MaxRetryQueueSize = 256;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Settings")
    int32 MaxRetries = 32;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Settings")
    int32 RetryStrategy = 0;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Settings")
    int32 MaxExponentialRetryThreshold = 32;

    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | User Gameplay Data | Settings")
    int32 PaginationSize = 100;
};