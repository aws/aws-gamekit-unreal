// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "IGameKitEditorFeatureExample.h"

class AWSGAMEKITEDITOR_API EditorAchievementFeatureExample : IGameKitEditorFeatureExample
{
public:
    EditorAchievementFeatureExample();
    virtual ~EditorAchievementFeatureExample() {}

    FName GetFeatureExampleClassName() override;
    FOnGetDetailCustomizationInstance GetDetailCustomizationForExampleCreationMethod() override;
};
