// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "AwsGameKitCore.h"

// Unreal public module dependencies
#include "Engine.h"

#define LOCTEXT_NAMESPACE "FAwsGameKitCoreModule"

DEFINE_LOG_CATEGORY(LogAwsGameKit);

void FAwsGameKitCoreModule::StartupModule()
{
}

void FAwsGameKitCoreModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FAwsGameKitCoreModule, AwsGameKitCore)
