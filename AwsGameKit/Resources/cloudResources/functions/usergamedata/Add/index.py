# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Lambda function for adding bundles and bundle items.
"""

import boto3
import botocore
from datetime import timezone, datetime
import os
import sys
import logging
sys.path.append(os.path.join(os.path.dirname(__file__)))
from gamekithelpers import handler_response, handler_request, user_game_play_constants, sanitizer

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_client = boto3.client('dynamodb')


def _build_bundle_update_request(player_id, bundle_name):
    """
    Build the Bundle update request.
    """
    return {
        'Key': {
            'player_id': {'S': player_id},
            'bundle_name': {'S': bundle_name}
        },
        'ReturnValues': 'ALL_NEW',
        'TableName': os.environ['BUNDLES_TABLE_NAME']
    }


def _build_bundleitems_update_request(player_id, bundle_name, bundle_item_key, bundle_item_value):
    """
    Build the Bundle Items update request.
    """
    player_id_bundle = f'{player_id}_{bundle_name}'
    timestamp = datetime.utcnow().replace(tzinfo=timezone.utc).isoformat()

    return {
        'Key': {
            'player_id_bundle': {'S': player_id_bundle},
            'bundle_item_key': {'S': bundle_item_key}
        },
        'ReturnValues': 'ALL_NEW',
        'TableName': os.environ['BUNDLE_ITEMS_TABLE_NAME'],
        'ExpressionAttributeNames': {
            '#bundle_item_value': 'bundle_item_value',
            '#created_at': 'created_at',
            '#updated_at': 'updated_at'
        },
        'ExpressionAttributeValues': {
            ':bundle_item_value': {'S': bundle_item_value},
            ':created_at': {'S': timestamp},
            ':updated_at': {'S': timestamp}
        },
        'UpdateExpression': 'SET #bundle_item_value = :bundle_item_value, '
                            '#created_at = if_not_exists(created_at, :created_at), #updated_at = :updated_at'
    }


def lambda_handler(event, context):
    """
    Entry point for the Add Lambda function.
    """
    handler_request.log_event(event)

    # Get gk_user_id from requestContext
    player_id = handler_request.get_player_id(event)
    if player_id is None:
        return handler_response.return_response(401, 'Unauthorized')

    # get payload from body (list of bundles in the form key=value)
    bundle = handler_request.get_body_as_json(event)
    if bundle is None:
        return handler_response.return_response(400, 'Missing payload')

    unprocessed_items_array = []

    bundle_data = bundle.items()
    if not bundle_data:
        return handler_response.return_response(400, 'Invalid payload')

    # get bundle_name from path
    bundle_name = handler_request.get_path_param(event, 'bundle_name')
    if bundle_name is None:
        unprocessed_items_array = [{'bundle_item_key': key, 'bundle_item_value': bundle.get(key)} for key in bundle]
        return handler_response.response_envelope(400, 'Invalid Request: Bundle Name Missing', {'bundle_name': bundle_name, 'unprocessed_items': unprocessed_items_array}, None, player_id)

    if len(bundle_name) > user_game_play_constants.BUNDLE_NAME_MAX_LENGTH:
        unprocessed_items_array = [{'bundle_item_key': key, 'bundle_item_value': bundle.get(key)} for key in bundle]
        return handler_response.response_envelope(400, 'Invalid Request: Bundle Name Max Length Exceeded', {'bundle_name': bundle_name, 'unprocessed_items': unprocessed_items_array}, None, player_id)

    bundle_name = sanitizer.sanitize(bundle_name)

    malformed_items_array = []

    for bundle_item_key, bundle_item_value in bundle_data:
        if len(bundle_item_key) > user_game_play_constants.BUNDLE_ITEM_NAME_MAX_LENGTH or len(
                bundle_item_value) > user_game_play_constants.BUNDLE_ITEM_VALUE_MAX_LENGTH:

            malformed_items_array.append({'bundle_item_key': bundle_item_key, 'bundle_item_value': bundle_item_value})

    if malformed_items_array:
        unprocessed_items_array = [{'bundle_item_key': key, 'bundle_item_value': bundle.get(key)} for key in bundle]
        logger.error(f'The following bundle items are malformed: {malformed_items_array}')
        return handler_response.response_envelope(413, 'Invalid Item contained inside bundle', {'bundle_name': bundle_name, 'unprocessed_items': unprocessed_items_array}, None, player_id)

    try:
        # add bundle
        ddb_client.update_item(**_build_bundle_update_request(player_id, bundle_name))

        # add items
        for bundle_item_key, bundle_item_value in bundle_data:

            sanitized_bundle_item_key = sanitizer.sanitize(bundle_item_key)
            sanitized_bundle_item_value = sanitizer.sanitize(bundle_item_value)

            try:
                ddb_client.update_item(**_build_bundleitems_update_request(player_id,
                                                                           bundle_name,
                                                                           sanitized_bundle_item_key,
                                                                           sanitized_bundle_item_value))

            except botocore.exceptions.ClientError as err:
                logger.error(f'Failed to update bundle item {bundle_item_key}: {err}')
                unprocessed_items_array.append({'bundle_item_key': bundle_item_key, 'bundle_item_value': bundle_item_value})

    except botocore.exceptions.ClientError as err:
        logger.error(f'Failed to update bundle {bundle_name}: {err}')
        unprocessed_items_array = [{'bundle_item_key': key, 'bundle_item_value': bundle.get(key)} for key in bundle]

        return handler_response.response_envelope(422, None, {'bundle_name': bundle_name, 'unprocessed_items': unprocessed_items_array}, None, player_id)

    # Return operation result
    return handler_response.response_envelope(201, 'Created', {'bundle_name': bundle_name, 'unprocessed_items': unprocessed_items_array}, None, player_id)
