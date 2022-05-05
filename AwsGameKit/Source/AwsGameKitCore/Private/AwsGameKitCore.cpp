// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitCore.h"

// Unreal
#include "Core/Logging.h"

// GameKit
#if PLATFORM_IOS
#include <aws/gamekit/core/exports.h>
#endif

#define LOCTEXT_NAMESPACE "FAwsGameKitCoreModule"

DEFINE_LOG_CATEGORY(LogAwsGameKit);

void FAwsGameKitCoreModule::StartupModule()
{
  UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitCoreModule::StartupModule()"));
#if PLATFORM_IOS
  ::GameKitInitializeAwsSdk(FGameKitLogging::LogCallBack);
#endif
}

void FAwsGameKitCoreModule::ShutdownModule()
{
  UE_LOG(LogAwsGameKit, Log, TEXT("FAwsGameKitCoreModule::ShutdownModule()"));
#if PLATFORM_IOS
  ::GameKitShutdownAwsSdk(FGameKitLogging::LogCallBack);
#endif
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAwsGameKitCoreModule, AwsGameKitCore);
