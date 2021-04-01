// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Identity/AwsGameKitIdentityExamples.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "FeatureResourceManager.h"
#include "Core/AwsGameKitErrors.h"

void AAwsGameKitIdentityExamples::BeginDestroy()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::BeginDestroy()"));
    Super::BeginDestroy();
}

bool AAwsGameKitIdentityExamples::IsEditorOnly() const
{
    return true;
}

bool AAwsGameKitIdentityExamples::ReloadSettings()
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");

    if (runtimeModule->AreFeatureSettingsLoaded(FeatureType::Identity))
    {
        return true;
    }

    FAwsGameKitEditorModule* editorModule = AWSGAMEKIT_EDITOR_MODULE_INSTANCE();
    return runtimeModule->ReloadConfigFile(editorModule->GetFeatureResourceManager()->GetClientConfigSubdirectory());
}

/**
 * Must be called before using any of the Identity APIs.
 *
 * Load the DLL and create a GameKitIdentity instance.
 */
bool AAwsGameKitIdentityExamples::InitializeIdentityLibrary()
{
    /*
     * This check is only meant for the examples.
     * This is to ensure that the Identity/Authentication feature has been deployed before running any of the sample code.
     */
    if (!ReloadSettings())
    {
        FMessageDialog::Open(EAppMsgType::Ok, FText::FromString("You need to deploy the Identity/Authentication feature first."));
        return false;
    }

    return true;
}

void AAwsGameKitIdentityExamples::CallRegisterApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallRegisterApi() called with parameters: UserName=%s, Email=%s, Password=<password hidden>"), *RegisterUserName, *RegisterEmail);

    const FUserRegistrationRequest request{
        RegisterUserName,
        RegisterPassword,
        RegisterEmail,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnRegistrationComplete);
    AwsGameKitIdentity::Register(request, Delegate);
};

void AAwsGameKitIdentityExamples::OnRegistrationComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnRegistrationComplete()"));
    RegisterReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitIdentityExamples::CallResendConfirmationCodeApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallResendConfirmationCodeApi() called with parameters: UserName=%s"), *ResendConfirmationCodeUserName);

    const FResendConfirmationCodeRequest request{
        ResendConfirmationCodeUserName,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnResendConfirmationComplete);
    AwsGameKitIdentity::ResendConfirmationCode(request, Delegate);
}

void AAwsGameKitIdentityExamples::OnResendConfirmationComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnResendConfirmationComplete()"));
    ResendConfirmationCodeReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitIdentityExamples::CallConfirmEmailApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallConfirmEmailApi() called with parameters: UserName=%s, ConfirmationCode=%s"), *ConfirmEmailUserName, *ConfirmEmailConfirmationCode);

    const FConfirmRegistrationRequest request{
        ConfirmEmailUserName,
        ConfirmEmailConfirmationCode,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnConfirmEmailComplete);
    AwsGameKitIdentity::ConfirmRegistration(request, Delegate);
}

void AAwsGameKitIdentityExamples::OnConfirmEmailComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnConfirmEmailComplete()"));
    ConfirmEmailReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitIdentityExamples::CallLoginApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallLoginApi() called with parameters: UserName=%s, Password=<password hidden>"), *LoginUserName);

    const FUserLoginRequest request{
        LoginUserName,
        LoginPassword,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnLoginComplete);
    AwsGameKitIdentity::Login(request, Delegate);
}

void AAwsGameKitIdentityExamples::OnLoginComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnLoginComplete()"));
    LoginReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitIdentityExamples::CallLogoutApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallLogoutApi()"));

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnLogoutComplete);
    AwsGameKitIdentity::Logout(Delegate);
}

void AAwsGameKitIdentityExamples::OnLogoutComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnLogoutComplete()"));
    LogoutReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitIdentityExamples::CallGetUserApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallGetUserApi()"));

    const auto ResultDelegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnGetUserInfoRecieved);
    AwsGameKitIdentity::GetUser(ResultDelegate);
}

void AAwsGameKitIdentityExamples::OnGetUserInfoRecieved(const IntResult& result, const FString& userInfo)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnGetUserInfoRecieved()"));
    GetUserReturnValue = GetResultMessage(result.Result);
    GetUserOutput = userInfo;
}

void AAwsGameKitIdentityExamples::CallForgotPasswordApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallForgotPasswordApi() called with parameters: UserName=%s"), *ForgotPasswordUserName);

    const FForgotPasswordRequest request{
        ForgotPasswordUserName,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnForgotPasswordComplete);
    AwsGameKitIdentity::ForgotPassword(request, Delegate);
}

void AAwsGameKitIdentityExamples::OnForgotPasswordComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnForgotPasswordComplete()"));
    ForgotPasswordReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitIdentityExamples::CallConfirmForgotPasswordApi()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallConfirmForgotPasswordApi() called with parameters: UserName=%s, NewPassword=<password hidden>, ConfirmationCode=%s"), *ConfirmForgotPasswordUserName, *ConfirmForgotPasswordConfirmationCode);

    const FConfirmForgotPasswordRequest request{
        ConfirmForgotPasswordUserName,
        ConfirmForgotPasswordNewPassword,
        ConfirmForgotPasswordConfirmationCode,
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnConfirmForgotPasswordComplete);
    AwsGameKitIdentity::ConfirmForgotPassword(request, Delegate);
}

void AAwsGameKitIdentityExamples::OnConfirmForgotPasswordComplete(const IntResult& result)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnConfirmForgotPasswordComplete()"));
    ConfirmForgotPasswordReturnValue = GetResultMessage(result.Result);
}

void AAwsGameKitIdentityExamples::CallOpenFacebookLogin()
{
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("CallOpenFacebookLogin()"));
    if (!InitializeIdentityLibrary())
    {
        return;
    }

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnGetFacebookLoginUrlComplete);
    AwsGameKitIdentity::GetFederatedLoginUrl(FederatedIdentityProvider_E::Facebook, Delegate);
}

void AAwsGameKitIdentityExamples::OnGetFacebookLoginUrlComplete(const IntResult& result, const FLoginUrlResponse& loginInfo)
{
    // Open Facebook on a separate browser
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnGetFacebookLoginUrlComplete(): %s"), *loginInfo.LoginUrl);
    FacebookSessionAccessToken = "PLEASE WAIT...";
    FPlatformProcess::LaunchURL(*loginInfo.LoginUrl, nullptr, nullptr);

    const FPollAndRetrieveFederatedTokensRequest request{
        FederatedIdentityProvider_E::Facebook,
        loginInfo.RequestId,
        60
    };

    const auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnCompletePollAndRetrieveFederatedTokens);
    AwsGameKitIdentity::PollAndRetrieveFederatedTokens(request, Delegate);
}

void AAwsGameKitIdentityExamples::OnCompletePollAndRetrieveFederatedTokens(const IntResult& result, const FederatedIdentityProvider_E& identityProvider)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnCompletePollAndRetrieveFederatedTokens()"));
    auto Delegate = MakeAwsGameKitDelegate(this, &AAwsGameKitIdentityExamples::OnCompleteIdentityGetAccessToken);
    AwsGameKitIdentity::GetFederatedAccessToken(identityProvider, Delegate);
}

void AAwsGameKitIdentityExamples::OnCompleteIdentityGetAccessToken(const IntResult& result, const FString& accessToken)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AAwsGameKitIdentityExamples::OnCompleteIdentityGetAccessToken()"));
    FacebookSessionAccessToken = accessToken;
    UE_LOG(LogAwsGameKit, Display, TEXT("Federated (Facebook) AccessToken: %s"), *FacebookSessionAccessToken);
}

/**
 * Convert the error code into a readable string.
 */
FString AAwsGameKitIdentityExamples::GetResultMessage(unsigned int errorCode)
{
    if (errorCode == GameKit::GAMEKIT_SUCCESS)
    {
        return "GAMEKIT_SUCCESS";
    }
    else
    {
        FString hexError = GameKit::StatusCodeToHexFStr(errorCode);
        return FString::Printf(TEXT("Error code: %s. Check output log."), *hexError);
    }
}

