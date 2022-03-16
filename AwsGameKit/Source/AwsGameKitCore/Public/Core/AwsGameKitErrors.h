// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
 * @brief This the GameKit status codes and their helpers.
 *
 * @details View the status codes by clicking "Go to the source code of this file" at the top of the page.
 */

#pragma once

 // GameKit
#include <aws/gamekit/core/errors.h>

// Standard library
#include <string>

// Unreal
#include "Containers/UnrealString.h"
namespace GameKit
{
    // Convert a Status code to Hex string.

    /**
     * @brief Convert a GameKit status code into a Hex `string`.
     *
     * @details GameKit status codes are defined in errors.h
     */
    std::string AWSGAMEKITCORE_API StatusCodeToHexStr(const unsigned int statusCode);

    /**
     * @brief Convert a GameKit status code into a Hex `FString`.
     *
     * @details GameKit status codes are defined in errors.h
     */
    FString AWSGAMEKITCORE_API StatusCodeToHexFStr(const unsigned int statusCode);
}

/**
 * @brief Class that encapsulates a result and an error message.
 *
 * @tparam R The type of the Result.
 * @tparam E The type of the ErrorMessage.
*/
template <typename R, typename E>
struct OperationResult
{
    /**
     * @brief Create an empty result. Struct members are set to default() values.
     */
    OperationResult() : Result(), ErrorMessage()
    {}

    /**
     * @brief Create a result with an empty error message.
     */
    OperationResult(const R& result) : Result(result), ErrorMessage()
    {}

    /**
     * @brief Create a failed result with error information.
     */
    OperationResult(const R& result, const E& errorMessage) : Result(result), ErrorMessage(errorMessage)
    {}

    /**
     * @brief The result of the operation.
     */
    R Result;

    /**
     * @brief An optional error message. It may be empty even when `Result` indicates an error.
     */
    E ErrorMessage;
};

#define LOG_RESULT(category, verbosity, result) UE_LOG(category, verbosity, TEXT("Error %s: %s"), *GameKit::StatusCodeToHexFStr(result.Result), *result.ErrorMessage)

/**
 * @brief Encapsulates the result of a GameKit API call and an optional error message.
 *
 * @tparam R GameKit status code which indicates the result of the API call. Status codes are defined in errors.h. The API call's documentation will list the possible status codes that may be returned.
 * @tparam E Optional error message. It may be empty even when the OperationResult::Result indicates an error.
*/
typedef OperationResult<unsigned int, FString> IntResult;
typedef OperationResult<std::string, FString> StringResult;
