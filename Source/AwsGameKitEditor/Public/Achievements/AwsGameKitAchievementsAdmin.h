// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimePublicHelpers.h"
#include "Achievements/AwsGameKitAchievementsAdminWrapper.h"
#include "Models/AwsGameKitAchievementModels.h"

// Unreal
#include "Containers/UnrealString.h"
#include "UObject/NoExportTypes.h"

struct AdminAchievement
{
    FString achievementId;
    FString title;
    FString lockedDescription;
    FString unlockedDescription;
    FString lockedIcon;
    FString unlockedIcon;
    int32 requiredAmount;
    int32 points;
    int32 sortOrder;
    bool isStateful;
    bool isSecret;
    bool isHidden;
    bool localLockedIcon;
    bool localUnlockedIcon;
};

struct AddAchievementsRequest
{
    TArray<AdminAchievement> achievements;
};

struct DeleteAchievementsRequest
{
    TArray<FString> achievementIdentifiers;
};

struct AchievementsAdminLibrary
{
    TSharedPtr<AwsGameKitAchievementsAdminWrapper> AchievementsAdminWrapper;
    GAMEKIT_ACHIEVEMENTS_INSTANCE_HANDLE AchievementsInstanceHandle = nullptr;
};

class AWSGAMEKITEDITOR_API AwsGameKitAchievementsAdmin
{
private:
    static AchievementsAdminLibrary GetAchievementsAdminLibrary();
    static TSharedPtr<AccountCredentialsCopy> accountCredCopy;
    static TSharedPtr<AccountInfoCopy> accountInfoCopy;
    static AchievementsAdminLibrary achievementsAdminLibrary;

    static bool UpdateCredentials();

public:

    /**
     * @brief Lists all game achievements, and will call PartialResultDelegate after every page.
     *
     * @param ListAchievementsRequest USTRUCT that contains what the page size should be (default 100), and whether the delegates should callback after every page.
     * @param PartialResultDelegate Delegate that processes the most recently listed page of achievements.
     * @param OperationCompleteDelegate Delegate that processes the status code after all pages have been listed.
    */
    static void ListAchievementsForGame(const FListAchievementsRequest& ListAchievementsRequest,
        TAwsGameKitDelegateParam<const TArray<AdminAchievement>&> PartialResultDelegate,
        FAwsGameKitStatusDelegateParam OperationCompleteDelegate);

    /**
     * @brief Lists non-hidden achievements, and will only call the delegate after all pages are returned.
     *
     * @param CombinedResultDelegate Delegate to process both the status code, and the returned array of achievements.
     * The **IntResult parameter** is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_REGION_CODE_CONVERSION_FAILED: The current region isn't in our template of shorthand region codes, unknown if the region is supported.
     * - GAMEKIT_ERROR_SIGN_REQUEST_FAILED: Was unable to sign the internal http request with account credentials and info, possibly because they do not have sufficient permissions.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
    */
    static void ListAchievementsForGame(TAwsGameKitDelegateParam<const IntResult&, const TArray<AdminAchievement>&> CombinedResultDelegate)
    {
        const FListAchievementsRequest request = { 100, true };
        TAwsGameKitResultArrayGatherer<AdminAchievement> Gather(CombinedResultDelegate);
        ListAchievementsForGame(request, Gather.OnResult(), Gather.OnStatus());
    }

    /**
     * @brief Add all achievements passed to the games dynamoDB achievements table.
     *
     * @param AddAchievementsRequest Struct containing a list of structs that hold an achievements metadata.
     * @param ResultDelegate Delegate function that will react to the status code of the API call.
     * The **IntResult parameter** is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_ACHIEVEMENTS_ICON_UPLOAD_FAILED: Was unable to take the local path given of an image and upload it to S3.
     * - GAMEKIT_ERROR_REGION_CODE_CONVERSION_FAILED: The current region isn't in our template of shorthand region codes, unknown if the region is supported.
     * - GAMEKIT_ERROR_SIGN_REQUEST_FAILED: Was unable to sign the internal http request with account credentials and info, possibly because they do not have sufficient permissions.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
    */
    static void AddAchievementsForGame(const AddAchievementsRequest& AddAchievementsRequest, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate);

    /**
     * @brief Deletes the set of achievements metadata from the game's dynamodb.
     *
     * @param DeleteAchievementsRequest Struct containing a list of achievement ID's.
     * @param ResultDelegate Delegate function that will react to the status code of the API call.
     * The **IntResult parameter** is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in AwsGameKitErrors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_REGION_CODE_CONVERSION_FAILED: The current region isn't in our template of shorthand region codes, unknown if the region is supported.
     * - GAMEKIT_ERROR_SIGN_REQUEST_FAILED: Was unable to sign the internal http request with account credentials and info, possibly because they do not have sufficient permissions.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the logs to see what the HTTP response code was.
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
    */
    static void DeleteAchievementsForGame(const DeleteAchievementsRequest& DeleteAchievementsRequest, TAwsGameKitDelegateParam<const IntResult&> ResultDelegate);

    /**
     * @brief Helper method to experiement with this feature. Adds a handful of pre-configured achievements to your game.
     *
     * @param ResultDelegate Delegate function that will react to the status code of the API call.
    */
    static void AddSampleData(TAwsGameKitDelegateParam<const IntResult&> ResultDelegate);

    /**
     * @brief Helper method to experiement with this feature. Deletes achivements added by AddSampleData()
     *
     * @param ResultDelegate Delegate function that will react to the status code of the API call.
    */
    static void DeleteSampleData(TAwsGameKitDelegateParam<const IntResult&> ResultDelegate);

    /**
     * @brief Helper method to convert from the json representation of an achievement to the struct version
     *
     * @param achievementData Json representation of the achievement configuration.
     * @return Struct with all the achievement configuration as members.
    */
    static AdminAchievement GetAdminAchievementFromJsonResponse(const TSharedPtr<FJsonObject>& achievementData)
    {
        AdminAchievement achievement;

        SetStringField(achievementData, achievement.achievementId, "achievement_id");
        SetStringField(achievementData, achievement.title, "title");
        SetStringField(achievementData, achievement.lockedDescription, "locked_description");
        SetStringField(achievementData, achievement.unlockedDescription, "unlocked_description");
        SetStringField(achievementData, achievement.lockedIcon, "locked_icon_url");
        SetStringField(achievementData, achievement.unlockedIcon, "unlocked_icon_url");

        SetNumberField(achievementData, achievement.requiredAmount, "max_value");
        SetNumberField(achievementData, achievement.points, "points");
        SetNumberField(achievementData, achievement.sortOrder, "order_number");

        achievement.isStateful = achievement.requiredAmount > 0;
        SetBoolField(achievementData, achievement.isSecret, "is_secret");
        SetBoolField(achievementData, achievement.isHidden, "is_hidden");
        SetBoolField(achievementData, achievement.localLockedIcon, "local_locked_icon");
        SetBoolField(achievementData, achievement.localUnlockedIcon, "local_unlocked_icon");

        return achievement;
    }

    /**
     * @brief Helper method to convert from a list of achievements in a json response or local file, and add the corresponding structs to the output argument.
     *
     * @param output Where structs reflecting the json encoded response string will be copied to.
     * @param response Json encoded string reprepresenting a list of achievements.
     * @param fromCloud Whether this response came from AWS Lambda, since the json schema is different.
    */
    static void GetListOfAdminAchievementsFromResponse(TArray<AdminAchievement>& output, const FString& response, bool fromCloud=true)
    {
        TSharedPtr<FJsonObject> JsonParsed;
        TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(response);

        if (!FJsonSerializer::Deserialize(JsonReader, JsonParsed))
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsAdmin::GetListOfAdminAchievementsFromResponse(): Could not deserialize json response."));
            return;
        }

        if (fromCloud)
        {
            JsonParsed = JsonParsed->GetObjectField("data");
        }
        TArray<TSharedPtr<FJsonValue>> achievements = JsonParsed->GetArrayField("achievements");

        for (auto a : achievements)
        {
            TSharedPtr<FJsonObject> ach = a.ToSharedRef()->AsObject();
            output.Add(GetAdminAchievementFromJsonResponse(ach));
        }
    }

    static void SetStringField(const TSharedPtr<FJsonObject>& data, FString& field, const FString& key)
    {
        if (data->HasField(key))
        {
            field = data->GetStringField(key);
        }
    }

    static void SetNumberField(const TSharedPtr<FJsonObject>& data, int32& field, const FString& key)
    {
        if (data->HasField(key))
        {
            field = data->GetNumberField(key);
        }
    }

    static void SetBoolField(const TSharedPtr<FJsonObject>& data, bool& field, const FString& key)
    {
        if (data->HasField(key))
        {
            field = data->GetBoolField(key);
        }
    }
};
