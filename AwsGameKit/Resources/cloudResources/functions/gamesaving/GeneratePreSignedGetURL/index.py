# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
import logging

from gamekithelpers import s3
from gamekithelpers.handler_request import get_player_id, get_path_param, get_query_string_param, log_event
from gamekithelpers.handler_response import response_envelope
from gamekithelpers.validation import is_valid_primary_identifier

DEFAULT_TIME_TO_LIVE_SECONDS = '120'

logger = logging.getLogger()
logger.setLevel(logging.INFO)

s3_client = s3.get_s3_client()


def lambda_handler(event, context):
    """
    Generate a pre-signed URL for downloading the player's specified save slot data from S3.

    If the save slot doesn't exist on S3, the URL will return a 404 "no such key" when it is used to download the file:
    https://boto3.amazonaws.com/v1/documentation/api/latest/reference/services/s3.html#S3.Client.get_object

    Parameters:

    Request Context:
        custom:gk_user_id: str
            The player_id of the save slot to download. This value comes from the Cognito Authorizer that validates the
            API Gateway request.

    Path Parameters:
        slot_name: str
            The slot name of the save file to download.

            Limited to 512 characters long, using alphanumeric characters, dashes (-), underscores (_), and periods (.).
            This lambda will return an error if a malformed slot name is provided.

    Query String Parameters:
        time_to_live: int
            The number of seconds the URL will be valid. The URL will no longer work after the time has expired.
            [Optional, defaults to 120 seconds (DEFAULT_TIME_TO_LIVE_SECONDS).]

    Errors:
        400 Bad Request  - Returned when a malformed 'slot_name' path parameter is provided.
        401 Unauthorized - Returned when the 'custom:gk_user_id' parameter is missing from the request context.
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

    # Get query param inputs:
    time_to_live = int(get_query_string_param(event, 'time_to_live', DEFAULT_TIME_TO_LIVE_SECONDS))

    # Generate URL:
    bucket_name = os.environ.get('GAMESAVES_BUCKET_NAME')
    url = generate_presigned_url(bucket_name, player_id, slot_name, time_to_live)

    # Construct response object:
    return response_envelope(
        status_code=200,
        response_obj={
            'url': url
        }
    )


def generate_presigned_url(bucket_name: str, player_id: str, slot_name: str, time_to_live: int) -> str:
    return s3_client.generate_presigned_url(
        ClientMethod='get_object',
        Params={
            'Bucket': bucket_name,
            'Key': f'{player_id}/{slot_name}',
        },
        ExpiresIn=time_to_live,
    )
