// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit Unreal Plugin
#include "AwsGameKitLibraryUtils.h"
#include "AwsGameKitLibraryWrapper.h"
#include "Logging.h"

// GameKit
#include <aws/gamekit/core/model/account_credentials.h>
#include <aws/gamekit/core/model/account_info.h>
#include <aws/gamekit/core/model/resource_environment.h>

// Standard Library
#include <string>

// Unreal
#include "Containers/UnrealString.h"

#define GAMEKIT_ACCOUNTINFO_CHAR_PTR_VIEW(accountInfoCopy) \
{ \
    GameKit::AccountInfo{ \
        accountInfoCopy.environment.GetEnvironmentString().c_str(), \
        accountInfoCopy.accountId.c_str(), \
        accountInfoCopy.companyName.c_str(), \
        accountInfoCopy.gameName.c_str() \
    } \
}

#define GAMEKIT_ACCOUNTCREDENTIALS_CHAR_PTR_VIEW(accountCredentialsCopy) \
{ \
    GameKit::AccountCredentials{ \
        accountCredentialsCopy.region.c_str(), \
        accountCredentialsCopy.accessKey.c_str(), \
        accountCredentialsCopy.accessSecret.c_str() \
    } \
}

using namespace GameKit;

struct AccountDetails
{
    FString environment;
    FString accountId;
    FString gameName;
    FString region;
    FString accessKey;
    FString accessSecret;

    GameKit::AccountInfoCopy CreateAccountInfoCopy() const
    {
        GameKit::AccountInfoCopy accountInfoCopy;
        accountInfoCopy.environment = GameKit::ResourceEnvironment(environment.IsEmpty()? "dev" : TCHAR_TO_UTF8(*environment));
        accountInfoCopy.accountId = TCHAR_TO_UTF8(*accountId);
        accountInfoCopy.gameName = TCHAR_TO_UTF8(*gameName);

        return accountInfoCopy;
    }

    GameKit::AccountCredentialsCopy CreateAccountCredentialsCopy() const
    {
        GameKit::AccountCredentialsCopy credentialsCopy;
        credentialsCopy.region = TCHAR_TO_UTF8(*region);
        credentialsCopy.accessKey = TCHAR_TO_UTF8(*accessKey);
        credentialsCopy.accessSecret = TCHAR_TO_UTF8(*accessSecret);

        return credentialsCopy;
    }
};

enum class TemplateType { Base, Instance };
