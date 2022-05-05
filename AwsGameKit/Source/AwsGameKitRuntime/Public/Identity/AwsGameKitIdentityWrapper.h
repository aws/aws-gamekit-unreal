// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
 * @brief Interface for the Identity low level C API.
 *
 * @details Most of this code is undocumented because it would be a duplicate of what's already found in AwsGameKitIdentity and AwsGameKitIdentityModels.h.
 */

#pragma once

// GameKit Unreal Plugin
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryWrapper.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitLibraryUtils.h>
#include <AwsGameKitCore/Public/Core/AwsGameKitDispatcher.h>

// GameKit
#if PLATFORM_IOS || PLATFORM_ANDROID
#include <aws/gamekit/identity/exports.h>
#endif
#include <aws/gamekit/core/enums.h>
#include <aws/gamekit/identity/gamekit_identity_models.h>

// Standard library
#include <string>

 /**
  * @brief Pointer to an instance of an Identity class created in the imported Identity C library.
  *
  * Most GameKit C APIs require an instance handle to be passed in.
  *
  * Instance handles are stored as a void* (instead of a class type) because the GameKit C libraries expose a C-level interface (not a C++ interface).
  */
typedef void* GAMEKIT_IDENTITY_INSTANCE_HANDLE;
//typedef void(*FuncResponseCallback)(const char* responsePayload, unsigned int size);
//typedef void(*FuncIdentityGetUserResponseCallback)(DISPATCH_RECEIVER_HANDLE dispatchReceiver, const GameKit::GetUserResponse* getUserResponse);

/**
 * @brief Wrapper for the Identity low level C API.
 *
 * @details See AwsGameKitIdentity for details. That class is a higher level version of this class and has full documentation.
 */

using namespace GameKit;

class AWSGAMEKITRUNTIME_API AwsGameKitIdentityWrapper : public AwsGameKitLibraryWrapper
{
private:
    DEFINE_FUNC_HANDLE(GAMEKIT_IDENTITY_INSTANCE_HANDLE, GameKitIdentityInstanceCreateWithSessionManager, (void* sessionManager, FuncLogCallback logCb));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityRegister, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, UserRegistration userRegistration));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityConfirmRegistration, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ConfirmRegistrationRequest request));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityResendConfirmationCode, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ResendConfirmationCodeRequest request));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityLogin, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, UserLogin userLogin));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityLogout, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityGetUser, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, DISPATCH_RECEIVER_HANDLE dispatchReceiver, FuncIdentityGetUserResponseCallback responseCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityForgotPassword, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ForgotPasswordRequest request));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitIdentityConfirmForgotPassword, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ConfirmForgotPasswordRequest request));
    DEFINE_FUNC_HANDLE(void, GameKitIdentityInstanceRelease, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetFederatedLoginUrl, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, DISPATCH_RECEIVER_HANDLE dispatchReceiver, KeyValueCharPtrCallbackDispatcher responseCallback));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitPollAndRetrieveFederatedTokens, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, const char* requestId, int timeout));
    DEFINE_FUNC_HANDLE(unsigned int, GameKitGetFederatedIdToken, (GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, DISPATCH_RECEIVER_HANDLE dispatchReceiver, CharPtrCallback responseCallback));

protected:
    virtual std::string getLibraryFilename() override
    {
#if PLATFORM_WINDOWS
        return "aws-gamekit-identity";
#elif PLATFORM_MAC
        return "libaws-gamekit-identity";
#else
        return "";
#endif
    }

    virtual void importFunctions(void* loadedDllHandle) override;

public:
    static const FString KEY_FEDERATED_LOGIN_URL_REQUEST_ID;
    static const FString KEY_FEDERATED_LOGIN_URL;

    AwsGameKitIdentityWrapper() {};
    virtual ~AwsGameKitIdentityWrapper();

    GAMEKIT_IDENTITY_INSTANCE_HANDLE GameKitIdentityInstanceCreateWithSessionManager(void* sessionManager, FuncLogCallback logCb);
    virtual void GameKitIdentityInstanceRelease(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance);
    virtual unsigned int GameKitIdentityRegister(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, UserRegistration userRegistration);
    virtual unsigned int GameKitIdentityConfirmRegistration(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ConfirmRegistrationRequest request);
    virtual unsigned int GameKitIdentityResendConfirmationCode(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ResendConfirmationCodeRequest request);
    virtual unsigned int GameKitIdentityLogin(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, UserLogin userLogin);
    virtual unsigned int GameKitIdentityLogout(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance);
    virtual unsigned int GameKitIdentityGetUser(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, DISPATCH_RECEIVER_HANDLE dispatchReceiver, FuncIdentityGetUserResponseCallback responseCallback);
    virtual unsigned int GameKitIdentityForgotPassword(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ForgotPasswordRequest request);
    virtual unsigned int GameKitIdentityConfirmForgotPassword(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, ConfirmForgotPasswordRequest request);
    virtual unsigned int GameKitGetFederatedLoginUrl(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, DISPATCH_RECEIVER_HANDLE dispatchReceiver, KeyValueCharPtrCallbackDispatcher responseCallback);
    virtual unsigned int GameKitPollAndRetrieveFederatedTokens(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, const char* requestId, int timeout);
    virtual unsigned int GameKitGetFederatedIdToken(GAMEKIT_IDENTITY_INSTANCE_HANDLE identityInstance, FederatedIdentityProvider identityProvider, DISPATCH_RECEIVER_HANDLE dispatchReceiver, CharPtrCallback responseCallback);
};
