// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "IGameKitEditorFeatureExample.h"

class AWSGAMEKITEDITOR_API EditorUserGameplayFeatureExample : IGameKitEditorFeatureExample
{
public:
    EditorUserGameplayFeatureExample();
    virtual ~EditorUserGameplayFeatureExample() {}

    FName GetFeatureExampleClassName() override;
    FOnGetDetailCustomizationInstance GetDetailCustomizationForExampleCreationMethod() override;
};
