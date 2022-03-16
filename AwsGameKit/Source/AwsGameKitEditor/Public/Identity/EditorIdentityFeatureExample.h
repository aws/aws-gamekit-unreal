// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "IGameKitEditorFeatureExample.h"

class AWSGAMEKITEDITOR_API EditorIdentityFeatureExample : IGameKitEditorFeatureExample
{
public:
    EditorIdentityFeatureExample();
    virtual ~EditorIdentityFeatureExample() {}

    FName GetFeatureExampleClassName() override;
    FOnGetDetailCustomizationInstance GetDetailCustomizationForExampleCreationMethod() override;
};
