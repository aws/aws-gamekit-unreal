# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Lambda function for retrieving a single bundle item.
"""

import boto3
from boto3.dynamodb.types import TypeDeserializer
import botocore
import os
import sys
import logging
sys.path.append(os.path.join(os.path.dirname(__file__)))
from gamekithelpers import handler_response, handler_request

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_client = boto3.client('dynamodb')


def _build_bundleitems_get_request(player_id, bundle_name, bundle_item_key):
    """
    Build the Bundle Items query request.
    """
    player_id_bundle = f'{player_id}_{bundle_name}'

    return {
        'Key': {
            'player_id_bundle': {'S': player_id_bundle},
            'bundle_item_key': {'S': bundle_item_key}
        },
        'TableName': os.environ['BUNDLE_ITEMS_TABLE_NAME']
    }


def lambda_handler(event, context):
    """
    Entry point for the Get Item Lambda function.
    """
    handler_request.log_event(event)

    # Get gk_user_id from requestContext
    player_id = handler_request.get_player_id(event)
    if player_id is None:
        return handler_response.return_response(401, 'Unauthorized.')

    # get bundle_name from path
    bundle_name = handler_request.get_path_param(event, 'bundle_name')
    if bundle_name is None:
        return handler_response.return_response(400, 'Invalid bundle')

    # get bundle_item_key from path
    bundle_item_key = handler_request.get_path_param(event, 'bundle_item_key')
    if bundle_item_key is None:
        return handler_response.return_response(400, 'Invalid bundle item')

    # retrieve a single bundle item
    item = {}
    try:
        deserializer = TypeDeserializer()
        result = ddb_client.get_item(**_build_bundleitems_get_request(player_id, bundle_name, bundle_item_key))

        if 'Item' not in result:
            return handler_response.return_response(404, 'Could not retrieve data')

        item = result['Item']
        item = {k: deserializer.deserialize(value=v) for k, v in item.items()}
    except botocore.exceptions.ClientError as err:
        logger.error(f'Error {err}')
        raise err

    # Return operation result
    return handler_response.response_envelope(200, None, item)
