// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitFeatureLayoutDetails.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"

// Unreal forward declarations
class FMessageEndpoint;

class AWSGAMEKITEDITOR_API AwsGameKitUserGameplayDataLayoutDetails : public AwsGameKitFeatureLayoutDetails
{
public:

    AwsGameKitUserGameplayDataLayoutDetails(const FAwsGameKitEditorModule* editorModule);
    ~AwsGameKitUserGameplayDataLayoutDetails() = default;

    /**
    * @brief Creates a new instance.
    * @return A new property type customization.
    */
    static TSharedRef<IDetailCustomization> MakeInstance(const FAwsGameKitEditorModule* editorModule);

    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};
