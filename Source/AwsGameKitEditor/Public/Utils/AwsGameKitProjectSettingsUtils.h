// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#define LEFT_COLUMN_BOX(content) SHorizontalBox::Slot() \
    .Padding(0, 5) \
    .AutoWidth() \
    .VAlign(VAlign_Center) \
    [ \
        SNew(SBox) \
        .WidthOverride(GameKit::PROJECT_SETTINGS_LEFT_COLUMN_WIDTH) \
        .MinDesiredWidth(GameKit::PROJECT_SETTINGS_LEFT_COLUMN_WIDTH) \
        [ content ] \
    ]

#define RIGHT_COLUMN_BOX(content) SHorizontalBox::Slot() \
    .Padding(0, 5) \
    .FillWidth(GameKit::PROJECT_SETTINGS_FILL_REMAINING) \
    [ content ]

#define PROJECT_SETTINGS_ROW(left, right) SNew(SHorizontalBox) \
    + LEFT_COLUMN_BOX(left) \
    + RIGHT_COLUMN_BOX(right)

namespace GameKit
{
    static const float PROJECT_SETTINGS_LEFT_COLUMN_WIDTH = 150.0f;
    static const float PROJECT_SETTINGS_FILL_REMAINING = 5.0f;
}