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


def _build_bundle_get_request(player_id, consistent_read, limit, start_bundle):
    """
    Build the Bundle query request.
    """

    request = {
        'TableName': os.environ['BUNDLES_TABLE_NAME'],
        'KeyConditionExpression': '#player_id = :player_id',
        'ExpressionAttributeNames': {
            '#player_id': 'player_id'
        },
        'ExpressionAttributeValues': {
            ':player_id': {'S': player_id}
        },
        'Limit': limit
    }

    if start_bundle is not None:
        key = {
            'player_id': {'S': player_id},
            'bundle_name': {'S': start_bundle}
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
        return handler_response.return_response(403, 'Unauthorized.')

    # get pagination arguments from query string
    start_key_tokens = handler_request.get_query_string_param_as_list(event, 'next_start_key')
    start_bundle_id = None
    if len(start_key_tokens) == 1:
        start_bundle_id = start_key_tokens[0]
        paging_token = handler_request.get_query_string_param(event, 'paging_token')
        # validate this pagination token
        if not validate_pagination_token(player_id, start_bundle_id, paging_token):
           return handler_response.response_envelope(403, "Invalid paging token")

    consistent_read = handler_request.get_query_string_param(event, 'use_consistent_read')
    limit = handler_request.get_query_string_param(event, 'limit', '100')
    limit = int(limit)
    if limit > 100 or limit <= 0:
        limit = 100

    bundle_names = []

    # get the bundle
    try:
        deserializer = TypeDeserializer()

        paginate_bundles = False

        result = ddb_client.query(
            **_build_bundle_get_request(player_id, consistent_read, limit, start_bundle_id))

        last_evaluated_bundle_key = None
        if result.get('LastEvaluatedKey') is not None:
            last_evaluated_bundle_key = result.get('LastEvaluatedKey')['bundle_name']['S']
            paginate_bundles = True

        # append the bundle items
        for item in result.get('Items'):
            bundle_names.append({'bundle_name': item.get('bundle_name')['S']})

    except botocore.exceptions.ClientError as err:
        logger.error(f'Error {err}')
        raise err

    # Return operation result
    if paginate_bundles:
        # There are more items to get
        return handler_response.response_envelope(200, None, {'bundle_names': bundle_names}, last_evaluated_bundle_key, player_id)

    # No more items to get
    return handler_response.response_envelope(200, None, {'bundle_names': bundle_names})
