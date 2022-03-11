// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "UserGameplayData/AwsGameKitUserGameplayDataStateHandler.h"

void AwsGameKitUserGameplayDataStateHandler::SetCacheProcessedDelegate(const FCacheProcessedDelegate& cacheProcessedDelegate)
{
    if (!this->onCacheProcessedDelegate.IsBound() && cacheProcessedDelegate.IsBound()) {
        this->onCacheProcessedDelegate = cacheProcessedDelegate;
    }
}
