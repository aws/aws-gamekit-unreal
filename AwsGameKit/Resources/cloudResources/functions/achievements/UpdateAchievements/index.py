# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Increments the current_value field for the given player_id and achievement_id in the player_achievements table.
If current_value == max_value defined in game_achievements, the earned column is set to true.

This is a player facing Lambda function and used in-game.
"""

import os

import boto3
import botocore
from gamekithelpers import handler_request, handler_response, ddb

ddb_client = boto3.client('dynamodb')

ddb_game_table = ddb.get_table(os.environ.get('ACHIEVEMENTS_TABLE_NAME'))
ddb_player_table = ddb.get_table(os.environ.get('PLAYER_ACHIEVEMENTS_TABLE_NAME'))


def _update_current_value_request(player_id, achievement_id, max_value, increment_by):
    """
    Create the DynamoDB update_item parameter request for incrementing the current_value attribute
    """

    now = ddb.timestamp()
    return {
        'Key': {
            'player_id': player_id,
            'achievement_id': achievement_id
        },
        'ReturnValues': 'ALL_NEW',
        'ExpressionAttributeNames': {
            '#current_value': 'current_value',
            '#earned': 'earned',
            '#created_at': 'created_at',
            '#updated_at': 'updated_at'
        },
        'ExpressionAttributeValues': {
            ':max_value': max_value,
            ':increment_by': increment_by,
            ':created_at': now,
            ':updated_at': now,
            ':zero': 0,
            ':false_value': False,
        },
        'ConditionExpression': 'attribute_not_exists(player_id) or '
                               '(attribute_exists(player_id) and current_value < :max_value)',
        'UpdateExpression': 'SET #current_value = if_not_exists(current_value, :zero) + :increment_by, '
                            '#earned = if_not_exists(earned, :false_value), '
                            '#created_at = if_not_exists(created_at, :created_at), #updated_at = :updated_at'
    }


def _update_earned_request(player_id, achievement_id, current_value, max_value):
    """
    Create the DynamoDB update_item parameter request to update earned attribute
    """

    now = ddb.timestamp()
    return {
        'Key': {
            'player_id': player_id,
            'achievement_id': achievement_id
        },
        'ReturnValues': 'ALL_NEW',
        'ExpressionAttributeNames': {
            '#current_value': 'current_value',
            '#earned': 'earned',
            '#updated_at': 'updated_at',
            '#earned_at': 'earned_at'
        },
        'ExpressionAttributeValues': {
            ':current_value': current_value,
            ':max_value': max_value,
            ':earned': True,
            ':updated_at': now,
            ':earned_at': now,
            ':true_value': True
        },
        'ConditionExpression': 'current_value >= :max_value and earned <> :true_value',
        'UpdateExpression': 'SET #current_value = :current_value, #earned = :earned, #earned_at = :earned_at, #updated_at = :updated_at'
    }


def _get_achievement(achievement_id):
    try:
        response = ddb_game_table.get_item(**ddb.get_item_request_param({'achievement_id': achievement_id}))
        achievement = ddb.get_response_item(response)
    except botocore.exceptions.ClientError as err:
        print(f"Error retrieving achievement_id: {achievement_id}. Error: {err}")
        raise err

    return achievement


def _increment_player_achievement(player_id, achievement_id, increment_by, max_value):
    player_achievement = None
    try:
        response = ddb_player_table.update_item(**_update_current_value_request(player_id,
                                                                                achievement_id,
                                                                                max_value,
                                                                                increment_by))
        player_achievement = ddb.get_update_response_item(response)
        player_achievement['max_value'] = max_value
    except ddb_client.exceptions.ConditionalCheckFailedException:
        # ignore condition expression failure
        pass
    except botocore.exceptions.ClientError as err:
        print(f"Error updating player_id: {player_id}, achievement_id: {achievement_id}. Error: {err}")
        raise err

    return player_achievement


def _attempt_unlock(player_id, achievement_id, current_value, max_value):
    player_achievement = None
    try:
        response = ddb_player_table.update_item(**_update_earned_request(player_id, achievement_id, current_value, max_value))
        player_achievement = ddb.get_update_response_item(response)
        player_achievement['max_value'] = max_value
    except ddb_client.exceptions.ConditionalCheckFailedException:
        # ignore condition expression failure
        pass
    except botocore.exceptions.ClientError as err:
        print(f"Error attempting to unlock player_id: {player_id}, achievement_id: {achievement_id}. Error: {err}")
        raise err
    return player_achievement


def _get_player_achievement(player_id, achievement_id, max_value):
    try:
        response = ddb_player_table.get_item(**ddb.get_item_request_param(
            {'player_id': player_id, 'achievement_id': achievement_id}, True))
        player_achievement = ddb.get_response_item(response)
        player_achievement['max_value'] = max_value
    except botocore.exceptions.ClientError as err:
        print(f"Error attempting to unlock player_id: {player_id}, achievement_id: {achievement_id}. Error: {err}")
        raise err
    return player_achievement


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    # Get player_id from requestContext
    player_id = handler_request.get_player_id(event)
    if player_id is None:
        return handler_response.response_envelope(401)

    # Get achievement_id from path
    achievement_id = event.get('pathParameters').get('achievement_id')
    if achievement_id is None:
        return handler_response.invalid_request()

    # Get increment_by from body
    body = handler_request.get_body_as_json(event)
    if body is None:
        return handler_response.invalid_request()

    increment_by = body.get('increment_by', 0)
    if increment_by <= 0:
        return handler_response.invalid_request()

    # Get achievement
    achievement = _get_achievement(achievement_id)
    if achievement is None:
        return handler_response.response_envelope(404)

    max_value = achievement['max_value']

    # If the achievement is hidden, return invalid request
    is_hidden = achievement.get('is_hidden', True)
    if is_hidden:
        return handler_response.response_envelope(404)

    # Increment player achievement
    player_achievement = _increment_player_achievement(player_id, achievement_id, increment_by, max_value)

    current_value = increment_by
    if player_achievement is not None:
        current_value = player_achievement['current_value']

    current_value = min(current_value, max_value)

    # Attemp to unlock achievement
    player_achievement = _attempt_unlock(player_id, achievement_id, current_value, max_value)

    if player_achievement is None:
        # conditional updates might have failed but we need to return the player achievement object
        player_achievement = _get_player_achievement(player_id, achievement_id, max_value)
    else:
        player_achievement['newly_earned'] = True

    # Merge achievement with player achievement
    achievement.update(player_achievement)
    return handler_response.response_envelope(200, None, achievement)
