// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once
#include "Achievements/AwsGameKitAchievements.h"

// GameKit
#include "AwsGameKitEditor/Public/ImageDownloader.h"
#include "AwsGameKitRuntime/Public/Models/AwsGameKitAchievementModels.h"

// Unreal
#include "UObject/NoExportTypes.h"
#include "GameFramework/Actor.h"
#include "Containers/UnrealString.h"

#include "AwsGameKitAchievementsExamples.generated.h" // Last include (Unreal requirement)

/**
* This class demonstrates how to call the GameKit Achievement player APIs through C++ code.
*
* This Actor's Details Panel let's you call the GameKit Achievements APIs against your deployed AWS resources (see AWS Command Center).
*
*
* Copy/paste snippets of the code into your own game in order to integrate your game with GameKit Achievements.
*/
UCLASS(DisplayName = "AWS GameKit Achievement Examples")
class AWSGAMEKITEDITOR_API AAwsGameKitAchievementsExamples : public AActor
{
    GENERATED_BODY()

private:
    static FString GetResultMessage(unsigned int errorCode);

    FString BaseIconUrl = "";
    IImageDownloaderPtr ImageDownloaderInstance;

    /**
     * Checks if settings file is loaded and load it if it's not.
     */
    bool ReloadSettings();
    bool InitializeAchievementsLibrary();

    /**
     * Checks if settings file is loaded for identity and load it if it's not.
     */
    bool ReloadIdentitySettings();
    bool InitializeIdentityLibrary();

    // Get Icon Base Url Delegate
    void OnGetIconBaseUrlComplete(const IntResult& result, const FString& url);

    // Login
    UFUNCTION(CallInEditor, Category = "1. Login")
    void CallLoginApi();
    void OnLoginComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Login", DisplayName = "User Name:")
    FString LoginUserName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Login", DisplayName = "Password:")
    FString LoginPassword;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "1. Login", DisplayName = "Return Value:")
    FString LoginReturnValue;

    // Populate/Delete Sample Achievements
    UFUNCTION(CallInEditor, Category = "2. Sample Data")
    void AddSampleData();

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "2. Sample Data", DisplayName = "Add Data Return Value:")
    FString AddDataReturnValue;

    UFUNCTION(CallInEditor, Category = "2. Sample Data")
    void DeleteSampleData();

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "2. Sample Data", DisplayName = "Delete Data Return Value:")
    FString DeleteDataReturnValue;

    // List Achievements
    UFUNCTION(CallInEditor, Category = "3. List Achievements")
    void CallListAchievementsForPlayerApi();
    void OnListAchievementsComplete(const IntResult& result, const TArray<FAchievement>& achievements);

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "3. List Achievements", DisplayName = "Return Value:")
    FString ListPlayerAchievementsReturnValue;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "3. List Achievements", DisplayName = "Response:")
    TArray<FAchievement> ListPlayerAchievementsResponse;

    // Get Achievement
    UFUNCTION(CallInEditor, Category = "4. Get Achievement")
    void CallGetAchievementForPlayerApi();
    void OnGetAchievementComplete(const IntResult& result, const FAchievement& achievement);

    UPROPERTY(Transient, EditInstanceOnly, Category = "4. Get Achievement", DisplayName = "Achievement ID:")
    FString GetAchievementId;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "4. Get Achievement", DisplayName = "Return Value:")
    FString GetAchievementReturnValue;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "4. Get Achievement", DisplayName = "Response:")
    FAchievement GetAchievementResponse;

    // Update Achievement
    UFUNCTION(CallInEditor, Category = "5. Update Achievement")
    void CallUpdateAchievementForPlayerApi();
    void OnUpdateAchievementComplete(const IntResult& result, const FAchievement& achievement);

    UPROPERTY(Transient, EditInstanceOnly, Category = "5. Update Achievement", DisplayName = "Achievement ID:")
    FString UpdateAchievementId;

    UPROPERTY(Transient, EditInstanceOnly, Category = "5. Update Achievement", DisplayName = "Increment Amount:")
    FString UpdateAchievementIncrement;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. Update Achievement", DisplayName = "Return Value:")
    FString UpdateAchievementReturnValue;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. Update Achievement", DisplayName = "Response:")
    FAchievement UpdateAchievementResponse;

    /*
     * ADVANCED USAGE: Only use this if you're using your own identity provider and you've set UseThirdPartyIdentityProvider to true in the feature's parameters.yml
     * Uncomment the UFUNCTION() and UPROPERTY() macros to have this example show up in the Details panel of this example actor.
     */
    // Set Token
    // UFUNCTION(CallInEditor, Category = "Set Token")
    void CallSetTokenApi();

    // UPROPERTY(Transient, EditInstanceOnly, Category = "Set Token", DisplayName = "Id Token")
    FString IdTokenValue;

public:
    virtual void BeginDestroy() override;
    virtual bool IsEditorOnly() const override;
};
