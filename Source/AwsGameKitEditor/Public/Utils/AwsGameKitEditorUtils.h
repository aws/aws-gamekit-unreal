// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "Containers/UnrealString.h"

class AwsGameKitEditorUtils
{
public:
    // Browser helpers
    static void OpenBrowser(const FString& url);

    // Message dialog helpers
    static void ShowMessageDialogAsync(const EAppMsgType::Type type, const FText& message);
    static void ShowMessageDialogAsync(const EAppMsgType::Type type, const FString& message);
    static void ShowMessageDialog(const EAppMsgType::Type type, const FString& message);
    static void ShowMessageDialog(const EAppMsgType::Type type, const FText& message);
};
