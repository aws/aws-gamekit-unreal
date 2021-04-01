// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitMarshalling.h>
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
    const std::string INTRO_COST_URL = "https://docs.aws.amazon.com/gamekit/latest/DevGuide/intro-costs.html";

    // Handles
    const FAwsGameKitEditorModule* editorModule;

    // Feature management
    const FeatureType featureType;
    TSharedRef<SVerticalBox> GetDeployControls(const bool addSeparator = true);
    TSharedRef<SVerticalBox> GetFeatureHeader(const FText& featureDescription);
    bool CanEditConfiguration() const;
    virtual FReply DeployFeature();
    virtual FReply DeleteFeature();

    // Credential management
    TSharedPtr<FMessageEndpoint, ESPMode::ThreadSafe> messageEndpoint;
    virtual void CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context);

    IDetailCategoryBuilder* featureCategoryBuilder = nullptr;

public:
    AwsGameKitFeatureLayoutDetails(const FeatureType featureType, const FAwsGameKitEditorModule* editorModule);
};
