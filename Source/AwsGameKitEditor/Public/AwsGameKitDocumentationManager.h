// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "Core/Logging.h"

// Unreal
#include "Containers/UnrealString.h"
#include "Misc/ConfigCacheIni.h"
#include "Templates/SharedPointer.h"

// Forward declarations
class SBox;

class AwsGameKitDocumentationManager
{
private:
    FConfigFile documentationConfig;
    FString section;
    TSharedPtr<FButtonStyle> helpStyle;

public:
    AwsGameKitDocumentationManager(const FString& targetSection);

    FString GetDocumentString(const FString& key);
    TSharedRef<SBox> BuildHelpButton(const FString& linkKey);

    FText GetDocumentText(const FString& key)
    {
        return FText::FromString(GetDocumentString(key));
    }

    void SetSection(const FString& targetSection)
    {
        this->section = targetSection;
    }
};
