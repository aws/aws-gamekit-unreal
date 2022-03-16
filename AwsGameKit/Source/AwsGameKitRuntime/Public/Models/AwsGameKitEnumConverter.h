// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

    static TokenType ConvertTokenTypeEnum(TokenType_E tokenType)
    {
        switch (tokenType)
        {
        case TokenType_E::AccessToken:
            return TokenType::AccessToken;
        case TokenType_E::RefreshToken:
            return TokenType::RefreshToken;
        case TokenType_E::IdToken:
            return TokenType::IdToken;
        case TokenType_E::IamSessionToken:
            return TokenType::IamSessionToken;
        default:
            return TokenType::AccessToken;
        }
    }
};
