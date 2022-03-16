# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Lambda function for deleting all user gameplay data for the calling user.
"""

import boto3
import botocore
from boto3.dynamodb.conditions import Key
import os
import sys
import logging
import json

sys.path.append(os.path.join(os.path.dirname(__file__)))
from gamekithelpers import handler_response, handler_request, user_game_play_constants

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_resource = boto3.resource('dynamodb')
lambda_client = boto3.client('lambda')


def _batch_delete_lambda_helper(table_name, delete_requests):
    """
    Sends a list of delete requests to an asynchronous lambda that handles batch writes
    Please refer to the BatchDeleteHelper Lambda for more information
    """
    inputBatchDeleteHelper = {'TableName': table_name, 'DeleteRequest': delete_requests}

    try:
        lambda_client.invoke(
            FunctionName=os.environ['BATCH_DELETE_HELPER_LAMBDA_NAME'],
            InvocationType='Event',
            Payload=json.dumps(inputBatchDeleteHelper)
        )

    except botocore.exceptions.ClientError as err:
        logger.error(f"Error invoking helper lambda. Error: {err}")
        raise err


def _batch_delete_bundles(bundles):
    """
    Queries bundles table and sends delete request to the BatchDeleteHelper Lambda in increments of 25 or less
    """
    delete_requests = []

    for bundle in bundles:
        delete_requests.append({
            'DeleteRequest': {
                'Key': {
                    'player_id': bundle['player_id'],
                    'bundle_name': bundle['bundle_name']
                }
            }
        })
        if len(delete_requests) == user_game_play_constants.DYNAMO_MAX_ITEM_WRITES:
            _batch_delete_lambda_helper(os.environ['BUNDLES_TABLE_NAME'], delete_requests)
            delete_requests = []

    if len(delete_requests) > 0:
        _batch_delete_lambda_helper(os.environ['BUNDLES_TABLE_NAME'], delete_requests)


def _batch_delete_bundle_items(bundles):
    """
    Queries bundle items table and sends delete request to the BatchDeleteHelper Lambda in increments of 25 or less
    """
    delete_requests = []

    for bundle in bundles:
        query_response = _get_player_bundle_items_request(bundle['player_id'] + '_' + bundle['bundle_name'])
        bundle_items = query_response['Items']

        while bundle_items:
            for bundle_item in bundle_items:
                delete_requests.append({
                    'DeleteRequest': {
                        'Key': {
                            'player_id_bundle': bundle_item['player_id_bundle'],
                            'bundle_item_key': bundle_item['bundle_item_key']
                        }
                    }
                })

                if len(delete_requests) == user_game_play_constants.DYNAMO_MAX_ITEM_WRITES:
                    _batch_delete_lambda_helper(os.environ['BUNDLE_ITEMS_TABLE_NAME'], delete_requests)
                    delete_requests = []

            if 'LastEvaluatedKey' in query_response:
                bundle_items = _get_player_bundle_items_request(bundle['player_id'] + '_' + bundle['bundle_name'],
                                                                query_response['LastEvaluatedKey'])
            else:
                bundle_items = []

    if len(delete_requests) > 0:
        _batch_delete_lambda_helper(os.environ['BUNDLE_ITEMS_TABLE_NAME'], delete_requests)


def _delete_all_user_gameplay_data(bundles):
    """
    Deletes the bundles associated with the player, first from the bundle items table and then from the bundles table
    """
    _batch_delete_bundle_items(bundles)
    _batch_delete_bundles(bundles)


def _get_player_bundles_request(player_id, start_key=None):
    """
    Gets bundles associated with the calling player for the UserGameDataBundles table
    Query returns max of 100 bundles
    """
    table = ddb_resource.Table(os.environ['BUNDLES_TABLE_NAME'])

    if start_key is not None and len(start_key) > 0:
        response = table.query(
            KeyConditionExpression=Key('player_id').eq(player_id),
            ExclusiveStartKey=start_key
        )
        return response

    response = table.query(
        KeyConditionExpression=Key('player_id').eq(player_id)
    )
    return response


def _get_player_bundle_items_request(player_id_bundle, start_key=None):
    """
    Gets bundle items associated with the calling player for the UserGameDataBundleItems table
    Query returns max of 100 bundle items
    """
    table = ddb_resource.Table(os.environ['BUNDLE_ITEMS_TABLE_NAME'])

    if start_key is not None and len(start_key) > 0:
        response = table.query(
            KeyConditionExpression=Key('player_id_bundle').eq(player_id_bundle),
            ExclusiveStartKey=start_key
        )
        return response

    response = table.query(
        KeyConditionExpression=Key('player_id_bundle').eq(player_id_bundle),
    )
    return response


def lambda_handler(event, context):
    """
    Entry point for the Delete All Lambda function.
    """
    handler_request.log_event(event)

    # Get gk_user_id from requestContext
    player_id = handler_request.get_player_id(event)
    if player_id is None:
        return handler_response.return_response(401, 'Unauthorized.')

    try:
        get_player_bundles_response = _get_player_bundles_request(player_id)
    except botocore.exceptions.ClientError as err:
        logger.error(f"Error retrieving bundles. Error: {err}")
        raise err

    if not get_player_bundles_response['Items']:
        return handler_response.return_response(204, 'User does not have any user gameplay data.')

    _delete_all_user_gameplay_data(get_player_bundles_response['Items'])
    while 'LastEvaluatedKey' in get_player_bundles_response:
        try:
            get_player_bundles_response = _get_player_bundles_request(player_id, get_player_bundles_response['LastEvaluatedKey'])
        except botocore.exceptions.ClientError as err:
            logger.error(f"Error retrieving bundles. Error: {err}")
            raise err
        _delete_all_user_gameplay_data(get_player_bundles_response['Items'])

    # Return operation result
    return handler_response.return_response(204, None)