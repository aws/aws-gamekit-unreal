// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitMarshalling.h>
#include "AwsGameKitDocumentationManager.h"
#include "AwsGameKitEditor.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"
#include "Widgets/SBoxPanel.h"

// Unreal forward declarations
class FMessageEndpoint;

class AWSGAMEKITEDITOR_API AwsGameKitFeatureLayoutDetails : public IDetailCustomization
{
protected:
    // URL for first deployment warning
    const FString INTRO_COST_URL = AwsGameKitDocumentationManager::GetDocumentString("dev_guide_url", "intro_cost");

    // Handles
    const FAwsGameKitEditorModule* editorModule;

    // Feature management
    const FeatureType featureType;
    TSharedRef<SVerticalBox> GetDeployControls(const bool addSeparator = true);
    TSharedRef<SVerticalBox> GetFeatureFooter(const FText& featureDescription);
    bool CanEditConfiguration() const;
    virtual FReply DeployFeature();
    virtual FReply DeleteFeature();

    bool ShowDashboardLink(const TSharedPtr<AwsGameKitFeatureControlCenter> featureControlCenter, const TSharedPtr<FeatureResourceManager> featureResourceManager);

    // Credential management
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> messageEndpoint;
    virtual void CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context);

    IDetailCategoryBuilder* featureCategoryBuilder = nullptr;

public:
     static const FString GAMEKIT_CLOUDWATCH_DASHBOARD_ENABLED;

    AwsGameKitFeatureLayoutDetails(const FeatureType featureType, const FAwsGameKitEditorModule* editorModule);
};
