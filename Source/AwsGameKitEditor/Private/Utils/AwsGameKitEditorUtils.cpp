// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Utils/AwsGameKitEditorUtils.h"

// GameKit
#include "AwsGameKitCore.h"

// Unreal
#include "HAL/PlatformProcess.h"
#include "Runtime/Core/Public/Async/Async.h"

void AwsGameKitEditorUtils::OpenBrowser(const FString& url)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitEditorUtils::OpenBrowser() Opening %s"), *url);
    FPlatformProcess::LaunchURL(*url, nullptr, nullptr);
}

void AwsGameKitEditorUtils::ShowMessageDialogAsync(const EAppMsgType::Type type, const FText& message)
{
    AsyncTask(ENamedThreads::GameThread, [message]()
        {
            ShowMessageDialog(EAppMsgType::Ok, message);
        });
}

void AwsGameKitEditorUtils::ShowMessageDialogAsync(const EAppMsgType::Type type, const FString& message)
{
    AsyncTask(ENamedThreads::GameThread, [message]()
        {
            ShowMessageDialog(EAppMsgType::Ok, message);
        });
}

void AwsGameKitEditorUtils::ShowMessageDialog(const EAppMsgType::Type type, const FString& message)
{
    ShowMessageDialog(type, FText::FromString(message));
}

void AwsGameKitEditorUtils::ShowMessageDialog(const EAppMsgType::Type type, const FText& message)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("%s"), *message.ToString());
    FMessageDialog::Open(type, message);
}