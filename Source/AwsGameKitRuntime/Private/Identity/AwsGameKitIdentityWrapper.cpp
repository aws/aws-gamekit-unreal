// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Identity/AwsGameKitIdentityWrapper.h"

// GameKit
#include "AwsGameKitCore.h"
#include "Core/AwsGameKitErrors.h"


const FString AwsGameKitIdentityWrapper::KEY_FEDERATED_LOGIN_URL_REQUEST_ID = "requestId";
const FString AwsGameKitIdentityWrapper::KEY_FEDERATED_LOGIN_URL = "loginUrl";

void AwsGameKitIdentityWrapper::importFunctions(void* loadedDllHandle)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitIdentityWrapper::importFunctions()"));

    LOAD_PLUGIN_FUNC(GameKitIdentityInstanceCreateWithSessionManager, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityRegister, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityConfirmRegistration, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityResendConfirmationCode, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityLogin, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityLogout, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityGetUser, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityForgotPassword, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityConfirmForgotPassword, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitIdentityInstanceRelease, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetFederatedLoginUrl, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitPollAndRetrieveFederatedTokens, loadedDllHandle);
    LOAD_PLUGIN_FUNC(GameKitGetFederatedAccessToken, loadedDllHandle);
}

GAMEKIT_IDENTITY_INSTANCE_HANDLE AwsGameKitIdentityWrapper::GameKitIdentityInstanceCreateWithSessionManager(void* sessionManager, FuncLogCallback logCb)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityInstanceCreateWithSessionManager, nullptr);

    return INVOKE_FUNC(GameKitIdentityInstanceCreateWithSessionManager, sessionManager, logCb);
}

void AwsGameKitIdentityWrapper::GameKitIdentityInstanceRelease(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityInstanceRelease);

    INVOKE_FUNC(GameKitIdentityInstanceRelease, identityInstance);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityRegister(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, UserRegistration userRegistration)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityRegister, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityRegister, identityInstance, userRegistration);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityConfirmRegistration(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ConfirmRegistrationRequest request)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityConfirmRegistration, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityConfirmRegistration, identityInstance, request);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityResendConfirmationCode(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ResendConfirmationCodeRequest request)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityResendConfirmationCode, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityResendConfirmationCode, identityInstance, request);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityLogin(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, UserLogin userLogin)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityLogin, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityLogin, identityInstance, userLogin);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityLogout(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityLogout, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityLogout, identityInstance);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityGetUser(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, DISPATCH_RECEIVER_HANDLE dispatchReceiver, CharPtrCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityGetUser, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityGetUser, identityInstance, dispatchReceiver, responseCallback);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityForgotPassword(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ForgotPasswordRequest request)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityForgotPassword, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityForgotPassword, identityInstance, request);
}

unsigned int AwsGameKitIdentityWrapper::GameKitIdentityConfirmForgotPassword(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ConfirmForgotPasswordRequest request)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitIdentityConfirmForgotPassword, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitIdentityConfirmForgotPassword, identityInstance, request);
}

unsigned int AwsGameKitIdentityWrapper::GameKitGetFederatedLoginUrl(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, DISPATCH_RECEIVER_HANDLE dispatchReceiver, KeyValueCharPtrCallbackDispatcher responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitGetFederatedLoginUrl, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitGetFederatedLoginUrl, identityInstance, identityProvider, dispatchReceiver, responseCallback);
}

void AwsGameKitIdentityWrapper::GameKitPollAndRetrieveFederatedTokens(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, const char* requestId, int timeout)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitPollAndRetrieveFederatedTokens);

    INVOKE_FUNC(GameKitPollAndRetrieveFederatedTokens, identityInstance, identityProvider, requestId, timeout);
}

unsigned int AwsGameKitIdentityWrapper::GameKitGetFederatedAccessToken(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, DISPATCH_RECEIVER_HANDLE dispatchReceiver, CharPtrCallback responseCallback)
{
    CHECK_PLUGIN_FUNC_IS_LOADED(Identity, GameKitGetFederatedAccessToken, GameKit::GAMEKIT_ERROR_GENERAL);

    return INVOKE_FUNC(GameKitGetFederatedAccessToken, identityInstance, identityProvider, dispatchReceiver, responseCallback);
}

#undef LOCTEXT_NAMESPACE
