// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "Common/AwsGameKitBlueprintCommon.h"
#include "Models/AwsGameKitIdentityModels.h"

// Unreal
#include "CoreMinimal.h"
#include "Engine/LatentActionManager.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AwsGameKitIdentityFunctionLibrary.generated.h"

/**
 * This class provides Blueprint APIs for signing players into your game.
 *
 * See AwsGameKitIdentity for details on login mechanisms and further info.
 */
UCLASS()
class AWSGAMEKITRUNTIME_API UAwsGameKitIdentityFunctionLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_UCLASS_BODY()
public:
    /**
     * Register a new player for email and password based sign in.
     *
     * After calling this method, you must call ConfirmRegistration() to confirm the player's identity.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_MALFORMED_PASSWORD: The provided Password is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_METHOD_NOT_IMPLEMENTED: You attempted to register a guest, which is not yet supported. To fix, make sure the request's FUserRegistrationRequest::UserId field is empty.
     * - GAMEKIT_ERROR_REGISTER_USER_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void Register(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FUserRegistrationRequest& Request,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Confirm registration of a new player that was registered through Register().
     *
     * The confirmation code is sent to the player's email and can be re-sent by calling ResendConfirmationCode().
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_CONFIRM_REGISTRATION_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void ConfirmRegistration(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FConfirmRegistrationRequest& Request,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Resend the registration confirmation code to the player's email.
     *
     * This resends the confirmation code that was sent by calling Register() or ResendConfirmationCode().
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_RESEND_CONFIRMATION_CODE_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void ResendConfirmationCode(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FResendConfirmationCodeRequest& Request,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Send a password reset code to the player's email.
     *
     * After calling this method, you must call ConfirmForgotPassword() to complete the password reset.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_FORGOT_PASSWORD_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void ForgotPassword(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FForgotPasswordRequest& Request,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Set the player's new password.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_MALFORMED_PASSWORD: The provided Password is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_CONFIRM_FORGOT_PASSWORD_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void ConfirmForgotPassword(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FConfirmForgotPasswordRequest& Request,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Get a login/signup URL for the specified federated identity provider.
     *
     * Players will be able to register and/or sign in when the URL is opened in a web browser.
     *
     * You should PollAndRetrieveFederatedTokens() afterward to sign the player into GameKit.
     *
     * @param IdentityProvider The federated identity provider to get the login URL for.
     * @param Results A struct containing the login URL and a unique request ID. On failure, the struct contains empty strings.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_INVALID_FEDERATED_IDENTITY_PROVIDER: The specified federated identity provider is invalid or is not yet supported.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void GetFederatedLoginUrl(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        FederatedIdentityProvider_E IdentityProvider,
        FLoginUrlResponse& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Continually check if the player has completed signing in with the federated identity provider, then store their access tokens in the AWS GameKit Session Manager.
     *
     * After calling this method, the player will be signed in and you'll be able to call the other GameKit APIs.
     * This method stores the player's authorized access tokens in the AWS GameKit Session Manager, which automatically refreshes them before they expire.
     *
     * To call this method, you must first call GetFederatedLoginUrl() to get a unique request ID.
     *
     * This method will timeout after the specified limit (FPollAndRetrieveFederatedTokensRequest::Timeout), in which case the player is not logged in.
     * You can call GetFederatedIdToken() to check if the login was successful.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Results The federated identity provider that was polled. This is the same provider that was given as an input parameter.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void PollAndRetrieveFederatedTokens(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FPollAndRetrieveFederatedTokensRequest& Request,
        FederatedIdentityProvider_E& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Get the player's authorized Id token for the specified federated identity provider.
     *
     * The returned Id token will be empty if the player is not logged in with the federated identity provider.
     *
     * @param IdentityProvider The federated identity provider to get the player's Id tokens for.
     * @param Results The player's authorized Id token, or an empty string if the call failed or the player is not logged in with the federated identity provider.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_INVALID_FEDERATED_IDENTITY_PROVIDER: The specified federated identity provider is invalid or is not yet supported.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void GetFederatedIdToken(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FederatedIdentityProvider_E& IdentityProvider,
        FString& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Sign in the player through email and password.
     *
     * After calling this method, the player will be signed in and you'll be able to call the other GameKit APIs.
     * This method stores the player's authorized access tokens in the AWS GameKit Session Manager, and automatically refreshes them before they expire.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void Login(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        const FUserLoginRequest& Request,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Sign out the currently logged in player.
     *
     * This revokes the player's access tokens and clears them from the AWS GameKit Session Manager.
     *
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void Logout(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);

    /**
     * Get information about the currently logged in player.
     *
     * The response is a JSON string containing the following information (or an empty string if the call failed):
     * - The date time when the player was registered.
     * - The date time of the last time the player's identity information was modified.
     * - The player's GameKit ID.
     *
     * @param Results The player's user information represented as a JSON string, or an empty string if the call failed. See this function's tooltip for details on the JSON string.
     * @param Error A GameKit status code indicating the reason the API call failed. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the output logs to see what the HTTP response code was
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Identity", meta = (WorldContext = "WorldContextObject", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "SuccessOrFailure"))
    static void GetUser(
        UObject* WorldContextObject,
        FLatentActionInfo LatentInfo,
        FGetUserResponse& Results,
        EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
        FAwsGameKitOperationResult& Error);
};
