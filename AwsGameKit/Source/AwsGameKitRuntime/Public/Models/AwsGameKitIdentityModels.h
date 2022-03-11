// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsGameKitCommonModels.h"
#include "Identity/AwsGameKitIdentityWrapper.h"
#include "AwsGameKitIdentityModels.generated.h"

using namespace GameKit;

/**
 * The request object for AwsGameKitIdentity::Register().
 */
USTRUCT(BlueprintType)
struct FUserRegistrationRequest
{
    GENERATED_BODY()

    /**
     * The username the player wants to have. The player must type this in whenever they log in.
     *
     * This has certain character restrictions, which will be shown in the Output Log if an invalid username is provided.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString UserName;

    /**
     * The password the player wants to use.
     *
     * This has certain character restrictions, which will be shown in the Output Log if an invalid username is provided.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString Password;

    /**
     * The player's email address.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString Email;

    /**
     * Do not use. This field will be used in the future to allow guest registration.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString UserId;

    /**
     * Do not use. This field will be used in the future to allow guest registration.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString UserIdHash;
};

/**
 * The request object for AwsGameKitIdentity::ConfirmRegistration().
 */
USTRUCT(BlueprintType)
struct FConfirmRegistrationRequest
{
    GENERATED_BODY()

    /**
     * The username of the player to confirm.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | ConfirmRegistration")
    FString UserName;

    /**
     * The registration confirmation code that was emailed to the player.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | ConfirmRegistration")
    FString ConfirmationCode;
};

/**
 * The request object for AwsGameKitIdentity::ResendConfirmationCode().
 */
USTRUCT(BlueprintType)
struct FResendConfirmationCodeRequest
{
    GENERATED_BODY()

    /**
     * The username of the player to email the new confirmation code.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | ResendConfirmationCode")
    FString UserName;
};

/**
 * The request object for AwsGameKitIdentity::Login().
 */
USTRUCT(BlueprintType)
struct FUserLoginRequest
{
    GENERATED_BODY()

    /**
     * The username of the player that is logging in.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserLogin")
    FString UserName;

    /**
     * The player's password.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserLogin")
    FString Password;
};

/**
 * The request object for AwsGameKitIdentity::ForgotPassword().
 */
USTRUCT(BlueprintType)
struct FForgotPasswordRequest
{
    GENERATED_BODY()

    /**
     * The username of the player to email the reset password code to.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | ForgotPassword")
    FString UserName;
};

/**
 * The request object for AwsGameKitIdentity::ConfirmForgotPassword().
 */
USTRUCT(BlueprintType)
struct FConfirmForgotPasswordRequest
{
    GENERATED_BODY()

    /**
     * The username of the player to set a new password for.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | ConfirmForgotPassword")
    FString UserName;

    /**
     * The new password the player wants to use.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | ConfirmForgotPassword")
    FString NewPassword;

    /**
     * The password reset code that was emailed to the player.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | ConfirmForgotPassword")
    FString ConfirmationCode;
};

/**
 * The response object for AwsGameKitIdentity::GetFederatedLoginUrl().
 */
USTRUCT(BlueprintType)
struct FLoginUrlResponse
{
    GENERATED_BODY()

    /**
     * A unique request identifier. This must be provided when calling AwsGameKitIdentity::PollAndRetrieveFederatedTokens().
     *
     * This will be an empty string if the call failed.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | GetFederatedLoginUrl")
    FString RequestId;

    /**
     * The login URL for the federated identity provider.
     *
     * Open this URL in a web browser to let your player register or sign in.
     *
     * This will be an empty string if the call failed.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | GetFederatedLoginUrl")
    FString LoginUrl;
};

/**
 * The federated identity providers which are supported by the GameKit Identity & Authentication feature.
 */
UENUM(BlueprintType)
enum class FederatedIdentityProvider_E : uint8
{
    Facebook = 0 UMETA(DisplayName = "Facebook"),
};

/**
 * The request object for AwsGameKitIdentity::PollAndRetrieveFederatedTokens().
 */
USTRUCT(BlueprintType)
struct FPollAndRetrieveFederatedTokensRequest
{
    GENERATED_BODY()

    /**
     * The federated identity provider to poll against.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | PollAndRetrieveFederatedTokens")
    FederatedIdentityProvider_E IdentityProvider;

    /**
     * The unique request identifier returned in the delegate of GetFederatedLoginUrl(). See FLoginUrlResponse::RequestId.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | PollAndRetrieveFederatedTokens")
    FString RequestId;

    /**
     * The number of seconds before the method will stop polling and will return with failure.
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | PollAndRetrieveFederatedTokens")
    int Timeout;
};


/**
 * @brief The request object for AwsGameKitIdentity::GetUser().
 */
USTRUCT(BlueprintType)
struct FGetUserResponse
{
    GENERATED_BODY()

    /**
     * @brief Unique Id generated for a user on registration
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString UserId;

    /**
     * @brief Timestamp of when user was created
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString CreatedAt;

    /**
     * @brief Timestamp of when user was last updated
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString UpdatedAt;

    /**
     * @brief Players Facebook external id
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString FacebookExternalId;

    /**
     * @brief Players Facebook refrence id
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString FacebookRefId;

    /**
     * @brief Players User Name
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString UserName;

    /**
     * @brief Players Email address
     */
    UPROPERTY(BlueprintReadWrite, Category = "AWS GameKit | Identity | UserRegistration")
    FString Email;

    /**
     * Convert this struct into a human-readable string for the purpose of logging.
     */
    operator FString() const
    {
        const FString formatString = FString(TEXT("FGetUserResponse(UserId={0}, UserName={1}, CreatedAt={2}, UpdatedAt={3}, FacebookExternalId={4}, FacebookRefId={5}, Email={6})"));
        return FString::Format(*formatString, { UserId, UserName, CreatedAt, UpdatedAt, FacebookExternalId, FacebookRefId, Email});
    }
};

/**
 * @brief Offers conversion methods between Unreal-friendly types and the plain C++ types used by the Identity low level C API.
 *
 * @details This class is used internally and you probably won't need to use this.
 */
class AWSGAMEKITRUNTIME_API AwsGameKitIdentityTypeConverter
{
public:

    /**
     * @brief Convert the Blueprint-friendly enum to a plain C++ enum that is used by the Identity low level C API.
     *
     * @details This method is used internally and you probably won't need to use this.
     */
    static FederatedIdentityProvider ConvertProviderEnum(FederatedIdentityProvider_E identityProvider)
    {
        FederatedIdentityProvider provider = FederatedIdentityProvider::Facebook;
        switch (identityProvider)
        {
        case FederatedIdentityProvider_E::Facebook:
            provider = FederatedIdentityProvider::Facebook;
        default:
            break;
        }

        return provider;
    }
};
