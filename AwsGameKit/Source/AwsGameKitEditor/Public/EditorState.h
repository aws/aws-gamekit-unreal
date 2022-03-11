// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitMarshalling.h>

// Unreal
#include "Containers/UnrealString.h"

// Unreal forward declarations
class IMessageContext;

// Last include (Unreal requirement)
#include "EditorState.generated.h"

USTRUCT()
struct FMsgCredentialsState
{
    GENERATED_BODY()
    bool IsSubmitted;
};

USTRUCT()
struct FMsgDeploymentState
{
    GENERATED_BODY()
    FeatureType FeatureType;
};

class EditorState
{
private:
    TMap<FString, FString> stateMap;
public:
    static const FString EDITOR_STATE_SHORT_GAME_NAME;
    static const FString EDITOR_STATE_SELECTED_ENVIRONMENT;
    static const FString EDITOR_STATE_ACCOUNT_ID;
    static const FString EDITOR_STATE_REGION;
    static const FString EDITOR_STATE_ACCESS_KEY;
    static const FString EDITOR_STATE_ACCESS_SECRET;
    static const FString EDITOR_STATE_CREDENTIALS_SUBMITTED;
    static const FString TrueString;
    static const FString FalseString;

    void SetCredentials(const AccountDetails& accountDetails);
    void SetCredentialState(bool isSubmitted);

    TMap<FString, FString> GetCredentials() const;
    bool GetCredentialState() const;
    bool AreCredentialsValid() const;

    // MessageBus Handlers
    void CredentialsStateMessageHandler(const struct FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context);
};
