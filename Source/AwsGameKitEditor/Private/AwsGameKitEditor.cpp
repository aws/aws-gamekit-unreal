// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitEditor.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "Achievements/EditorAchievementFeatureExample.h"
#include "AwsGameKitControlCenterLayout.h"
#include "AwsGameKitSettings.h"
#include "AwsGameKitSettingsLayoutDetails.h"
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"
#include "GameSaving/EditorGameSavingFeatureExample.h"
#include "Identity/EditorIdentityFeatureExample.h"
#include "IGameKitEditorFeatureExample.h"
#include "UserGameplayData/EditorUserGameplayFeatureExample.h"

// Unreal
#include "Async/Async.h"
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#include "MessageEndpointBuilder.h"
#include "MessagingCommon/Public/MessageEndpoint.h"

#define LOCTEXT_NAMESPACE "FAwsGameKitEditor"

#if UE_BUILD_DEVELOPMENT && WITH_EDITOR
DEFINE_LOG_CATEGORY(LogAwsGameKit);
#endif

void FAwsGameKitEditorModule::AddGameKitFeatureExample(FPropertyEditorModule& propertyModule, IGameKitEditorFeatureExample* gamekitFeatureExample)
{
    // Add feature to map
    this->gamekitFeatureExamples.Add(gamekitFeatureExample->GetFeatureExampleClassName(), gamekitFeatureExample);

    // Register custom detail panel for example
    propertyModule.RegisterCustomClassLayout(gamekitFeatureExample->GetFeatureExampleClassName(), gamekitFeatureExample->GetDetailCustomizationForExampleCreationMethod());
}

void FAwsGameKitEditorModule::OpenProjectSettings()
{
    ISettingsModule* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if (settingsModule != nullptr)
    {
        settingsModule->ShowViewer("Project", "Plugins", "AWS GameKit");
    }
}

void FAwsGameKitEditorModule::OpenControlCenter()
{
    const FName controlCenter = AwsGameKitControlCenterLayout::ControlCenterHomeTabName;
    const TSharedRef<FGlobalTabmanager> tabManager = FGlobalTabmanager::Get();
    TSharedPtr<SDockTab> ccTab = tabManager->FindExistingLiveTab(controlCenter);
    if (!ccTab.IsValid())
    {
        tabManager->TryInvokeTab(controlCenter);
        this->GetControlCenterLayout()->SetInitialState();
        ccTab = tabManager->FindExistingLiveTab(controlCenter);
    }
    tabManager->DrawAttention(ccTab.ToSharedRef());
}

void FAwsGameKitEditorModule::CloseControlCenter()
{
    const FName controlCenter = AwsGameKitControlCenterLayout::ControlCenterHomeTabName;
    const TSharedRef<FGlobalTabmanager> tabManager = FGlobalTabmanager::Get();
    TSharedPtr<SDockTab> ccTab = tabManager->FindExistingLiveTab(controlCenter);
    if (ccTab.IsValid())
    {
        ccTab->RequestCloseTab();
    }
}

TSharedPtr<EditorState> FAwsGameKitEditorModule::GetEditorState() const
{
    return this->editorState;
}

TSharedPtr<AwsCredentialsManager> FAwsGameKitEditorModule::GetCredentialsManager() const
{
    return this->credentialsManager;
}

TSharedPtr<FeatureResourceManager> FAwsGameKitEditorModule::GetFeatureResourceManager() const
{
    return this->featureResourceManager;
}

TSharedPtr<AwsGameKitControlCenterLayout> FAwsGameKitEditorModule::GetControlCenterLayout()
{
    // Lazy initialization of the Control Center Layout
    if (this->controlCenterLayout == nullptr)
    {
        this->controlCenterLayout = AwsGameKitControlCenterLayout::MakeInstance();
    }

    return this->controlCenterLayout;
}

TSharedPtr<AwsGameKitFeatureControlCenter> FAwsGameKitEditorModule::GetFeatureControlCenter() const
{
    return this->featureControlCenter;
}

TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> FAwsGameKitEditorModule::GetMessageBus() const
{
    return this->messageEndpoint;
}

void FAwsGameKitEditorModule::StartupModule()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::StartupModule()"));
    FPropertyEditorModule& propertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

    // Initialize Editor State
    this->editorState = MakeShareable(new EditorState());

    // Initialize Message Bus
    this->messageEndpoint = FMessageEndpoint::Builder(AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME)
        .Handling<FMsgCredentialsState>(this->editorState.Get(), &EditorState::CredentialsStateMessageHandler)
        .Build();
    this->messageEndpoint->Subscribe<FMsgCredentialsState>();

    // Initialize CredentialsManager
    this->credentialsManager = MakeShareable(new AwsCredentialsManager());

    // Initialize StyleSet
    AwsGameKitStyleSet();

    // Initialize FeatureResourceManager
    this->featureResourceManager = MakeShareable(new FeatureResourceManager());
    this->featureResourceManager->Initialize();

    // Initialize Feature Control Center
    this->featureControlCenter = MakeShareable(new AwsGameKitFeatureControlCenter());

    // Add GameKit feature examples to editor
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorIdentityFeatureExample());
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorAchievementFeatureExample());
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorUserGameplayFeatureExample());
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorGameSavingFeatureExample());

    // Register project settings after all AWS GameKit modules have come online
    FCoreDelegates::OnPostEngineInit.AddRaw(this, &FAwsGameKitEditorModule::RegisterProjectSettings);

    // Restore existing state if it exists
    this->BootstrapExistingState();
}

void FAwsGameKitEditorModule::BootstrapExistingState()
{
    const FString& gamekitRoot = this->featureResourceManager->GetRootPath();
    TArray<FString> filesFound;
    IFileManager::Get().FindFilesRecursive(filesFound, *gamekitRoot, TEXT("saveInfo.yml"), true, false, true);

    if (filesFound.Num() > 0)
    {
        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, filesFound]()
        {
            // Wrap this in a try because there are many things that could go wrong:
            // * saveInfo.yml corrupted (manual editing?)
            // * GetAccountId() failed with no network connection
            // * accessKey/secretKey removed from ~/.aws/credentials
            try
            {
                // First load gameName, which allows us to
                //  then get subsequent information
                AccountDetails* accountDetails = new AccountDetails();
                FString configDir = FPaths::GetPathLeaf(FPaths::GetPath(filesFound[0]));
                accountDetails->gameName = configDir;
                this->featureResourceManager->InitializeSettings(false);

                // Load environment, region, and AWS credentials
                accountDetails->environment = this->featureResourceManager->GetLastUsedEnvironment(accountDetails->gameName);
                this->credentialsManager->SetGameName(accountDetails->gameName);
                this->credentialsManager->SetEnv(accountDetails->environment);
                accountDetails->region = this->featureResourceManager->GetLastUsedRegion();
                accountDetails->accessKey = this->credentialsManager->GetAccessKey();
                accountDetails->accessSecret = this->credentialsManager->GetSecretKey();
                this->featureResourceManager->SetAccountDetails(*accountDetails);

                // Ensure we have the information necessary before initializing FeatureResourceManager
                if (accountDetails->accessKey.IsEmpty() || accountDetails->accessSecret.IsEmpty() || accountDetails->accountId.IsEmpty() || accountDetails->region.IsEmpty())
                {
                    throw std::exception("Existing state lacking complete AWS credentials.");
                }

                // Automatically detect existing accountId
                accountDetails->accountId = this->featureResourceManager->GetAccountId(accountDetails->accessKey, accountDetails->accessSecret);

                this->featureResourceManager->SetAccountDetails(*accountDetails);
                this->editorState->SetCredentials(*accountDetails);
                this->editorState->SetCredentialState(true);
                this->featureResourceManager->InitializeSettings(true);

                this->featureResourceManager->BootstrapAccount();
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                runtimeModule->ReloadConfigFile(this->featureResourceManager->GetClientConfigSubdirectory());
            }
            catch (const std::exception& e)
            {
                UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::BootstrapExistingState() failed: %s"), *FString(ANSI_TO_TCHAR(e.what())));
            }
        });
    }
}

void FAwsGameKitEditorModule::ShutdownModule()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::ShutdownModule()"));
    FPropertyEditorModule& propertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    // Unregister all custom examples Details Panels:
    for (auto const& keyValue : this->gamekitFeatureExamples)
    {
        propertyModule.UnregisterCustomPropertyTypeLayout(keyValue.Key);
    }

    this->controlCenterLayout.Reset();
    this->credentialsManager.Reset();
    this->editorState.Reset();
    this->featureResourceManager.Reset();
    this->gamekitFeatureExamples.Reset();
    this->messageEndpoint.Reset();

    const TSharedPtr<SDockTab> controlCenter = FGlobalTabmanager::Get()->FindExistingLiveTab(AwsGameKitControlCenterLayout::ControlCenterHomeTabName);
    if (controlCenter.IsValid())
    {
        FSlateApplication::Get().RequestDestroyWindow(controlCenter->GetParentWindow().ToSharedRef());
    }

    UnregisterProjectSettings();
}

void FAwsGameKitEditorModule::RegisterProjectSettings() const
{
    ISettingsModule* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if (settingsModule != nullptr)
    {
        // Register the AWS GameKit section
        settingsModule->RegisterSettings("Project", "Plugins", "AWS GameKit",
            FText::FromString("AWS GameKit"),
            FText::FromString("Configuration for AWS GameKit features"),
            GetMutableDefault<UAwsGameKitSettings>());

        // Combine all AWS GameKit configurations into a single detail panel for the settings section
        FPropertyEditorModule& propertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
        propertyModule.RegisterCustomClassLayout(UAwsGameKitSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic(&AwsGameKitSettingsLayoutDetails::MakeInstance));
    }
}

void FAwsGameKitEditorModule::UnregisterProjectSettings() const
{
    ISettingsModule* settingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");
    if (settingsModule != nullptr)
    {
        settingsModule->UnregisterSettings("Project", "Plugins", "AWS GameKit");
    }
}

TMap<FName, IGameKitEditorFeatureExample*> FAwsGameKitEditorModule::GetGameKitFeatureExamples() const
{
    return this->gamekitFeatureExamples;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAwsGameKitEditorModule, AwsGameKitEditor)
