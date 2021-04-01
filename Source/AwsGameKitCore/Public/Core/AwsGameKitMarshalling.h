// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitLibraryUtils.h"
#include "AwsGameKitLibraryWrapper.h"
#include "Logging.h"

// Standard Library
#include <string>

// Unreal
#include "Containers/UnrealString.h"

/**
 * Structs/Enums
 */
struct AccountInfo
{
    const char* environment;
    const char* accountId;
    const char* companyName;
    const char* gameName;
};

struct AccountCredentials
{
    const char* region;
    const char* accessKey;
    const char* accessSecret;
};

struct AccountDetails
{
    FString environment;
    FString accountId;
    FString gameName;
    FString region;
    FString accessKey;
    FString accessSecret;
};

// Struct for deep-copy of account info, use only in Unreal.
struct AccountInfoCopy
{
    std::string environment;
    std::string accountId;
    std::string companyName;
    std::string gameName;

    AccountInfoCopy() {}

    // Make a deep-copy of AccountDetails
    AccountInfoCopy(const AccountDetails& accountDetails) :
        environment(TCHAR_TO_UTF8(*accountDetails.environment)),
        accountId(TCHAR_TO_UTF8(*accountDetails.accountId)),
        gameName(TCHAR_TO_UTF8(*accountDetails.gameName))
    {}

    // Use to create an AccountInfo that points to this instance
    const AccountInfo ToCharPtrView() const
    {
        AccountInfo accountInfo;
        accountInfo.environment = environment.c_str();
        accountInfo.accountId = accountId.c_str();
        accountInfo.companyName = companyName.c_str();
        accountInfo.gameName = gameName.c_str();

        return accountInfo;
    }
};

// Struct for deep-copy of account info, use only in Unreal.
struct AccountCredentialsCopy
{
    std::string region;
    std::string accessKey;
    std::string accessSecret;

    AccountCredentialsCopy() {}

    // Make a deep-copy of AccountDetails
    AccountCredentialsCopy(const AccountDetails& accountDetails) :
        region(TCHAR_TO_UTF8(*accountDetails.region)),
        accessKey(TCHAR_TO_UTF8(*accountDetails.accessKey)),
        accessSecret(TCHAR_TO_UTF8(*accountDetails.accessSecret))
    {}

    // Use to create an AccountCredentials that points to this instance
    const AccountCredentials ToCharPtrView() const
    {
        AccountCredentials credentials;
        credentials.region = region.c_str();
        credentials.accessKey = accessKey.c_str();
        credentials.accessSecret = accessSecret.c_str();

        return credentials;
    }
};

FString AWSGAMEKITCORE_API FeatureToApiString(FeatureType feature);
FString AWSGAMEKITCORE_API FeatureToUIString(FeatureType feature);
FString AWSGAMEKITCORE_API FeatureResourcesUIString(FeatureType feature);

enum class TemplateType { Base, Instance };
