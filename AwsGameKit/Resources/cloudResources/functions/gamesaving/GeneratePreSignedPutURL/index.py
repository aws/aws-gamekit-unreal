# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import boto3
import os
import logging
from distutils.util import strtobool
from enum import Enum

from gamekithelpers import ddb, s3
from gamekithelpers.handler_request import get_player_id, get_header_param, get_path_param, get_query_string_param, log_event
from gamekithelpers.handler_response import response_envelope
from gamekithelpers.validation import is_valid_primary_identifier, is_valid_base_64

# Defaults:
DEFAULT_TIME_TO_LIVE_SECONDS = '120'
DEFAULT_CONSISTENT_READ = 'True'
DEFAULT_METADATA = ''

# Constraints:

# S3 only allows 2KB of user metadata. 113 of these are used for the following:
#   24 bytes for the metadata key ('x-amz-meta-slot_metadata'),
#   15 bytes for the SHA-256 key ('x-amz-meta-hash') and 44 for the Base64 encode value,
#   16 bytes for the epoch key ('x-amz-meta-epoch') and 14 for the epoch milliseconds (allowing dates up to Nov 5138).
# See: https://docs.aws.amazon.com/AmazonS3/latest/userguide/UsingMetadata.html#UserMetadata
MAX_METADATA_BYTES = 1887

# Dictionary Keys:
S3_SLOT_METADATA_KEY = 'slot_metadata'
S3_HASH_METADATA_KEY = 'hash'
S3_EPOCH_METADATA_KEY = 'epoch'

# Magic Numbers:
ENCODE_TO_ASCII_EXTRA_BYTES = 4  # 'encode_to_ascii()' adds 4 extra bytes due to the JSON wrapper ('[""]').
BASE_64_ENCODED_SHA_256_BYTES = 44  # Base64 encoding of a SHA-256 hash takes up 44 bytes

# String literals
UTF_8 = "utf-8"

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_client = boto3.client('dynamodb')
s3_client = s3.get_s3_client()


# Response statuses:
class ResponseStatus(str, Enum):
    MALFORMED_SLOT_NAME = 'Malformed Slot Name'
    MAX_METADATA_BYTES_EXCEEDED = 'Max Metadata Bytes Exceeded'
    MALFORMED_METADATA = 'Malformed Metadata'
    MALFORMED_HASH_SIZE_MISMATCH = 'Malformed Hash Size Mismatch'
    MAX_CLOUD_SAVE_SLOTS_EXCEEDED = 'Max Cloud Save Slots Exceeded'
    GENERIC_STATUS = 'Unexpected Error'


def lambda_handler(event, context):
    """
    Generate a pre-signed URL that allows a save file to be uploaded to S3 in the player's specified save slot. If the
    slot is new, will verify that MAX_SAVE_SLOTS_PER_PLAYER has not been reached.

    Parameters:

    Request Context:
        custom:gk_user_id: str
            The player_id to associate the save file with. This value comes from the Cognito Authorizer that validates
            the API Gateway request.

    Header Parameters:
        metadata: str
            An arbitrary Base64 encoded string to associate with the save file.
            [Optional, defaults to an empty string: '']

            The total size of the metadata string cannot exceed 1887 bytes (MAX_METADATA_BYTES, see docs above) and must
            be Base64 encoded, otherwise the Lambda will return an error. The 2KB limit comes from an S3 limitation, and
            the Base64 encoding saves space compared to S3's native behavior for non-ASCII strings:
            https://docs.aws.amazon.com/AmazonS3/latest/userguide/UsingMetadata.html#UserMetadata

            The GameKit SDK handles encoding and decoding the metadata string for you; if not using the SDK, please
            Base64 encode your metadata values before calling this lambda function.

            Examples:
            A string, representing the save slot's description:
            unencoded_metadata = 'about to fight the boss 👍'
            metadata = 'YWJvdXQgdG8gZmlnaHQgdGhlIGJvc3Mg8J+RjQ==' # Pass this to the lambda

            A JSON blob, containing several metadata fields:
            unencoded_metadata = '{"description": "about to fight the boss 👍", "total_playtime_seconds": "16200"}'
            metadata = 'eyJkZXNjcmlwdGlvbiI6ICJhYm91dCB0byBmaWdodCB0aGUgYm9zcyDwn5GNIiwgInRvdGFsX3BsYXl0aW1lX3NlY29uZHMiOiAiMTYyMDAifQ==' # Pass this to the lambda

        hash: str
            The Base64 encoded SHA-256 hash of the file to upload.

            The total size of the hash string will be 44 bytes; the SHA-256 hash itself is 32 bytes, and the Base64
            encoding of it will bring the size up to 44. Base64 encoding is used to convert the SHA-256 hash from a
            byte stream to an ASCII compliant string.

        last_modified_epoch_time: int
            The number of milliseconds since epoch of the last UTC time when the save slot was modified on the caller's
            device.

    Path Parameters:
        slot_name: str
            The slot name to use for the save file.

            Limited to 512 characters long, using alphanumeric characters, dashes (-), underscores (_), and periods (.).
            This lambda will return an error if a malformed slot name is provided.

            If the slot_name is not occupied with another save file, the Lambda will check whether this new save file
            will exceed the MAX_SAVE_SLOTS_PER_PLAYER. If it would be exceeded, the Lambda will return an error.

    Query String Parameters:
        time_to_live: int
            The number of seconds the URL will be valid. The URL will no longer work after the time has expired.
            [Optional, defaults to 120 seconds (DEFAULT_TIME_TO_LIVE_SECONDS).]

        consistent_read: bool
            Whether to use "Consistent Read" when querying DynamoDB.
            [Optional, defaults to True (DEFAULT_CONSISTENT_READ).]

    Errors:
        400 Bad Request  - Returned when a malformed 'slot_name' path parameter is provided.
        400 Bad Request  - Returned when the 'metadata' parameter exceeds 1883 bytes (MAX_METADATA_BYTES) after being
                           ASCII encoded.
        400 Bad Request  - Returned when the 'hash' parameter is not exactly 44 bytes (BASE_64_ENCODED_SHA_256_BYTES)
                           in size.
        400 Bad Request  - Returned when the save slot is new and would exceed the player's MAX_SAVE_SLOTS_PER_PLAYER.
        401 Unauthorized - Returned when the 'custom:gk_user_id' parameter is missing from the request context.
    """
    log_event(event)

    # Get player_id from requestContext:
    player_id = get_player_id(event)
    if player_id is None:
        return response_envelope(status_code=401)

    # Get header inputs:
    metadata = get_header_param(event, 'metadata', DEFAULT_METADATA)
    sha_hash: str = get_header_param(event, S3_HASH_METADATA_KEY)
    last_modified_epoch_time = int(get_header_param(event, 'last_modified_epoch_time'))

    # Get path param inputs:
    slot_name = get_path_param(event, 'slot_name')

    # Get query param inputs:
    time_to_live = int(get_query_string_param(event, 'time_to_live', DEFAULT_TIME_TO_LIVE_SECONDS))
    consistent_read = bool(strtobool(get_query_string_param(event, 'consistent_read', DEFAULT_CONSISTENT_READ)))

    # Validate inputs:
    if not is_valid_primary_identifier(slot_name):
        logger.error((f'Malformed slot_name: {slot_name} provided for player_id: {player_id}').encode(UTF_8))
        return response_envelope(status_code=400, status_message=ResponseStatus.MALFORMED_SLOT_NAME)

    if get_bytes_length(metadata) > MAX_METADATA_BYTES:
        return response_envelope(status_code=400, status_message=ResponseStatus.MAX_METADATA_BYTES_EXCEEDED)

    if not is_valid_base_64(metadata):
        logger.error((f'Malformed metadata provided, expected a Base64 encoded string. Metadata: {metadata}').encode(UTF_8))
        return response_envelope(status_code=400, status_message=ResponseStatus.MALFORMED_METADATA)

    if len(sha_hash) != BASE_64_ENCODED_SHA_256_BYTES or not sha_hash.isascii():
        logger.error((f'Malformed SHA-256 hash: {sha_hash} provided. Must be 44 characters and Base64 encoded.').encode(UTF_8))
        return response_envelope(status_code=400, status_message=ResponseStatus.MALFORMED_HASH_SIZE_MISMATCH)

    # Verify MAX_SAVE_SLOTS_PER_PLAYER won't be exceeded:
    if is_new_save_slot(player_id, slot_name, consistent_read) and would_exceed_slot_limit(player_id, consistent_read):
        return response_envelope(status_code=400, status_message=ResponseStatus.MAX_CLOUD_SAVE_SLOTS_EXCEEDED)

    # Generate URL:
    bucket_name = os.environ.get('GAMESAVES_BUCKET_NAME')
    url = generate_presigned_url(
        bucket_name, player_id, slot_name, metadata, sha_hash, last_modified_epoch_time, time_to_live
    )

    # Construct response object:
    return response_envelope(
        status_code=200,
        response_obj={
            'url': url
        }
    )


def get_bytes_length(input_string: str) -> int:
    """Return the number of bytes consumed by the string."""
    return len(input_string.encode(UTF_8))


def is_new_save_slot(player_id: str, slot_name: str, consistent_read: bool) -> bool:
    """Return True if the player's slot_name doesn't exist in the DynamoDB table, or False if it does exist."""
    gamesaves_table = ddb.get_table(table_name=os.environ.get('GAMESAVES_TABLE_NAME'))
    response = gamesaves_table.get_item(
        Key={
            'player_id': player_id,
            'slot_name': slot_name,
        },
        ConsistentRead=consistent_read,
    )
    item = response.get('Item')
    return item is None


def would_exceed_slot_limit(player_id: str, consistent_read: bool) -> bool:
    """Return True if adding an additional save slot would cause the player to exceed the MAX_SAVE_SLOTS_PER_PLAYER."""
    slot_limit = int(os.environ.get('MAX_SAVE_SLOTS_PER_PLAYER'))
    number_of_slots_in_use = get_number_of_slots_used(player_id, consistent_read)
    return number_of_slots_in_use + 1 > slot_limit


def get_number_of_slots_used(player_id: str, consistent_read: bool) -> int:
    """
    Get the number of save slots used by the player in DynamoDB.

    Implementation Note:
        This method is costly because it consumes Provisioned Read Capacity Units equal to fetching every single one of
        the player's records. A more performant yet complex approach is to use a Global Secondary Index as a
        Materialized Aggregation:
        https://docs.aws.amazon.com/amazondynamodb/latest/developerguide/bp-gsi-aggregation.html
    """
    paginator = ddb_client.get_paginator('query')
    pages = paginator.paginate(
        TableName=os.environ.get('GAMESAVES_TABLE_NAME'),
        Select='COUNT',
        KeyConditionExpression='player_id = :player_id',
        ExpressionAttributeValues={
            ':player_id': {'S': player_id},
        },
        ConsistentRead=consistent_read,
    )

    number_of_slots = 0
    for page in pages:
        number_of_slots = number_of_slots + page['Count']

    return number_of_slots


def generate_presigned_url(bucket_name: str, player_id: str, slot_name: str, metadata: str, sha_hash: str,
                           last_modified_epoch_time: int, time_to_live: int) -> str:
    return s3_client.generate_presigned_url(
        ClientMethod='put_object',
        Params={
            'Bucket': bucket_name,
            'Key': f'{player_id}/{slot_name}',
            'Metadata': {
                S3_SLOT_METADATA_KEY: metadata,
                S3_HASH_METADATA_KEY: sha_hash,
                S3_EPOCH_METADATA_KEY: str(last_modified_epoch_time)
            }
        },
        ExpiresIn=time_to_live,
    )
