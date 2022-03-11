// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Identity/AwsGameKitIdentity.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimeInternalHelpers.h"
#include "AwsGameKitRuntimePublicHelpers.h"

// Unreal
#include "Async/Async.h"
#include "Templates/Function.h"

IdentityLibrary AwsGameKitIdentity::GetIdentityLibraryFromModule()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitIdentity::GetIdentityLibraryFromModule()"));
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    return runtimeModule->GetIdentityLibrary();
}

void AwsGameKitIdentity::Register(const FUserRegistrationRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;

        FAwsGameKitInternalTempStrings ConvertString;
        UserRegistration wrapperArgs
        {
            ConvertString(Request.UserName),
            ConvertString(Request.Password),
            ConvertString(Request.Email),
            ConvertString(Request.UserId),
            ConvertString(Request.UserIdHash),
        };

        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityRegister(identityLibrary.IdentityInstanceHandle, wrapperArgs));
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitIdentity::ConfirmRegistration(const FConfirmRegistrationRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        FAwsGameKitInternalTempStrings ConvertString;
        ConfirmRegistrationRequest wrapperArgs
        {
            ConvertString(Request.UserName),
            ConvertString(Request.ConfirmationCode),
        };

        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityConfirmRegistration(identityLibrary.IdentityInstanceHandle, wrapperArgs));
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitIdentity::ResendConfirmationCode(const FResendConfirmationCodeRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        FAwsGameKitInternalTempStrings ConvertString;
        ResendConfirmationCodeRequest wrapperArgs
        {
            ConvertString(Request.UserName),
        };

        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityResendConfirmationCode(identityLibrary.IdentityInstanceHandle, wrapperArgs));
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitIdentity::ForgotPassword(const FForgotPasswordRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        FAwsGameKitInternalTempStrings ConvertString;
        ForgotPasswordRequest wrapperArgs
        {
            ConvertString(Request.UserName),
        };

        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityForgotPassword(identityLibrary.IdentityInstanceHandle, wrapperArgs));
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitIdentity::ConfirmForgotPassword(const FConfirmForgotPasswordRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        FAwsGameKitInternalTempStrings ConvertString;
        ConfirmForgotPasswordRequest wrapperArgs
        {
            ConvertString(*Request.UserName),
            ConvertString(*Request.NewPassword),
            ConvertString(*Request.ConfirmationCode),
        };

        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityConfirmForgotPassword(identityLibrary.IdentityInstanceHandle, wrapperArgs));
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitIdentity::GetFederatedLoginUrl(const FederatedIdentityProvider_E& IdentityProvider, TAwsGameKitDelegateParam<const IntResult&, const FLoginUrlResponse&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();
        FGraphEventRef OrderedWorkChain;

        TMap<FString, FString> loginUrlInfo;
        auto loginUrlInfoSetter = [&loginUrlInfo](const char* key, const char* value)
        {
            loginUrlInfo.Add(key, value);
        };
        typedef LambdaDispatcher<decltype(loginUrlInfoSetter), void, const char*, const char*> LoginUrlInfoSetter;

        IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitGetFederatedLoginUrl(
            identityLibrary.IdentityInstanceHandle,
            AwsGameKitIdentityTypeConverter::ConvertProviderEnum(IdentityProvider),
            &loginUrlInfoSetter,
            LoginUrlInfoSetter::Dispatch));

        const FString requestId = loginUrlInfo.FindRef(AwsGameKitIdentityWrapper::KEY_FEDERATED_LOGIN_URL_REQUEST_ID);
        const FString loginUrl = loginUrlInfo.FindRef(AwsGameKitIdentityWrapper::KEY_FEDERATED_LOGIN_URL);
        const FLoginUrlResponse loginUrlResponse = FLoginUrlResponse{ *requestId, *loginUrl };
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, loginUrlResponse);
    });
}

void AwsGameKitIdentity::PollAndRetrieveFederatedTokens(const FPollAndRetrieveFederatedTokensRequest& Request, TAwsGameKitDelegateParam<const IntResult&, const FederatedIdentityProvider_E&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        IntResult result(identityLibrary.IdentityWrapper->GameKitPollAndRetrieveFederatedTokens(identityLibrary.IdentityInstanceHandle, AwsGameKitIdentityTypeConverter::ConvertProviderEnum(Request.IdentityProvider), TCHAR_TO_UTF8(*Request.RequestId), Request.Timeout));
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, Request.IdentityProvider);
    });
}

void AwsGameKitIdentity::GetFederatedIdToken(const FederatedIdentityProvider_E& IdentityProvider, TAwsGameKitDelegateParam<const IntResult&, const FString&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        FString accessToken;
        auto getUserInfoDispatcher = [&](const char* response)
        {
            accessToken = UTF8_TO_TCHAR(response);
        };
        typedef LambdaDispatcher<decltype(getUserInfoDispatcher), void, const char*> GetUserInfoDispatcher;
        IntResult result(identityLibrary.IdentityWrapper->GameKitGetFederatedIdToken(identityLibrary.IdentityInstanceHandle, AwsGameKitIdentityTypeConverter::ConvertProviderEnum(IdentityProvider), &getUserInfoDispatcher, GetUserInfoDispatcher::Dispatch));
        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, accessToken);
    });
}

void AwsGameKitIdentity::Login(const FUserLoginRequest& Request, FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        FAwsGameKitInternalTempStrings ConvertString;
        UserLogin wrapperArgs
        {
            ConvertString(Request.UserName),
            ConvertString(Request.Password),
        };
        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityLogin(identityLibrary.IdentityInstanceHandle, wrapperArgs));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitIdentity::Logout(FAwsGameKitStatusDelegateParam OnCompleteDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FGraphEventRef OrderedWorkChain;
        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityLogout(identityLibrary.IdentityInstanceHandle));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, OnCompleteDelegate, result);
    });
}

void AwsGameKitIdentity::GetUser(TAwsGameKitDelegateParam<const IntResult&, const FGetUserResponse&> ResultDelegate)
{
    InternalAwsGameKitRunLambdaOnWorkThread([=]
    {
        FGraphEventRef OrderedWorkChain;
        IdentityLibrary identityLibrary = GetIdentityLibraryFromModule();

        FString userInfo;
        FGetUserResponse response;

        auto getUserInfoDispatcher = [&](const GetUserResponse* getUserResponse)
        {
            response.UserId = UTF8_TO_TCHAR(getUserResponse->userId);
            response.CreatedAt = UTF8_TO_TCHAR(getUserResponse->createdAt);
            response.UpdatedAt = UTF8_TO_TCHAR(getUserResponse->updatedAt);
            response.FacebookExternalId = UTF8_TO_TCHAR(getUserResponse->facebookExternalId);
            response.FacebookRefId = UTF8_TO_TCHAR(getUserResponse->facebookRefId);
            response.UserName = UTF8_TO_TCHAR(getUserResponse->userName);
            response.Email = UTF8_TO_TCHAR(getUserResponse->email);
        };
        typedef LambdaDispatcher<decltype(getUserInfoDispatcher), void, const GetUserResponse*> GetUserInfoDispatcher;

        IntResult result(identityLibrary.IdentityWrapper->GameKitIdentityGetUser(identityLibrary.IdentityInstanceHandle, &getUserInfoDispatcher, GetUserInfoDispatcher::Dispatch));

        InternalAwsGameKitRunDelegateOnGameThread(OrderedWorkChain, ResultDelegate, result, response);
    });
}
