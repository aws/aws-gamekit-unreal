# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Deletes a single bundle for the calling user. If the body contains bundle_items only the bundle_items within the body will be deleted.
"""

import json
import boto3
import botocore
from boto3.dynamodb.conditions import Key
import os
import sys
import logging

sys.path.append(os.path.join(os.path.dirname(__file__)))
from gamekithelpers import handler_response, handler_request, user_game_play_constants

logger = logging.getLogger()
logger.setLevel(logging.INFO)

ddb_resource = boto3.resource('dynamodb')


def _batch_delete_bundle_write_params(bundle_items):
    """
    Create the DynamoDB batch_write_item parameter request for deleting player data from the bundle items table
    """

    delete_requests = []
    for bundle_item in bundle_items:
        delete_requests.append({
            'DeleteRequest': {
                'Key': {
                    'player_id_bundle': bundle_item['player_id_bundle'],
                    'bundle_item_key': bundle_item['bundle_item_key']
                }
            }
        })

    return {
        'RequestItems': {
            os.environ['BUNDLE_ITEMS_TABLE_NAME']: delete_requests
        }
    }


def _batch_delete_bundle_items_write_params(player_id_bundle, bundle_item_keys):
    """
    Create the DynamoDB batch_write_item parameter request for deleting player data from the bundle items table
    """

    delete_requests = []
    for item_key in bundle_item_keys:
        delete_requests.append({
            'DeleteRequest': {
                'Key': {
                    'player_id_bundle': player_id_bundle,
                    'bundle_item_key': item_key
                }
            }
        })

    return {
        'RequestItems': {
            os.environ['BUNDLE_ITEMS_TABLE_NAME']: delete_requests
        }
    }


def _get_player_bundle_items_request(player_id_bundle):
    table = ddb_resource.Table(os.environ['BUNDLE_ITEMS_TABLE_NAME'])

    queryResponse = table.query(
        KeyConditionExpression=Key('player_id_bundle').eq(player_id_bundle)
    )

    return queryResponse['Items']


def prepare_for_batching(item_list):
    for i in range(0, len(item_list), user_game_play_constants.DYNAMO_MAX_ITEM_WRITES):
        yield item_list[i:i + user_game_play_constants.DYNAMO_MAX_ITEM_WRITES]


def _invalid_request():
    return handler_response.return_response(400, 'Invalid Request')


def lambda_handler(event, context):
    """
    Entry point for the Delete Bundle Lambda function.
    """
    handler_request.log_event(event)

    # Get gk_user_id from requestContext
    player_id = handler_request.get_player_id(event)
    if player_id is None:
        return handler_response.return_response(403, 'Unauthorized.')

    # Get bundle_name from path
    bundle_name = handler_request.get_path_param(event, 'bundle_name')
    if bundle_name is None or len(bundle_name) > user_game_play_constants.BUNDLE_NAME_MAX_LENGTH:
        return _invalid_request()

    body = event.get('body')

    if body is None:  # If body is None we are deleting the entire bundle
        try:
            bundle_items = _get_player_bundle_items_request(player_id + "_" + bundle_name)
        except botocore.exceptions.ClientError as err:
            logger.error(f"Error retrieving bundles. Error: {err}")
            raise err

        if not bundle_items:
            return handler_response.return_response(404, 'User does not have the requested bundle.')

        bundle_item_chunks = prepare_for_batching(list(bundle_items))
        for chunk in bundle_item_chunks:
            try:
                params = _batch_delete_bundle_write_params(chunk)
                ddb_resource.batch_write_item(**params)
            except botocore.exceptions.ClientError as err:
                logger.error(f"Error deleting player bundle items. Error: {err}")
                raise err
    else:
        bundle_item_keys = json.loads(body).get('bundle_item_keys')
        if bundle_item_keys is None or len(bundle_item_keys) == 0:
            return _invalid_request()

        bundle_key_chunks = prepare_for_batching(list(bundle_item_keys))
        for chunk in bundle_key_chunks:
            try:
                params = _batch_delete_bundle_items_write_params(player_id + '_' + bundle_name, chunk)
                ddb_resource.batch_write_item(**params)
            except botocore.exceptions.ClientError as err:
                logger.error(f"Error deleting player bundle items. Error: {err}")
                raise err

    # If the entire bundle is deleted the bundle reference for the player is also deleted from the bundle table
    if (body is None) or (not _get_player_bundle_items_request(player_id + "_" + bundle_name)):
        table = ddb_resource.Table(os.environ['BUNDLES_TABLE_NAME'])
        try:
            table.delete_item(
                Key={
                    'player_id': player_id,
                    'bundle_name': bundle_name
                },
            )
        except botocore.exceptions.ClientError as err:
            logger.error(f"Error deleting player bundle. Error: {err}")
            raise err

    # If the entire bundle is deleted the bundle reference for the player is also deleted from the bundle table
    if (body is None) or (not _get_player_bundle_items_request(player_id + "_" + bundle_name)):
        table = ddb_resource.Table(os.environ['BUNDLES_TABLE_NAME'])
        try:
            table.delete_item(
                Key={
                    'player_id': player_id,
                    'bundle_name': bundle_name
                },
            )
        except botocore.exceptions.ClientError as err:
            print(f"Error deleting player bundle. Error: {err}")
            raise err

    # Return operation result
    return handler_response.return_response(204, None)
