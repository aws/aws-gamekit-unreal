// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "Identity/AwsGameKitIdentityExamplesLayout.h" // First include (Unreal requirement)

// Unreal
#include "DetailLayoutBuilder.h"

TSharedRef<IDetailCustomization> AwsGameKitIdentityExamplesLayout::MakeInstance()
{
    return MakeShareable(new AwsGameKitIdentityExamplesLayout);
}

void AwsGameKitIdentityExamplesLayout::CustomizeDetails(IDetailLayoutBuilder& detailBuilder)
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
