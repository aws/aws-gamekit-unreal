# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Adds or updates achievements.

This is a non-player facing Lambda function and used from the GameKit plugin.
"""

import os

import botocore
from gamekithelpers import handler_request, handler_response, ddb, sanitizer


def _update_item_params(achievements):
    """
    Create an array DynamoDB of update_item requests from the array of achievements
    """

    params = []
    now = ddb.timestamp()
    for achievement in achievements:
        param_item = {
            'Key': {
                'achievement_id': sanitizer.sanitize(achievement.get("achievement_id")),
            },
            'ReturnValues': 'ALL_NEW',
            'ExpressionAttributeNames': {
                '#title': 'title',
                '#locked_description': 'locked_description',
                '#unlocked_description': 'unlocked_description',
                '#locked_icon_url': 'locked_icon_url',
                '#unlocked_icon_url': 'unlocked_icon_url',
                '#points': 'points',
                '#is_stateful': 'is_stateful',
                '#max_value': 'max_value',
                '#is_secret': 'is_secret',
                '#is_hidden': 'is_hidden',
                '#order_number': 'order_number',
                '#created_at': 'created_at',
                '#updated_at': 'updated_at'
            },
            'ExpressionAttributeValues': {
                ':title': sanitizer.sanitize(achievement.get("title")),
                ':locked_description': sanitizer.sanitize(achievement.get("locked_description")),
                ':unlocked_description': sanitizer.sanitize(achievement.get("unlocked_description")),
                ':locked_icon_url': sanitizer.sanitize(achievement.get("locked_icon_url")),
                ':unlocked_icon_url': sanitizer.sanitize(achievement.get("unlocked_icon_url")),
                ':points': achievement.get("points"),
                ':is_stateful': achievement.get("is_stateful"),
                ':max_value': achievement.get("max_value"),
                ':is_secret': achievement.get("is_secret"),
                ':is_hidden': achievement.get("is_hidden"),
                ':order_number': achievement.get("order_number"),
                ':created_at': now,
                ':updated_at': now
            },
            'UpdateExpression': 'SET #title = :title, #locked_description = :locked_description, '
                                '#unlocked_description = :unlocked_description, #locked_icon_url = :locked_icon_url, '
                                '#unlocked_icon_url = :unlocked_icon_url, #points = :points, '
                                '#is_stateful = :is_stateful, #max_value = :max_value, '
                                '#is_secret = :is_secret, #is_hidden = :is_hidden,#order_number = :order_number, '
                                '#created_at = if_not_exists(created_at, :created_at), #updated_at = :updated_at '
        }
        params.append(param_item)

    return params


def lambda_handler(event, context):
    """
    This is the lambda function handler.
    """
    handler_request.log_event(event)

    body = handler_request.get_body_as_json(event)
    if body is None:
        return handler_response.invalid_request()

    achievements = body.get('achievements')
    if achievements is None or len(achievements) == 0:
        return handler_response.invalid_request()

    response_achievements = []
    update_params = _update_item_params(achievements)

    ddb_table = ddb.get_table(os.environ['ACHIEVEMENTS_TABLE_NAME'])
    for update_param in update_params:
        try:
            response = ddb_table.update_item(**update_param)
            response_achievements.append(ddb.get_update_response_item(response))
        except botocore.exceptions.ClientError as err:
            print(f"Error updating items. Error: {err}")
            raise err

    return handler_response.response_envelope(200, None, {'achievements': response_achievements})
