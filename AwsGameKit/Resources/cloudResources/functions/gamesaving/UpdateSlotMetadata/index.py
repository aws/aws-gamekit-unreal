# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import boto3
import botocore
import logging
import os
import urllib.parse
from typing import Any
from gamekithelpers import ddb

# Custom Types:
S3Object = Any

# Magic Values:
ISO_8601_FORMAT = '%Y-%m-%dT%H:%M:%SZ'

# String Literals:
UTF_8 = "utf-8"

logger = logging.getLogger()
logger.setLevel(logging.INFO)

s3_resource = boto3.resource('s3')
s3_client = boto3.client('s3')
gamesaves_table = ddb.get_table(table_name=os.environ.get('GAMESAVES_TABLE_NAME'))


def lambda_handler(event, context):
    """
    This Lambda is triggered by an S3 event each time a save file is uploaded to S3 (including when it overwrites an
    existing save file).

    This Lambda creates/overwrites the save file's corresponding slot metadata in the DynamoDB table.

    Parameters:
        See https://docs.aws.amazon.com/AmazonS3/latest/userguide/notification-content-structure.html
    """

    for record in event['Records']:
        # Get inputs from event:
        object_info = record['s3']['object']
        object_key = urllib.parse.unquote_plus(object_info['key'], encoding=UTF_8)

        player_id = object_key.split('/')[0]
        slot_name = object_key.split('/')[1]

        # Get S3 object:
        s3_save_file = get_s3_save_file(object_key, player_id, slot_name)

        if s3_save_file is None:
            # At this point the warning has already been logged, so the lambda can exit
            return

        # Create DynamoDB attributes:
        metadata = get_metadata(s3_save_file)
        last_modified = get_last_modified_timestamp(s3_save_file)
        size = get_size(s3_save_file)

        # Logging:
        logger.info(f'Creating DynamoDB slot metadata for player_id: {player_id} and slot_name: {slot_name}'.encode(UTF_8))

        # Create/update DynamoDB item:
        write_metadata_to_dynamodb(player_id, slot_name, metadata, last_modified, size)


def get_s3_save_file(object_key: str, player_id: str, slot_name: str) -> S3Object:
    bucket_name = os.environ.get('GAMESAVES_BUCKET_NAME')
    s3_object = s3_resource.Object(bucket_name, object_key)

    try:
        s3_object.load()
    except (AttributeError, botocore.exceptions.ClientError) as object_err:
        try:
            s3_client.head_bucket(Bucket=bucket_name)
        except botocore.exceptions.ClientError as bucket_err:
            logger.warning(f'Failed to update metadata for player_id: {player_id} and slot_name: {slot_name} '
                           f'- the game saves bucket: {bucket_name} does not exist')
            raise bucket_err

        response = gamesaves_table.get_item(**ddb.get_item_request_param(
            {'player_id': player_id, 'slot_name': slot_name}, True))

        if response.get('Item') is None:
            logger.warning(
                f'Database entry and save slot not found for slot_name: {slot_name} and player_id: {player_id}. This '
                f'is likely caused by a delay on the S3 trigger, signaling that an independent event has already '
                f'deleted the save before this event was processed. If these conditions are not met and the file\'s '
                f'metadata should have been updated, refer to the S3 error listed below.\nError: {object_err}')
            return

        logger.error(f'Failed to update metadata for player_id: {player_id} and slot_name: {slot_name} '
                     f'- there was an unknown error getting data from the S3 bucket: {bucket_name}')
        raise object_err

    return s3_object


def get_metadata(s3_save_file: S3Object) -> str:
    """Get the slot metadata stored on the S3 object."""
    return s3_save_file.metadata['slot_metadata']


def get_last_modified_timestamp(s3_save_file: S3Object) -> int:
    """Get the save file's last modified timestamp in epoch milliseconds from the S3 object's metadata."""
    return int(s3_save_file.metadata['epoch'])


def get_size(s3_save_file: S3Object) -> int:
    """Get the size of the S3 object body in bytes."""
    return int(s3_save_file.content_length)


def write_metadata_to_dynamodb(player_id: str, slot_name: str, metadata: str, last_modified: int, size: int) -> None:
    """
    All passed in attributes should be from the S3 save file. In the case the S3 event trigger is delayed and this
    Lambda is called out of sequence, the S3 object referenced will always be a source of truth, not the event itself.
    """
    gamesaves_table.put_item(
        Item={
            'player_id': player_id,
            'slot_name': slot_name,
            'metadata': metadata,
            'last_modified': last_modified,
            'size': size,
        },
    )
