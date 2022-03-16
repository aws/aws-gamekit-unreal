// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "Achievements/EditorAchievementFeatureExample.h" // First include (Unreal requirement)
#include "Achievements/AwsGameKitAchievementsExamples.h"
#include "Achievements/AwsGameKitAchievementsExamplesLayout.h"

EditorAchievementFeatureExample::EditorAchievementFeatureExample()
{}

FName EditorAchievementFeatureExample::GetFeatureExampleClassName()
{
    return AAwsGameKitAchievementsExamples::StaticClass()->GetFName();
}

FOnGetDetailCustomizationInstance EditorAchievementFeatureExample::GetDetailCustomizationForExampleCreationMethod()
{
    return FOnGetDetailCustomizationInstance::CreateStatic(&AwsGameKitAchievementsExamplesLayout::MakeInstance);
}
