# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

"""
Purpose

Adds or updates achievements.

This is a non-player facing Lambda function and used from the GameKit plugin.
"""

import os

import botocore
from gamekithelpers import handler_request, handler_response, ddb, sanitizer, s3

achievements_table = ddb.get_table(os.environ['ACHIEVEMENTS_TABLE_NAME'])
achievements_bucket = os.environ['ACHIEVEMENTS_BUCKET_NAME']
s3_client = s3.get_s3_client()


def _get_achievement_update_request(achievement):
    """
    Creates a DynamoDB request to update an achievement
    """
    now = ddb.timestamp()
    return {
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


def _get_achievement_icons_request(achievement):
    """
    Creates a DynamoDB request to retrieve the achievement icons for a given achievement
    """
    return {
        'Key': {
            'achievement_id': sanitizer.sanitize(achievement.get("achievement_id")),
        },
        'ConsistentRead': True,
        'ProjectionExpression': 'locked_icon_url, unlocked_icon_url'
    }


def _get_old_achievement_icon_keys(new_achievement) -> [str]:
    """
    Retrieve old icons for the given achievement. Depending on achievement state, returns between 0 and 2
    achievement icon keys, as each achievement can have a locked and unlocked icon.

    If an achievement's icon will not be updated, or no previous icon exists, the icon key will not be returned.
    """
    get_icons_request = _get_achievement_icons_request(new_achievement)
    ddb_response = achievements_table.get_item(**get_icons_request)
    old_achievement_icons = ddb.get_response_item(ddb_response)
    # No achievement exists - no icons to delete
    if old_achievement_icons is None:
        return []

    # If new icons were uploaded, keep track of previous versions for deletion after updating the achievement.
    # Do not mark icons for deletion if they are still in use (previous icon url == new icon url)
    # or the icon doesn't exist (empty string, None provided).
    old_icon_keys = []
    old_locked_icon_url = old_achievement_icons.get('locked_icon_url')
    if old_locked_icon_url and new_achievement.get('locked_icon_url') != old_locked_icon_url:
        old_icon_keys.append(old_locked_icon_url)

    old_unlocked_icon_url = old_achievement_icons.get('unlocked_icon_url')
    if old_unlocked_icon_url and new_achievement.get('unlocked_icon_url') != old_unlocked_icon_url:
        old_icon_keys.append(old_unlocked_icon_url)

    print(f'Found outdated icons for achievement {new_achievement.get("achievement_id")}: {old_icon_keys}')
    return old_icon_keys


def _update_achievement(achievement):
    # Update item with new values
    update_request = _get_achievement_update_request(achievement)
    response = achievements_table.update_item(**update_request)
    return ddb.get_update_response_item(response)


def _delete_achievement_icons(icon_keys: [str]):
    delete_request = {
        'Objects': [{"Key": icon_key} for icon_key in icon_keys]
    }

    print(f'Deleting icons {icon_keys} from bucket {achievements_bucket}')
    response = s3_client.delete_objects(Bucket=achievements_bucket, Delete=delete_request)
    print(f'S3 deletion response: {response}')


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
    for achievement in achievements:
        try:
            # Fetch existing achievement icons from Dynamo
            old_icon_keys = _get_old_achievement_icon_keys(achievement)

            # Update the achievement
            response_achievements.append(_update_achievement(achievement))

            # Delete the old icons from S3
            if len(old_icon_keys) > 0:
                _delete_achievement_icons(old_icon_keys)
        except botocore.exceptions.ClientError as err:
            print(f"Error updating items. Error: {err}")
            raise err

    return handler_response.response_envelope(200, None, {'achievements': response_achievements})
