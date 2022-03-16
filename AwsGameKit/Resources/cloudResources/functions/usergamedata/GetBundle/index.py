# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Lambda function for retrieving a bundle and its bundle items.
"""

import boto3
from boto3.dynamodb.types import TypeDeserializer
import botocore
import os
import sys
import logging

sys.path.append(os.path.join(os.path.dirname(__file__)))
from gamekithelpers import handler_response, handler_request
from gamekithelpers.pagination import validate_pagination_token

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_client = boto3.client('dynamodb')


def _build_bundle_get_request(player_id, bundle_name, consistent_read, limit, start_bundle_id, start_item_key):
    """
    Build the Bundle query request.
    """
    player_id_bundle = f'{player_id}_{bundle_name}'

    request = {
        'TableName': os.environ['BUNDLE_ITEMS_TABLE_NAME'],
        'KeyConditionExpression': '#player_id_bundle = :player_id_bundle',
        'ExpressionAttributeNames': {
            '#player_id_bundle': 'player_id_bundle'
        },
        'ExpressionAttributeValues': {
            ':player_id_bundle': {'S': player_id_bundle}
        },
        'Limit': limit
    }

    if start_bundle_id is not None and start_item_key is not None:
        key = {
            'player_id_bundle': {'S': start_bundle_id},
            'bundle_item_key': {'S': start_item_key}
        }
        request['ExclusiveStartKey'] = key

    if consistent_read is not None:
        request['ConsistentRead'] = bool(consistent_read)

    return request


def lambda_handler(event, context):
    """
    Entry point for the Get Bundle Lambda function.
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

        # get pagination arguments from query string
    start_key_tokens = handler_request.get_query_string_param_as_list(event, 'next_start_key')
    start_bundle_id = None
    start_item_key = None
    if len(start_key_tokens) == 2:
        start_bundle_id = start_key_tokens[0]
        start_item_key = start_key_tokens[1]
        paging_token = handler_request.get_query_string_param(event, 'paging_token')
        # validate this pagination token
        if not validate_pagination_token(player_id, ",".join(start_key_tokens), paging_token):
           return handler_response.response_envelope(403, "Invalid paging token")

    consistent_read = handler_request.get_query_string_param(event, 'use_consistent_read')
    limit = handler_request.get_query_string_param(event, 'limit', '100')
    limit = int(limit)
    if limit > 100 or limit <= 0:
        limit = 100

    bundle_items = []
    items_response = {}

    # get the bundle
    try:
        deserializer = TypeDeserializer()
        result = ddb_client.query(
            **_build_bundle_get_request(player_id, bundle_name, consistent_read, limit, start_bundle_id,
                                        start_item_key))

        # add the pagination key, if present
        last_evaluated_key = result.get('LastEvaluatedKey')
        if last_evaluated_key is not None:
            next_start_key = {k: deserializer.deserialize(value=v) for k, v in last_evaluated_key.items()}

            bundle = next_start_key['player_id_bundle']
            key = next_start_key['bundle_item_key']
            items_response['next_start_key'] = f'{bundle},{key}'

        # append the bundle items
        for item in result.get('Items'):
            bundle_items.append({k: deserializer.deserialize(value=v) for k, v in item.items()})

    except botocore.exceptions.ClientError as err:
        logger.error(f'Error {err}')
        raise err

    # Return operation result
    if 'next_start_key' in items_response:
        # There are more items to get
        return handler_response.response_envelope(200, None, {'bundle_items': bundle_items}, items_response['next_start_key'], player_id)

    # No more items to get
    return handler_response.response_envelope(200, None, {'bundle_items': bundle_items})
