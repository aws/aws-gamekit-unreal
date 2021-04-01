// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
            
            sessionManagerLibrary.SessionManagerWrapper->ReloadConfig(sessionManagerLibrary.SessionManagerInstanceHandle);
            State->Err = FAwsGameKitOperationResult{};
        });
    }
}

void UAwsGameKitSessionManagerFunctionLibrary::AreSettingsLoaded(UObject* WorldContextObject,
    FLatentActionInfo LatentInfo,
    const FeatureType_E featureType,
    bool& Result,
    EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("UAwsGameKitSessionManagerFunctionLibrary::AreSettingsLoaded()"));

    TAwsGameKitInternalActionStatePtr<bool> State;
    FAwsGameKitOperationResult ResultStatus;
    FNoopStruct ResultDelegate;
    if (auto Action = InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, nullptr, SuccessOrFailure, ResultStatus, Result, ResultDelegate))
    {
        Action->LaunchThreadedWork([featureType, ResultStatus, State]
        {
            FAwsGameKitRuntimeModule* runtimeModule = FModuleManager::GetModulePtr<FAwsGameKitRuntimeModule>("AwsGameKitRuntime");
            SessionManagerLibrary sessionManagerLibrary = runtimeModule->GetSessionManagerLibrary();

            State->Results = sessionManagerLibrary.SessionManagerWrapper->GameKitSessionManagerAreSettingsLoaded(sessionManagerLibrary.SessionManagerInstanceHandle, AwsGameKitEnumConverter::ConvertFeatureEnum(featureType));
            State->Err = FAwsGameKitOperationResult{};
        });
    }
}
