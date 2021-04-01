// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Async/Async.h"


// FAwsGameKitInternalTempStrings is a helper class meant to be used as a callable object
// that translates string parameters (char*, TCHAR*, or FString) into an appropriate form
// for storing in a temporary char* variable, duplicating the value into the heap for as
// long as the helper object is in scope.
// 
// For example, given some FString variables and a GameKit model that looks like this:
//   struct Model { char* Value; char* Value2; };
//   void GameKitFunction(const Model& modelInput);
// 
// You could be tempted to write the following which is INCORRECT:
//   Model modelInstance = { TCHAR_TO_UTF8(*UnrealString), TCHAR_TO_UTF8(*OtherString) };
//   ...
//   GameKitFunction(modelInstance /* potential crash on read, or corrupt data */);
// 
// The problem is that TCHAR_TO_UTF8 uses a temporary stack object that will be destroyed
// and freed for reuse by other stack variables at the end of its enclosing statement.
// Instead, do this:
//   FAwsGameKitInternalTempStrings ConvertString;
//   Model modelInstance = { ConvertString(*UnrealString), ConvertString(*OtherString) };
//   ...
//   GameKitFunction(modelInstance);
//
class FAwsGameKitInternalTempStrings
{
public:
    FAwsGameKitInternalTempStrings()
    {
    }

    ~FAwsGameKitInternalTempStrings()
    {
        for (char* Str : OwnedStrings)
        {
            delete[] Str;
        }
    }

    char* Dup(const char* Str)
    {
        size_t Len = strlen(Str);
        char* Buffer = new char[Len + 1];
        FPlatformMemory::Memcpy(Buffer, Str, Len + 1);
        OwnedStrings.Add(Buffer);
        return Buffer;
    }

    char* operator()(const char* Str)
    {
        return Dup(Str);
    }

    char* operator()(const wchar_t* Str)
    {
        return Dup(TCHAR_TO_UTF8(Str));
    }

    char* operator()(const FString& Str)
    {
        return Dup(TCHAR_TO_UTF8(*Str));
    }

private:
    TArray<char*,TInlineAllocator<4>> OwnedStrings;
};


template <typename T>
inline void InternalAwsGameKitRunLambdaOnWorkThread(T&& Work)
{
    Async(EAsyncExecution::Thread, Forward<T>(Work));
}


class FAwsGameKitInternalMainThreadOrderedTask
{
public:
    FAwsGameKitInternalMainThreadOrderedTask(TUniqueFunction<void()>&& InFunction) : Function(MoveTemp(InFunction)) {}
    void DoTask(ENamedThreads::Type, const FGraphEventRef&) { Function(); }
    ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }
    static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
    TStatId GetStatId() const { return GET_STATID(STAT_TaskGraph_OtherTasks); }
private:
    TUniqueFunction<void()> Function;
};

template <typename DelegateType, typename ParamType>
inline void InternalAwsGameKitRunDelegateOnGameThread(FGraphEventRef& OrderedWorkChain, const DelegateType& Delegate, ParamType&& Param)
{
    TUniqueFunction<void()> Function([Delegate, Param = Forward<ParamType>(Param)]{ Delegate.ExecuteIfBound(Param); });
    if (OrderedWorkChain && !OrderedWorkChain->IsComplete())
    {
        FGraphEventArray Prereqs;
        Prereqs.Add(MoveTemp(OrderedWorkChain));
        OrderedWorkChain = TGraphTask<FAwsGameKitInternalMainThreadOrderedTask>::CreateTask(&Prereqs).ConstructAndDispatchWhenReady(MoveTemp(Function));
    }
    else
    {
        OrderedWorkChain = TGraphTask<FAwsGameKitInternalMainThreadOrderedTask>::CreateTask().ConstructAndDispatchWhenReady(MoveTemp(Function));
    }
}

template <typename DelegateType, typename Param1Type, typename Param2Type>
inline void InternalAwsGameKitRunDelegateOnGameThread(FGraphEventRef& OrderedWorkChain, const DelegateType& Delegate, Param1Type&& Param1, Param2Type&& Param2)
{
    TUniqueFunction<void()> Function([Delegate, Param1 = Forward<Param1Type>(Param1), Param2 = Forward<Param2Type>(Param2)]{ Delegate.ExecuteIfBound(Param1, Param2); });
    if (OrderedWorkChain && !OrderedWorkChain->IsComplete())
    {
        FGraphEventArray Prereqs;
        Prereqs.Add(MoveTemp(OrderedWorkChain));
        OrderedWorkChain = TGraphTask<FAwsGameKitInternalMainThreadOrderedTask>::CreateTask(&Prereqs).ConstructAndDispatchWhenReady(MoveTemp(Function));
    }
    else
    {
        OrderedWorkChain = TGraphTask<FAwsGameKitInternalMainThreadOrderedTask>::CreateTask().ConstructAndDispatchWhenReady(MoveTemp(Function));
    }
}
