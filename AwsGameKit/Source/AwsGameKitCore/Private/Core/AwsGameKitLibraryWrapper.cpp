// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "Core/AwsGameKitLibraryWrapper.h"

// GameKit
#include "AwsGameKitCore.h"

#define LOCTEXT_NAMESPACE "AwsGameKitLibraryWrapper"

static const std::string WINDOWS_LIBRARY_EXTENSION = ".dll";
static const std::string MAC_LIBRARY_EXTENSION = ".dylib";

bool AwsGameKitLibraryWrapper::Initialize()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitLibraryWrapper::Initialize()"));
    return loadDll();
}

void AwsGameKitLibraryWrapper::Shutdown()
{
    UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitLibraryWrapper::Shutdown()"));
    freeDll();
}

bool AwsGameKitLibraryWrapper::loadDll()
{
#if PLATFORM_WINDOWS || PLATFORM_MAC
    libraryPath = getPlatformDependentFilename().c_str();
    dllHandle = !libraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*libraryPath) : nullptr;

    if (dllHandle)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitLibraryWrapper::loadDll(); DLL Loaded: %s"), *libraryPath);
        importFunctions(dllHandle);
        return true;
    }
    else
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("Failed to load AWS GameKit library: %s"), *libraryPath);
        return false;
    }
#else // Libraries are statically compiled
    // Always return true if the the GameKit libraries are statically compiled
    return true;
#endif
}

void AwsGameKitLibraryWrapper::freeDll()
{
    if (dllHandle)
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("AwsGameKitLibraryWrapper::freeDll(); DLL Unloaded: %s"), *libraryPath);
        FPlatformProcess::FreeDllHandle(dllHandle);
        dllHandle = nullptr;
    }
}

std::string AwsGameKitLibraryWrapper::getPlatformDependentFilename()
{
    const std::string filename = getLibraryFilename();

#if PLATFORM_WINDOWS
    return filename + WINDOWS_LIBRARY_EXTENSION;
#elif PLATFORM_MAC
    FString projectPath = FPaths::ProjectDir();
    projectPath = IFileManager::Get().ConvertToAbsolutePathForExternalAppForRead(*projectPath);
    return std::string(TCHAR_TO_UTF8(*projectPath)) + "Binaries/Mac/" + filename + MAC_LIBRARY_EXTENSION;
#elif PLATFORM_IOS
    return "";
#elif PLATFORM_ANDROID
    return "";
#elif PLATFORM_LINUX
    return filename + "LINUX_LIBRARY_NOT_IMPLEMENTED_YET";
#else
    return filename + "UNKNOWN_PLATFORM_NOT_IMPLEMENTED_YET";
#endif
}

#undef LOCTEXT_NAMESPACE
