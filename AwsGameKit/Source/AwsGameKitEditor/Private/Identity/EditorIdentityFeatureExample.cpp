// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "Identity/EditorIdentityFeatureExample.h" // First include (Unreal requirement)
#include "Identity/AwsGameKitIdentityExamples.h"
#include "Identity/AwsGameKitIdentityExamplesLayout.h"

EditorIdentityFeatureExample::EditorIdentityFeatureExample()
{}

FName EditorIdentityFeatureExample::GetFeatureExampleClassName()
{
    return AAwsGameKitIdentityExamples::StaticClass()->GetFName();
}

FOnGetDetailCustomizationInstance EditorIdentityFeatureExample::GetDetailCustomizationForExampleCreationMethod()
{
    return FOnGetDetailCustomizationInstance::CreateStatic(&AwsGameKitIdentityExamplesLayout::MakeInstance);
}
