# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Deletes achievements.

This is a non-player facing Lambda function and used from the GameKit plugin.
"""

import boto3
import botocore
from gamekithelpers import handler_request, handler_response
import os

ddb_client = boto3.client('dynamodb')


def _batch_write_params(achievement_ids):
    """
    Create the DynamoDB batch_write_item parameter request
    """

    delete_requests = []
    for achievement_id in achievement_ids:
        delete_requests.append({
            'DeleteRequest': {
                'Key': {
                    'achievement_id': {
                        'S': achievement_id
                    }
                }
            }
        })

    return {
        'RequestItems': {
            os.environ['ACHIEVEMENTS_TABLE_NAME']: delete_requests
        }
    }


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    body = handler_request.get_body_as_json(event)
    if body is None:
        return handler_response.invalid_request()

    achievement_ids = body.get('achievement_ids')
    if achievement_ids is None or len(achievement_ids) == 0:
        return handler_response.invalid_request()

    params = _batch_write_params(achievement_ids)
    try:
        ddb_client.batch_write_item(**params)
    except botocore.exceptions.ClientError as err:
        print(f"Error deleting items. Error: {err}")
        raise err

    return handler_response.return_response(204, None)
