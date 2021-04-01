// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "EditorState.h"

// GameKit
#include "AwsGameKitCore.h"

// Unreal
#include "IMessageContext.h"
#include "MessageEndpoint.h"

const FString EditorState::EDITOR_STATE_SHORT_GAME_NAME = "shortName";
const FString EditorState::EDITOR_STATE_SELECTED_ENVIRONMENT = "environment";
const FString EditorState::EDITOR_STATE_ACCOUNT_ID = "accountId";
const FString EditorState::EDITOR_STATE_REGION = "region";
const FString EditorState::EDITOR_STATE_ACCESS_KEY = "accessKey";
const FString EditorState::EDITOR_STATE_ACCESS_SECRET = "accessSecret";
const FString EditorState::EDITOR_STATE_CREDENTIALS_SUBMITTED = "credentials_submitted";
const FString EditorState::EDITOR_STATE_FEATURES_TAB_OPEN = "features_tab_open";
const FString EditorState::TrueString = "true";
const FString EditorState::FalseString = "false";

void EditorState::SetCredentials(const AccountDetails& accountDetails)
{
    this->stateMap.Add(EDITOR_STATE_SELECTED_ENVIRONMENT) = accountDetails.environment;
    this->stateMap.Add(EDITOR_STATE_ACCOUNT_ID) = accountDetails.accountId;
    this->stateMap.Add(EDITOR_STATE_SHORT_GAME_NAME) = accountDetails.gameName;
    this->stateMap.Add(EDITOR_STATE_REGION) = accountDetails.region;
    this->stateMap.Add(EDITOR_STATE_ACCESS_KEY) = accountDetails.accessKey;
    this->stateMap.Add(EDITOR_STATE_ACCESS_SECRET) = accountDetails.accessSecret;
}

void EditorState::SetCredentialState(bool isSubmitted)
{
    this->stateMap.Add(EDITOR_STATE_CREDENTIALS_SUBMITTED) = isSubmitted ? TrueString : FalseString;
}

void EditorState::SetFeatureTabState(bool isOpen)
{
    this->stateMap.Add(EDITOR_STATE_FEATURES_TAB_OPEN) = isOpen ? TrueString : FalseString;
}

TMap<FString, FString> EditorState::GetCredentials() const
{
    TMap<FString, FString> creds;
    creds.Add(EDITOR_STATE_SELECTED_ENVIRONMENT, this->stateMap[EDITOR_STATE_SELECTED_ENVIRONMENT]);
    creds.Add(EDITOR_STATE_ACCOUNT_ID) = this->stateMap[EDITOR_STATE_ACCOUNT_ID];
    creds.Add(EDITOR_STATE_SHORT_GAME_NAME) = this->stateMap[EDITOR_STATE_SHORT_GAME_NAME];
    creds.Add(EDITOR_STATE_REGION) = this->stateMap[EDITOR_STATE_REGION];
    creds.Add(EDITOR_STATE_ACCESS_KEY) = this->stateMap[EDITOR_STATE_ACCESS_KEY];
    creds.Add(EDITOR_STATE_ACCESS_SECRET) = this->stateMap[EDITOR_STATE_ACCESS_SECRET];

    return creds;
}

bool EditorState::GetCredentialState() const
{
    auto credsState = this->stateMap.Find(EDITOR_STATE_CREDENTIALS_SUBMITTED);
    return credsState != nullptr && credsState->Equals(TrueString);
}

bool EditorState::GetFeatureTabState() const
{
    auto tabState = this->stateMap.Find(EDITOR_STATE_FEATURES_TAB_OPEN);
    return tabState != nullptr && tabState->Equals(TrueString);
}

bool EditorState::AreCredentialsValid() const
{
    auto env = this->stateMap.Find(EDITOR_STATE_SELECTED_ENVIRONMENT);
    auto acctId = this->stateMap.Find(EDITOR_STATE_ACCOUNT_ID);
    auto shortName = this->stateMap.Find(EDITOR_STATE_SHORT_GAME_NAME);
    auto region = this->stateMap.Find(EDITOR_STATE_REGION);
    auto key = this->stateMap.Find(EDITOR_STATE_ACCESS_KEY);
    auto secret = this->stateMap.Find(EDITOR_STATE_ACCESS_SECRET);

    return !(env == nullptr || env->IsEmpty())
        && !(acctId == nullptr || acctId->IsEmpty())
        && !(shortName == nullptr || shortName->IsEmpty())
        && !(region == nullptr || region->IsEmpty())
        && !(key == nullptr || key->IsEmpty())
        && !(secret == nullptr || secret->IsEmpty());
}

void EditorState::CredentialsStateMessageHandler(const FMsgCredentialsState& message, const TSharedRef<IMessageContext, ESPMode::ThreadSafe>& context)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("EditorState::CredentialsStateMessageHandler(); Message(%d)"), message.IsSubmitted);
    this->SetCredentialState(message.IsSubmitted);
}
