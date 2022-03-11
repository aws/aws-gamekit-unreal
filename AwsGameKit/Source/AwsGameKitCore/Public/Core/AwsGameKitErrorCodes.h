// Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

/** @file
* @brief The Blueprint-friendly GameKit status codes. This was generated by a script, do not modify!"
*/

#pragma once

// GameKit
#include <aws/gamekit/core/errors.h>

// Unreal
#include "UObject/Object.h"
#include "Containers/UnrealString.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AwsGameKitErrorCodes.generated.h"

/**
* @brief The Blueprint-friendly GameKit status codes.
*/
UCLASS()
class AWSGAMEKITCORE_API UAwsGameKitErrorCodes : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    static const FString UnknownError;
    static const TMap<int32, FString> CodeToFriendlyName;

public:
    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static const FString& GetGameKitErrorCodeFriendlyName(int32 ErrorCode)
    {
        const FString* name = CodeToFriendlyName.Find(ErrorCode);

        return name != nullptr ? *name : UnknownError;
    }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_SUCCESS() { return GameKit::GAMEKIT_SUCCESS; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_INVALID_PROVIDER() { return GameKit::GAMEKIT_ERROR_INVALID_PROVIDER; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_PARAMETERS_FILE_SAVE_FAILED() { return GameKit::GAMEKIT_ERROR_PARAMETERS_FILE_SAVE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_FILE_SAVE_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_FILE_SAVE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FUNCTIONS_COPY_FAILED() { return GameKit::GAMEKIT_ERROR_FUNCTIONS_COPY_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_SETTINGS_FILE_SAVE_FAILED() { return GameKit::GAMEKIT_ERROR_SETTINGS_FILE_SAVE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_NO_ID_TOKEN() { return GameKit::GAMEKIT_ERROR_NO_ID_TOKEN; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_HTTP_REQUEST_FAILED() { return GameKit::GAMEKIT_ERROR_HTTP_REQUEST_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_PARSE_JSON_FAILED() { return GameKit::GAMEKIT_ERROR_PARSE_JSON_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED() { return GameKit::GAMEKIT_ERROR_SETTINGS_FILE_READ_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FILE_OPEN_FAILED() { return GameKit::GAMEKIT_ERROR_FILE_OPEN_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FILE_WRITE_FAILED() { return GameKit::GAMEKIT_ERROR_FILE_WRITE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FILE_READ_FAILED() { return GameKit::GAMEKIT_ERROR_FILE_READ_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_DIRECTORY_CREATE_FAILED() { return GameKit::GAMEKIT_ERROR_DIRECTORY_CREATE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_DIRECTORY_NOT_FOUND() { return GameKit::GAMEKIT_ERROR_DIRECTORY_NOT_FOUND; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_METHOD_NOT_IMPLEMENTED() { return GameKit::GAMEKIT_ERROR_METHOD_NOT_IMPLEMENTED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GENERAL() { return GameKit::GAMEKIT_ERROR_GENERAL; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_REGION_CODE_CONVERSION_FAILED() { return GameKit::GAMEKIT_ERROR_REGION_CODE_CONVERSION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CREDENTIALS_FILE_NOT_FOUND() { return GameKit::GAMEKIT_ERROR_CREDENTIALS_FILE_NOT_FOUND; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CREDENTIALS_FILE_SAVE_FAILED() { return GameKit::GAMEKIT_ERROR_CREDENTIALS_FILE_SAVE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CREDENTIALS_NOT_FOUND() { return GameKit::GAMEKIT_ERROR_CREDENTIALS_NOT_FOUND; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CREDENTIALS_FILE_MALFORMED() { return GameKit::GAMEKIT_ERROR_CREDENTIALS_FILE_MALFORMED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_REQUEST_TIMED_OUT() { return GameKit::GAMEKIT_ERROR_REQUEST_TIMED_OUT; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_BOOTSTRAP_BUCKET_LOOKUP_FAILED() { return GameKit::GAMEKIT_ERROR_BOOTSTRAP_BUCKET_LOOKUP_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_BOOTSTRAP_BUCKET_CREATION_FAILED() { return GameKit::GAMEKIT_ERROR_BOOTSTRAP_BUCKET_CREATION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_BOOTSTRAP_INVALID_REGION_CODE() { return GameKit::GAMEKIT_ERROR_BOOTSTRAP_INVALID_REGION_CODE; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_BOOTSTRAP_MISSING_PLUGIN_ROOT() { return GameKit::GAMEKIT_ERROR_BOOTSTRAP_MISSING_PLUGIN_ROOT; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_BOOTSTRAP_REGION_CODE_CONVERSION_FAILED() { return GameKit::GAMEKIT_ERROR_BOOTSTRAP_REGION_CODE_CONVERSION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FUNCTIONS_PATH_NOT_FOUND() { return GameKit::GAMEKIT_ERROR_FUNCTIONS_PATH_NOT_FOUND; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_PATH_NOT_FOUND() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_PATH_NOT_FOUND; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FUNCTION_ZIP_INIT_FAILED() { return GameKit::GAMEKIT_ERROR_FUNCTION_ZIP_INIT_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FUNCTION_ZIP_WRITE_FAILED() { return GameKit::GAMEKIT_ERROR_FUNCTION_ZIP_WRITE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_PARAMSTORE_WRITE_FAILED() { return GameKit::GAMEKIT_ERROR_PARAMSTORE_WRITE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_BOOTSTRAP_BUCKET_UPLOAD_FAILED() { return GameKit::GAMEKIT_ERROR_BOOTSTRAP_BUCKET_UPLOAD_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_SECRETSMANAGER_WRITE_FAILED() { return GameKit::GAMEKIT_ERROR_SECRETSMANAGER_WRITE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_STACK_CREATION_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_STACK_CREATION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_STACK_UPDATE_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_STACK_UPDATE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_RESOURCE_CREATION_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_RESOURCE_CREATION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_STACK_DELETE_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_STACK_DELETE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_DESCRIBE_RESOURCE_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_DESCRIBE_RESOURCE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_DESCRIBE_STACKS_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_DESCRIBE_STACKS_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_APIGATEWAY_DEPLOYMENT_CREATION_FAILED() { return GameKit::GAMEKIT_ERROR_APIGATEWAY_DEPLOYMENT_CREATION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_APIGATEWAY_STAGE_DEPLOYMENT_FAILED() { return GameKit::GAMEKIT_ERROR_APIGATEWAY_STAGE_DEPLOYMENT_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_LAYERS_PATH_NOT_FOUND() { return GameKit::GAMEKIT_ERROR_LAYERS_PATH_NOT_FOUND; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_LAYER_ZIP_INIT_FAILED() { return GameKit::GAMEKIT_ERROR_LAYER_ZIP_INIT_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_LAYER_ZIP_WRITE_FAILED() { return GameKit::GAMEKIT_ERROR_LAYER_ZIP_WRITE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_LAYER_CREATION_FAILED() { return GameKit::GAMEKIT_ERROR_LAYER_CREATION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_GET_TEMPLATE_FAILED() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_GET_TEMPLATE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_PARAMSTORE_READ_FAILED() { return GameKit::GAMEKIT_ERROR_PARAMSTORE_READ_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CLOUDFORMATION_NO_CURRENT_STACK_STATUS() { return GameKit::GAMEKIT_ERROR_CLOUDFORMATION_NO_CURRENT_STACK_STATUS; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_REGISTER_USER_FAILED() { return GameKit::GAMEKIT_ERROR_REGISTER_USER_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CONFIRM_REGISTRATION_FAILED() { return GameKit::GAMEKIT_ERROR_CONFIRM_REGISTRATION_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_RESEND_CONFIRMATION_CODE_FAILED() { return GameKit::GAMEKIT_ERROR_RESEND_CONFIRMATION_CODE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_LOGIN_FAILED() { return GameKit::GAMEKIT_ERROR_LOGIN_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_FORGOT_PASSWORD_FAILED() { return GameKit::GAMEKIT_ERROR_FORGOT_PASSWORD_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_CONFIRM_FORGOT_PASSWORD_FAILED() { return GameKit::GAMEKIT_ERROR_CONFIRM_FORGOT_PASSWORD_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_LOGOUT_FAILED() { return GameKit::GAMEKIT_ERROR_LOGOUT_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_MALFORMED_USERNAME() { return GameKit::GAMEKIT_ERROR_MALFORMED_USERNAME; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_MALFORMED_PASSWORD() { return GameKit::GAMEKIT_ERROR_MALFORMED_PASSWORD; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_INVALID_FEDERATED_IDENTITY_PROVIDER() { return GameKit::GAMEKIT_ERROR_INVALID_FEDERATED_IDENTITY_PROVIDER; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_ACHIEVEMENTS_ICON_UPLOAD_FAILED() { return GameKit::GAMEKIT_ERROR_ACHIEVEMENTS_ICON_UPLOAD_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_ACHIEVEMENTS_INVALID_ID() { return GameKit::GAMEKIT_ERROR_ACHIEVEMENTS_INVALID_ID; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_INVALID() { return GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_PAYLOAD_INVALID; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED() { return GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED() { return GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_API_CALL_DROPPED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED() { return GameKit::GAMEKIT_WARNING_USER_GAMEPLAY_DATA_API_CALL_ENQUEUED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME() { return GameKit::GAMEKIT_ERROR_MALFORMED_BUNDLE_NAME; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY() { return GameKit::GAMEKIT_ERROR_MALFORMED_BUNDLE_ITEM_KEY; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_WRITE_FAILED() { return GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_WRITE_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_READ_FAILED() { return GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_CACHE_READ_FAILED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_USER_GAMEPLAY_DATA_UNPROCESSED_ITEMS() { return GameKit::GAMEKIT_ERROR_USER_GAMEPLAY_DATA_UNPROCESSED_ITEMS; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_SLOT_NOT_FOUND; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_CLOUD_SLOT_IS_NEWER() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_CLOUD_SLOT_IS_NEWER; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_SYNC_CONFLICT; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_DOWNLOAD_SLOT_ALREADY_IN_SYNC() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_DOWNLOAD_SLOT_ALREADY_IN_SYNC; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_UPLOAD_SLOT_ALREADY_IN_SYNC() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_UPLOAD_SLOT_ALREADY_IN_SYNC; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_EXCEEDED_MAX_SIZE() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_EXCEEDED_MAX_SIZE; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_FILE_EMPTY() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_FILE_EMPTY; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_FILE_FAILED_TO_OPEN() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_FILE_FAILED_TO_OPEN; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_LOCAL_SLOT_IS_NEWER() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_LOCAL_SLOT_IS_NEWER; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_SLOT_UNKNOWN_SYNC_STATUS() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_SLOT_UNKNOWN_SYNC_STATUS; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_MALFORMED_SLOT_NAME; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_MISSING_SHA() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_MISSING_SHA; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_SLOT_TAMPERED() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_SLOT_TAMPERED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_BUFFER_TOO_SMALL() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_BUFFER_TOO_SMALL; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_ERROR_GAME_SAVING_MAX_CLOUD_SLOTS_EXCEEDED() { return GameKit::GAMEKIT_ERROR_GAME_SAVING_MAX_CLOUD_SLOTS_EXCEEDED; }

    UFUNCTION(BlueprintPure, Category = "AWS GameKit | Status Codes")
    static int32 GAMEKIT_WARNING_SECRETSMANAGER_SECRET_NOT_FOUND() { return GameKit::GAMEKIT_WARNING_SECRETSMANAGER_SECRET_NOT_FOUND; }

};