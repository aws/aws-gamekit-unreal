// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Standard Library
#include <string>

// Unreal
#include "Containers/UnrealString.h"
#include "UObject/NoExportTypes.h"

enum class FeatureType
{
    Main,
    Identity,
    Authentication,
    Achievements,
    GameStateCloudSaving,
    UserGameplayData
};


/**
 * Base class for all AWS GameKit library wrappers.
 */
class AWSGAMEKITCORE_API AwsGameKitLibraryWrapper
{
private:
    FString libraryPath;
    void* dllHandle = nullptr;

    std::string getPlatformDependentFilename();

    /**
     * Load the DLL from disk. Must be called before using any of the wrapped APIs.
     *
     * @return True if the DLL was successfully loaded. False otherwise.
     */
    bool loadDll();

protected:
    /**
     * Get the library's filename without it's extension.
     */
    virtual std::string getLibraryFilename() = 0;

    /**
     * Import the function pointers from the loadedDllHandle.
     *
     * Is only called if ::loadDll() was successful and loadedDllHandle is non-null.
     */
    virtual void importFunctions(void* loadedDllHandle) = 0;

    /**
     * Release the DLL handle. Must be called before this object is destroyed to prevent a memory leak.
     */
    void freeDll();

public:
    AwsGameKitLibraryWrapper() {};
    virtual ~AwsGameKitLibraryWrapper() {};

    /**
     * Load the DLL from disk. Must be called before using any of the wrapped APIs.
     *
     * @return True if the DLL was successfully loaded. False otherwise.
     */
    virtual bool Initialize();

    /**
     * Release resources to prevent a memory leak.
     */
    virtual void Shutdown();
};
