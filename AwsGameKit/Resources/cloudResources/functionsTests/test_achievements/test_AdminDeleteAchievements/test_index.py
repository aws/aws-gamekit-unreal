# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

from typing import Dict, List

from functionsTests.helpers.sample_lambda_events import http_event

ACHIEVEMENTS_TABLE_NAME = 'gamekit_dev_foogamename_game_achievements'
ACHIEVEMENT_ID = 'EAT_THOUSAND_BANANAS'

with patch.dict(os.environ, {
    'ACHIEVEMENTS_TABLE_NAME': ACHIEVEMENTS_TABLE_NAME,
    'ACHIEVEMENTS_BUCKET_NAME': 'gamekit-dev-uswe2-abcd123-foogamename-achievements'
}) as mock_env:
    with patch("boto3.resource") as boto_resource_mock:
        with patch("gamekithelpers.s3.get_bucket") as get_bucket_mock:
            from functions.achievements.AdminDeleteAchievements import index


@patch("gamekithelpers.s3.delete_items")
class TestIndex(TestCase):
    def setUp(self):
        index.ddb_resource = MagicMock()
        index.achievements_bucket = MagicMock()

    def test_lambda_returns_a_400_error_code_when_body_is_empty(self, delete_items_mock):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = None

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        self.assert_no_resources_called(delete_items_mock)

    def test_lambda_returns_a_400_error_code_when_achievements_id_key_is_missing(self, delete_items_mock):
        # Arrange
        event = self.get_lambda_event()
        event['body'] = '{}'

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        self.assert_no_resources_called(delete_items_mock)

    def test_lambda_returns_a_400_error_code_when_achievement_ids_array_is_empty(self, delete_items_mock):
        # Arrange
        event = self.get_lambda_event([])

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(400, result['statusCode'])
        self.assert_no_resources_called(delete_items_mock)

    def test_lambda_returns_a_200_success_code_when_achievement_ids_passed_in_body(self, delete_items_mock):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_resource.batch_get_item.return_value = \
            self.get_achievement_icons_query_response([ACHIEVEMENT_ID])
        index.ddb_resource.batch_write_item.return_value = self.get_achievements_delete_response()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])
        index.ddb_resource.batch_get_item.assert_called_once()
        index.ddb_resource.batch_write_item.assert_called_once()
        delete_items_mock.assert_called_once()

    def test_lambda_handles_multiple_dynamo_batches(self, delete_items_mock):
        # Arrange
        event = self.get_lambda_event(['achievement_1', 'achievement_2'])
        query_side_effects = [
            self.get_achievement_icons_query_response(['achievement_1'], ['achievement_2']),
            self.get_achievement_icons_query_response(['achievement_2'], [])
        ]
        index.ddb_resource.batch_get_item.side_effect = query_side_effects

        write_side_effects = [
            self.get_achievements_delete_response(['achievement_2']),
            self.get_achievements_delete_response()
        ]
        index.ddb_resource.batch_write_item.side_effect = write_side_effects

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])
        self.assertEqual(2, index.ddb_resource.batch_get_item.call_count)
        self.assertEqual(2, index.ddb_resource.batch_write_item.call_count)
        delete_items_mock.assert_called_once()

    def test_lambda_does_not_delete_from_s3_when_there_are_no_icons(self, delete_items_mock):
        # Arrange
        event = self.get_lambda_event()
        index.ddb_resource.batch_get_item.return_value = self.get_achievement_icons_query_response([])
        index.ddb_resource.batch_write_item.return_value = self.get_achievements_delete_response()

        # Act
        result = index.lambda_handler(event, None)

        # Assert
        self.assertEqual(204, result['statusCode'])
        index.ddb_resource.batch_write_item.assert_called_once()
        index.ddb_resource.batch_get_item.assert_called_once()
        delete_items_mock.assert_not_called()

    @classmethod
    def assert_no_resources_called(cls, delete_items_mock: MagicMock):
        """Assert that no external resources (Dynamo, S3) were called."""
        index.ddb_resource.batch_get_item.assert_not_called()
        index.ddb_resource.batch_write_item.assert_not_called()
        delete_items_mock.assert_not_called()

    @classmethod
    def get_achievement_icons_query_response(cls, achievement_ids: List[str],
                                             unprocessed_achievement_ids: List[str] = None) -> Dict[str, any]:
        """Return a Dynamo batch_get_item response with the given achievement icons and unprocessed achievement ids."""
        achievement_icons = cls.get_achievement_icons_response(achievement_ids)
        unprocessed_keys = {}
        if unprocessed_achievement_ids:
            unprocessed_keys = {
                ACHIEVEMENTS_TABLE_NAME: {
                    'Keys': [{
                        'achievement_id': achievement_id
                    } for achievement_id in unprocessed_achievement_ids],
                    'ConsistentRead': True,
                    'ProjectionExpression': 'achievement_id, locked_icon_url, unlocked_icon_url'
                }
            }
        return {
            'Responses': {
                ACHIEVEMENTS_TABLE_NAME: achievement_icons
            },
            'UnprocessedKeys': unprocessed_keys
        }

    @classmethod
    def get_achievement_icons_response(cls, achievements_ids: List[str]) -> List[Dict[str, str]]:
        """Return the sparse achievement icon attributes object for a given achievement id."""
        return [{
            'achievement_id': achievement_id,
            'locked_icon_url': f'{achievement_id}_locked',
            'unlocked_icon_url': f'{achievement_id}_unlocked',
        } for achievement_id in achievements_ids]

    @classmethod
    def get_achievements_delete_response(cls, unprocessed_achievement_ids: List[str] = None) -> Dict[str, dict]:
        """Return a Dynamo batch_write_item (delete only) response with the given unprocessed achievement ids."""
        unprocessed_items = {}
        if unprocessed_achievement_ids:
            unprocessed_items = {
                ACHIEVEMENTS_TABLE_NAME: [{
                    'DeleteRequest': {
                        'Key': {
                            'achievement_id': achievement_id
                        }
                    }
                } for achievement_id in unprocessed_achievement_ids]
            }

        return {
            'UnprocessedItems': unprocessed_items
        }

    @classmethod
    def get_lambda_event(cls, achievement_ids: List[str] = None):
        if achievement_ids is None:
            achievement_ids = [ACHIEVEMENT_ID]
        body = {
            'achievement_ids': achievement_ids
        }
        return http_event(body=json.dumps(body))
