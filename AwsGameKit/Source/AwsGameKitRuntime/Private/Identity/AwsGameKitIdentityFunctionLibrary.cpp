// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Identity/AwsGameKitIdentityFunctionLibrary.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitRuntimeInternalHelpers.h"
#include "Core/AwsGameKitDispatcher.h"
#include "Core/AwsGameKitErrors.h"

// Unreal
#include "LatentActions.h"

UAwsGameKitIdentityFunctionLibrary::UAwsGameKitIdentityFunctionLibrary(const FObjectInitializer& Initializer)
    : UBlueprintFunctionLibrary(Initializer)
{}

void UAwsGameKitIdentityFunctionLibrary::Register(
    UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FUserRegistrationRequest& Request,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::Register()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();
            FAwsGameKitInternalTempStrings ConvertString;
            UserRegistration wrapperArgs
            {
                ConvertString(Request.UserName),
                ConvertString(Request.Password),
                ConvertString(Request.Email),
                ConvertString(Request.UserId),
                ConvertString(Request.UserIdHash),
            };

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityRegister(identityLibrary.IdentityInstanceHandle, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::ConfirmRegistration(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FConfirmRegistrationRequest& Request,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::ConfirmRegistration()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();
            FAwsGameKitInternalTempStrings ConvertString;
            ConfirmRegistrationRequest wrapperArgs
            {
                ConvertString(Request.UserName),
                ConvertString(Request.ConfirmationCode)
            };

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityConfirmRegistration(identityLibrary.IdentityInstanceHandle, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::ResendConfirmationCode(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FResendConfirmationCodeRequest& Request,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::ResendConfirmationCode()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();
            FAwsGameKitInternalTempStrings ConvertString;
            ResendConfirmationCodeRequest wrapperArgs
            {
                ConvertString(Request.UserName),
            };

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityResendConfirmationCode(identityLibrary.IdentityInstanceHandle, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::ForgotPassword(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FForgotPasswordRequest& Request,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::ForgotPassword()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();
            FAwsGameKitInternalTempStrings ConvertString;
            ForgotPasswordRequest wrapperArgs
            {
                ConvertString(Request.UserName),
            };

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityForgotPassword(identityLibrary.IdentityInstanceHandle, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::ConfirmForgotPassword(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FConfirmForgotPasswordRequest& Request,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::ConfirmForgotPassword()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();
            FAwsGameKitInternalTempStrings ConvertString;
            ConfirmForgotPasswordRequest wrapperArgs
            {
                ConvertString(Request.UserName),
                ConvertString(Request.NewPassword),
                ConvertString(Request.ConfirmationCode),
            };

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityConfirmForgotPassword(identityLibrary.IdentityInstanceHandle, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::GetFederatedLoginUrl(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    FederatedIdentityProvider_E IdentityProvider,
    FLoginUrlResponse& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::GetFederatedLoginUrl()"));

    TAwsGameKitInternalActionStatePtr<FLoginUrlResponse> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, IdentityProvider, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([IdentityProvider, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();

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

            State->Results = FLoginUrlResponse{ *requestId, *loginUrl };
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::PollAndRetrieveFederatedTokens(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FPollAndRetrieveFederatedTokensRequest& Request,
    FederatedIdentityProvider_E& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::PollAndRetrieveFederatedTokens()"));

    TAwsGameKitInternalActionStatePtr<FederatedIdentityProvider_E> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();

            IntResult result(identityLibrary.IdentityWrapper->GameKitPollAndRetrieveFederatedTokens(
                identityLibrary.IdentityInstanceHandle,
                AwsGameKitIdentityTypeConverter::ConvertProviderEnum(Request.IdentityProvider),
                TCHAR_TO_UTF8(*Request.RequestId),
                Request.Timeout));
            State->Results = Request.IdentityProvider;
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::GetFederatedIdToken(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FederatedIdentityProvider_E& IdentityProvider,
    FString& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::GetFederatedIdToken()"));

    TAwsGameKitInternalActionStatePtr<FString> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, IdentityProvider, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([IdentityProvider, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();

            FString accessToken;
            auto getUserInfoDispatcher = [&](const char* response)
            {
                accessToken = UTF8_TO_TCHAR(response);
            };
            typedef LambdaDispatcher<decltype(getUserInfoDispatcher), void, const char*> GetUserInfoDispatcher;
            IntResult result(identityLibrary.IdentityWrapper->GameKitGetFederatedIdToken(
                identityLibrary.IdentityInstanceHandle,
                AwsGameKitIdentityTypeConverter::ConvertProviderEnum(IdentityProvider),
                &getUserInfoDispatcher,
                GetUserInfoDispatcher::Dispatch));

            State->Results = accessToken;
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::Login(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FUserLoginRequest& Request,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::Login()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();
            FAwsGameKitInternalTempStrings ConvertString;
            UserLogin wrapperArgs
            {
                ConvertString(Request.UserName),
                ConvertString(Request.Password),
            };

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityLogin(identityLibrary.IdentityInstanceHandle, wrapperArgs));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::Logout(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::Logout()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityLogout(identityLibrary.IdentityInstanceHandle));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}

void UAwsGameKitIdentityFunctionLibrary::GetUser(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    FGetUserResponse& Results,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitIdentityBlueprintFunctionLibrary::GetUser()"));

    TAwsGameKitInternalActionStatePtr<FGetUserResponse> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error, Results))
    {
        Action->LaunchThreadedWork([State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            IdentityLibrary identityLibrary = runtimeModule->GetIdentityLibrary();

            auto getUserInfoDispatcher = [&](const GetUserResponse* getUserResponse)
            {
                FGetUserResponse response;
                response.UserId = UTF8_TO_TCHAR(getUserResponse->userId);
                response.CreatedAt = UTF8_TO_TCHAR(getUserResponse->createdAt);
                response.UpdatedAt = UTF8_TO_TCHAR(getUserResponse->updatedAt);
                response.FacebookExternalId = UTF8_TO_TCHAR(getUserResponse->facebookExternalId);
                response.FacebookRefId = UTF8_TO_TCHAR(getUserResponse->facebookRefId);
                response.UserName = UTF8_TO_TCHAR(getUserResponse->userName);
                response.Email = UTF8_TO_TCHAR(getUserResponse->email);

                State->Results = response;
            };
            typedef LambdaDispatcher<decltype(getUserInfoDispatcher), void, const GetUserResponse*> GetUserInfoDispatcher;

            IntResult result = IntResult(identityLibrary.IdentityWrapper->GameKitIdentityGetUser(identityLibrary.IdentityInstanceHandle, &getUserInfoDispatcher, GetUserInfoDispatcher::Dispatch));
            State->Err = FAwsGameKitOperationResult{ static_cast<int>(result.Result), result.ErrorMessage };
        });
    }
}
