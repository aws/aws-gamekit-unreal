// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "SessionManager/AwsGameKitSessionManagerFunctionLibrary.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "Core/AwsGameKitErrors.h"
#include "Models/AwsGameKitEnumConverter.h"

// Unreal
#include "LatentActions.h"

UAwsGameKitSessionManagerFunctionLibrary::UAwsGameKitSessionManagerFunctionLibrary(const FObjectInitializer& Initializer)
    : UBlueprintFunctionLibrary(Initializer)
{}

void UAwsGameKitSessionManagerFunctionLibrary::ReloadConfig(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitSessionManagerFunctionLibrary::ReloadConfig()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();

#if WITH_EDITOR
            // This call is only needed in Editor mode. Packaged builds will load the configuration when the FAwsGameKitRuntimeModule module is loaded.
            sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
#else
            UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitSessionManagerFunctionLibrary::ReloadConfig(): No-op in non-Editor build."));
#endif

            State->Err = FAwsGameKitOperationResult{};
        });
    }
}

bool UAwsGameKitSessionManagerFunctionLibrary::AreSettingsLoaded(const FeatureType_E featureType)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitSessionManagerFunctionLibrary::AreSettingsLoaded()"));

    FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
    SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();

    return sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerAreSettingsLoaded(sessionManagerLibrary.SessionManagerInstanceHandle, AwsGameKitEnumConverter::ConvertFeatureEnum(featureType));
}

void UAwsGameKitSessionManagerFunctionLibrary::SetToken(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FSetTokenRequest& Request,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure,
    FAwsGameKitOperationResult& Error)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitSessionManagerFunctionLibrary::SetToken()"));

    TAwsGameKitInternalActionStatePtr<> State;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, Error))
    {
        Action->LaunchThreadedWork([Request, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();

            sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerSetToken(sessionManagerLibrary.SessionManagerInstanceHandle, AwsGameKitEnumConverter::ConvertTokenTypeEnum(Request.TokenType), TCHAR_TO_UTF8(*Request.TokenValue));
            State->Err = FAwsGameKitOperationResult{};
        });
    }
}
