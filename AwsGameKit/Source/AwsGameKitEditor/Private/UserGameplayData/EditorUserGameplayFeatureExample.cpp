// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "UserGameplayData/EditorUserGameplayFeatureExample.h" // First include (Unreal requirement)
#include "UserGameplayData/AwsGameKitUserGameplayDataExamples.h"
#include "UserGameplayData/AwsGameKitUserGameplayDataExamplesLayout.h"

EditorUserGameplayFeatureExample::EditorUserGameplayFeatureExample()
{}

FName EditorUserGameplayFeatureExample::GetFeatureExampleClassName()
{
    return AAwsGameKitUserGameplayDataExamples::StaticClass()->GetFName();
}

FOnGetDetailCustomizationInstance EditorUserGameplayFeatureExample::GetDetailCustomizationForExampleCreationMethod()
{
    return FOnGetDetailCustomizationInstance::CreateStatic(&AwsGameKitUserGameplayDataExamplesLayout::MakeInstance);
}
