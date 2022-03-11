// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsGameKitCommonModels.h"
#include "AwsGameKitSessionManagerModels.generated.h"

USTRUCT(BlueprintType)
struct FSetTokenRequest
{
    GENERATED_BODY()

    /**
     * The token type to set.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | SessionManager | Token")
    TokenType_E TokenType;

    /**
     * The value of the token
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | SessionManager | Token")
    FString TokenValue;
};
