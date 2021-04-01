// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "Containers/UnrealString.h"
#include "Misc/ConfigCacheIni.h"

class AwsCredentialsManager
{
private:
    FConfigFile credentials;
    FString path;
    FString gameName;
    FString env;

    inline FString GetProfile() const
    {
        return FString("GameKit-") + *gameName + "-" + *env;
    }

    FString GetKey(const FString& key) const;

public:
    AwsCredentialsManager();

    inline void SetGameName(const FString& name)
    {
        this->gameName = name;
    }

    inline void SetEnv(const FString& environment)
    {
        this->env = environment;
    }

    void SaveCredentials();
    void SetAccessKey(const FString& val);
    void SetSecretKey(const FString& val);
    FString GetAccessKey() const;
    FString GetSecretKey() const;
};
