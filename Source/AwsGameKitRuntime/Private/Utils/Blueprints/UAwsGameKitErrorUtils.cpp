// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/Blueprints/UAwsGameKitErrorUtils.h"

// GameKit
#include <AwsGameKitCore/Public/Core/AwsGameKitErrors.h>

FString UAwsGameKitErrorUtils::StatusCodeToHexFString(int statusCode)
{
    return GameKit::StatusCodeToHexFStr((unsigned int) statusCode);
}