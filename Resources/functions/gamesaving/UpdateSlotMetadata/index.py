# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import os
from typing import Any
import urllib.parse

import boto3

from gamekithelpers import ddb


# Custom Types:
S3Object = Any

# Magic Values:
ISO_8601_FORMAT = '%Y-%m-%dT%H:%M:%SZ'

s3_resource = boto3.resource('s3')


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
        object_key = urllib.parse.unquote_plus(object_info['key'], encoding='utf-8')
        object_size = object_info['size']

        # Get S3 object:
        s3_save_file = get_s3_save_file(object_key)

        # Create DynamoDB attributes:
        player_id = object_key.split('/')[0]
        slot_name = object_key.split('/')[1]
        metadata = get_metadata(s3_save_file)
        last_modified = get_last_modified_timestamp(s3_save_file)
        size = object_size

        # Logging:
        print(f'Creating DynamoDB slot metadata for player_id: {player_id} and slot_name: {slot_name}')

        # Create/update DynamoDB item:
        write_metadata_to_dynamodb(player_id, slot_name, metadata, last_modified, size)


def get_s3_save_file(object_key: str) -> S3Object:
    bucket_name = os.environ.get('GAMESAVES_BUCKET_NAME')
    return s3_resource.Object(bucket_name, object_key)


def get_metadata(s3_save_file: S3Object) -> str:
    """Get the slot metadata stored on the S3 object."""
    return s3_save_file.metadata['slot_metadata']


def get_last_modified_timestamp(s3_save_file: S3Object) -> int:
    """Get the save file's last modified timestamp in epoch milliseconds from the S3 object's metadata."""
    return int(s3_save_file.metadata['epoch'])


def write_metadata_to_dynamodb(player_id: str, slot_name: str, metadata: str, last_modified: int, size: int) -> None:
    gamesaves_table = ddb.get_table(table_name=os.environ.get('GAMESAVES_TABLE_NAME'))
    gamesaves_table.put_item(
        Item={
            'player_id': player_id,
            'slot_name': slot_name,
            'metadata': metadata,
            'last_modified': last_modified,
            'size': size,
        },
    )
