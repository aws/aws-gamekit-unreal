// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "Utils/Blueprints/UAwsGameKitErrorUtils.h"

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitErrors.h>

FString UAwsGameKitErrorUtils::StatusCodeToHexFString(int statusCode)
{
    return GameKit::StatusCodeToHexFStr((unsigned int) statusCode);
}
