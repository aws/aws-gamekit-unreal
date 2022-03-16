// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "AwsGameKitCommonModels.generated.h"

USTRUCT(BlueprintType)
struct AWSGAMEKITRUNTIME_API FAwsGameKitOperationResult
{
    GENERATED_USTRUCT_BODY()
public:
    /**
     * A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h.
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

UENUM(BlueprintType)
enum class TokenType_E : uint8
{
    AccessToken = 0 UMETA(DisplayName = "AccessToken"),
    RefreshToken =1 UMETA(DisplayName = "RefreshToken"),
    IdToken = 2 UMETA(DisplayName = "IdToken"),
    IamSessionToken = 3 UMETA(DisplayName = "IamSessionToken")
};
