// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "AwsGameKitFeatureLayoutDetails.h"

// Unreal
#include "IDetailCustomization.h"
#include "Containers/UnrealString.h"
#include "DetailCategoryBuilder.h"
#include "Templates/SharedPointer.h"

// Unreal forward declarations
class FMessageEndpoint;
class IMessageContext;
class SButton;
template <typename>
class SSpinBox;
class SVerticalBox;
struct FMsgCredentialsState;

class AWSGAMEKITEDITOR_API AwsGameKitGameSavingLayoutDetails : public AwsGameKitFeatureLayoutDetails
{
private:
    // UI elements
    TSharedPtr<SSpinBox<int>> maximumCloudSaveSlotsPerPlayer;

    // Event Handlers
    void SliderValueChanged(const int newValue);
    void LoadFeatureVars();

    // Deployment override
    FReply DeployFeature() override;

    // MessageBus & Handler
    virtual void CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context) override;

public:
    static const FString MAX_SAVE_SLOTS_PER_PLAYER;
    static const int DEFAULT_MAX_SAVE_SLOTS_PER_PLAYER;
    static const int SLIDER_MINIMUM_MAX_SAVE_SLOTS_PER_PLAYER;

    /**
     * The maximum value the UI slider allows while dragging the slider bar.
     *
     * To use a larger value for your game, type the desired number into the UI instead of dragging the slider, or increase the value below:
     */
    static const int SLIDER_MAXIMUM_MAX_SAVE_SLOTS_PER_PLAYER;

    /**
     * The maximum value that can be manually typed into the UI slider field.
     *
     * This value is "effectively infinite" in case a game developer wants no limit on the number of cloud save slots each player may use.
     *
     * It's necessary to have a limit, otherwise the UI allows for integer overflow. For example, typing "9999999999" into the UI results in "-2147483648" being written to the settings file.
     *
     * The number 100,000,000 was selected because: it's small enough to prevent integer overflow, it's large enough to be effectively infinite, and it looks nice.
     */
    static const int SLIDER_MANUALLY_ENTERED_MAXIMUM_MAX_SAVE_SLOTS_PER_PLAYER;

    AwsGameKitGameSavingLayoutDetails(const FAwsGameKitEditorModule* editorModule);
    ~AwsGameKitGameSavingLayoutDetails() = default;

    /**
    * @brief Creates a new instance.
    * @return A new property type customization.
    */
    static TSharedRef<IDetailCustomization> MakeInstance(const FAwsGameKitEditorModule* editorModule);

    // IDetailCustomization interface
    virtual void CustomizeDetails(IDetailLayoutBuilder& DetailLayout) override;
};
