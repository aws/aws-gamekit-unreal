// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#pragma once

// Unreal
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UAwsGameKitLifecycleUtils.generated.h" // Last include (Unreal requirement)

/**
 * @brief A library with useful utility functions for GameKit lifecycle management
 */
UCLASS()
class AWSGAMEKITRUNTIME_API UAwsGameKitLifecycleUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
    * Shutsdown GameKit Modules
    */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Lifecycle")
    static void ShutdownGameKit();
};
