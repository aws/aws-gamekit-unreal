// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "Containers/UnrealString.h"
#include "Templates/SharedPointer.h"
#include "HttpModule.h"

// Unreal forward declarations
class SImage;

SLATECORE_API class GameKitImage :
    public SImage
{
public:
    const FSlateBrush* brush;
};

class IImageDownloader
{
public:
    IImageDownloader() {}
    virtual ~IImageDownloader() {}
    virtual void SetImageFromUrl(const FString& iconUrl, const TSharedPtr<GameKitImage>& iconImg, int retryCount) {};
    virtual void HandleImageDownload(FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded) {};
};

typedef TSharedPtr<IImageDownloader, ESPMode::ThreadSafe> IImageDownloaderPtr;

struct ImageResource
{
    TSharedPtr<GameKitImage> iconImg;
    int attempts;
};

class AWSGAMEKITEDITOR_API ImageDownloader :
    public TSharedFromThis<ImageDownloader, ESPMode::ThreadSafe>,
    public IImageDownloader
{
private:
    FCriticalSection downloadMutex;
    TMap<FString, ImageResource> imageDownloads;

public:
    static const int DOWNLOAD_MAX_ATTEMPTS = 5;
    static const int DOWNLOAD_RETRY_DELAY_IN_SECONDS = 1;

    static IImageDownloaderPtr MakeInstance();
    virtual void SetImageFromUrl(const FString& iconUrl, const TSharedPtr<GameKitImage>& iconImg, int retryCount) override;
    virtual void HandleImageDownload(FHttpRequestPtr request, FHttpResponsePtr response, bool succeeded) override;
};
