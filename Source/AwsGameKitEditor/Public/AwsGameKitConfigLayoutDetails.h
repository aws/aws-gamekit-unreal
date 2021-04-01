// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitEditor.h"

// Unreal
#include "DetailCategoryBuilder.h"
#include "IDetailCustomization.h"

class AWSGAMEKITEDITOR_API AwsGameKitConfigLayoutDetails : public IDetailCustomization
{
private:
    IDetailCategoryBuilder* configCategoryBuilder = nullptr;
    FAwsGameKitEditorModule* editorModule;
    FReply OpenControlCenter();

public:
	AwsGameKitConfigLayoutDetails(FAwsGameKitEditorModule* editorModule);
	~AwsGameKitConfigLayoutDetails();

    /**
    * @brief Creates a new instance.
    * @return A new property type customization.
    */
    static TSharedRef<IDetailCustomization> MakeInstance(FAwsGameKitEditorModule* editorModule);

    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};
