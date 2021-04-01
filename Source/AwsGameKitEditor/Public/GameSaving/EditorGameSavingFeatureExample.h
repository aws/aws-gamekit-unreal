// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "IGameKitEditorFeatureExample.h"

class AWSGAMEKITEDITOR_API EditorGameSavingFeatureExample : IGameKitEditorFeatureExample
{
public:
    EditorGameSavingFeatureExample();
    virtual ~EditorGameSavingFeatureExample() {}

    FName GetFeatureExampleClassName() override;
    FOnGetDetailCustomizationInstance GetDetailCustomizationForExampleCreationMethod() override;
};
