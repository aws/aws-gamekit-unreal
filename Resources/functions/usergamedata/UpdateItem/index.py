# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Lambda function for updating bundle items.
"""

import boto3
import botocore
from datetime import timezone, datetime
import logging
import os
import sys

sys.path.append(os.path.join(os.path.dirname(__file__)))
from gamekithelpers import handler_response, handler_request, user_game_play_constants, sanitizer

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_client = boto3.client('dynamodb')


def _build_bundle_item_update_request(player_id_bundle, bundle_item_key, bundle_item_value):
    """
     Build the Bundle Item update request.
     """
    timestamp = datetime.utcnow().replace(tzinfo=timezone.utc).isoformat()

    return {
        'Key': {
            'player_id_bundle': {'S': player_id_bundle},
            'bundle_item_key': {'S': bundle_item_key}
        },
        'ReturnValues': 'UPDATED_NEW',
        'TableName': os.environ['BUNDLE_ITEMS_TABLE_NAME'],
        'ExpressionAttributeNames': {
            '#bundle_item_value': 'bundle_item_value',
            '#updated_at': 'updated_at'
        },
        'ExpressionAttributeValues': {
            ':bundle_item_value': {'S': bundle_item_value},
            ':updated_at': {'S': timestamp}
        },
        'ConditionExpression': 'attribute_exists(player_id_bundle) and attribute_exists(bundle_item_key)',
        'UpdateExpression': 'SET #bundle_item_value = :bundle_item_value, '
                            '#updated_at = :updated_at'
    }


def lambda_handler(event, context):
    """
    Entry point for the Get All Lambda function.
    """
    handler_request.log_event(event)

    # Get gk_user_id from requestContext
    player_id = handler_request.get_player_id(event)
    if player_id is None:
        return handler_response.return_response(403, 'Unauthorized.')

    # get bundle_name from path
    bundle_name = handler_request.get_path_param(event, 'bundle_name')
    if bundle_name is None:
        return handler_response.return_response(400, 'Invalid bundle name')

    if len(bundle_name) > user_game_play_constants.BUNDLE_NAME_MAX_LENGTH:
        return handler_response.return_response(414, 'Invalid bundle name')

    bundle_name = sanitizer.sanitize(bundle_name)

    # get bundle_item_key from path
    bundle_item_key = handler_request.get_path_param(event, 'bundle_item_key')
    if bundle_item_key is None:
        return handler_response.return_response(400, 'Invalid bundle item key')

    if len(bundle_item_key) > user_game_play_constants.BUNDLE_NAME_MAX_LENGTH:
        return handler_response.return_response(414, 'Invalid bundle item key')

    bundle_item_key = sanitizer.sanitize(bundle_item_key)

    # get payload from body (an items value)
    item_data = handler_request.get_body_as_json(event)
    if item_data is None:
        return handler_response.return_response(400, 'Missing payload')

    if "bundle_item_value" not in item_data:
        return handler_response.return_response(400, 'Invalid payload')

    item_key = sanitizer.sanitize(item_data["bundle_item_value"])
    if not item_key:
        return handler_response.return_response(400, 'Invalid payload')

    player_id_bundle = f'{player_id}_{bundle_name}'

    try:
        ddb_client.update_item(**_build_bundle_item_update_request(player_id_bundle, bundle_item_key, item_key))
    except ddb_client.exceptions.ConditionalCheckFailedException:
        return handler_response.return_response(404, 'Bundle and/or bundle item not found.')
    except botocore.exceptions.ClientError as err:
        logger.error(f'Error updating bundle item, please ensure bundle item exists. Error: {err}')
        raise err

    # Return operation result
    return handler_response.return_response(204, None)
