// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit forward declarations
class UAwsGameKitIdentityCallableWrapper;
#include "Identity/AwsGameKitIdentity.h"

// Unreal
#include "Containers/UnrealString.h"
#include "GameFramework/Actor.h"

#include "AwsGameKitIdentityExamples.generated.h" // Last include (Unreal requirement)

/**
* This class demonstrates how to call the GameKit Identity APIs through C++ code.
*
* This Actor's Details Panel let's you call the GameKit Identity APIs against your deployed AWS resources (see AWS Command Center).
*
* The Output Log shows any errors that may occur (example: an incorrect password while calling Identity::Login).
* Enable the Output Log by selecting: Window -> Developer Tools -> Output Log
*
* Copy/paste snippets of the code into your own game in order to integrate your game with GameKit Identity.
*/
UCLASS(DisplayName = "AWS GameKit Identity Examples")
class AWSGAMEKITEDITOR_API AAwsGameKitIdentityExamples : public AActor
{
    GENERATED_BODY()

private:
    /**
     * Provides the GameKit Identity APIs.
     */
    TSharedPtr<AwsGameKitIdentity> gameKitIdentity = nullptr;

    static FString GetResultMessage(unsigned int errorCode);

    /**
     * Checks if settings file is loaded and load it if it's not.
     */
    bool ReloadSettings();

    // InitializeIdentityLibrary
    bool InitializeIdentityLibrary();

    UFUNCTION(CallInEditor, Category = "1. Register Player")
    void CallRegisterApi();
    void OnRegistrationComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Register Player", DisplayName = "User Name:")
    FString RegisterUserName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Register Player", DisplayName = "Email:")
    FString RegisterEmail;

    UPROPERTY(Transient, EditInstanceOnly, Category = "1. Register Player", DisplayName = "Password:")
    FString RegisterPassword;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "1. Register Player", DisplayName = "Return Value:")
    FString RegisterReturnValue;

    // ResendConfirmationCode API
    UFUNCTION(CallInEditor, Category = "2. Resend Confirmation Code")
    void CallResendConfirmationCodeApi();
    void OnResendConfirmationComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "2. Resend Confirmation Code", DisplayName = "User Name:")
    FString ResendConfirmationCodeUserName;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "2. Resend Confirmation Code", DisplayName = "Return Value:")
    FString ResendConfirmationCodeReturnValue;

    // ConfirmEmail API
    UFUNCTION(CallInEditor, Category = "3. Confirm Email")
    void CallConfirmEmailApi();
    void OnConfirmEmailComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "3. Confirm Email", DisplayName = "User Name:")
    FString ConfirmEmailUserName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "3. Confirm Email", DisplayName = "Confirmation Code:")
    FString ConfirmEmailConfirmationCode;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "3. Confirm Email", DisplayName = "Return Value:")
    FString ConfirmEmailReturnValue;

    // IdentityLogin API
    UFUNCTION(CallInEditor, Category = "4. Login")
    void CallLoginApi();
    void OnLoginComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "4. Login", DisplayName = "User Name:")
    FString LoginUserName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "4. Login", DisplayName = "Password:")
    FString LoginPassword;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "4. Login", DisplayName = "Return Value:")
    FString LoginReturnValue;

     // IdentityGetUser API
    UFUNCTION(CallInEditor, Category = "5. GetUser")
    void CallGetUserApi();
    void OnGetUserInfoRecieved(const IntResult& result, const FGetUserResponse& userInfo);

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. GetUser", DisplayName = "Return Value:")
    FString GetUserReturnValue;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "5. GetUser", DisplayName = "Response:")
    FString GetUserOutput;

    // IdentityForgotPassword API
    UFUNCTION(CallInEditor, Category = "6. Forgot Password")
    void CallForgotPasswordApi();
    void OnForgotPasswordComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "6. Forgot Password", DisplayName = "User Name:")
    FString ForgotPasswordUserName;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "6. Forgot Password", DisplayName = "Return Value:")
    FString ForgotPasswordReturnValue;

    // IdentityConfirmForgotPassword API
    UFUNCTION(CallInEditor, Category = "7. Confirm Forgot Password")
    void CallConfirmForgotPasswordApi();
    void OnConfirmForgotPasswordComplete(const IntResult& result);

    UPROPERTY(Transient, EditInstanceOnly, Category = "7. Confirm Forgot Password", DisplayName = "User Name:")
    FString ConfirmForgotPasswordUserName;

    UPROPERTY(Transient, EditInstanceOnly, Category = "7. Confirm Forgot Password", DisplayName = "New Password:")
    FString ConfirmForgotPasswordNewPassword;

    UPROPERTY(Transient, EditInstanceOnly, Category = "7. Confirm Forgot Password", DisplayName = "Confirmation Code:")
    FString ConfirmForgotPasswordConfirmationCode;

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "7. Confirm Forgot Password", DisplayName = "Return Value:")
    FString ConfirmForgotPasswordReturnValue;

    UFUNCTION(CallInEditor, Category = "8. Open Facebook Login")
    void CallOpenFacebookLogin();
    void OnGetFacebookLoginUrlComplete(const IntResult& result, const FLoginUrlResponse& loginInfo);
    void OnCompletePollAndRetrieveFederatedTokens(const IntResult& result, const FederatedIdentityProvider_E& identityProvider);
    void OnCompleteIdentityGetIdToken(const IntResult& result, const FString& accessToken);

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "8. Open Facebook Login", DisplayName = "Return Value:")
    FString FacebookLoginReturnValue;

    // IdentityLogout API
    UFUNCTION(CallInEditor, Category = "9. Logout")
    void CallLogoutApi();
    void OnLogoutComplete(const IntResult& result);

    UPROPERTY(Transient, VisibleInstanceOnly, Category = "9. Logout", DisplayName = "Return Value:")
    FString LogoutReturnValue;

public:
    virtual void BeginDestroy() override;
    virtual bool IsEditorOnly() const override;
};
