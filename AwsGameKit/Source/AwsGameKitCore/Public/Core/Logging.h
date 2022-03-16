// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "Containers/UnrealString.h"
#include "HAL/CriticalSection.h"

/**
 * Signature for a callback function the AWS GameKit library can use to log a message.
 */
typedef void(*FuncLogCallback)(unsigned int level, const char* message, int size);

/**
 * Interface that defines a Child logger. Use to forward logging messages.
 */
class IChildLogger
{
public:
    IChildLogger() {}
    virtual ~IChildLogger() {}
    virtual void Log(unsigned int level, const FString& message) {};
};

/**
 * Default implementation for ::FuncFuncLogCallback.
 */
class AWSGAMEKITCORE_API FGameKitLogging
{
private:
    static int32 toggleVerbose;
    static TArray<IChildLogger*> childLoggers;
    static FCriticalSection childLoggerMutex;

public:
    static void AttachLogger(IChildLogger* logger);
    static void DetachLogger(IChildLogger* logger);
    static void LogCallBack(unsigned int level, const char* message, int size);
};
