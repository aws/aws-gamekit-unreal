// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"

class AWSGAMEKITEDITOR_API AwsGameKitSettingsLayoutDetails : public IDetailCustomization
{
private:
    TSharedPtr<IDetailCustomization> credentialsLayout;
    TSharedPtr<IDetailCustomization> achievementsLayout;
    TSharedPtr<IDetailCustomization> gameSavingLayout;
    TSharedPtr<IDetailCustomization> identityLayout;
    TSharedPtr<IDetailCustomization> userGameplayDataLayout;
    
public:
    AwsGameKitSettingsLayoutDetails();

    /**
    * @brief Creates a new instance.
    * @return A new property type customization.
    */
    static TSharedRef<IDetailCustomization> MakeInstance();

    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};