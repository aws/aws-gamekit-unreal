# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0


import boto3
import botocore
import os
import logging

from gamekithelpers import ddb
from gamekithelpers.handler_request import get_player_id, get_path_param, log_event
from gamekithelpers.handler_response import response_envelope, return_response
from gamekithelpers.validation import is_valid_primary_identifier

logger = logging.getLogger()
logger.setLevel(logging.INFO)

s3_resource = boto3.resource('s3')


def lambda_handler(event, context):
    """
    Delete the player's specified save slot from S3 and DynamoDB.

    It's okay to call this Lambda for a non-existent save slot. The Lambda will still return a success.

    After a save slot is deleted it no longer counts towards the player's max SLOT_LIMIT.

    Parameters:

    Request Context:
        custom:gk_user_id: str
            The player_id of the save slot to delete. This value comes from the Cognito Authorizer that validates the
            API Gateway request.

    Path Parameters:
        slot_name: str
            The slot name of the save file to delete.

            Limited to 512 characters long, using alphanumeric characters, dashes (-), underscores (_), and periods (.).
            This lambda will return an error if a malformed slot name is provided.

    Errors:
        400 Bad Request  - Returned when a malformed 'slot_name' path parameter is provided.
        401 Unauthorized - Returned when the 'custom:gk_user_id' parameter is missing from the request context.
        403 Forbidden    - Returned when the owner AWS account of the bucket does not match the caller account.
    """
    log_event(event)

    # Get player_id from requestContext:
    player_id = get_player_id(event)
    if player_id is None:
        return response_envelope(status_code=401)

    # Get path param inputs:
    slot_name = get_path_param(event, 'slot_name')
    if not is_valid_primary_identifier(slot_name):
        logger.error(f'Malformed slot_name: {slot_name} provided for player_id: {player_id}')
        return response_envelope(status_code=400)

    # Delete the save:
    try:
        delete_save_slot(player_id, slot_name)
        # Construct response object:
        return return_response(204, None)
    except botocore.exceptions.ClientError as error:
        if error.response['Error']['Code'] == 'AccessDenied':
            # Construct forbidden response object:
            return response_envelope(status_code=403)
        else:
            # Maintain previous behavior until more error codes are implemented
            raise error


def delete_save_slot(player_id: str, slot_name: str) -> None:
    """Delete the save slot object from S3 and the metadata from DynamoDB."""
    delete_slot_object(player_id, slot_name)
    delete_slot_metadata(player_id, slot_name)


def delete_slot_object(player_id: str, slot_name: str) -> None:
    """Delete the save slot object from S3."""
    bucket_name = os.environ.get('GAMESAVES_BUCKET_NAME')
    key = f"{player_id}/{slot_name}"
    s3_object = s3_resource.Object(bucket_name, key)
    s3_object.delete(
        ExpectedBucketOwner=os.environ.get('AWS_ACCOUNT_ID')
    )


def delete_slot_metadata(player_id: str, slot_name: str) -> None:
    """Delete the slot metadata from DynamoDB."""
    gamesaves_table = ddb.get_table(table_name=os.environ.get('GAMESAVES_TABLE_NAME'))
    gamesaves_table.delete_item(
        Key={
            'player_id': player_id,
            'slot_name': slot_name,
        },
        ReturnValues='NONE',
        ReturnConsumedCapacity='NONE',
    )
