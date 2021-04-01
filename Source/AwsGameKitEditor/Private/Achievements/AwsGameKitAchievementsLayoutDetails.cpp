// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Achievements/AwsGameKitAchievementsLayoutDetails.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitEditor.h"
#include "AwsGameKitFeatureControlCenter.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"
#include "FeatureResourceManager.h"
#include "Achievements/AwsGameKitAchievements.h"
#include "Achievements/AwsGameKitAchievementUI.h"
#include "Utils/AwsGameKitProjectSettingsUtils.h"
#include "Utils/Blueprints/UAwsGameKitFileUtils.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "HttpModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Json.h"
#include "MessageEndpointBuilder.h"
#include "PropertyCustomizationHelpers.h"
#include "Algo/Sort.h"
#include "Async/Async.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "AwsGameKitAchievementLayoutDetails"

const FText AwsGameKitAchievementsLayoutDetails::SAVE_BUTTON_TEXT = LOCTEXT("AchievementsSaveButtonText", "Save Data");

AwsGameKitAchievementsLayoutDetails::AwsGameKitAchievementsLayoutDetails(const FAwsGameKitEditorModule* editorModule) : AwsGameKitFeatureLayoutDetails(FeatureType::Achievements, editorModule)
{
}

AwsGameKitAchievementsLayoutDetails::~AwsGameKitAchievementsLayoutDetails()
{
    if (configWindowOpen && achievementsConfig != nullptr)
    {
        achievementsConfig->RequestDestroyWindow();
    }
}

TSharedRef<IDetailCustomization> AwsGameKitAchievementsLayoutDetails::MakeInstance(const FAwsGameKitEditorModule* editorModule)
{
    TSharedRef<AwsGameKitAchievementsLayoutDetails> layoutDetails = MakeShareable(new AwsGameKitAchievementsLayoutDetails(editorModule));

    // Get editor module and update feature status
    const TSharedPtr<EditorState> editorState = editorModule->GetEditorState();
    const auto featureControlCenter = editorModule->GetFeatureControlCenter();
    const auto featureResourceManager = editorModule->GetFeatureResourceManager();

    if (editorState->AreCredentialsValid())
    {
        featureResourceManager->SetAccountDetails(editorState->GetCredentials());

        // Update feature status
        featureControlCenter->GetFeatureStatusAsync(FeatureType::Achievements);
    }

    layoutDetails->imageDownloader = ImageDownloader::MakeInstance();
    
    const FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    if (!runtimeModule->AreFeatureSettingsLoaded(FeatureType::Achievements))
    {
        runtimeModule->ReloadConfigFile(featureResourceManager->GetClientConfigSubdirectory());
    }

    layoutDetails->ConfigureLocalStatePath();

    layoutDetails->newAchievementCounter = 1;
    layoutDetails->configWindowOpen = false;
    layoutDetails->achievementsDeployed = false;
    layoutDetails->RefreshAchievementIconBaseUrl();

    return layoutDetails;
}

void AwsGameKitAchievementsLayoutDetails::ConfigureLocalStatePath()
{
    const FString localStateDir = FPaths::ConvertRelativePathToFull(FPaths::GameSourceDir().Replace(TEXT("source/"), ToCStr(AWSGAMEKIT_EDITOR_MODULE_INSTANCE()->GetFeatureResourceManager()->GetClientConfigSubdirectory())));
    this->localStatePath = FPaths::Combine(localStateDir, TEXT("achievements_local_state.json"));
}

void AwsGameKitAchievementsLayoutDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    static const FName BindingsCategory = TEXT("Achievements");
    featureCategoryBuilder = &DetailBuilder.EditCategory(BindingsCategory);

    featureCategoryBuilder->AddCustomRow(LOCTEXT("AchievementsDescriptionRowFilter", "Achievements | Description | GameKit | Game Kit | AWS"))
    [
        GetFeatureHeader(LOCTEXT("AchievementsDescription", "Add an achievements system where players can earn awards for their gameplay prowess."))
    ];

    featureCategoryBuilder->AddCustomRow(LOCTEXT("AchievementsConfigRowFilter", "Achievements | Config | GameKit | Game Kit | AWS"))
    [
        SNew(SVerticalBox)
        .IsEnabled_Lambda([this] { return !this->editorModule->GetFeatureControlCenter()->IsFeatureUpdating(this->featureType); })
        + SVerticalBox::Slot()
        .Padding(0,5)
        .AutoHeight()
        [
            PROJECT_SETTINGS_ROW(
                // Left - this page intentionally left blank (placeholder for spacing)
                SNew(STextBlock)
                .Text(LOCTEXT("ConfigureAchievementData", "Achievement Data")),

                // Right - open achievement configuration window
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .AutoWidth()
                [
                    SNew(SButton)
                    .HAlign(HAlign_Center)
                    .Text_Lambda([this]
                        {
                            return FText::FromString(this->configWindowOpen ? "Window Open" : "Configure");
                        })
                    .TextStyle(AwsGameKitStyleSet::Style, "Button.WhiteText")
                    .IsEnabled_Lambda([this] {return !this->configWindowOpen; })
                    .ButtonColorAndOpacity(AwsGameKitStyleSet::Style->GetColor("ButtonGreen"))
                    .ContentPadding(FMargin(10, 2))
                    .OnClicked(this, &AwsGameKitAchievementsLayoutDetails::ConfigAchievements)
                ]
                + SHorizontalBox::Slot()
                .Padding(10, 5, 5, 5)
                .MaxWidth(15)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .MaxHeight(15)
                    [
                        SNew(SImage)
                        .Image(AwsGameKitStyleSet::Style->GetBrush("ExternalIcon"))
                    ]
                ]
            )
        ]
    ];

    featureCategoryBuilder->AddCustomRow(LOCTEXT("AchievementsDeployRowFilter", "Achievements | Deploy | GameKit | Game Kit | AWS"))
    [
        this->GetDeployControls()
    ];
}

FReply AwsGameKitAchievementsLayoutDetails::ConfigAchievements()
{
    if (configWindowOpen)
    {
        return FReply::Handled();
    }
    auto padding = 5;

    achievementsConfig = SNew(SWindow)
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(FVector2D(600.f, 720.f))
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        [
            SNew(SBorder)
            .IsEnabled_Lambda([this] { return !this->editorModule->GetFeatureControlCenter()->IsFeatureUpdating(this->featureType); })
            .BorderImage(AwsGameKitStyleSet::Style->GetBrush("BackgroundGreyBrush"))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                .Padding(padding)
                .VAlign(VAlign_Top)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .FillWidth(1)
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Achievements"))
                        .Font(AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold11"))
                    ]

                    // Error banner
                    + SHorizontalBox::Slot()
                    .FillWidth(2)
                    [
                        SAssignNew(syncError, SBorder)
                        .BorderImage(AwsGameKitStyleSet::Style->GetBrush("ErrorRedBrush"))
                        .HAlign(HAlign_Center)
                        [
                            SNew(STextBlock)
                            .Text(FText::FromString("*Cloud sync failure. Check output log.*"))
                        ]
                        .Visibility(EVisibility::Hidden)
                    ]

                    + SHorizontalBox::Slot()
                    .HAlign(HAlign_Right)
                    .FillWidth(1)
                    [
                        SAssignNew(addAchievementButton, SButton)
                        .HAlign(HAlign_Center)
                        .Text(FText::FromString("Add Achievement"))
                        .OnClicked_Lambda([this]{return AddAchievement();})
                    ]
                ]

                + SVerticalBox::Slot()
                .Padding(padding)
                [
                    SAssignNew(achievementsSection, SScrollBox)
                ]

                + SVerticalBox::Slot()
                .Padding(padding*2)
                .AutoHeight()
                [
                    SNew(SSeparator)
                ]

                + SVerticalBox::Slot()
                .Padding(padding,0,0,10)
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString("Modify your achievement list locally in JSON"))
                    .Font(AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold10"))
                ]

                // JSON import/export
                + SVerticalBox::Slot()
                .Padding(padding)
                .AutoHeight()
                .HAlign(HAlign_Fill)
                [
                    SNew(SHorizontalBox)

                    + SHorizontalBox::Slot()
                    .HAlign(HAlign_Left)
                    .FillWidth(1)
                    [
                        SNew(SButton)
                        .HAlign(HAlign_Center)
                        .Text(FText::FromString("Get Template"))
                        .OnClicked(this, &AwsGameKitAchievementsLayoutDetails::GetJsonTemplate)
                    ]

                    + SHorizontalBox::Slot()
                    .FillWidth(1)
                    [
                        SNew(SHorizontalBox)

                        + SHorizontalBox::Slot()
                        .Padding(0,0,5,0)
                        [
                            SNew(SButton)
                            .HAlign(HAlign_Center)
                            .Text(FText::FromString("Export data to desktop"))
                            .OnClicked(this, &AwsGameKitAchievementsLayoutDetails::ExportJson)
                        ]

                        + SHorizontalBox::Slot()
                        [

                            SNew(SButton)
                            .HAlign(HAlign_Center)
                            .Text(FText::FromString("Import to AWS GameKit"))
                            .OnClicked(this, &AwsGameKitAchievementsLayoutDetails::ImportJson)
                        ]
                    ]
                ]

                + SVerticalBox::Slot()
                .Padding(10,20,10,10)
                .AutoHeight()
                [
                    SNew(SSeparator)
                ]

                + SVerticalBox::Slot()
                .Padding(padding,0,0,10)
                .AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .Padding(0,0,5,0)
                    .AutoWidth()
                    [
                        SNew(SImage).Image(AwsGameKitStyleSet::Style->GetBrush("CloudIcon"))
                    ]
                    + SHorizontalBox::Slot()
                    .AutoWidth()
                    [
                        SNew(STextBlock)
                        .Text(FText::FromString("Sync your achievements with the cloud."))
                        .Font(AwsGameKitStyleSet::Style->GetFontStyle("RobotoBold10"))
                    ]
                ]

                + SVerticalBox::Slot()
                .Padding(padding)
                .AutoHeight()
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot()
                    .FillWidth(2)
                    [
                        // empty for alignment
                        SNew(SOverlay)
                    ]

                    + SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Center)
                    .FillWidth(1)
                    .Padding(0,0,padding,0)
                    [
                        SAssignNew(getLatestButton, SButton)
                        .HAlign(HAlign_Center)
                        .Text(FText::FromString("Get Latest"))
                        .OnClicked(this, &AwsGameKitAchievementsLayoutDetails::GetLatestWindow)
                        .ToolTip(SNew(SToolTip).Text(FText::FromString("Loads all achievements from achievements metadata database.")))
                        .IsEnabled(achievementsDeployed)
                    ]

                    + SHorizontalBox::Slot()
                    .HAlign(HAlign_Fill)
                    .VAlign(VAlign_Center)
                    .FillWidth(1)
                    [
                        SAssignNew(saveButton, SButton)
                        .HAlign(HAlign_Center)
                        .OnClicked_Lambda([this]{
                            this->UploadAchievements();
                            return FReply::Handled();
                        })
                        .ToolTipText_Lambda([this]
                        {
                            if (invalidIds.Num() > 0)
                            {
                               const FString msg = "The following IDs are invalid: " + FString::Join(invalidIds, TEXT(", "));
                               return FText::FromString(msg);
                            }
                            return LOCTEXT("SaveButtonNormalTooltipl", "Writes contents of local work space to DynamoDB.");
                        })
                        .IsEnabled_Lambda([this](){return achievementsDeployed && invalidIds.Num() == 0;})
                        [
                            SAssignNew(saveButtonText, STextBlock)
                            .Text(SAVE_BUTTON_TEXT)
                        ]
                    ]

                ]
            ]
        ];
    achievementsConfig->SetRequestDestroyWindowOverride(FRequestDestroyWindowOverride::CreateLambda([this](const TSharedRef<SWindow>& window)
    {
        this->configWindowOpen = false;
        SaveStateToJsonFile();
        achievements.Empty();
        cloudSyncedAchievements.Empty();
        FSlateApplicationBase::Get().RequestDestroyWindow(this->achievementsConfig.ToSharedRef());
    }));
    SetCloudActionButtonState();
    LoadAchievementsFromJsonFile(this->localStatePath);
    ListAchievements();
    FSlateApplication::Get().AddWindow(this->achievementsConfig.ToSharedRef());
    this->configWindowOpen = true;
    return FReply::Handled();
}

FReply AwsGameKitAchievementsLayoutDetails::GetLatestWindow()
{
    TSharedPtr<SButton> merge;
    TSharedPtr<SButton> overwrite;
    TSharedPtr<SButton> cancel;

    TSharedRef<SWindow> getLatestWindow = SNew(SWindow)
        .SizingRule(ESizingRule::UserSized)
        .ClientSize(FVector2D(400.f, 100.f))
        .AutoCenter(EAutoCenter::PreferredWorkArea)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .Padding(10)
            .AutoHeight()
            [
                SNew(STextBlock)
                .Text(FText::FromString("Do you want to non-destructively merge cloud achievements with your local achievements, or overwrite them?"))
                .AutoWrapText(true)
            ]
            + SVerticalBox::Slot()
            .Padding(10)
            .AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot()
                .Padding(10)
                [
                    SAssignNew(merge, SButton)
                    .HAlign(HAlign_Center)
                    .Text(FText::FromString("Merge"))
                ]
                + SHorizontalBox::Slot()
                .Padding(10)
                [
                    SAssignNew(overwrite, SButton)
                    .HAlign(HAlign_Center)
                    .Text(FText::FromString("Overwrite"))
                ]
                + SHorizontalBox::Slot()
                .Padding(10)
                [
                    SAssignNew(cancel, SButton)
                    .HAlign(HAlign_Center)
                    .Text(FText::FromString("Cancel"))
                ]
            ]
        ];

    auto getLatest = [this, getLatestWindow](bool overwrite=false)->FReply
    {
        if (overwrite)
        {
            this->achievements.Empty();
        }

        this->cloudSyncedAchievements.Empty();
        ListAchievements();
        getLatestWindow->RequestDestroyWindow();
        return FReply::Handled();
    };

    auto cancelClicked = [this, getLatestWindow]()->FReply
    {
        getLatestWindow->RequestDestroyWindow();
        return FReply::Handled();
    };

    merge->SetOnClicked(FOnClicked::CreateLambda(getLatest));
    overwrite->SetOnClicked(FOnClicked::CreateLambda(getLatest, true));
    cancel->SetOnClicked(FOnClicked::CreateLambda(cancelClicked));

    FSlateApplication::Get().AddWindow(getLatestWindow);
    return FReply::Handled();
}

void AwsGameKitAchievementsLayoutDetails::RefreshAchievementIconBaseUrl()
{
    auto Delegate = TAwsGameKitDelegate<const IntResult&, const FString&>::CreateLambda([this](const IntResult& result, const FString& iconUrl)
    {
        const FString credsNotSubmittedUrl = "/";
        this->achievementIconsBaseUrl = (iconUrl.Compare(credsNotSubmittedUrl) != 0) ? iconUrl : FString();
    });
    AwsGameKitAchievements::GetAchievementIconBaseUrl(Delegate);
}

FReply AwsGameKitAchievementsLayoutDetails::AddAchievement()
{
    TSharedPtr<AwsGameKitAchievementUI> achievement = MakeShareable(new AwsGameKitAchievementUI(this));
    auto achievementField = achievement->GetRepresentation();
    auto key = FString::FromInt(newAchievementCounter);
    this->achievements.Add(key, achievement);
    achievement->idString = key;

    achievementsSection->AddSlot().Padding(0,1,0,1)[achievementField];
    this->newAchievementCounter += 1;
    return FReply::Handled();
}

void AwsGameKitAchievementsLayoutDetails::ListAchievements()
{
    // Check if achievements has been deployed
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    if (!runtimeModule->AreFeatureSettingsLoaded(FeatureType::Achievements))
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsLayoutDetails::ListAchievements(): Cannot list achievements from the cloud until the feature has been deployed in the Control Center."))
        return;
    }

    RefreshAchievementIconBaseUrl();

    auto Delegate = TAwsGameKitDelegate<const IntResult&, const TArray<AdminAchievement>&>::CreateRaw(this, &AwsGameKitAchievementsLayoutDetails::OnListAchievementsComplete);
    AwsGameKitAchievementsAdmin::ListAchievementsForGame(Delegate);
}

void AwsGameKitAchievementsLayoutDetails::OnListAchievementsComplete(const IntResult& result, const TArray<AdminAchievement>& listedAchievements)
{
    if (result.Result != GameKit::GAMEKIT_SUCCESS)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsLayoutDetails::ListAchievements() didn't successfully get achievements."))
        syncError->SetVisibility(EVisibility::Visible);

        Repopulate();
        return;
    }

    syncError->SetVisibility(EVisibility::Hidden);
    ProcessAchievements(listedAchievements, true);
}

void AwsGameKitAchievementsLayoutDetails::ProcessAchievements(const TArray<AdminAchievement>& incomingAchievements, bool fromCloud)
{
    for (AdminAchievement a : incomingAchievements)
    {
        TSharedPtr<AwsGameKitAchievementUI> achievement = MakeShareable(new AwsGameKitAchievementUI(this, a));

        FString targetId = achievement->id->GetText().ToString();
        if (fromCloud)
        {
           TSharedPtr<AwsGameKitAchievementUI> cloudAch = MakeShareable(new AwsGameKitAchievementUI(this, a));
           cloudSyncedAchievements.Add(targetId, cloudAch);

            if (achievements.Contains(targetId))
            {
                TSharedPtr<AwsGameKitAchievementUI> local = *achievements.Find(targetId);
                local->status = achievement->IsSynchronized(local) ? Synced::Synchronized : Synced::Unsynchronized;
                local->lockedIconImg->SetEnabled(false);
                local->unlockedIconImg->SetEnabled(false);
            }
            else
            {
                achievement->status = Synced::Synchronized;
                achievement->localLockedIcon = false;
                achievement->localUnlockedIcon = false;
                achievements.Add(targetId, achievement);
            }
        }
        else
        {
            this->achievements.Add(targetId, achievement);
        }
    }

    if (fromCloud)
    {
        for (auto a : achievements)
        {
            if (a.Value->status == Synced::Unknown)
            {
                a.Value->status = Synced::Unsynchronized;
            }
            else if (a.Value->status == Synced::Synchronized)
            {
                a.Value->id->SetEnabled(false);
            }
        }
    }

    Repopulate();
}

void AwsGameKitAchievementsLayoutDetails::Repopulate()
{
    if(achievementsSection.IsValid())
    {
        achievementsSection->ClearChildren();
    }

    TArray<TSharedPtr<AwsGameKitAchievementUI>> sortedAchievements;
    achievements.GenerateValueArray(sortedAchievements);

    auto sortAlg = [](TSharedPtr<AwsGameKitAchievementUI>&first, TSharedPtr<AwsGameKitAchievementUI>&second) -> bool
    {
        auto firstNum = first->SortOrder;
        auto secondNum = second->SortOrder;
        if (firstNum == secondNum)
        {
            return first->id->GetText().CompareTo(second->id->GetText()) < 1;
        }
        return firstNum < secondNum;
    };

    Algo::Sort(sortedAchievements, sortAlg);

    for (auto a : sortedAchievements)
    {
        FString id = a->id->GetText().ToString();

        auto lockedUrl = a->lockedIcon.Get()->GetText().ToString();
        if (!lockedUrl.IsEmpty() && !a->lockedIconImg->IsEnabled() && !a->localLockedIcon)
        {
            this->imageDownloader->SetImageFromUrl(*achievementIconsBaseUrl + lockedUrl, a->lockedIconImg, 1);
        }
        auto unlockedUrl = a->unlockedIcon.Get()->GetText().ToString();
        if (!unlockedUrl.IsEmpty() && !a->unlockedIconImg->IsEnabled() && !a->localUnlockedIcon)
        {
            this->imageDownloader->SetImageFromUrl(*achievementIconsBaseUrl + unlockedUrl, a->unlockedIconImg, 1);
        }

        if (this->achievementsDeployed)
        {
            if (cloudSyncedAchievements.Contains(id))
            {
                a->status = a->IsSynchronized(*cloudSyncedAchievements.Find(id)) ? Synced::Synchronized : Synced::Unsynchronized;
            }
            else
            {
                a->status = Synced::Unsynchronized;
            }
        }

        achievementsSection->AddSlot().Padding(0,1,0,1)[a->GetRepresentation()];
    }
    SaveStateToJsonFile();
}

FReply AwsGameKitAchievementsLayoutDetails::UploadAchievements()
{
    AddAchievementsRequest updateStruct;
    DeleteAchievementsRequest deleteStruct;
    for (auto a : achievements)
    {
        if (a.Value->markedForDeletion)
        {
            auto text = a.Value->id->GetText().ToString();
            deleteStruct.achievementIdentifiers.Add(text);
        }
        else if (a.Value->status != Synced::Synchronized)
        {
            AdminAchievement ach;
            a.Value->ToAchievement(ach);
            updateStruct.achievements.Add(ach);
        }
    }

    this->saveButtonText->SetText(LOCTEXT("AchievementsSavingProgress", "Saving ..."));

    auto DeleteDelegate = TAwsGameKitDelegate<const IntResult&>::CreateRaw(this, &AwsGameKitAchievementsLayoutDetails::OnDeleteAchievementsComplete);
    AwsGameKitAchievementsAdmin::DeleteAchievementsForGame(deleteStruct, DeleteDelegate);

    auto AddDelegate = TAwsGameKitDelegate<const IntResult&>::CreateRaw(this, &AwsGameKitAchievementsLayoutDetails::OnAddAchievementsComplete);
    AwsGameKitAchievementsAdmin::AddAchievementsForGame(updateStruct, AddDelegate);

    return FReply::Handled();
}

void AwsGameKitAchievementsLayoutDetails::OnAddAchievementsComplete(const IntResult& result)
{
    if (result.Result == GameKit::GAMEKIT_SUCCESS)
    {
        this->syncError->SetVisibility(EVisibility::Hidden);
        UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievementsLayoutDetails::OnAddAchievementsComplete() successfully added achievements."))
        achievements.Empty();
        this->cloudSyncedAchievements.Empty();
        ListAchievements();
    }
    else
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsLayoutDetails::OnAddAchievementsComplete() didn't successfully upload achievements."))
        this->syncError->SetVisibility(EVisibility::Visible);
    }
    this->saveButtonText->SetText(SAVE_BUTTON_TEXT);
}

void AwsGameKitAchievementsLayoutDetails::OnDeleteAchievementsComplete(const IntResult& result)
{
    if (result.Result == GameKit::GAMEKIT_SUCCESS)
    {
        this->syncError->SetVisibility(EVisibility::Hidden);
        UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitAchievementsLayoutDetails::OnDeleteAchievementsComplete() successfully deleted achievements."))
        achievements.Empty();
        this->cloudSyncedAchievements.Empty();
        ListAchievements();
    }
    else
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("AwsGameKitAchievementsLayoutDetails::UploadAchievements() didn't successfully delete achievements."))
        this->syncError->SetVisibility(EVisibility::Visible);
    }
    this->saveButtonText->SetText(SAVE_BUTTON_TEXT);
}

FReply  AwsGameKitAchievementsLayoutDetails::GetJsonTemplate()
{
    auto pluginBaseDir = IPluginManager::Get().FindPlugin("AwsGameKit")->GetBaseDir();
    auto templatePath = FPaths::Combine(*pluginBaseDir, TEXT("Resources"), TEXT("misc"), TEXT("achievements"));
    FString fileName = "achievements_template.json";

    auto originalTemplate = FPaths::Combine(templatePath, fileName);

    int suffix = 1;
    while (FPaths::FileExists(FPaths::Combine(templatePath, fileName)))
    {
        fileName = "achievements_template(" + FString::FromInt(suffix) + ").json";
        suffix += 1;
    }

    IPlatformFile& fileManager = FPlatformFileManager::Get().GetPlatformFile();
    auto path = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*FPaths::Combine(templatePath, fileName));
    fileManager.CopyFile(*path, *originalTemplate);
    FPlatformProcess::LaunchFileInDefaultExternalApplication(*path);

    return FReply::Handled();
}

FReply  AwsGameKitAchievementsLayoutDetails::ImportJson()
{
    FString message;
    FString fileName = UAwsGameKitFileUtils::PickFile("Select Achievements JSON template", FString("JSON file (*.json)|*.json"));

    if (fileName.IsEmpty())
    {
        return FReply::Handled();
    }

    LoadAchievementsFromJsonFile(fileName);
    return FReply::Handled();
}

void AwsGameKitAchievementsLayoutDetails::LoadAchievementsFromJsonFile(FString fileName)
{
    FString message;
    FFileHelper::LoadFileToString(message, *fileName);

    achievements.Empty();
    TArray<AdminAchievement> output;
    AwsGameKitAchievementsAdmin::GetListOfAdminAchievementsFromResponse(output, message, false);
    ProcessAchievements(output);
}

FReply AwsGameKitAchievementsLayoutDetails::ExportJson()
{
    FString fileName = UAwsGameKitFileUtils::PickFile("Export JSON achievements configuration.", FString("JSON file (*.json)|*.json"), false);

    if (fileName.IsEmpty())
    {
        return FReply::Handled();
    }

    SaveStateToJsonFile(fileName);
    return FReply::Handled();
}

void AwsGameKitAchievementsLayoutDetails::SaveStateToJsonFile(const FString& fileName)
{
    TSharedPtr<FJsonObject> jsonObject = MakeShareable(new FJsonObject);
    TArray<TSharedPtr<FJsonValue>> achievementObjects;

    for (auto achievement : achievements)
    {
        if (achievement.Value->markedForDeletion)
        {
            continue;
        }

        TSharedPtr<FJsonObject> achObject = MakeShareable(new FJsonObject);
        achievement.Value->ToJsonObject(achObject);

        TSharedRef<FJsonValueObject> jsonValue = MakeShareable(new FJsonValueObject(achObject));
        achievementObjects.Add(jsonValue);
    }
    jsonObject->SetArrayField("achievements", achievementObjects);

    FString output;
    TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> writer = TJsonWriterFactory<>::Create(&output);
    FJsonSerializer::Serialize(jsonObject.ToSharedRef(), writer);

    FFileHelper::SaveStringToFile(output, *fileName);
}

void  AwsGameKitAchievementsLayoutDetails::SetCloudActionButtonState()
{
    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    if (runtimeModule->AreFeatureSettingsLoaded(FeatureType::Achievements))
    {
        achievementsDeployed = true;
        getLatestButton->SetEnabled(true);
    }
    else
    {
        achievementsDeployed = false;
        getLatestButton->SetEnabled(false);
    }
}

void AwsGameKitAchievementsLayoutDetails::CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    if (message.IsSubmitted)
    {
        this->ConfigureLocalStatePath();
        this->LoadAchievementsFromJsonFile();

        if (saveButton != nullptr && getLatestButton != nullptr)
        {
            SetCloudActionButtonState();
            this->cloudSyncedAchievements.Empty();
            ListAchievements();
        }
    }
}

#undef LOCTEXT_NAMESPACE
