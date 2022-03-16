// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "Delegates/Delegate.h"
#include "UObject/NoExportTypes.h"

#include "AwsGameKitUserGameplayDataStateHandler.generated.h"

/**
 * @brief This class sets and keeps the state of what delegates and other long term variables are to be used for the User Gameplay Data library.
 */
UDELEGATE(BlueprintCallable, Category = "AWS GameKit | User Gameplay Data | Cache Processed Delegate")
DECLARE_DYNAMIC_DELEGATE_OneParam(FCacheProcessedDelegate, bool, isCacheProcessed);
class AWSGAMEKITRUNTIME_API AwsGameKitUserGameplayDataStateHandler
{
public:
    // Delegate that gets triggered when the offline cache is finished processing
    FCacheProcessedDelegate onCacheProcessedDelegate;

    /**
     * @brief Set the callback to invoke when the offline cache finishes processing.
     *
     * @param cacheProcessedDelegate Reference to receiver that will be notified on when the offline cache is finished processing
    */
    void SetCacheProcessedDelegate(const FCacheProcessedDelegate& cacheProcessedDelegate);
};
