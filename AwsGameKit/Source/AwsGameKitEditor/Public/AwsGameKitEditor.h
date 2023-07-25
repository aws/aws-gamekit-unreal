// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit Forward declarations
class IGameKitEditorFeatureExample;
class AwsGameKitFeatureControlCenter;
class AwsCredentialsManager;
class EditorState;
class FeatureResourceManager;

// Unreal
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Templates/SharedPointer.h"
#include "PropertyEditorModule.h"

// Unreal forward declarations
class FMessageEndpoint;

// Editor module macros
#define AWSGAMEKIT_EDITOR_MODULE_NAME "AwsGameKitEditor"
#define AWSGAMEKIT_EDITOR_MESSAGE_BUS_NAME "AwsGameKitEditorMessageBus"
#define AWSGAMEKIT_EDITOR_MODULE_INSTANCE() FModuleManager::GetModulePtr<FAwsGameKitEditorModule>(AWSGAMEKIT_EDITOR_MODULE_NAME)
#define AWSGAMEKIT_EDITOR_PLUGIN_STATE() AWSGAMEKIT_EDITOR_MODULE_INSTANCE()->GetEditorPluginState();
#define AWSGAMEKIT_EDITOR_MESSAGE_BUS() AWSGAMEKIT_EDITOR_MODULE_INSTANCE()->GetMessageBus();
#define AWSGAMEKIT_EDITOR_STATE() AWSGAMEKIT_EDITOR_MODULE_INSTANCE()->GetEditorState();

class FAwsGameKitEditorModule : public IModuleInterface
{
private:
    TSharedPtr<EditorState> editorState;
    TMap<FName, IGameKitEditorFeatureExample*> gamekitFeatureExamples;
    TSharedPtr<AwsCredentialsManager> credentialsManager;
    TSharedPtr<FeatureResourceManager> featureResourceManager;
    TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter;
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> messageEndpoint;

    void RegisterProjectSettings() const;
    void UnregisterProjectSettings() const;

public:
    /** IModuleInterface implementation */
    virtual void StartupModule() override;
    void BootstrapExistingState();
    virtual void ShutdownModule() override;

    TMap<FName, IGameKitEditorFeatureExample*> GetGameKitFeatureExamples() const;

    /**  EditorModule methods */
    void AddGameKitFeatureExample(FPropertyEditorModule& propertyModule, IGameKitEditorFeatureExample* gamekitFeature);
    void OpenProjectSettings();
    void OpenControlCenter();
    TSharedPtr<EditorState> GetEditorState() const;
    TSharedPtr<AwsCredentialsManager> GetCredentialsManager() const;
    TSharedPtr<FeatureResourceManager> GetFeatureResourceManager() const;
    TSharedPtr<AwsGameKitFeatureControlCenter> GetFeatureControlCenter() const;
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> GetMessageBus() const;
};
