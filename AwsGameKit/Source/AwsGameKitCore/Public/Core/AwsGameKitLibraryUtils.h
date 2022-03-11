// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "Logging.h"

// Helper macro to define a Func handle type and instantiate it (set to nullptr).
#define DEFINE_FUNC_HANDLE(RetType, Func, ...) \
typedef RetType (*__##Func) __VA_ARGS__ ; \
__##Func func##Func = nullptr;

// Helper macro to check that the function pointer is valid. If function is invalid,
// logs a message and returns an error code. (Assumes the FuncPtr was declared with DEFINE_FUNC_HANDLE)
#if PLATFORM_WINDOWS || PLATFORM_MAC
#define CHECK_PLUGIN_FUNC_IS_LOADED(Plugin, FuncPtr, ...) \
{ \
    if (func##FuncPtr == nullptr) \
    { \
        UE_LOG(LogAwsGameKit, Error, TEXT("AWS GameKit " #Plugin " Plugin Function (" #FuncPtr ") is null")); \
        return __VA_ARGS__ ; \
    } \
}
#else
#define CHECK_PLUGIN_FUNC_IS_LOADED(Plugin, FuncPtr, ...) {}
#endif

// Helper macro to invoke a Func that was declared with DEFINE_FUNC_HANDLE
#if PLATFORM_WINDOWS || PLATFORM_MAC
#define INVOKE_FUNC(Func, ...) (func##Func)(__VA_ARGS__)
#else
#define INVOKE_FUNC(Func, ...) (::Func)(__VA_ARGS__)
#endif

// Helper macro to assign an exported Func (Func must be declared with DEFINE_FUNC_HANDLE)
#if PLATFORM_WINDOWS || PLATFORM_MAC
#define LOAD_PLUGIN_FUNC(ProcName, DllHandle) func##ProcName = (__##ProcName)FPlatformProcess::GetDllExport(DllHandle, UTF8_TO_TCHAR(#ProcName))
#else
#define LOAD_PLUGIN_FUNC(ProcName, DllHandle) {}
#endif
