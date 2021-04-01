// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "IDetailCustomization.h"

class AWSGAMEKITEDITOR_API IGameKitEditorFeatureExample
{
public:
    IGameKitEditorFeatureExample() {}
    virtual ~IGameKitEditorFeatureExample() {}
    virtual FName GetFeatureExampleClassName() = 0;
    virtual FOnGetDetailCustomizationInstance GetDetailCustomizationForExampleCreationMethod() = 0;
};
