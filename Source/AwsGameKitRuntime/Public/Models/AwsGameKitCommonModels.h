// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Json.h"
#include "AwsGameKitCommonModels.generated.h"

USTRUCT(BlueprintType)
struct AWSGAMEKITRUNTIME_API FAwsGameKitOperationResult
{
    GENERATED_USTRUCT_BODY()
public:
    /**
     * A GameKit status code indicating the reason the API call failed. Status codes are defined in AwsGameKitErrors.h.
     * Each API's possible status codes should be documented on the "Error" pin of the API node.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AWS GameKit | Core")
    int Status;

    /**
     * An optional error message which may explain why the API call failed.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "AWS GameKit | Core")
    FString Message;
};

UENUM(BlueprintType)
enum class FeatureType_E : uint8
{
    Main = 0 UMETA(DisplayName = "Main"),
    Identity = 1 UMETA(DisplayName = "Identity"),
    Authentication = 2 UMETA(DisplayName = "Authentication"),
    Achievements = 3 UMETA(DisplayName = "Achievements"),
    GameStateCloudSaving = 5 UMETA(DisplayName = "Game State Cloud Saving"),
    UserGameplayData = 6 UMETA(DisplayName = "User Gameplay Data")
};
