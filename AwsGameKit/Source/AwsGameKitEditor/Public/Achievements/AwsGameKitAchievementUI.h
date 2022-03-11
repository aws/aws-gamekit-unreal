// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit forward declarations
#include <AwsGameKitEditor/Public/Achievements/AwsGameKitAchievementsLayoutDetails.h>

// Unreal
#include "Containers/UnrealString.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Templates/SharedPointer.h"

// Unreal forward declarations
class SScrollBox;
class SExpandableArea;
class SEditableTextBox;
class SImage;
class SButton;
class SCheckBox;

enum class Synced : uint8 
{ 
    Unknown, 
    Synchronized, 
    Unsynchronized 
};

class AwsGameKitAchievementUI
{
private:
    AwsGameKitAchievementsLayoutDetails* parent;
    TSharedPtr<SScrollBox> representation;
    TSharedPtr<STextBlock> titleText;
    TSharedPtr<SButton> trashIconButton;

    void Initialize(AwsGameKitAchievementsLayoutDetails* layout, int32 points, int32 max, int32 sortOrder);
    FReply DeleteAchievement();
    void OnIdChanged(const FText& newText);

public:
    AwsGameKitAchievementUI(AwsGameKitAchievementsLayoutDetails* layout, int32 points = 0, int32 max = 1, int32 sortOrder = 0);
    AwsGameKitAchievementUI(AwsGameKitAchievementsLayoutDetails* layout, const AdminAchievement& adminAchievement);
    AwsGameKitAchievementUI() = default;
    ~AwsGameKitAchievementUI();

    bool markedForDeletion;
    Synced status;
    FString idString;

    bool localLockedIcon;
    bool localUnlockedIcon;

    TSharedPtr<SExpandableArea> expandableArea;

    TSharedPtr<SEditableTextBox> id;
    TSharedPtr<SEditableTextBox> title;
    TSharedPtr<SEditableTextBox> lockedDescription;
    TSharedPtr<SEditableTextBox> unlockedDescription;
    TSharedPtr<SEditableTextBox> lockedIcon;
    TSharedPtr<GameKitImage> lockedIconImg;
    TSharedPtr<SEditableTextBox> unlockedIcon;
    TSharedPtr<GameKitImage> unlockedIconImg;

    TSharedPtr<SCheckBox> secret;
    TSharedPtr<SCheckBox> hidden;

    TSharedPtr<SBorder> idValidationWarning;

    int32 Points;
    int32 MaxValue;
    int32 SortOrder;

    void ToAchievement(AdminAchievement& result);
    void ToJsonObject(TSharedPtr<FJsonObject>& result);

    TSharedRef<SScrollBox> GetRepresentation()
    {
        return this->representation.ToSharedRef();
    }

    bool IsSynchronized(TSharedPtr<AwsGameKitAchievementUI> other) const
    {
        return id->GetText().CompareTo(other->id->GetText()) == 0 &&
            title->GetText().CompareTo(other->title->GetText()) == 0 &&
            Points == other->Points &&
            lockedDescription->GetText().CompareTo(other->lockedDescription->GetText()) == 0 &&
            unlockedDescription->GetText().CompareTo(other->unlockedDescription->GetText()) == 0 &&
            MaxValue == other->MaxValue &&
            SortOrder == other->SortOrder &&
            secret->IsChecked() == other->secret->IsChecked() &&
            hidden->IsChecked() == other->hidden->IsChecked() &&
            lockedIcon->GetText().CompareTo(other->lockedIcon->GetText()) == 0 &&
            unlockedIcon->GetText().CompareTo(other->unlockedIcon->GetText()) == 0;
    }
};