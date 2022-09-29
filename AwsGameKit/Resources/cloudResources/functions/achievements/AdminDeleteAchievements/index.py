# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Deletes achievements.

This is a non-player facing Lambda function and used from the GameKit plugin.
"""

import boto3
import botocore
from gamekithelpers import handler_request, handler_response, s3
import json
import os
from typing import Dict, List
import urllib.parse

# Use base Dynamo resource in order to catch errors for batch read / writes
ddb_resource = boto3.resource('dynamodb')
achievements_bucket = s3.get_bucket(os.environ['ACHIEVEMENTS_BUCKET_NAME'])
achievements_table_name = os.environ['ACHIEVEMENTS_TABLE_NAME']


def _get_batch_delete_request(achievement_ids):
    """
    Create the DynamoDB batch_write_item parameter request
    """
    delete_requests = [{
        'DeleteRequest': {
            'Key': {
                'achievement_id': achievement_id
            }
        }
    } for achievement_id in achievement_ids]

    return {
        'RequestItems': {
            achievements_table_name: delete_requests
        }
    }


def _delete_achievements(achievement_ids: List[str]):
    """Delete the provided achievements from Dynamo."""
    batch_delete_request = _get_batch_delete_request(achievement_ids)
    while batch_delete_request is not None:
        try:
            response = ddb_resource.batch_write_item(**batch_delete_request)
            if response.get('UnprocessedItems'):
                batch_delete_request['RequestItems'][achievements_table_name] = response['UnprocessedItems'].get(
                    achievements_table_name)
            else:
                batch_delete_request = None
        except botocore.exceptions.ClientError as err:
            print(f'Failed to delete achievements. Error: {err}')
            raise err


def _delete_achievement_icons(icon_keys: List[str]):
    """Delete the provided achievement icons from S3."""
    response = s3.delete_items(icon_keys, achievements_bucket)
    if response.get('Errors'):
        print(f'Failed to delete {len(response["Errors"])} items:\n{response["Errors"]}')


def _get_batch_get_icons_request(achievement_ids: List[str]) -> Dict[str, dict]:
    """Return a Dynamo batch_get_item request for the provided achievement ids that will fetch achievement icon urls."""
    achievement_keys = [{'achievement_id': achievement_id} for achievement_id in achievement_ids]
    return {
        'RequestItems': {
            achievements_table_name: {
                'Keys': achievement_keys,
                'ConsistentRead': True,
                'ProjectionExpression': 'achievement_id, locked_icon_url, unlocked_icon_url'
            },
        }
    }


def _get_icon_keys_from_dynamo_response(response) -> List[str]:
    """Return a list containing locked and unlocked achievement icon urls from a Dynamo batch_get_item response."""
    achievements = response['Responses'][achievements_table_name]
    achievement_icon_keys = []
    for achievement in achievements:
        if achievement.get('locked_icon_url'):
            achievement_icon_keys.append(achievement['locked_icon_url'])
        if achievement.get('unlocked_icon_url'):
            achievement_icon_keys.append(achievement['unlocked_icon_url'])

    return achievement_icon_keys


def _get_achievement_icon_keys(achievement_ids: List[str]) -> List[str]:
    """Query the provided achievement ids from Dynamo, and return the S3 object keys belonging to their icons."""
    batch_get_icons_request = _get_batch_get_icons_request(achievement_ids)
    achievement_icon_keys = []
    while batch_get_icons_request is not None:
        try:
            response = ddb_resource.batch_get_item(**batch_get_icons_request)
            achievement_icon_keys += _get_icon_keys_from_dynamo_response(response)
            if response.get('UnprocessedKeys'):
                batch_get_icons_request['RequestItems'][achievements_table_name] = response['UnprocessedKeys'].get(
                    achievements_table_name)
            else:
                batch_get_icons_request = None
        except botocore.exceptions.ClientError as err:
            print(f'Failed to retrieve achievement icon keys. Error: {err}')
            raise err

    print(f'Found achievement icons: {achievement_icon_keys}')
    return achievement_icon_keys


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    payload = handler_request.get_query_string_param(event, "payload")
    if payload is None:
        return handler_response.invalid_request()
    parsedBody = json.loads(urllib.parse.unquote(payload))
    achievement_ids = parsedBody.get("achievement_ids")

    if achievement_ids is None or len(achievement_ids) == 0:
        return handler_response.invalid_request()

    achievement_icon_keys = _get_achievement_icon_keys(achievement_ids)
    _delete_achievements(achievement_ids)

    if len(achievement_icon_keys) > 0:
        _delete_achievement_icons(achievement_icon_keys)

    return handler_response.return_response(204, None)
