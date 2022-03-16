# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

from functionsTests.helpers.sample_lambda_events import http_event

with patch.dict(os.environ, {
    'ACHIEVEMENTS_TABLE_NAME': 'gamekit_dev_foogamename_game_achievements',
    'ACHIEVEMENTS_BUCKET_NAME': 'gamekit-dev-uswe2-abcd123-foogamename-achievements'
}) as mock_env:
    with patch("gamekithelpers.ddb.get_table") as mock_ddb_get_table:
        with patch("gamekithelpers.s3.get_s3_client") as mock_get_s3_client:
            from functions.achievements.AdminAddAchievements import index


NEW_UNLOCKED_ICON = 'https://mygame.cloudfront.net/achievements/icon/new_unlocked_fc98837a-99bb-4eae-a69c-d5b965878164.png'
NEW_LOCKED_ICON = 'https://mygame.cloudfront.net/achievements/icon/new_locked_f44bfb64-6d92-427e-ac82-24ee96476913.png'

OLD_UNLOCKED_ICON = 'https://mygame.cloudfront.net/achievements/icon/old_unlocked_5f8c4f2d-9002-488b-92c6-8feaca433816.png'
OLD_LOCKED_ICON = 'https://mygame.cloudfront.net/achievements/icon/old_locked_2dc7f9a7-a7cd-4278-aa8b-efe75343ba86.png'


class TestIndex(TestCase):
    def setUp(self) -> None:
        index.s3_client = MagicMock()
        index.achievements_table = MagicMock()

    def test_lambda_returns_a_400_error_code_when_body_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = None

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.achievements_table.update_item.assert_not_called()
        index.s3_client.delete_objects.assert_not_called()

    def test_lambda_returns_a_400_error_code_when_achievements_key_is_missing(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.achievements_table.update_item.assert_not_called()
        index.s3_client.delete_objects.assert_not_called()

    def test_lambda_returns_a_400_error_code_when_achievements_array_is_empty(self):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{"achievements": []}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        index.achievements_table.update_item.assert_not_called()
        index.s3_client.delete_objects.assert_not_called()

    def test_lambda_returns_a_200_success_code_when_achievements_passed_in_body(self):
        # Arrange
        event = self.get_lambda_event()

        index.achievements_table.get_item.return_value = {'Item': None}
        index.achievements_table.update_item.return_value = self.get_achievement_update_value()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(200, result['statusCode'])
        achievements = json.loads(result['body']).get('data').get('achievements')
        self.assertIsNotNone(achievements)
        self.assertEqual(1, len(achievements))
        index.achievements_table.update_item.assert_called_once()
        index.achievements_table.get_item.assert_called_once()
        index.s3_client.delete_objects.assert_not_called()

    def test_lambda_only_deletes_old_icons(self):
        test_cases = [
            ('existing icons - both icons new', OLD_LOCKED_ICON, NEW_LOCKED_ICON, OLD_UNLOCKED_ICON, NEW_UNLOCKED_ICON, True),
            ('existing icons - unlocked icon new', OLD_LOCKED_ICON, OLD_LOCKED_ICON, OLD_UNLOCKED_ICON, NEW_UNLOCKED_ICON, True),
            ('existing icons - locked icon new', OLD_LOCKED_ICON, NEW_LOCKED_ICON, OLD_UNLOCKED_ICON, OLD_UNLOCKED_ICON, True),
            ('existing icons - no new icons', OLD_LOCKED_ICON, OLD_LOCKED_ICON, OLD_UNLOCKED_ICON, OLD_UNLOCKED_ICON, False),
            ('no unlocked icon - new unlocked icon', OLD_LOCKED_ICON, NEW_LOCKED_ICON, '', NEW_UNLOCKED_ICON, True),
            ('no locked icon - new locked icon', '', NEW_LOCKED_ICON, OLD_UNLOCKED_ICON, NEW_UNLOCKED_ICON, True),
            ('no icons - both new icons', '', NEW_LOCKED_ICON, '', NEW_UNLOCKED_ICON, False)
        ]

        for test_name, old_locked_url, new_locked_url, old_unlocked_url, new_unlocked_url, should_delete in test_cases:
            with self.subTest(test_name):
                # Arrange
                # Reset all mocks between subtests - this isn't done automatically
                self.setUp()
                event = self.get_lambda_event(new_locked_url, new_unlocked_url)

                index.achievements_table.get_item.return_value = {'Item': {
                    'unlocked_icon_url': old_unlocked_url,
                    'locked_icon_url': old_locked_url
                }}
                index.achievements_table.update_item.return_value = self.get_achievement_update_value(new_locked_url, new_unlocked_url)

                # Act
                result = index.lambda_handler(event, None)

                # Assert
                self.assertEqual(200, result['statusCode'])
                achievements = json.loads(result['body']).get('data').get('achievements')
                self.assertIsNotNone(achievements)
                self.assertEqual(1, len(achievements))

                index.achievements_table.update_item.assert_called_once()
                index.achievements_table.get_item.assert_called_once()

                if should_delete:
                    index.s3_client.delete_objects.assert_called_once()

                    # Expect old icons to be deleted if a new icon was uploaded
                    def append_if_should_delete(items, old_url, new_url):
                        if old_url and old_url != new_url:
                            items.append({'Key': old_url})

                    deleted_objects = []
                    append_if_should_delete(deleted_objects, old_locked_url, new_locked_url)
                    append_if_should_delete(deleted_objects, old_unlocked_url, new_unlocked_url)

                    delete_args = index.s3_client.delete_objects.call_args_list[0][1]['Delete']
                    self.assertEqual(delete_args, {'Objects': deleted_objects})
                else:
                    index.s3_client.delete_objects.assert_not_called()

    @staticmethod
    def get_achievement_update_value(locked_icon=NEW_LOCKED_ICON, unlocked_icon=NEW_UNLOCKED_ICON):
        return {
            'Attributes': {
                'created_at': '2021-07-27T00:38:42.927855+00:00',
                'locked_description': 'Eat 1,000 bananas',
                'achievement_id': 'EAT_THOUSAND_BANANAS',
                'unlocked_icon_url': locked_icon,
                'max_value': 1000,
                'unlocked_description': 'You ate 1,000 bananas!',
                'is_secret': False,
                'locked_icon_url': unlocked_icon,
                'updated_at': '2021-07-27T17:25:39.302081+00:00',
                'is_stateful': True,
                'points': 10,
                'order_number': 1,
                'is_hidden': False,
                'title': 'Hangry Chicken'
            }
        }

    @staticmethod
    def get_lambda_event(locked_icon=NEW_LOCKED_ICON, unlocked_icon=NEW_UNLOCKED_ICON):
        body = {
            "achievements": [{
                "achievement_id": "EAT_THOUSAND_BANANAS",
                "title": "Hangry Chicken",
                "locked_description": "Eat 1,000 bananas",
                "unlocked_description": "You ate 1,000 bananas!",
                "locked_icon_url": locked_icon,
                "unlocked_icon_url": unlocked_icon,
                "points": 10, "is_stateful": True, "max_value": 1000, "is_secret": False,
                "is_hidden": False, "order_number": 1
            }]
        }
        return http_event(body=json.dumps(body))
