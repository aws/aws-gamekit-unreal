// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsGameKitCommonModels.h"
#include "AwsGameKitAchievementModels.generated.h"

USTRUCT(BlueprintType)
struct FAchievement
{
    GENERATED_BODY()

    /**
     * Unique identifier for the achievement
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    FString AchievementId;

    /**
     * Title for the achievement, can be used for display purposes.
    */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Achievement")
    FString Title;

    /**
     * Description that should show if the achievement is unearned and/or secret.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    FString LockedDescription;

    /**
     * Description that should show after an achievement is earned.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    FString UnlockedDescription;

    /**
     * Icon path that should be concatenated onto base icon url, this icon should
     * show when the achievement is unearned or secret.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    FString LockedIcon;

    /**
     * Icon path that should be concatenated onto base icon url, this icon should
     * be shown after the achievement is earned.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    FString UnlockedIcon;

    /**
     * The current user's progress on the achievement.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    int32 CurrentValue;

    /**
     * The number of steps a player must make on the achievement before it is earned.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    int32 RequiredAmount;

    /**
     * How many points should be attributed to earning this achievement.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    int32 Points;

    /**
     * A Number you can use to sort which achievements should be displayed first.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    int32 OrderNumber;

    /**
     * Describes whether this achievement only requires one step to complete, or multiple.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    bool IsStateful;

    /**
     * A flag that can be used to filter out achievements from the player's view.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    bool IsSecret;

    /**
     * When hidden players cannot make progress on or earn the achievement, until set to false.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    bool IsHidden;

    /**
     * The completion status by the current user.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    bool IsEarned;

    /**
     * When an update is performed, this will tell whether the update completed
     * the achievement for the current user.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    bool IsNewlyEarned;

    /**
     * Timestamp of when the achievement was earned, otherwise empty.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    FString EarnedAt;

    /**
     * Timestamp of when the achievement was last updated.
    */
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Achievement")
    FString UpdatedAt;
};

USTRUCT(BlueprintType)
struct FListAchievementsRequest
{
    GENERATED_USTRUCT_BODY()
public:
    /**
     * The number achievements each page should contain when listing.
    */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Achievements")
    int32 PageSize;

    /**
     * Whether the result delegate should be called after every page, or only after all pages are retrieved.
    */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Achievements")
    bool WaitForAllPages;
};

USTRUCT(BlueprintType)
struct FUpdateAchievementRequest
{
    GENERATED_BODY()

    /**
     * Unique Achievement Identifier.
    */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit Achievement")
    FString AchievementId;

    /**
     * The amount of steps the player has progressed on this achievement since the last update.
    */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit Achievement")
    int32 IncrementBy;
};

USTRUCT(BlueprintType)
struct FGetAchievementRequest
{
    GENERATED_BODY()

    /**
     * Unique Achievement Identifier.
    */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit Achievement")
    FString AchievementId;
};

class AWSGAMEKITRUNTIME_API AwsGamekitAchievementsResponseProcessor
{
public:
    /**
     * @brief Helper to convert a Json formatted FString to an FJsonObject
     *
     * @param response Json formatted FString
     * @return FJsonObject which contains all fields from the response.
    */
    static TSharedPtr<FJsonObject> UnpackResponseAsJson(const FString& response)
    {
        TSharedPtr<FJsonObject> JsonParsed;
        TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(response);

        if (!FJsonSerializer::Deserialize(JsonReader, JsonParsed))
        {
            return JsonParsed;
        }

        return JsonParsed->GetObjectField("data");
    }

    /**
     * @brief Helper that creates an FAchievement from a JsonObject.
     *
     * @param achievementData JsonObject which contains fields only pertaining to one achievement.
     * @return FAchievement with all properties populated from the info in the JsonObject.
    */
    static FAchievement GetAchievementFromJsonResponse(const TSharedPtr<FJsonObject>& achievementData)
    {
        FAchievement achievement;
        achievement.IsNewlyEarned = false;

        SetStringField(achievementData, achievement.AchievementId, "achievement_id");
        SetStringField(achievementData, achievement.Title, "title");
        SetStringField(achievementData, achievement.LockedDescription, "locked_description");
        SetStringField(achievementData, achievement.UnlockedDescription, "unlocked_description");
        SetStringField(achievementData, achievement.LockedIcon, "locked_icon_url");
        SetStringField(achievementData, achievement.UnlockedIcon, "unlocked_icon_url");
        SetStringField(achievementData, achievement.UpdatedAt, "updated_at");
        SetStringField(achievementData, achievement.EarnedAt, "earned_at");

        SetNumberField(achievementData, achievement.RequiredAmount, "max_value");
        SetNumberField(achievementData, achievement.Points, "points");
        SetNumberField(achievementData, achievement.OrderNumber, "order_number");
        SetNumberField(achievementData, achievement.CurrentValue, "current_value");

        achievement.IsStateful = achievement.RequiredAmount > 1;
        SetBoolField(achievementData, achievement.IsSecret, "is_secret");
        SetBoolField(achievementData, achievement.IsHidden, "is_hidden");
        SetBoolField(achievementData, achievement.IsEarned, "earned");
        SetBoolField(achievementData, achievement.IsNewlyEarned, "newly_earned");

        return achievement;
    }

    /**
     * @brief Helper that returns mutliple FAchievements in the output parameter given a Json formatted response FString.
     *
     * @param output Where all FAchievement objects will be copied to.
     * @param response Json formatted response string, that will be passed to the delegate for ListAchievementsForPlayer() API.
    */
    static void GetListOfAchievementsFromResponse(TArray<FAchievement>& output, const FString& response)
    {
        TSharedPtr<FJsonObject> json = UnpackResponseAsJson(response);
        TArray<TSharedPtr<FJsonValue>> achievements = json->GetArrayField("achievements");
        for (TSharedPtr<FJsonValue> a : achievements)
        {
            TSharedPtr<FJsonObject> ach = a.ToSharedRef()->AsObject();
            output.Add(GetAchievementFromJsonResponse(ach));
        }
    }

    static void SetStringField(const TSharedPtr<FJsonObject>& data, FString& field, const FString& key)
    {
        data->TryGetStringField(key, field);
    }

    static void SetNumberField(const TSharedPtr<FJsonObject>& data, int32& field, const FString& key)
    {
        data->TryGetNumberField(key, field);
    }

    static void SetBoolField(const TSharedPtr<FJsonObject>& data, bool& field, const FString& key)
    {
        data->TryGetBoolField(key, field);
    }
};
