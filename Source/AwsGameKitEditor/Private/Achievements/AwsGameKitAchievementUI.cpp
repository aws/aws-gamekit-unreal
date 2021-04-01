// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

// GameKit
#include "Achievements/AwsGameKitAchievementUI.h" // First include (Unreal requirement)
#include "Achievements/AwsGameKitAchievementsLayoutDetails.h"
#include "AwsGameKitFeatureControlCenter.h"
#include "AwsGameKitRuntime/Public/Utils/Blueprints/UAwsGameKitFileUtils.h"
#include "AwsGameKitStyleSet.h"
#include "ImageDownloader.h"

// Unreal
#include "Async/Async.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Interfaces/IPluginManager.h"
#include "Runtime/Online/HTTP/Public/Http.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitAchievementUI"

#define UPDATE_LOCAL_STATE() \
.OnTextCommitted_Lambda([this](const FText& newText, ETextCommit::Type commitType) \
{ \
    this->parent->Repopulate(); \
})

AwsGameKitAchievementUI::AwsGameKitAchievementUI(AwsGameKitAchievementsLayoutDetails* layout, int32 points, int32 max, int32 sortOrder)
{
    // Can't reuse this constructor in other constructors because slate nullptr issues
    Initialize(layout, points, max, sortOrder);
}

AwsGameKitAchievementUI::AwsGameKitAchievementUI(AwsGameKitAchievementsLayoutDetails* layout, const AdminAchievement& adminAchievement)
{
    Initialize(layout, adminAchievement.points, adminAchievement.requiredAmount, adminAchievement.sortOrder);

    idString = adminAchievement.achievementId;
    this->id->SetText(FText::FromString(adminAchievement.achievementId));
    this->title->SetText(FText::FromString(adminAchievement.title));
    this->unlockedIcon->SetText(FText::FromString(adminAchievement.unlockedIcon));
    this->lockedIcon->SetText(FText::FromString(adminAchievement.lockedIcon));
    this->unlockedDescription->SetText(FText::FromString(adminAchievement.unlockedDescription));
    this->lockedDescription->SetText(FText::FromString(adminAchievement.lockedDescription));
    this->secret->SetIsChecked(adminAchievement.isSecret ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
    this->hidden->SetIsChecked(adminAchievement.isHidden ? ECheckBoxState::Checked : ECheckBoxState::Unchecked);
    this->localLockedIcon = adminAchievement.localLockedIcon;
    this->localUnlockedIcon = adminAchievement.localUnlockedIcon;
}

AwsGameKitAchievementUI::~AwsGameKitAchievementUI()
{
    parent->invalidIds.Remove(idString);
}

void AwsGameKitAchievementUI::Initialize(AwsGameKitAchievementsLayoutDetails* layout, int32 points, int32 max, int32 sortOrder)
{
    this->parent = layout;
    this->status = Synced::Unknown;
    this->markedForDeletion = false;
    this->localLockedIcon = true;
    this->localUnlockedIcon = true;
    this->Points = points;
    this->MaxValue = max;
    this->SortOrder = sortOrder;

    const FSlateBrush* deleteIcon = AwsGameKitStyleSet::Style->GetBrush("DeleteIcon");

    static const int ROW_PADDING = 3;

    static const int LEFT_COL_WIDTH = 1;
    static const int RIGHT_COL_WIDTH = 3;

    static const int NUM_FIELD_WIDTH = 1;
    static const int NUM_ROW_PADDING = 2;

    static const int ACHIEVEMENT_ICON_SIZE = 50;
    static const int TRASH_ICON_SIZE = 32;

    auto warningBox = [this](TSharedPtr<SBorder>& boxBorder, const FString& message)
    {
        SAssignNew(boxBorder, SBorder)
        .BorderBackgroundColor(AwsGameKitStyleSet::Style->GetColor("ErrorRed"))
        .BorderImage(AwsGameKitStyleSet::Style->GetBrush("ErrorRedBrush"))
        .Visibility(EVisibility::Collapsed) // warnings are hidden by default
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .AutoWidth()
            .VAlign(VAlign_Center)
            .Padding(2, 3, 2, 2)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .MaxHeight(10)
                [
                    SNew(SImage)
                    .Image(AwsGameKitStyleSet::Style->GetBrush("WarningIconSmall"))
                ]
            ]
            + SHorizontalBox::Slot()
            .Padding(2, 1, 0, 0)
            .VAlign(EVerticalAlignment::VAlign_Center)
            [
                SNew(STextBlock)
                .AutoWrapText(true)
                .Justification(ETextJustify::Left)
                .Font(AwsGameKitStyleSet::Style->GetFontStyle("RobotoRegular8"))
                .Text(FText::FromString(message))
            ]
        ];

        return boxBorder.ToSharedRef();
    };

    this->representation =
        SNew(SScrollBox)
        .Visibility_Lambda([this]{return this->markedForDeletion ? EVisibility::Collapsed : EVisibility::Visible;})
        + SScrollBox::Slot()
        [
            SAssignNew(this->expandableArea, SExpandableArea)
            .InitiallyCollapsed(true)
            .HeaderContent()
            [
                SNew(SHorizontalBox)

                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .AutoWidth()
                .Padding(ROW_PADDING)
                [
                    SNew(SImage)
                    .Image_Lambda([this]()->const FSlateBrush*
                    {
                        if (this->status == Synced::Synchronized)
                        {
                            return AwsGameKitStyleSet::Style->GetBrush("DeployedIcon");
                        }
                        else if (this->status == Synced::Unsynchronized)
                        {
                            return AwsGameKitStyleSet::Style->GetBrush("UnsynchronizedIcon");
                        }
                        else
                        {
                            return nullptr;
                        }
                    })
                    .ToolTipText_Lambda([this]{
                        if (this->status == Synced::Synchronized)
                        {
                            return FText::FromString("Synced with cloud");
                        }
                        else if (this->status == Synced::Unsynchronized)
                        {
                            return FText::FromString("Not in sync with cloud");
                        }
                        else
                        {
                            return FText::FromString("Unknown sync status");
                        }
                    })
                ]

                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .Padding(ROW_PADDING)
                [
                    SAssignNew(titleText, STextBlock)
                    .Text(FText::FromString("New Achievement(" + FString::FromInt(this->parent->newAchievementCounter) + ")"))
                ]
                + SHorizontalBox::Slot()
                .VAlign(VAlign_Center)
                .HAlign(HAlign_Right)
                .Padding(ROW_PADDING)
                [
                    SNew(SBox)
                    .WidthOverride(TRASH_ICON_SIZE)
                    .HeightOverride(TRASH_ICON_SIZE)
                    [
                        SAssignNew(trashIconButton, SButton)
                        .ButtonStyle(FCoreStyle::Get(), "NoBorder")
                        .HAlign(HAlign_Center)
                        .VAlign(VAlign_Center)
                        .OnClicked(FOnClicked::CreateRaw(this, &AwsGameKitAchievementUI::DeleteAchievement))
                        .ForegroundColor(FSlateColor::UseForeground())
                        .Cursor(EMouseCursor::Hand)
                        [
                            SNew(SImage).Image(deleteIcon)
                        ]
                        .Visibility_Lambda([this]
                        {
                            if (this->expandableArea.IsValid() && this->expandableArea->IsHovered())
                            {
                                return EVisibility::Visible;
                            }
                            return EVisibility::Hidden;
                        })
                    ]
                ]
            ]
            .BodyContent()
            [
                SNew(SBox)
                .Padding(10)
                [
                    SNew(SVerticalBox)

                    // ID 
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .FillWidth(LEFT_COL_WIDTH)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString("ID* (primary key)"))
                            ]
                            + SHorizontalBox::Slot()
                            .FillWidth(RIGHT_COL_WIDTH)
                            [
                                SAssignNew(this->id, SEditableTextBox)
                                .HintText(LOCTEXT("ValidAchievementIdRequirements", "Valid ID characters: a-z, A-Z, 0-9, _"))
                                .OnTextChanged(FOnTextChanged::CreateRaw(this, &AwsGameKitAchievementUI::OnIdChanged))
                            ]
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        [
                            warningBox(this->idValidationWarning, "Please enter a valid Achievement ID. Valid ID characters: a-z, A-Z, 0-9, _ \n Can't begin or end with an underscore, and must be at least 2 characters.")
                        ]
                    ]

                    // Title (has custom lambda can't use macro)
                    + SVerticalBox::Slot()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Title"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SAssignNew(this->title, SEditableTextBox)
                            .HintText(FText::FromString("Eat 10 Bananas"))
                            .OnTextChanged_Lambda([this](const FText& newText)
                                {this->titleText->SetText(newText);})
                            UPDATE_LOCAL_STATE()
                        ]
                    ]

                    // Points
                    + SVerticalBox::Slot()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Points"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(NUM_FIELD_WIDTH)
                        [
                            SNew(SSpinBox<int32>)
                            .MinValue(0)
                            .MaxValue(TNumericLimits<int32>::Max())
                            .Value(points)
                            .OnValueCommitted(FOnInt32ValueCommitted::CreateLambda([this](int32 newValue, ETextCommit::Type type)
                            {
                                Points = newValue;
                                this->parent->Repopulate();
                            }))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(NUM_ROW_PADDING)
                        [
                            // for alignment
                            SNew(SOverlay)
                        ]
                    ]

                    // Locked Description
                    + SVerticalBox::Slot()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Description (locked)"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SAssignNew(this->lockedDescription, SEditableTextBox)
                            .HintText(FText::FromString("Description players see when unearned."))
                            UPDATE_LOCAL_STATE()
                        ]
                    ]

                    // Unlocked Description
                    + SVerticalBox::Slot()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Description (unlocked)"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SAssignNew(this->unlockedDescription, SEditableTextBox)
                            .HintText(FText::FromString("Description players see after earned."))
                            UPDATE_LOCAL_STATE()
                        ]
                    ]

                    // Locked icon
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Image/icon (locked)"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SNew(SButton)
                                    .Text(FText::FromString("Browse"))
                                    .OnClicked_Lambda([this]{
                                        this->lockedIcon->SetText(FText::FromString(UAwsGameKitFileUtils::PickFile(FString("Pick locked icon file."), FString("PNG file (*.png)|*.png"))));
                                        this->localLockedIcon = true;
                                        this->parent->Repopulate();
                                        return FReply::Handled();
                                })
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .Padding(ROW_PADDING, 0, 0, 0)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SAssignNew(this->lockedIcon, SEditableTextBox)
                                    .HintText(FText::FromString("C:/images/locked_path.png"))
                                    .IsEnabled(false)
                                    UPDATE_LOCAL_STATE()                            
                                ]
                            ]                    
                            + SHorizontalBox::Slot()
                            .Padding(ROW_PADDING, 0, 0, 0)
                            .AutoWidth()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .MaxHeight(ACHIEVEMENT_ICON_SIZE)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .MaxWidth(ACHIEVEMENT_ICON_SIZE)
                                    [
                                        SAssignNew(this->lockedIconImg, GameKitImage)
                                        .IsEnabled(false)
                                    ]
                                ]
                            ]
                        ]
                    ]

                    // Unlocked icon
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Image/icon (unlocked)"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SNew(SButton)
                                    .Text(FText::FromString("Browse"))
                                    .OnClicked_Lambda([this]{
                                        this->unlockedIcon->SetText(FText::FromString(UAwsGameKitFileUtils::PickFile(FString("Pick unlocked icon file."), FString("PNG file (*.png)|*.png"))));
                                        this->localUnlockedIcon = true;
                                        this->parent->Repopulate();
                                        return FReply::Handled();
                                    })
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .Padding(ROW_PADDING,0,0,0)
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                [
                                    SAssignNew(this->unlockedIcon, SEditableTextBox)
                                    .HintText(FText::FromString("C:/images/path.png"))
                                    .IsEnabled(false)
                                    UPDATE_LOCAL_STATE()
                                ]
                            ]
                            + SHorizontalBox::Slot()
                            .Padding(ROW_PADDING, 0, 0, 0)
                            .AutoWidth()
                            [
                                SNew(SVerticalBox)
                                + SVerticalBox::Slot()
                                .AutoHeight()
                                .MaxHeight(ACHIEVEMENT_ICON_SIZE)
                                [
                                    SNew(SHorizontalBox)
                                    + SHorizontalBox::Slot()
                                    .MaxWidth(ACHIEVEMENT_ICON_SIZE)
                                    [
                                        SAssignNew(this->unlockedIconImg, GameKitImage)
                                        .IsEnabled(false)
                                    ]
                                ]
                            ]
                        ]
                    ]

                    // Iterations required
                    + SVerticalBox::Slot()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("No. steps to earn"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(NUM_FIELD_WIDTH)
                        [
                            SNew(SSpinBox<int32>)
                            .MinValue(0)
                            .MaxValue(TNumericLimits<int32>::Max())
                            .Value(max)
                            .OnValueCommitted(FOnInt32ValueCommitted::CreateLambda([this](int32 newValue, ETextCommit::Type type)
                            {
                                MaxValue = newValue;
                                this->parent->Repopulate();
                            }))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(NUM_ROW_PADDING)
                        [
                            // for alignment
                            SNew(SOverlay)
                        ]
                    ]

                    // Visibility
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Visibility"))
                            .AutoWrapText(true)
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(RIGHT_COL_WIDTH)
                        [
                            SNew(SHorizontalBox)
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .HAlign(HAlign_Left)
                            [
                                SAssignNew(secret, SCheckBox)
                                .OnCheckStateChanged_Lambda([this](ECheckBoxState InCheckBoxState){this->parent->Repopulate();})
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .HAlign(HAlign_Left)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString("Invisible to players"))
                            ]

                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .HAlign(HAlign_Left)
                            .Padding(25,0,0,0)
                            [
                                SAssignNew(hidden, SCheckBox)
                                .OnCheckStateChanged_Lambda([this](ECheckBoxState InCheckBoxState){this->parent->Repopulate();})
                            ]
                            + SHorizontalBox::Slot()
                            .AutoWidth()
                            .HAlign(HAlign_Left)
                            [
                                SNew(STextBlock)
                                .Text(FText::FromString("Cannot be achieved"))
                            ]
                        ]
                    ]

                    // Sort order
                    + SVerticalBox::Slot()
                    .Padding(ROW_PADDING)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .FillWidth(LEFT_COL_WIDTH)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("Sort order"))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(NUM_FIELD_WIDTH)
                        [
                            SNew(SSpinBox<int32>)
                            .MinValue(0)
                            .MaxValue(TNumericLimits<int32>::Max())
                            .Value(sortOrder)
                            .OnValueCommitted(FOnInt32ValueCommitted::CreateLambda([this](int32 newValue, ETextCommit::Type type)
                            {
                                SortOrder = newValue;
                                this->parent->Repopulate();
                            }))
                        ]
                        + SHorizontalBox::Slot()
                        .FillWidth(NUM_ROW_PADDING)
                        [
                            // for alignment
                            SNew(SOverlay)
                        ]
                    ]
                ]
            ]
        ];
}

FReply AwsGameKitAchievementUI::DeleteAchievement()
{
    TSharedPtr<SButton> yes;
    TSharedPtr<SButton> no;
    static const int padding = 15;
    
    // can't use FMessageDialog without minimizing other SWindows (config achievements window)
    auto confirmWindow = SNew(SWindow)
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(FVector2D(250.f, 100.f))
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .Padding(padding)
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(FText::FromString("Do you want to delete this achievement?"))
            ]
            + SVerticalBox::Slot()
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .Padding(padding)
                [
                    SAssignNew(yes, SButton)
                    .Text(FText::FromString("Yes"))
                ]
                + SHorizontalBox::Slot()
                .Padding(padding)
                [
                    SAssignNew(no, SButton)
                    .Text(FText::FromString("No"))
                ]
            ]
        ];

    auto onDestroy = [confirmWindow]()
    {
        confirmWindow->RequestDestroyWindow();
        return FReply::Handled();
    };

    auto onYes = [this, onDestroy]()
    {
        this->markedForDeletion = true;
        if (!this->parent->achievements.Contains(this->idString))
        {
            // this achievement isn't in the cloud as far as we know, delete locally.
            this->parent->achievements.Remove(this->idString);
        }
        this->parent->invalidIds.Remove(idString);
        return onDestroy();
    };

    yes->SetOnClicked(FOnClicked::CreateLambda(onYes));
    no->SetOnClicked(FOnClicked::CreateLambda(onDestroy));
    FSlateApplication::Get().AddWindow(confirmWindow);

    this->parent->Repopulate();
    return FReply::Handled();
}

void AwsGameKitAchievementUI::OnIdChanged(const FText& newId)
{
    if (newId.IsEmpty())
    {
        this->parent->invalidIds.Remove(this->idString);
        return;
    }

    // Valid ID is any combination of alphanumeric characters and underscore that doesn't begin or end with an underscore, length >= 2
    const FRegexPattern validAchievementId("^[a-zA-Z0-9][a-zA-Z0-9_]*[a-zA-Z0-9]$");
    FRegexMatcher matcher(validAchievementId, newId.ToString());
    if (!matcher.FindNext() && this->idValidationWarning.IsValid())
    {
        this->idValidationWarning->SetVisibility(EVisibility::Visible);
        this->parent->invalidIds.Remove(this->idString);
        this->parent->invalidIds.Add(newId.ToString());
    }
    else if (this->idValidationWarning.IsValid())
    {
        this->idValidationWarning->SetVisibility(EVisibility::Collapsed);
        this->parent->invalidIds.Remove(this->idString);
    }

    if (this->parent->achievements.Contains(this->idString))
    {
        TSharedPtr<AwsGameKitAchievementUI> ptr = *this->parent->achievements.Find(this->idString);
        this->parent->achievements.Remove(this->idString);
        this->parent->achievements.Add(newId.ToString(), ptr);
        this->idString = newId.ToString();
        this->parent->Repopulate();
    }
}

void AwsGameKitAchievementUI::ToAchievement(AdminAchievement& result)
{
    // Strings
    result.achievementId = id->GetText().ToString();
    result.title = title->GetText().ToString();
    result.lockedDescription = lockedDescription->GetText().ToString();
    result.unlockedDescription = unlockedDescription->GetText().ToString();
    result.lockedIcon = lockedIcon->GetText().ToString();
    result.unlockedIcon = unlockedIcon->GetText().ToString();

    // Numbers
    result.points = Points;
    result.sortOrder = SortOrder;
    result.requiredAmount = MaxValue;

    // Bools
    result.isStateful = result.requiredAmount > 1;
    result.isHidden = hidden->IsChecked();
    result.isSecret = secret->IsChecked();
    result.localLockedIcon = localLockedIcon;
    result.localUnlockedIcon = localUnlockedIcon;
}

void AwsGameKitAchievementUI::ToJsonObject(TSharedPtr<FJsonObject>& result)
{
    result->SetStringField("achievement_id", id->GetText().ToString());
    result->SetStringField("title", title->GetText().ToString());
    result->SetStringField("locked_description", lockedDescription->GetText().ToString());
    result->SetStringField("unlocked_description", unlockedDescription->GetText().ToString());
    result->SetStringField("locked_icon_url", lockedIcon->GetText().ToString());
    result->SetStringField("unlocked_icon_url", unlockedIcon->GetText().ToString());

    result->SetNumberField("max_value", MaxValue);
    result->SetNumberField("points", Points);
    result->SetNumberField("order_number", SortOrder);

    result->SetBoolField("is_stateful", MaxValue > 1);
    result->SetBoolField("is_secret", secret->IsChecked());
    result->SetBoolField("is_hidden", hidden->IsChecked());
    result->SetBoolField("local_locked_icon", localLockedIcon);
    result->SetBoolField("local_unlocked_icon", localUnlockedIcon);
}
