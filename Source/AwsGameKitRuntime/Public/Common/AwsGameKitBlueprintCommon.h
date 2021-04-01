// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Models/AwsGameKitCommonModels.h"

#include "Async/Async.h"
#include "Engine/LatentActionManager.h"
#include "Engine/World.h"
#include "LatentActions.h"
#include "Misc/Optional.h"

UENUM()
enum class EAwsGameKitSuccessOrFailureExecutionPin : uint8
{
    OnSuccess, OnFailure
};

template <typename ResultType>
struct TAwsGameKitInternalActionState
{
    FAwsGameKitOperationResult Err;
    ResultType Results;
    TOptional<TQueue<ResultType>> PartialResultsQueue;
};

template <typename ResultType = FNoopStruct>
using TAwsGameKitInternalActionStatePtr = TSharedPtr<TAwsGameKitInternalActionState<ResultType>, ESPMode::ThreadSafe>;


template <typename RequestType, typename ResultType, typename PartialResultsDelegateType = FNoopStruct>
class AWSGAMEKITRUNTIME_API TAwsGameKitInternalThreadedAction : public FPendingLatentAction
{
public:
    // The output reference captures may look wildly unsafe, but when Blueprint calls latent actions with output parameters,
    // the parameters have stable heap addresses which are owned by the blueprint virtual machine. Note, the blueprint VM
    // may be destroyed during app shutdown (or other UObject cleanup) before the async action has completed, so the async
    // code MUST not reference the output variables directly. We proxy the output through a heap-allocated shared object.
    TAwsGameKitInternalThreadedAction(const FLatentActionInfo& LatentInfoParam, const RequestType& RequestParam, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailureParam, FAwsGameKitOperationResult& StatusParam, ResultType& ResultsParam, const PartialResultsDelegateType& PartialResultsDelegateParam)
        : ThreadedState(new TAwsGameKitInternalActionState<ResultType>),
        LatentInfo(LatentInfoParam),
        InRequest(RequestParam),
        OutSuccessOrFailure(SuccessOrFailureParam),
        OutResults(ResultsParam),
        OutStatus(StatusParam),
        PartialResultsDelegate(PartialResultsDelegateParam)
    {
        InitializePartialResultQueue(PartialResultsDelegate);
    }

    // Note: async threaded work may outlive this Action object or the entire Blueprint VM,
    // so any data being passed back to this Action needs to bounce via a shared heap object
    TAwsGameKitInternalActionStatePtr<ResultType> ThreadedState;

    // LaunchThreadedWork MUST be called immediately; the lambda should capture + fill ThreadedState,
    // and should stream partial result sets into ThreadedState->PartialResultsQueue if it is valid.
    // (If ThreadedState->PartialResultsQueue is not a valid object, it means that no partial-results
    // delegate was provided and there is no need to stream partial results via threadsafe queueing.)
    template <typename LambdaType>
    void LaunchThreadedWork(LambdaType&& Lambda)
    {
        ThreadedResult = Async(EAsyncExecution::Thread, MoveTemp(Lambda));
    }

private:
    // This override function is regularly called by the latent action manager
    virtual void UpdateOperation(FLatentResponse& Response) override
    {
        check(ThreadedResult.IsValid()); // If this check fires, it means Launch was not called
        if (ThreadedResult.IsReady())
        {
            DispatchPartialResults(PartialResultsDelegate, true);
            OutResults = MoveTemp(ThreadedState->Results);
            OutStatus = ThreadedState->Err;
            OutSuccessOrFailure = ThreadedState->Err.Status == GameKit::GAMEKIT_SUCCESS ? EAwsGameKitSuccessOrFailureExecutionPin::OnSuccess : EAwsGameKitSuccessOrFailureExecutionPin::OnFailure;
            Response.FinishAndTriggerIf(true, LatentInfo.ExecutionFunction, LatentInfo.Linkage, LatentInfo.CallbackTarget);
        }
        else
        {
            DispatchPartialResults(PartialResultsDelegate, false);
        }
    }


    // Note, partial-results logic is wrapped into overloaded helper functions to avoid repeating too
    // much Action logic for specialization of cases where we we don't have partial-result delegates

    static void InitializePartialResultQueue(FNoopStruct&)
    {}

    static void DispatchPartialResults(FNoopStruct&, bool)
    {}

    template <typename T>
    void InitializePartialResultQueue(T& Delegate)
    {
        if (Delegate.IsBound())
        {
            ThreadedState->PartialResultsQueue.Emplace();
        }
    }

    template <typename T>
    void DispatchPartialResults(T& Delegate, bool bThreadComplete)
    {
        if (!Delegate.IsBound())
            return;

        check(ThreadedState->PartialResultsQueue);

        bool bInvokedWithFinal = false;

        ResultType TempResults;
        while (ThreadedState->PartialResultsQueue->Dequeue(TempResults))
        {
            // Note: if !bThreadComplete, IsEmpty is unsafe since thread may still be producing
            bool bFinalInvoke = bThreadComplete && ThreadedState->PartialResultsQueue->IsEmpty();
            bInvokedWithFinal |= bFinalInvoke;
            Delegate.Execute(InRequest, TempResults, bFinalInvoke);
        }

        if (bThreadComplete && !bInvokedWithFinal)
        {
            ResultType EmptyResults;
            Delegate.Execute(InRequest, EmptyResults, true);
        }
    }

private:
    FLatentActionInfo LatentInfo;
    RequestType InRequest;
    EAwsGameKitSuccessOrFailureExecutionPin& OutSuccessOrFailure;
    ResultType& OutResults;
    FAwsGameKitOperationResult& OutStatus;
    PartialResultsDelegateType PartialResultsDelegate;
    TFuture<void> ThreadedResult;
};


template <typename RequestType, typename ResultType, typename StreamingDelegateType = FNoopStruct>
auto InternalMakeAwsGameKitThreadedAction(TAwsGameKitInternalActionStatePtr<ResultType>& State,
    const UObject* WorldContextObject, const FLatentActionInfo& LatentInfo,
    const RequestType& Request, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Status,
    ResultType& Results, const StreamingDelegateType& Delegate = StreamingDelegateType())
    -> TAwsGameKitInternalThreadedAction<RequestType, ResultType, StreamingDelegateType>*
{
    typedef TAwsGameKitInternalThreadedAction<RequestType, ResultType, StreamingDelegateType> ActionType;

    FLatentActionManager& LatentActionManager = WorldContextObject->GetWorld()->GetLatentActionManager();

    ActionType* Action = new ActionType(LatentInfo, Request, SuccessOrFailure, Status, Results, Delegate);
    LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
    State = Action->ThreadedState;
    return Action;
}

template <typename RequestType>
auto InternalMakeAwsGameKitThreadedAction(TAwsGameKitInternalActionStatePtr<>& State,
    const UObject* WorldContextObject, const FLatentActionInfo& LatentInfo,
    const RequestType& Request, EAwsGameKitSuccessOrFailureExecutionPin& SuccessOrFailure, FAwsGameKitOperationResult& Status)
    -> TAwsGameKitInternalThreadedAction<RequestType, FNoopStruct, FNoopStruct>*
{
    static FNoopStruct Result;
    return InternalMakeAwsGameKitThreadedAction(State, WorldContextObject, LatentInfo, Request, SuccessOrFailure, Status, Result, FNoopStruct());
}
