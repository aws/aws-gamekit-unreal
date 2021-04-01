// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "IDetailCustomization.h"
#include "Templates/SharedPointer.h"

/**
 * This class defines the custom Details Panel that goes with the Aws GameKit User Gameplay Data Examples Actor.
 */
class AWSGAMEKITEDITOR_API AwsGameKitUserGameplayDataExamplesLayout : public IDetailCustomization
{
public:
    /**
     * Make a new instance of this detail layout class for a specific detail view requesting it.
     */
    static TSharedRef<IDetailCustomization> MakeInstance();

    virtual void CustomizeDetails(IDetailLayoutBuilder& detailBuilder) override;
};
