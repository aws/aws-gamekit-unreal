// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitCoreWrapper.h"
#include "AwsGameKitDocumentationManager.h"

// Unreal
#include "Containers/UnrealString.h"
#include "HAL/PlatformProcess.h"
#include "Runtime/Core/Public/Async/Async.h"

class AwsGameKitEditorUtils
{
public:
    // Browser helpers
    static void OpenBrowser(const FString& url)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitEditorUtils::OpenBrowser() Opening %s"), *url);
        FPlatformProcess::LaunchURL(*url, nullptr, nullptr);
    }

    // Message dialog helpers
    static void ShowMessageDialogAsync(const EAppMsgType::Type type, const FText& message)
    {
        AsyncTask(ENamedThreads::GameThread, [message]()
            {
                ShowMessageDialog(EAppMsgType::Ok, message);
            });
    }

    static void ShowMessageDialogAsync(const EAppMsgType::Type type, const FString& message)
    {
        AsyncTask(ENamedThreads::GameThread, [message]()
            {
                ShowMessageDialog(EAppMsgType::Ok, message);
            });
    }

    static void ShowMessageDialog(const EAppMsgType::Type type, const FString& message)
    {
        ShowMessageDialog(type, FText::FromString(message));
    }

    static void ShowMessageDialog(const EAppMsgType::Type type, const FText& message)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("%s"), *message.ToString());
        FMessageDialog::Open(type, message);
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
                return AwsGameKitDocumentationManager::GetDocumentString("dev_guide_url", "identity");
            case FeatureType::Achievements:
                return AwsGameKitDocumentationManager::GetDocumentString("dev_guide_url", "achievements");
            case FeatureType::GameStateCloudSaving:
                return AwsGameKitDocumentationManager::GetDocumentString("dev_guide_url", "game_state_saving");
            case FeatureType::UserGameplayData:
                return AwsGameKitDocumentationManager::GetDocumentString("dev_guide_url", "user_gameplay_data");
            default:
                return AwsGameKitDocumentationManager::GetDocumentString("url", "gamekit_home");
        }
    }
};
