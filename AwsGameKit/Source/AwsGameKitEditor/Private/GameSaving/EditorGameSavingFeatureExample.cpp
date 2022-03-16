// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "GameSaving/EditorGameSavingFeatureExample.h" // First include (Unreal requirement)
#include "GameSaving/AwsGameKitGameSavingExamples.h"
#include "GameSaving/AwsGameKitGameSavingExamplesLayout.h"

EditorGameSavingFeatureExample::EditorGameSavingFeatureExample()
{}

FName EditorGameSavingFeatureExample::GetFeatureExampleClassName()
{
    return AAwsGameKitGameSavingExamples::StaticClass()->GetFName();
}

FOnGetDetailCustomizationInstance EditorGameSavingFeatureExample::GetDetailCustomizationForExampleCreationMethod()
{
    return FOnGetDetailCustomizationInstance::CreateStatic(&AwsGameKitGameSavingExamplesLayout::MakeInstance);
}
