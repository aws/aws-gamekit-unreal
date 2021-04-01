# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Retrieves a player achievement, earned or unearned.

This is a player facing Lambda function and used in-game.
"""

import botocore
from gamekithelpers import handler_request, handler_response, ddb
import distutils.core
import os

ddb_player_table = ddb.get_table(os.environ['PLAYER_ACHIEVEMENTS_TABLE_NAME'])
ddb_game_table = ddb.get_table(os.environ['ACHIEVEMENTS_TABLE_NAME'])


def _get_player_achievement(player_id, achievement_id, use_consistent_read):
    try:
        response = ddb_player_table.get_item(
            **ddb.get_item_request_param({'player_id': player_id, 'achievement_id': achievement_id},
                                         use_consistent_read))
        player_achievement = ddb.get_response_item(response)
    except botocore.exceptions.ClientError as err:
        print(f"Error retrieving achievement_id: {achievement_id} for player_id: {player_id}. Error: {err}")
        raise err

    if player_achievement is None:
        player_achievement = {
            'current_value': 0,
            'earned': False,
            'earned_at': None
        }

    return player_achievement


def _get_achievement(player_id, achievement_id, use_consistent_read):
    response = ddb_game_table.get_item(**ddb.get_item_request_param({'achievement_id': achievement_id}, use_consistent_read))
    achievement = ddb.get_response_item(response)

    if achievement is not None and achievement['is_hidden']:
        return None

    if achievement is not None:
        # get player achievement
        player_achievement = _get_player_achievement(player_id,
                                                     achievement['achievement_id'],
                                                     use_consistent_read)

        if achievement['is_secret'] and not player_achievement['earned']:
            return None

        # merge results; the timestamp attributes will be from the player's achievement
        achievement.update(player_achievement)
        achievement.setdefault('current_value', 0)
        achievement.setdefault('earned', False)
        achievement.setdefault('earned_at', None)

    return achievement


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
    achievement_id = handler_request.get_path_param(event, 'achievement_id')
    if achievement_id is None:
        return handler_response.invalid_request()

    use_consistent_read = bool(distutils.util.strtobool(handler_request.get_query_string_param(event, 'use_consistent_read', 'false')))

    try:
        achievement = _get_achievement(player_id, achievement_id, use_consistent_read)
    except botocore.exceptions.ClientError as err:
        print(f"Error retrieving items. Error: {err}")
        raise err

    if achievement is None:
        return handler_response.response_envelope(404, None)

    return handler_response.response_envelope(200, None, achievement)
