// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "ImageDownloader.h"

// GameKit
#include "AwsGameKitCore.h"

// Unreal
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Async/Async.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

IImageDownloaderPtr ImageDownloader::MakeInstance()
{
    return MakeShareable(new ImageDownloader());
}

void ImageDownloader::SetImageFromUrl(const FString& iconUrl, const TSharedPtr<GameKitImage>& iconImg, int retryCount)
{
    UE_LOG(LogAwsGameKit, Display, TEXT("ImageDownloader::SetImageFromUrl: %s; Attempts: %d"), *iconUrl, retryCount);
    if (iconUrl.IsEmpty() || iconImg == nullptr)
    {
        return;
    }

    // Request to download image and add it to the processed queue
    TSharedRef<class IHttpRequest, ESPMode::ThreadSafe> httpRequest = FHttpModule::Get().CreateRequest();
    httpRequest->SetURL(iconUrl);
    httpRequest->SetVerb(TEXT("GET"));
    httpRequest->OnProcessRequestComplete().BindThreadSafeSP(this, &ImageDownloader::HandleImageDownload);
    {
        FScopeLock lock(&this->downloadMutex);
        const ImageResource resource{ iconImg, retryCount };
        this->imageDownloads.Add(iconUrl, resource);
    }

    httpRequest->ProcessRequest();
}

void ImageDownloader::HandleImageDownload(FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded)
{
    // Get url from completed request
    const FString url = request->GetURL();
    UE_LOG(LogAwsGameKit, Display, TEXT("ImageDownloader::HandleImageDownload: %s"), *url);

    // Get the SImage widget that needs to be set from the processed queue
    bool found;
    ImageResource resource;
    {
        FScopeLock lock(&this->downloadMutex);
        found = this->imageDownloads.RemoveAndCopyValue(url, resource);
    }

    if (!succeeded)
    {
        UE_LOG(LogAwsGameKit, Error, TEXT("Failed to download %s; %d: %s"), *url, response->GetResponseCode(), *response->GetContentAsString());
        // enable so it doesn't keep retrying bad url every ui update (most likely local path).
        if (resource.iconImg != nullptr)
        {
            resource.iconImg->SetEnabled(true);
        }
        return;
    }

    if (!found || resource.iconImg == nullptr)
    {
        UE_LOG(LogAwsGameKit, Warning, TEXT("Cannot set SImage Widget for %s"), *url);
        return;
    }

    // Get response body of completed request
    IImageWrapperModule& imageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
    TSharedPtr<IImageWrapper> imgWrapper = imageWrapperModule.CreateImageWrapper(EImageFormat::PNG);
    TArray<uint8> imgData = response->GetContent();

    // Set image wrapper's compressed data
    bool valid = imgWrapper.IsValid();
    bool compressed = imgWrapper->SetCompressed(imgData.GetData(), imgData.Num());
    if (!EHttpResponseCodes::IsOk(response->GetResponseCode()) || !valid || !compressed)
    {
        // Retry
        AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [&, url, resource]()
        {
            if (resource.attempts < DOWNLOAD_MAX_ATTEMPTS)
            {
                UE_LOG(LogAwsGameKit, Warning, TEXT("Retrying to download image %s..."), *url);
                FPlatformProcess::Sleep(DOWNLOAD_RETRY_DELAY_IN_SECONDS);
                SetImageFromUrl(url, resource.iconImg, resource.attempts + 1);
            }
            else
            {
                UE_LOG(LogAwsGameKit, Error, TEXT("The image %s is not valid."), *url);
                AsyncTask(ENamedThreads::GameThread, [resource]()
                {
                    resource.iconImg->SetEnabled(true);
                });
            }
        });

        return;
    }

    // Get the raw data and decode
    uint32 bytesPerPixel = 4;
    uint32 bitDepth = 8;

    TArray<uint8> decodedImage;
    UE_LOG(LogAwsGameKit, Display, TEXT("Downloaded %s"), *url);
    if (imgWrapper->GetRaw(ERGBFormat::BGRA, bitDepth, decodedImage))
    {
        decodedImage.AddUninitialized(imgWrapper->GetWidth() * imgWrapper->GetHeight() * bytesPerPixel);
    }

    // Clear any previous dynamic brush
    if (resource.iconImg->brush != nullptr)
    {
        FSlateApplication::Get().GetRenderer()->ReleaseDynamicResource(*resource.iconImg->brush);
    }

    // Render decoded data into a brush
    const FName resName = FName(*url);
    if(FSlateApplication::Get().GetRenderer()->GenerateDynamicImageResource(resName, imgWrapper->GetWidth(), imgWrapper->GetHeight(), decodedImage))
    {
        const FSlateBrush* iconBrush = new FSlateDynamicImageBrush(resName, FVector2D(imgWrapper->GetWidth(), imgWrapper->GetHeight()));

        // Set the SImage widget's image
        resource.iconImg->SetEnabled(true);
        resource.iconImg->SetImage(iconBrush);
        resource.iconImg->brush = iconBrush;
        UE_LOG(LogAwsGameKit, Display, TEXT("SImage Widget set for %s"), *url);
    }
    else
    {
        UE_LOG(LogAwsGameKit, Display, TEXT("Was unable to generate dynamic image brush for %s"), *url);
    }
}
