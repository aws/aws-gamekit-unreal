// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
 * @brief Delegate type aliases which provide clarity for GameKit function signatures.
 */

#pragma once

#include "AwsGameKitCore/Public/Core/AwsGameKitErrors.h"
#include "Delegates/Delegate.h"

/**
 * @brief A function with the signature `void Function(Param1, Param2, ... ParamN)`.
*/
template <typename ...Params> using TAwsGameKitDelegate = TDelegate<void(Params...)>;

/**
 * @brief A reference to a bound delegate which can be executed by a GameKit API.
 *
 * @details The delegate must have a signature matching TAwsGameKitDelegate, which is `void Function(Param1, Param2, ... ParamN)`.
*/
template <typename ...Params> using TAwsGameKitDelegateParam = const TAwsGameKitDelegate<Params...>&;

/**
 * @brief A function with the signature `void Function(IntResult)`. See ::IntResult.
 *
 * @details This delegate is for GameKit APIs which return only a status code, no response object.
 */
DECLARE_DELEGATE_OneParam(FAwsGameKitStatusDelegate, const IntResult&);

/**
 * @brief A reference to a bound delegate which can be executed by a GameKit API.
 *
 * @details The delegate must have a signature matching FAwsGameKitStatusDelegate, which is `void Function(IntResult)`. See ::IntResult.
 */
using FAwsGameKitStatusDelegateParam = const FAwsGameKitStatusDelegate&;

/**
 * @brief Helper function to reduce verbosity of TAwsGameKitDelegate<DelegateParams...>::CreateUObject(...) by deducing template parameters.
*/
template <typename UObjectDerivedType, typename ...DelegateParams>
TAwsGameKitDelegate<DelegateParams...> MakeAwsGameKitDelegate(UObjectDerivedType* Obj, void(UObjectDerivedType::* MemFn)(DelegateParams...))
{
    return TAwsGameKitDelegate<DelegateParams...>::CreateUObject(Obj, MemFn);
}

/**
 * @brief Semi-internal helper to adapt partial-result functions into a combined-result delegate.
*/
template <typename ResultElementType>
class TAwsGameKitResultArrayGatherer
{
public:
    typedef TAwsGameKitDelegate<const TArray<ResultElementType>&> PartialDelegateType;
    typedef TAwsGameKitDelegate<const IntResult&, const TArray<ResultElementType>&> CombinedDelegateType;

    TAwsGameKitResultArrayGatherer(const CombinedDelegateType& Delegate)
        : Delegate(Delegate)
    {}

    template <typename UObjectDerivedType>
    TAwsGameKitResultArrayGatherer(UObjectDerivedType* Obj, void(UObjectDerivedType::* MemFn)(const IntResult&, const TArray<ResultElementType>&))
        : Delegate(CombinedDelegateType::CreateUObject(Obj, MemFn))
    {}

    template <typename LambdaOrFunction>
    TAwsGameKitResultArrayGatherer(LambdaOrFunction&& Lambda)
        : Delegate(CombinedDelegateType::CreateLambda(Forward<LambdaOrFunction>(Lambda)))
    {}

    PartialDelegateType OnResult()
    {
        auto Lambda = [Gathered = this->Gathered](const TArray<ResultElementType>& Partial)
        {
            // These const_cast and MoveTemp calls are safe due to our internal delegate call patterns.
            if (Gathered->Num() == 0)
            {
                *Gathered = MoveTemp(const_cast<TArray<ResultElementType>&>(Partial));
            }
            else
            {
                Gathered->Append(MoveTemp(const_cast<TArray<ResultElementType>&>(Partial)));
            }
        };
        return PartialDelegateType::CreateLambda(MoveTemp(Lambda));
    }

    FAwsGameKitStatusDelegate OnStatus()
    {
        auto Lambda = [Gathered = this->Gathered, Delegate = this->Delegate](const IntResult& Status)
        {
            Delegate.ExecuteIfBound(Status, *Gathered);
        };
        return FAwsGameKitStatusDelegate::CreateLambda(MoveTemp(Lambda));
    }

private:
    CombinedDelegateType Delegate;
    TSharedPtr<TArray<ResultElementType>> Gathered = TSharedPtr<TArray<ResultElementType>>(new TArray<ResultElementType>);
};

