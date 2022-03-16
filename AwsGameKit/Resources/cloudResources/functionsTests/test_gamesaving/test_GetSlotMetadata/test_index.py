# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

import json
import os
from unittest import TestCase
from unittest.mock import patch, MagicMock

from functions.gamesaving.GetSlotMetadata import index
from functionsTests.helpers.boto3.mock_responses.DynamoDB.Table import GetItem
from functionsTests.helpers.sample_lambda_events import http_event


# Patch Lambda environment variables:
@patch.dict(os.environ, {
    'GAMESAVES_TABLE_NAME': 'gamekit_dev_foogamename_player_gamesaves'
})
class TestIndex(TestCase):

    @patch('gamekithelpers.ddb.get_table')
    def test_can_get_slot_metadata_successfully(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        expected_metadata = {
            'slot_name': 'manual-save-0',
            'description': 'Sample description.',
            'last_modified': 1626751549000,
            'gk_user_id': '123456abcdef',
            'size': 1234567,
        }
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.get_item.return_value = GetItem.mock_response(item=expected_metadata)

        # Act
        result = index.lambda_handler(event, context)
        actual_metadata = json.loads(result['body'])['data']

        # Assert
        self.assertEqual(200, result['statusCode'])
        self.assertEqual(expected_metadata, actual_metadata)

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_an_empty_response_body_when_the_named_slot_is_not_found(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        context = None
        mock_gamesaves_table = mock_get_table()
        mock_gamesaves_table.get_item.return_value = GetItem.mock_response_no_matching_item()
        expected_metadata = {
            # Empty dict
        }

        # Act
        result = index.lambda_handler(event, context)
        actual_metadata = json.loads(result['body'])['data']

        # Assert
        self.assertEqual(200, result['statusCode'])
        self.assertEqual(expected_metadata, actual_metadata)

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_raises_a_value_error_when_the_consistent_read_parameter_is_not_valid_boolean(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['queryStringParameters'] = {
            'consistent_read': 'not a valid truth, such as yes/no, true/false, 0/1, etc.'
        }
        context = None
        mock_gamesaves_table = mock_get_table()

        # Act/Assert
        with self.assertRaises(ValueError):
            index.lambda_handler(event, context)

        # Assert
        mock_gamesaves_table.get_item.assert_not_called()

    def test_lambda_returns_a_401_error_code_when_player_id_is_missing_from_the_request_context(self):
        # Arrange
        event = self.get_lambda_event()
        event['requestContext']['authorizer']['claims'].pop('custom:gk_user_id')
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(401, result['statusCode'])

    @patch('gamekithelpers.ddb.get_table')
    def test_lambda_returns_a_400_error_code_when_slot_name_is_malformed(self, mock_get_table: MagicMock):
        # Arrange
        event = self.get_lambda_event()
        event['pathParameters']['slot_name'] = '$om3 ma!f*rm#d slot name%^z__09'
        context = None

        # Act
        result = index.lambda_handler(event, context)

        # Assert
        self.assertEqual(400, result['statusCode'])

    @staticmethod
    def get_lambda_event() -> dict:
        """Get a sample Lambda event with none of the optional parameters included."""
        return http_event(
            path_parameters={
                'slot_name': 'foo_slot_name'
            }
        )
