// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Core/AwsGameKitMarshalling.h"
#include "AwsGameKitCommonModels.h"
#include "AwsGameKitIdentityModels.h"

class AWSGAMEKITRUNTIME_API AwsGameKitEnumConverter
{
public:
    static FString FeatureToApiString(FeatureType feature)
    {
        switch (feature)
        {
        case FeatureType::Main:
            return "main";
        case FeatureType::Identity:
            return "identity";
        case FeatureType::Authentication:
            return "authentication";
        case FeatureType::Achievements:
            return "achievements";
        case FeatureType::GameStateCloudSaving:
            return "gamesaving";
        case FeatureType::UserGameplayData:
            return "usergamedata";
        default:
            return "";
        }
    }

    static FString FeatureToUIString(FeatureType feature)
    {
        switch (feature)
        {
        case FeatureType::Main:
            return "Main";
        case FeatureType::Identity:
            return "Identity And Authentication";
        case FeatureType::Authentication:
            return "Authentication";
        case FeatureType::Achievements:
            return "Achievements";
        case FeatureType::GameStateCloudSaving:
            return "Game State Cloud Saving";
        case FeatureType::UserGameplayData:
            return "User Gameplay Data";
        default:
            return "";
        }
    }

    static FString FeatureResourcesUIString(FeatureType feature)
    {
        switch (feature)
        {
        case FeatureType::Identity:
            return "API Gateway, CloudWatch, Cognito, DynamoDB, IAM, Key Management Service, and Lambda. ";
        case FeatureType::Achievements:
            return "API Gateway, CloudFront, CloudWatch, Cognito, DynamoDB, Lambda, S3, and Security Token Service. ";
        case FeatureType::GameStateCloudSaving:
            return "API Gateway, CloudWatch, Cognito, DynamoDB, Lambda, and S3. ";
        case FeatureType::UserGameplayData:
            return "API Gateway, CloudWatch, Cognito, DynamoDB, and Lambda. ";
        default:
            return "";
        }
    }

    static FString FeatureToDocumentationUrl(FeatureType feature)
    {
        switch (feature)
        {
        case FeatureType::Identity:
            return "https://docs.aws.amazon.com/gamekit/latest/DevGuide/identity-auth.html";
        case FeatureType::Achievements:
            return "https://docs.aws.amazon.com/gamekit/latest/DevGuide/achievements.html";
        case FeatureType::GameStateCloudSaving:
            return "https://docs.aws.amazon.com/gamekit/latest/DevGuide/game-state-saving.html";
        case FeatureType::UserGameplayData:
            return "https://docs.aws.amazon.com/gamekit/latest/DevGuide/gameplay-data.html";
        default:
            return "https://docs.aws.amazon.com/gamekit/index.html";
        }
    }

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

    static FeatureType ConvertFeatureEnum(FeatureType_E featureType)
    {
        switch (featureType)
        {
        case FeatureType_E::Main:
            return FeatureType::Main;
        case FeatureType_E::Identity:
            return FeatureType::Identity;
        case FeatureType_E::Authentication:
            return FeatureType::Authentication;
        case FeatureType_E::Achievements:
            return FeatureType::Achievements;
        case FeatureType_E::GameStateCloudSaving:
            return FeatureType::GameStateCloudSaving;
        case FeatureType_E::UserGameplayData:
            return FeatureType::UserGameplayData;
        default:
            return FeatureType::Main;
        }
    }
};
