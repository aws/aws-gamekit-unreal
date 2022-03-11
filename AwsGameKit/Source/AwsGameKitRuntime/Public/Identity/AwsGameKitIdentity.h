// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimePublicHelpers.h"
#include "Core/AwsGameKitErrors.h"
#include "Identity/AwsGameKitIdentityWrapper.h"
#include "Models/AwsGameKitIdentityModels.h"

// Unreal
#include "CoreMinimal.h"

/**
 * @brief This class provides APIs for signing players into your game.
 *
 * **Important:** The Identity & Authentication feature is a pre-requisite for all other features.
 *
 * You must sign in a player before most GameKit APIs will work. After signing in, GameKit will internally store and
 * refresh the player's access tokens, and pass the access tokens to all API calls that require authentication.
 *
 * ## Login Mechanisms
 * Players can log in through either of two mechanisms:
 * - Email and password, by calling Login().
 * - A federated identity provider's webpage, by calling GetFederatedLoginUrl() followed by PollAndRetrieveFederatedTokens().
 *
 * A player is free to switch between either login mechanism. It doesn't matter whether they first register through email and
 * password, or through a federated identity provider.
 *
 * ### Email and Password
 * The following methods support email and password based sign in:
 * - Register()
 * - ConfirmRegistration()
 * - ResendConfirmationCode()
 * - Login()
 * - Logout()
 * - ForgotPassword()
 * - ConfirmForgotPassword()
 *
 * ### Federated Identity Providers
 * The following methods support sign in through a federated identity provider:
 * - GetFederatedLoginUrl()
 * - PollAndRetrieveFederatedTokens()
 * - GetFederatedIdToken()
 * - Logout()
 *
 * Note that by signing into the federated identity provider at the webpage provided by GetFederatedLoginUrl(),
 * the player automatically is registered and confirmed in the Identity & Authentication feature.
 */
class AWSGAMEKITRUNTIME_API AwsGameKitIdentity
{
private:
    static IdentityLibrary GetIdentityLibraryFromModule();

public:
    /**
     * @brief Register a new player for email and password based sign in.
     *
     * @details After calling this method, you must call ConfirmRegistration() to confirm the player's identity.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param OnCompleteDelegate The delegate to invoke when this method has completed. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_MALFORMED_PASSWORD: The provided Password is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_METHOD_NOT_IMPLEMENTED: You attempted to register a guest, which is not yet supported. To fix, make sure the request's FUserRegistrationRequest::UserId field is empty.
     * - GAMEKIT_ERROR_REGISTER_USER_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    static void Register(const FUserRegistrationRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Confirm registration of a new player that was registered through Register().
     *
     * @details The confirmation code is sent to the player's email and can be re-sent by calling ResendConfirmationCode().
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param OnCompleteDelegate The delegate to invoke when this method has completed. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_CONFIRM_REGISTRATION_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    static void ConfirmRegistration(const FConfirmRegistrationRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Resend the registration confirmation code to the player's email.
     *
     * @details This resends the confirmation code that was sent by calling Register() or ResendConfirmationCode().
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param OnCompleteDelegate The delegate to invoke when this method has completed. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_RESEND_CONFIRMATION_CODE_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    static void ResendConfirmationCode(const FResendConfirmationCodeRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Send a password reset code to the player's email.
     *
     * @details After calling this method, you must call ConfirmForgotPassword() to complete the password reset.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param OnCompleteDelegate The delegate to invoke when this method has completed. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_FORGOT_PASSWORD_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    static void ForgotPassword(const FForgotPasswordRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Set the player's new password.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param OnCompleteDelegate The delegate to invoke when this method has completed. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_MALFORMED_USERNAME: The provided UserName is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_MALFORMED_PASSWORD: The provided Password is malformed. Check the output logs to see what the required format is.
     * - GAMEKIT_ERROR_CONFIRM_FORGOT_PASSWORD_FAILED: The backend web request failed. Check the output logs to see what the error was.
     */
    static void ConfirmForgotPassword(const FConfirmForgotPasswordRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Get a login/signup URL for the specified federated identity provider.
     *
     * @details Players will be able to register and/or sign in when the URL is opened in a web browser.
     *
     * @details You should call PollAndRetrieveFederatedTokens() afterward to sign the player into GameKit.
     *
     * @param IdentityProvider The federated identity provider to get the login URL for.
     * @param ResultDelegate The delegate to invoke when this method has completed. The delegate's **FLoginUrlResponse parameter** contains the login URL and a unique
     * request ID, or two empty strings if the call failed. The delegate's ::IntResult parameter is a GameKit status code and indicates the result of the API call.
     * Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_INVALID_FEDERATED_IDENTITY_PROVIDER: The specified federated identity provider is invalid or is not yet supported.
     */
    static void GetFederatedLoginUrl(const FederatedIdentityProvider_E& IdentityProvider, TAwsGameKitDelegateParam<const IntResult&, const FLoginUrlResponse&> ResultDelegate);

    /**
     * @brief Continually check if the player has completed signing in with the federated identity provider, then store their access tokens in the AwsGameKitSessionManager.
     *
     * @details After calling this method, the player will be signed in and you'll be able to call the other GameKit APIs.
     * This method stores the player's authorized access tokens in the AWS GameKit Session Manager, which automatically refreshes them before they expire.
     *
     * @details To call this method, you must first call GetFederatedLoginUrl() to get a unique request ID.
     *
     * @details This method will timeout after the specified limit (FPollAndRetrieveFederatedTokensRequest::Timeout), in which case the player is not logged in.
     * You can call GetFederatedIdToken() to check if the login was successful.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param ResultDelegate The delegate to invoke when this method has completed. The delegate's **FederatedIdentityProvider_E parameter** specifies which
     * federated identity provider was polled. This is the same provider as given in the Request object's FPollAndRetrieveFederatedTokensRequest::IdentityProvider field.
     * The delegate's ::IntResult parameter is a GameKit status code and indicates the result of the API call. Status codes are defined in errors.h.
     * This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    static void PollAndRetrieveFederatedTokens(const FPollAndRetrieveFederatedTokensRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FederatedIdentityProvider_E&> ResultDelegate);

    /**
     * @brief Get the player's authorized Id token for the specified federated identity provider.
     *
     * @details The returned access token will be empty if the player is not logged in with the federated identity provider.
     *
     * @param IdentityProvider The federated identity provider to get the player's Id tokens for.
     * @param ResultDelegate The delegate to invoke when this method has completed. The delegate's **FString parameter** is the player's authorized Id token,
     * or an empty string if the call failed or the player is not logged in with the federated identity provider. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_INVALID_FEDERATED_IDENTITY_PROVIDER: The specified federated identity provider is invalid or is not yet supported.
     */
    static void GetFederatedIdToken(const FederatedIdentityProvider_E& IdentityProvider, TAwsGameKitDelegateParam<const IntResult&, const FString&> ResultDelegate);

    /**
     * @brief Sign in the player through email and password.
     *
     * @details After calling this method, the player will be signed in and you'll be able to call the other GameKit APIs.
     * This method stores the player's authorized access tokens in the AwsGameKitSessionManager, and automatically refreshes them before they expire.
     *
     * @param Request A struct containing all parameters required to call this method.
     * @param OnCompleteDelegate The delegate to invoke when this method has completed. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    static void Login(const FUserLoginRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Sign out the currently logged in player.
     *
     * @details This revokes the player's access tokens and clears them from the AwsGameKitSessionManager.
     *
     * @param OnCompleteDelegate The delegate to invoke when this method has completed. The delegate's ::IntResult parameter is a GameKit status code and
     * indicates the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     */
    static void Logout(FAwsGameKitStatusDelegateParam OnCompleteDelegate);

    /**
     * @brief Get information about the currently logged in player.
     *
     * @details The response is a JSON string containing the following information (or an empty string if the call failed):
     * - The date time when the player was registered.
     * - The date time of the last time the player's identity information was modified.
     * - The player's GameKit ID.
     *
     * @param ResultDelegate The delegate to invoke when this method has completed. The delegate's **FString parameter** is the player's user information represented as
     * a JSON string, or an empty string if the call failed. See above for details. The delegate's ::IntResult parameter is a GameKit status code and indicates
     * the result of the API call. Status codes are defined in errors.h. This method's possible status codes are listed below:
     * - GAMEKIT_SUCCESS: The API call was successful.
     * - GAMEKIT_ERROR_NO_ID_TOKEN: The player is not logged in.
     * - GAMEKIT_ERROR_HTTP_REQUEST_FAILED: The backend HTTP request failed. Check the output logs to see what the HTTP response code was
     * - GAMEKIT_ERROR_PARSE_JSON_FAILED: The backend returned a malformed JSON payload. This should not happen. If it does, it indicates there is a bug in the backend code.
     */
    static void GetUser(TAwsGameKitDelegateParam<const IntResult&, const FGetUserResponse&> ResultDelegate);
};
