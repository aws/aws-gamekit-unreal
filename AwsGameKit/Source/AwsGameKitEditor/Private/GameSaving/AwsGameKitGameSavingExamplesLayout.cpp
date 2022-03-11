// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "GameSaving/AwsGameKitGameSavingExamplesLayout.h"

// Unreal
#include "DetailLayoutBuilder.h"

TSharedRef<IDetailCustomization> AwsGameKitGameSavingExamplesLayout::MakeInstance()
{
    return MakeShareable(new AwsGameKitGameSavingExamplesLayout);
}

void AwsGameKitGameSavingExamplesLayout::CustomizeDetails(IDetailLayoutBuilder& detailBuilder)
{
    // Hide these categories from Details panel:
    detailBuilder.HideCategory("Rendering");
    detailBuilder.HideCategory("Replication");
    detailBuilder.HideCategory("Collision");
    detailBuilder.HideCategory("Input");
    detailBuilder.HideCategory("Actor");
    detailBuilder.HideCategory("LOD");
    detailBuilder.HideCategory("Cooking");
}
