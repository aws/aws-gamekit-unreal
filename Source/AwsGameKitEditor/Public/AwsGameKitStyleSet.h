// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "Templates/SharedPointer.h"

// Unreal forward declarations
class FSlateStyleSet;

class AwsGameKitStyleSet
{
public:
    static TSharedPtr<FSlateStyleSet> Style;
    AwsGameKitStyleSet();
    ~AwsGameKitStyleSet() = default;
};