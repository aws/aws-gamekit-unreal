// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitEditor.h"

// GameKit
#include "Achievements/EditorAchievementFeatureExample.h"
#include "AwsCredentialsManager.h"
#include "AwsGameKitCore.h"
#include "AwsGameKitFeatureControlCenter.h"
#include "AwsGameKitRuntime.h"
#include "AwsGameKitSettings.h"
#include "AwsGameKitSettingsLayoutDetails.h"
#include "AwsGameKitStyleSet.h"
#include "EditorState.h"
#include "FeatureResourceManager.h"
#include "GameSaving/EditorGameSavingFeatureExample.h"
#include "Identity/EditorIdentityFeatureExample.h"
#include "IGameKitEditorFeatureExample.h"
#include "UserGameplayData/EditorUserGameplayFeatureExample.h"

// Standard Library
#include <stdexcept>

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
    this->featureControlCenter = MakeShareable(new AwsGameKitFeatureControlCenter(AWSGAMEKIT_EDITOR_MODULE_INSTANCE()));

    // Add GameKit feature examples to editor
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorIdentityFeatureExample());
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorAchievementFeatureExample());
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorUserGameplayFeatureExample());
    AddGameKitFeatureExample(propertyModule, (IGameKitEditorFeatureExample*) new EditorGameSavingFeatureExample());

    // Restore existing state if it exists
    this->BootstrapExistingState();

    // Register project settings after all AWS GameKit modules have come online
    FCoreDelegates::OnPostEngineInit.AddRaw(this, &FAwsGameKitEditorModule::RegisterProjectSettings);
}

void FAwsGameKitEditorModule::BootstrapExistingState()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::BootstrapExistingState()"));
    const FString& projectRoot = this->featureResourceManager->GetRootPath();
    TArray<FString> saveInfoFiles;
    IFileManager::Get().FindFilesRecursive(saveInfoFiles, *projectRoot, TEXT("saveInfo.yml"), true, false, true);

    if (saveInfoFiles.Num() > 0)
    {
        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, saveInfoFiles]()
        {
            // Wrap this in a try because there are many things that could go wrong:
            // * saveInfo.yml corrupted (manual editing?)
            // * GetAccountId() failed with no network connection
            // * accessKey/secretKey removed from ~/.aws/credentials
            try
            {
                // Extract the game name from the saveInfo.yml file path; it should be the directory name directly above it.
                FString gameName = FPaths::GetPathLeaf(FPaths::GetPath(saveInfoFiles[0]));

                // Set the game name, sparking the feature resource manager to reload settings.
                // Dev environment and auxilliary settings will be loaded; do not read feature specific settings until the correct environment is specified later.
                this->featureResourceManager->SetGameName(gameName);

                AccountDetails accountDetails;
                accountDetails.gameName = gameName;
                accountDetails.environment = this->featureResourceManager->GetLastUsedEnvironment();
                accountDetails.region = this->featureResourceManager->GetLastUsedRegion();

                // Initialize our credentials manager
                this->credentialsManager->SetGameName(accountDetails.gameName);
                this->credentialsManager->SetEnv(accountDetails.environment);

                // Credentials are loaded from {userDocumentsDir}/../.aws/credentials
                accountDetails.accessKey = this->credentialsManager->GetAccessKey();
                accountDetails.accessSecret = this->credentialsManager->GetSecretKey();
                accountDetails.accountId = this->featureResourceManager->GetAccountId(accountDetails.accessKey, accountDetails.accessSecret);

                // Ensure we have the information necessary before initializing FeatureResourceManager
                if (accountDetails.accessKey.IsEmpty() || accountDetails.accessSecret.IsEmpty() || accountDetails.accountId.IsEmpty() || accountDetails.region.IsEmpty())
                {
                    throw std::runtime_error("Existing state lacking complete AWS credentials.");
                }

                // Triggers a settings reload, ensuring all features are initialized with the proper settings for the last used environment
                this->featureResourceManager->SetAccountDetails(accountDetails);

                // Store credentials and mark as submitted
                this->editorState->SetCredentials(accountDetails);
                this->editorState->SetCredentialState(true);

                // Ensure account has been bootstrapped
                this->featureResourceManager->BootstrapAccount();

                // Tell runtime module to reload the config file for the last used environment
                FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
                runtimeModule->ReloadConfigFile(this->featureResourceManager->GetClientConfigSubdirectory());

                // "Prime the pump" with our feature statuses
                this->featureControlCenter->RefreshFeatureStatuses();
            }
            catch (const std::exception& e)
            {
                UE_LOG(LogAwsGameKit, Error, TEXT("FAwsGameKitEditorModule::BootstrapExistingState() failed: %s"), *FString(ANSI_TO_TCHAR(e.what())));
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
        UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::ShutdownModule(): Unregistering CustomPropertyTypeLayout: %s"), *keyValue.Key.ToString());
        propertyModule.UnregisterCustomPropertyTypeLayout(keyValue.Key);
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::ShutdownModule(): All CustomPropertyTypeLayout unregistered"));
    this->credentialsManager.Reset();
    this->editorState.Reset();
    this->featureResourceManager.Reset();
    this->gamekitFeatureExamples.Reset();
    this->messageEndpoint.Reset();
    this->featureControlCenter.Reset();

    UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::ShutdownModule(): Unregistering GameKit Project Settings"));
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
        UE_LOG(LogAwsGameKit, Display, TEXT("FAwsGameKitEditorModule::ShutdownModule(): Unregistered GameKit Project Settings"));
    }
}

TMap<FName, IGameKitEditorFeatureExample*> FAwsGameKitEditorModule::GetGameKitFeatureExamples() const
{
    return this->gamekitFeatureExamples;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAwsGameKitEditorModule, AwsGameKitEditor);
