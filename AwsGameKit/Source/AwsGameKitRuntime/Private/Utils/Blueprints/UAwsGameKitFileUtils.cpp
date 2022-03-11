// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Utils/Blueprints/UAwsGameKitFileUtils.h"

// GameKit
#include "AwsGameKitCore.h"
#include "AwsGameKitRuntime.h"
#include "Core/AwsGameKitErrors.h"
#include "Core/AwsGameKitMarshalling.h"
#include "SessionManager/AwsGameKitSessionManager.h"

// Unreal
#include "Core/Public/Misc/Paths.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "HAL/FileManagerGeneric.h"
#include "Misc/FileHelper.h"


bool UAwsGameKitFileUtils::GetFileLastModifiedTimestamp(const FString filePath, int64& outLastModifiedEpochMilliseconds)
{
    IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

    const FDateTime lastModifiedTimestampUTC = platformFile.GetTimeStamp(*filePath);
    if (lastModifiedTimestampUTC == FDateTime::MinValue())
    {
        outLastModifiedEpochMilliseconds = 0;
        return false;
    }

    outLastModifiedEpochMilliseconds = lastModifiedTimestampUTC.ToUnixTimestamp() * 1000; // Convert seconds to milliseconds
    return true;
}

int32 UAwsGameKitFileUtils::LoadFileIntoByteArray(const FString filePath, TArray<uint8>& fileContents)
{
    if (!FFileHelper::LoadFileToArray(fileContents, *filePath))
    {
        FString errorMessage = "ERROR: Unable to read file: " + filePath;
        UE_LOG(LogAwsGameKit, Error, TEXT("LoadFileIntoByteArray() %s"), *errorMessage);
        return GameKit::GAMEKIT_ERROR_FILE_READ_FAILED;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("LoadFileIntoByteArray() copied file %s into byte array"), *filePath);
    return GameKit::GAMEKIT_SUCCESS;
}

int32 UAwsGameKitFileUtils::SaveByteArrayToFile(const FString filePath, TArray<uint8>& fileContents)
{
    if (!FFileHelper::SaveArrayToFile(fileContents, *filePath))
    {
        FString errorMessage = "ERROR: Unable to save to file: " + filePath;
        UE_LOG(LogAwsGameKit, Error, TEXT("SaveByteArrayToFile() %s"), *errorMessage);
        return GameKit::GAMEKIT_ERROR_FILE_WRITE_FAILED;
    }

    UE_LOG(LogAwsGameKit, Display, TEXT("SaveByteArrayToFile() copied byte array to file %s"), *filePath);
    return GameKit::GAMEKIT_SUCCESS;
}

FString UAwsGameKitFileUtils::PickFile(const FString message, const FString fileTypes, const bool opening)
{
    TArray<FString> OpenFilenames;
    IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
    if (DesktopPlatform != nullptr)
    {
        if (opening)
        {
            DesktopPlatform->OpenFileDialog(
                FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
                message,
                FPaths::GetPath(FPaths::GetProjectFilePath()),
                FString(""),
                fileTypes,
                EFileDialogFlags::None,
                OpenFilenames);
        }
        else
        {
            DesktopPlatform->SaveFileDialog(
                FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr),
                message,
                FPaths::GetPath(FPaths::GetProjectFilePath()),
                FString(""),
                fileTypes,
                EFileDialogFlags::None,
                OpenFilenames);
        }

        if (OpenFilenames.Num() == 1)
        {
            IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
            
            if (opening)
            {
                return FileManager.ConvertToAbsolutePathForExternalAppForRead(*OpenFilenames[0]);
            }

            return FileManager.ConvertToAbsolutePathForExternalAppForWrite(*OpenFilenames[0]);
        }
        else if (OpenFilenames.Num() == 0)
        {
            UE_LOG(LogAwsGameKit, Display, TEXT("PickFile() file selection cancelled"))
        }
        else
        {
            UE_LOG(LogAwsGameKit, Error, TEXT("PickFile() multiple files selected. Must select only one file."))
        }
    }
    else
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("PickFile() Desktop platform could not be determined."))
    }

    return "";
}

FString UAwsGameKitFileUtils::GetFeatureSaveDirectory(const FeatureType_E featureType)
{
    return FPaths::Combine(
        FPaths::ProjectSavedDir(),
        FString("AwsGameKit"),
        AwsGameKitSessionManager::FeatureTypeToApiString(featureType)
    );
}

void UAwsGameKitFileUtils::GetFilesInDirectory(FFilePaths& result, const FString& directoryPath, const FString& fileExtension)
{
    IFileManager::Get().FindFiles(result.FilePaths, *directoryPath, *fileExtension);

    for (int32 i = 0; i < result.FilePaths.Num(); i++)
    {
        result.FilePaths[i] = FPaths::Combine(directoryPath, result.FilePaths[i]);
    }
}

void UAwsGameKitFileUtils::DeleteFile(const FString& path)
{
    FPlatformFileManager::Get().GetPlatformFile().DeleteFile(*path);
}
