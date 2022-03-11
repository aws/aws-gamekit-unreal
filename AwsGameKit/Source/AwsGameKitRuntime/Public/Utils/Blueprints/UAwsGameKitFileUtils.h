// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// GameKit
#include "Models/AwsGameKitCommonModels.h"

// Unreal
#include "Containers/UnrealString.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SharedPointer.h"

#include "UAwsGameKitFileUtils.generated.h" // Last include (Unreal requirement)

USTRUCT(BlueprintType)
struct FFilePaths
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AWS GameKit | Utilities | Files")
    TArray<FString> FilePaths;
};

/**
 * @brief A library with useful utility functions for interacting with files that can be called from both Blueprint and C++.
 */
UCLASS()
class AWSGAMEKITRUNTIME_API UAwsGameKitFileUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * Get the number of milliseconds since epoch of the file's last modified UTC timestamp.
     *
     * @param filePath The absolute path of the target file.
     * @param outLastModifiedEpochMilliseconds Upon return, will contain the file's last modified timestamp in epoch milliseconds.
     *
     * @return True if the timestamp was returned successfully, false if there was an error.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Files")
    static bool GetFileLastModifiedTimestamp(const FString filePath, UPARAM(ref) int64& outLastModifiedEpochMilliseconds);

    /**
     * Loads the contents of a file into a byte array.
     *
     * @param filePath The absolute path of the target file.
     * @param fileContents The empty byte array to copy the file contents to.
     *
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult errors.h file for details.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Files")
    static int32 LoadFileIntoByteArray(const FString filePath, UPARAM(ref) TArray<uint8>& fileContents);

    /**
     * Saves the contents of a byte array to the provided file.
     * 
     * @param filePath The absolute path of the target file.
     * @param fileContents The byte array to write to the file.
     * 
     * @return GameKit status code, GAMEKIT_SUCCESS on success else non-zero value. Consult errors.h file for details.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Files")
    static int32 SaveByteArrayToFile(const FString filePath, UPARAM(ref) TArray<uint8>& fileContents);

    /**
     * Open a native file browser to let the user select a file. Supports Windows, MacOS, and Linux.
     * 
     * @return The absolute path of the chosen file, or an empty string if there was an error.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Files")
    static FString PickFile(const FString message, const FString fileTypes, const bool opening = true);

    /**
     * Get the folder where files are saved for the featureType.
     *
     * The folder's path is "<ProjectSavedDirectory>/AwsGameKit/<featureType>".
     *
     * ProjectSavedDirectory can be retrieved in Blueprints by calling the Unreal API: Utilities > Paths > Get Project Saved Directory
     * or in C++ by calling the Unreal API: FPaths::ProjectSavedDir()
     *
     * @return The save folder for the GameKit feature.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Files")
    static FString GetFeatureSaveDirectory(const FeatureType_E featureType);

    /**
    * Gets absolute paths of all files in specified directory.
    * 
    * @param result USTRUCT containing a single TArray member, which all paths will be copied to. 
    * @param directoryPath The absolute path of the directory you wish to return the contents of.
    * @param fileExtension The file extension of the files to search for. If NULL or an empty string "" then all files are found.
    * Otherwise fileExtension can be of the form .EXT or just EXT and only files with that extension will be returned.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Files")
    static void GetFilesInDirectory(UPARAM(ref) FFilePaths& result, UPARAM(ref) const FString& directoryPath, UPARAM(ref) const FString& fileExtension);

    /**
    * Deletes the specified file
    *
    * @param path The absolute path of the file you wish you delete.
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Files")
    static void DeleteFile(UPARAM(ref) const FString& path);
};
