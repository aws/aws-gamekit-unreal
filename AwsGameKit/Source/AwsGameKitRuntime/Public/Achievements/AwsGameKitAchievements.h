// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsGameKitRuntime/Public/Models/AwsGameKitAchievementModels.h"

// GameKit
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimePublicHelpers.h"
#include "Achievements/AwsGameKitAchievementsWrapper.h"
#include "Core/AwsGameKitErrors.h"

// Unreal
#include "Modules/ModuleManager.h"

/**
 * @brief This class provides APIs for an achievements system where players can earn awards for their gameplay prowess.
 */
class AWSGAMEKITRUNTIME_API AwsGameKitAchievements
{
private:
    static AchievementsLibrary GetAchievementsLibraryFromModule();
public:
    /**
     * @brief Lists non-hidden achievements, and will call delegates after every page.
     *
     * @details Secret achievements will be included, and developers will be responsible for processing those as they see fit.
     *
     * @param ListAchievementsRequest USTRUCT that contains what the page size should be (default 100), and whether the delegates should callback after every page.
     * @param PartialResultDelegate Delegate that processes the most recently listed page of achievements.
     * @param OperationCompleteDelegate Delegate that processes the status code after all pages have been listed.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
    */
    static void ListAchievementsForPlayer(const FListAchievementsRequest& ListAchievementsRequest,
        TAwsGameKitDelegateParam<const TArray<FAchievement>&> PartialResultDelegate,
        FAwsGameKitStatusDelegateParam OperationCompleteDelegate);

    /**
     * @brief Lists non-hidden achievements, and will only call the delegate after all pages are returned.
     *
     * @details Secret achievements will be included, and developers will be responsible for processing those as they see fit.
     *
     * @param CombinedResultDelegate Delegate to process both the status code, and the returned array of achievements.
     * The ::IntResult parameter is a GameKit status code and indicates the result of the API call. 
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
    */
    static void ListAchievementsForPlayer(TAwsGameKitDelegateParam<const IntResult&, const TArray<FAchievement>&> CombinedResultDelegate)
    {
        const FListAchievementsRequest request = { 100, true };
        TAwsGameKitResultArrayGatherer<FAchievement> Gather(CombinedResultDelegate);
        ListAchievementsForPlayer(request, Gather.OnResult(), Gather.OnStatus());
    }

    /**
     * @brief Gets the specified achievement for currently logged in user, and passes it to ResultDelegate
     *
     * @param GetAchievementRequest USTRUCT specifying the achievment ID.
     * @param ResultDelegate Delegate that processes the status code and returned achievement.
     * The ::IntResult parameter is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     * - GAMEKIT_ERROR_ACHIEVEMENTS_INVALID_ID: The Achievement ID given is empty or malformed.
    */
    static void GetAchievementForPlayer(const FGetAchievementRequest& GetAchievementRequest,
        TAwsGameKitDelegateParam<const IntResult&, const FAchievement&> ResultDelegate);

    /**
     * @brief Increments the currently logged in user's progress on a specific achievement.
     *
     * @param UpdateAchievementRequest USTRUCT containing the achievement ID, and how much to increment the player's progress by.
     * @param ResultDelegate Delegate that processes the status code and updated achievement which
     * contains info about whether it was just earned.
     * The ::IntResult parameter is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in. You must login the player through the Identity & Authentication feature (AwsGameKitIdentity) before calling this method.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
    */
    static void UpdateAchievementForPlayer(const FUpdateAchievementRequest& UpdateAchievementRequest,
        TAwsGameKitDelegateParam<const IntResult&, const FAchievement&> ResultDelegate);

    /**
     * @brief Gets the AWS CloudFront url which all achievement icons for this game/environment can be accessed from.
     *
     * @param ResultDelegate Delegate that processes the status code and returned Url.
     * The ::IntResult parameter is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
    */
    static void GetAchievementIconBaseUrl(TAwsGameKitDelegateParam<const IntResult&, const FString&> ResultDelegate);
};
