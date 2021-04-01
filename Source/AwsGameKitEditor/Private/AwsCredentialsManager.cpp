// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "AwsCredentialsManager.h"

AwsCredentialsManager::AwsCredentialsManager()
{
    // default to development until changed
    this->env = "Development";

#if defined(PLATFORM_WINDOWS) || defined(PLATFORM_MAC)
    // UserDir returns path to documents dir, not implemented on linux
    FString documentsDir = FPlatformProcess::UserDir();
    this->path = FPaths::Combine(documentsDir, TEXT("../.aws/credentials"));
#else
    this->path = FString("~/.aws/credentials");
#endif
    if (FPaths::FileExists(this->path))
    {
        this->credentials.Read(this->path);
    }
}

void AwsCredentialsManager::SetAccessKey(const FString& val)
{
    FString profile = this->GetProfile();
    this->credentials.SetString(*profile, TEXT("aws_access_key_id"), *val);
}

void AwsCredentialsManager::SetSecretKey(const FString& val)
{
    FString profile = this->GetProfile();
    this->credentials.SetString(*profile, TEXT("aws_secret_access_key"), *val);
}

FString AwsCredentialsManager::GetKey(const FString& key) const
{
    FString profile = this->GetProfile();
    FString result;
    this->credentials.GetString(*profile, *key, result);
    return result;
}

FString AwsCredentialsManager::GetAccessKey() const
{
    return this->GetKey(FString("aws_access_key_id"));
}

FString AwsCredentialsManager::GetSecretKey() const
{
    return this->GetKey(FString("aws_secret_access_key"));
}

void AwsCredentialsManager::SaveCredentials()
{
    this->credentials.Write(this->path);
}

