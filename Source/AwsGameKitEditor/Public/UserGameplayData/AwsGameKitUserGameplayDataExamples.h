// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "Identity/AwsGameKitIdentity.h"
#include "UserGameplayData/AwsGameKitUserGameplayData.h"

// Unreal
#include "GameFramework/Actor.h"
#include "Containers/UnrealString.h"
#include "AwsGameKitUserGameplayDataExamples.generated.h" // Last include (Unreal requirement)

/**
* This class demonstrates how to call the GameKit User Gameplay Data APIs through C++ code.
*
* This Actor's Details Panel let's you call the GameKit User Gameplay Data APIs against your deployed AWS resources (see AWS Command Center).
*
* The Output Log shows any errors that may occur (example: an no corresponding bundle while calling UserGameplayData::DeleteUserGameplayDataBundle).
* Enable the Output Log by selecting: Window -> Developer Tools -> Output Log
*
* Copy/paste snippets of the code into your own game in order to integrate your game with GameKit User Gameplay Data.
*/
UCLASS(DisplayName = "AWS GameKit User Gameplay Data Examples")
class AWSGAMEKITEDITOR_API AAwsGameKitUserGameplayDataExamples : public AActor
{
    GENERATED_BODY()

private:
    /**
     * Provides the GameKit Identity APIs.
     */
    TSharedPtr<AwsGameKitIdentity> gameKitIdentity = nullptr;

    static FString GetResultMessage(unsigned int errorCode);

    static FString GetResultMessage(const IntResult& result);

    /**
     * Provides the GameKit UserGameplayData APIs.
     */
    TSharedPtr<AwsGameKitUserGameplayData> gameKitUserGameplayData = nullptr;

    /**
     * Checks if settings file is loaded and load it if it's not.
     */
    bool ReloadSettings() const;

    /**
     * Checks if settings file is loaded for identity and load it if it's not.
     */
    bool ReloadIdentitySettings() const;

    // InitializeIdentityLibrary
    bool InitializeIdentityLibrary();

    // Initialize User Gameplay Data Library
    bool InitializeUserGameplayDataLibrary();

    // Login
    UFUNCTION(CallInEditor, Category = "1. Login")
    void CallLoginApi();
    void OnLoginApiComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Login", DisplayName = "User Name:")
    FString LoginUserName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Login", DisplayName = "Password:")
    FString LoginPassword;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "1. Login", DisplayName = "Return Value:")
    FString LoginReturnValue;

    // Add bundle data
    UFUNCTION(CallInEditor, Category = "2. Add User Gameplay Data")
    void CallAddDataApi();
    void OnAddDataComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "2. Add User Gameplay Data", DisplayName = "Bundle Name:")
    FString AddBundleName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "2. Add User Gameplay Data", DisplayName = "Bundle Item Keys:")
    TArray<FString> AddBundleItemKeys;

    UPROPERTY(Transient, EditInstanceOnly, Category = "2. Add User Gameplay Data", DisplayName = "Bundle Item Values:")
    TArray<FString> AddBundleItemValues;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "2. Add User Gameplay Data", DisplayName = "Return Value:")
    FString AddReturnValue;

    // List All Bundles
    UFUNCTION(CallInEditor, Category = "3. List User Gameplay Data Bundles")
    void CallListBundlesApi();
    void OnListBundlesComplete(const IntResult& result, const TArray<FString>& values);

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "3. List User Gameplay Data Bundles", DisplayName = "Return Value:")
    TArray<FString> ListBundlesReturnValue;

    // Get Bundle
    UFUNCTION(CallInEditor, Category = "4. Get Bundle")
    void CallGetBundleApi();
    void OnGetBundleComplete(const IntResult& result, const FUserGameplayDataBundle& values);

    UPROPERTY(Transient, EditInstanceOnly, Category = "4. Get Bundle", DisplayName = "Bundle Name:")
    FString GetBundleName;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "4. Get Bundle", DisplayName = "Returned Key Value:")
    TMap<FString,FString> GetBundleKeyReturnValue;

    // Get Bundle Item
    UFUNCTION(CallInEditor, Category = "5. Get Bundle Item")
    void CallGetBundleItemApi();
    void OnGetBundleItemComplete(const IntResult& result, const FUserGameplayDataBundleItemValue& value);

    UPROPERTY(Transient, EditInstanceOnly, Category = "5. Get Bundle Item", DisplayName = "Bundle Name:")
    FString GetBundleItemBundleName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "5. Get Bundle Item", DisplayName = "Bundle Item Key:")
    FString GetBundleItemKey;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. Get Bundle Item", DisplayName = "Return Value:")
    FString GetBundleItemReturnValue;

    // Get Bundle Item
    UFUNCTION(CallInEditor, Category = "6. Update Bundle Item")
    void CallUpdateItemApi();
    void OnUpdateItemComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "6. Update Bundle Item", DisplayName = "Bundle Name:")
    FString UpdateBundleName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "6. Update Bundle Item", DisplayName = "Bundle Item Key:")
    FString UpdateBundleItemKey;

    UPROPERTY(Transient, EditInstanceOnly, Category = "6. Update Bundle Item", DisplayName = "New Value:")
    FString UpdateBundleItemValue;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "6. Update Bundle Item", DisplayName = "Return Value:")
    FString UpdateReturnValue;

    UFUNCTION(CallInEditor, Category = "7. Delete All User Gameplay Data")
    void CallDeleteAllDataApi();
    void OnDeleteAllDataComplete(const IntResult& result);

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "7. Delete All User Gameplay Data", DisplayName = "Return Value:")
    FString DeleteAllReturnValue;

    // Get Bundle
    UFUNCTION(CallInEditor, Category = "8. Delete Bundle")
    void CallDeleteBundleApi();
    void OnDeleteBundleComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "8. Delete Bundle", DisplayName = "Bundle Name:")
    FString DeleteBundleName;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "8. Delete Bundle", DisplayName = "Return Value:")
    FString DeleteBundleReturnValue;

    // Get Bundle Item
    UFUNCTION(CallInEditor, Category = "9. Delete Bundle Items")
    void CallDeleteBundleItemsApi();
    void OnDeleteBundleItemsComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "9. Delete Bundle Items", DisplayName = "Bundle Name:")
    FString DeleteBundleItemBundleName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "9. Delete Bundle Items", DisplayName = "Bundle Items Keys:")
    TArray<FString> DeleteBundleItemKeys;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "9. Delete Bundle Items", DisplayName = "Return Value:")
    FString DeleteBundleItemReturnValue;

public:
    virtual void BeginDestroy() override;
    virtual bool IsEditorOnly() const override;
};
