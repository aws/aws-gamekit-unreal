// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
    static TSharedPtr<FConfigFile> documentationConfig;

public:

    static FString GetDocumentString(const FString& section, const FString& key);
    static TSharedRef<SBox> BuildHelpButton(const FString& section, const FString& linkKey);

    static FText GetDocumentText(const FString& section, const FString& key)
    {
        return FText::FromString(GetDocumentString(section, key));
    }
};
