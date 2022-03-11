# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Retrieves all player achievements, earned and unearned.

This is a player facing Lambda function and used in-game.
"""

import botocore
import distutils.core
import json
import os

from boto3.dynamodb.conditions import Key
from gamekithelpers import handler_request, handler_response, ddb
from gamekithelpers.pagination import validate_pagination_token

ddb_player_table = ddb.get_table(os.environ['PLAYER_ACHIEVEMENTS_TABLE_NAME'])
ddb_game_table = ddb.get_table(os.environ['ACHIEVEMENTS_TABLE_NAME'])


def _get_player_achievement(player_id, achievement_id, use_consistent_read):
    try:
        response = ddb_player_table.get_item(**ddb.get_item_request_param(
            {'player_id': player_id, 'achievement_id': achievement_id}, use_consistent_read))
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


def _get_achievements(player_id, response_limit, use_consistent_read, start_key):
    response = ddb_game_table.scan(**ddb.scan_request_param(response_limit,
                                                            use_consistent_read,
                                                            start_key,
                                                            Key('is_hidden').eq(False)))

    achievements, next_start_key = ddb.get_response_items(response)
    for achievement in achievements:
        # get player achievement
        player_achievement = _get_player_achievement(player_id,
                                                     achievement['achievement_id'],
                                                     use_consistent_read)

        # merge results; the timestamp attributes will be from the player's achievement
        achievement.update(player_achievement)
        achievement.setdefault('current_value', 0)
        achievement.setdefault('earned', False)
        achievement.setdefault('earned_at', None)

    return achievements, next_start_key


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    # Get player_id from requestContext
    player_id = handler_request.get_player_id(event)
    if player_id is None:
        return handler_response.response_envelope(401)

    start_key = handler_request.get_query_string_param(event, 'start_key')
    if start_key is not None and len(start_key) > 0:
        start_key = json.loads(start_key)
        paging_token = handler_request.get_query_string_param(event, 'paging_token')
        # validate this pagination token
        if not validate_pagination_token(player_id, start_key, paging_token):
           return handler_response.response_envelope(403, "Invalid paging token")

    wait_for_all_pages = bool(distutils.util.strtobool(handler_request.get_query_string_param(event, 'wait_for_all_pages', 'false')))
    use_consistent_read = bool(distutils.util.strtobool(handler_request.get_query_string_param(event, 'use_consistent_read', 'false')))
    limit = handler_request.get_query_string_param(event, 'limit', '100')

    response_limit = int(limit)
    if response_limit > 100 or response_limit <= 0:
        response_limit = 100

    try:
        all_achievements = []
        achievements, next_start_key = _get_achievements(player_id,
                                                         response_limit,
                                                         use_consistent_read,
                                                         start_key)
        all_achievements.extend(achievements)
        while wait_for_all_pages and next_start_key:
            achievements, next_start_key = _get_achievements(player_id,
                                                             response_limit,
                                                             use_consistent_read,
                                                             next_start_key)
            all_achievements.extend(achievements)

    except botocore.exceptions.ClientError as err:
        print(f"Error retrieving items. Error: {err}")
        raise err

    return handler_response.response_envelope(200, None, {'achievements': all_achievements}, next_start_key, player_id)
