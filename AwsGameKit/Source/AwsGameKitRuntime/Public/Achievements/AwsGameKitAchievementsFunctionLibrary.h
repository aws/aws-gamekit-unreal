// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "Common/AwsGameKitBlueprintCommon.h"
#include "Models/AwsGameKitAchievementModels.h"

// Unreal
#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AwsGameKitAchievementsFunctionLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FDelegateOnListAchievementsResultReceived, const FListAchievementsRequest&, Request, const TArray<FAchievement>&, PartialResults, bool, bIsLastResult);

/**
 * @brief This class provides Blueprint APIs for an achievements system where players can earn awards for their gameplay prowess.
 */
UCLASS()
class AWSGAMEKITRUNTIME_API UAwsGameKitAchievementsFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_UCLASS_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Achievements", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void GetAchievementIconsBaseUrl(
        UObject* WorldContextObject,
        struct FLatentActionInfo LatentInfo,
        FString& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Lists non-hidden achievements configured for this game.
     *
     * Secret achievements will be included, and developers will be responsible for processing those as they see fit.
     *
     * @param ListAchievementsRequest Information about whether the delegate should execute after every page, and what the page size should be.
     * @param OnPartialResults Delegate to execute after a page has returned.
     * @param Results Contains a list of achievement structs which contain all metadata and progress info for each achievement.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Achievements", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure", AutoCreateRefTerm = "OnPartialResults"))
    static void ListAchievementsForPlayer(
        UObject* WorldContextObject,
        struct FLatentActionInfo LatentInfo,
        const FListAchievementsRequest& ListAchievementsRequest,
        const FDelegateOnListAchievementsResultReceived OnPartialResults,
        TArray<FAchievement>& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Increments the currently logged in user's progress on a specific achievement.
     *
     * @param AchievementId The ID of the achievement you are updating.
     * @param Results UStruct containing all metadata and the updated player progress for the achievement.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     * - GAMEKIT_ERROR_ACHIEVEMENTS_INVALID_ID: The Achievement ID given is empty or malformed.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Achievements", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure", AutoCreateRefTerm = "OnPartialResults"))
    static void UpdateAchievementForPlayer(
        UObject* WorldContextObject,
        struct FLatentActionInfo LatentInfo,
        const FUpdateAchievementRequest& UpdateAchievementsRequest,
        FAchievement& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Gets the specified achievement for currently logged in user, and passes it to ResultDelegate
     *
     * @param AchievementId The ID of the achievement you are retrieving.
     * @param Results UStruct containing all metadata and player progress of the retrieved achievement.
     * @param Error Ustruct containing a GameKit status code and optional error message.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (Aws GameKit Identity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     * - GAMEKIT_ERROR_ACHIEVEMENTS_INVALID_ID: The Achievement ID given is empty or malformed.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Achievements", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure", AutoCreateRefTerm = "OnPartialResults"))
    static void GetAchievementForPlayer(
        UObject* WorldContextObject,
        struct FLatentActionInfo LatentInfo,
        const FString& AchievementId,
        FAchievement& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);
};
