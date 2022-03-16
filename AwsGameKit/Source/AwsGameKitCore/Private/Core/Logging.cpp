// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Core/Logging.h"

// GameKit
#include "AwsGameKitCore.h"

// Unreal
#include "HAL/IConsoleManager.h"
#include "Misc/ScopeLock.h"

int32 FGameKitLogging::toggleVerbose = false;
TArray<IChildLogger*> FGameKitLogging::childLoggers;
FCriticalSection FGameKitLogging::childLoggerMutex;

TAutoConsoleVariable<int32> CVarGameKitToggleVerboseLevel(
    TEXT("GameKit.ToggleVerboseLevel"),
    0,
    TEXT("Activates or deactivates logging messages with Verbose level to the Log window.\n")
    TEXT("  0: deactivates\n")
    TEXT(" >0: activates\n"));

void FGameKitLogging::AttachLogger(IChildLogger* logger)
{
    FScopeLock scopeLock(&(FGameKitLogging::childLoggerMutex));
    UE_LOG(LogAwsGameKit, Log, TEXT("FGameKitLogging::AttachLogger()"))
    childLoggers.Add(logger);
}

void FGameKitLogging::DetachLogger(IChildLogger* logger)
{
    FScopeLock scopeLock(&(FGameKitLogging::childLoggerMutex));
    UE_LOG(LogAwsGameKit, Log, TEXT("FGameKitLogging::DetachLogger()"))
    childLoggers.Remove(logger);
}

void FGameKitLogging::LogCallBack(unsigned int level, const char* message, int size)
{
    switch (level)
    {
    case 1:
        toggleVerbose = CVarGameKitToggleVerboseLevel.GetValueOnAnyThread();
        if (toggleVerbose)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("%s"), *FString(ANSI_TO_TCHAR(message)));
        }
        else
        {
            UE_LOG(LogAwsGameKit, Verbose, TEXT("%s"), *FString(ANSI_TO_TCHAR(message)));
        }
        break;
    case 2:
        UE_LOG(LogAwsGameKit, Display, TEXT("%s"), *FString(ANSI_TO_TCHAR(message)));
        break;
    case 3:
        UE_LOG(LogAwsGameKit, Warning, TEXT("%s"), *FString(ANSI_TO_TCHAR(message)));
        break;
    case 4:
        UE_LOG(LogAwsGameKit, Error, TEXT("%s"), *FString(ANSI_TO_TCHAR(message)));
        break;
    default:
        UE_LOG(LogAwsGameKit, Display, TEXT("%s"), *FString(ANSI_TO_TCHAR(message)));
        break;
    }

    for (auto logger : childLoggers)
    {
        logger->Log(level, FString(message));
    }
}
