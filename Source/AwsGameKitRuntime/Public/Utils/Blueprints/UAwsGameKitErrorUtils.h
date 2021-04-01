// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

// Unreal
#include "Containers/UnrealString.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "UAwsGameKitErrorUtils.generated.h" // Last include (Unreal requirement)

/**
 * @brief A library with useful utility functions for handling GameKit status codes that can be called from both Blueprint and C++.
 */
UCLASS()
class AWSGAMEKITRUNTIME_API UAwsGameKitErrorUtils : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Convert a GameKit status code from decimal integer to a hexadecimal string representation.
     *
     * For example, convert 69632 (GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND) to "0x11000".
     *
     * All GameKit status codes are defined in AwsGameKitErrors.h. Each method's documentation lists the possible status codes which the method may return.
     *
     * @param statusCode The GameKit status code in integer form.
     * 
     * @return The GameKit status code in hexadecimal string.
     */
    UFUNCTION(BlueprintCallable, Category = "AWS GameKit | Utilities | Errors")
    static FString StatusCodeToHexFString(int statusCode);
};
